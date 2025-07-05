#include <esp32_smartdisplay_dma_helpers.h>
#include <esp32_smartdisplay.h>

// Minimum transfer size to justify DMA overhead (configurable)
#ifndef SMARTDISPLAY_DMA_MIN_TRANSFER_SIZE
#define SMARTDISPLAY_DMA_MIN_TRANSFER_SIZE 1024  // 1KB minimum
#endif

void smartdisplay_dma_lvgl_flush_callback(bool success, void *user_data)
{
    lv_display_t *display = (lv_display_t *)user_data;
    if (!success)
        log_e("DMA transfer failed for LVGL flush");
    
    lv_display_flush_ready(display);
}

bool smartdisplay_dma_should_use_for_size(size_t transfer_size)
{
    // Only use DMA for transfers above minimum threshold
    // Small transfers may be faster with direct CPU copy
    return transfer_size >= SMARTDISPLAY_DMA_MIN_TRANSFER_SIZE;
}

esp_err_t smartdisplay_dma_flush_with_byteswap(lv_display_t *display, const lv_area_t *area, 
                                               uint8_t *px_map, esp_lcd_panel_handle_t panel_handle, 
                                               const char *panel_name)
{
    uint32_t pixels = lv_area_get_size(area);
    size_t transfer_size = pixels * sizeof(uint16_t);
    
    // Perform byte swapping for SPI
    uint16_t *p = (uint16_t *)px_map;
    for (uint32_t i = 0; i < pixels; i++)
    {
        p[i] = (p[i] >> 8) | (p[i] << 8);
    }

    // Check if DMA is worth it for this transfer size
    if (!smartdisplay_dma_should_use_for_size(transfer_size))
    {
        // Transfer too small for DMA, use direct transfer
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map));
        lv_display_flush_ready(display);
        return ESP_OK;
    }

    // Try DMA first, fall back to direct transfer if it fails
    esp_err_t ret = smartdisplay_dma_draw_bitmap(area->x1, area->y1, area->x2 + 1, area->y2 + 1, 
                                                 px_map, smartdisplay_dma_lvgl_flush_callback, display, false);
    if (ret == ESP_OK)
    {
        // DMA transfer initiated successfully, callback will handle flush_ready
        return ESP_OK;
    }
    
    // DMA failed, use direct transfer
    log_w("DMA transfer failed for %s, using direct transfer", panel_name);
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map));
    lv_display_flush_ready(display);
    return ESP_OK;
}

esp_err_t smartdisplay_dma_init_with_logging(esp_lcd_panel_handle_t panel_handle, const char *panel_name)
{
    esp_err_t dma_init_result = smartdisplay_dma_init(panel_handle);
    if (dma_init_result == ESP_OK)
        log_i("DMA initialized successfully for %s display", panel_name);
    else
        log_w("DMA initialization failed for %s (error: 0x%x), will use direct transfers", panel_name, dma_init_result);
    
    return dma_init_result;
}

void smartdisplay_dma_rotation_callback(bool success, void *user_data)
{
    rotation_callback_data_t *data = (rotation_callback_data_t *)user_data;
    if (!success)
        log_e("DMA transfer failed for rotated display");
    
    // Free the rotation buffer now that DMA is complete
    free(data->rotation_buffer);
    
    // Signal LVGL that flush is complete
    lv_display_flush_ready(data->display);
    
    // Free the callback data structure
    free(data);
}

esp_err_t smartdisplay_dma_flush_with_rotation(lv_display_t *display, const lv_area_t *area, 
                                               uint8_t *px_map, esp_lcd_panel_handle_t panel_handle, 
                                               const char *panel_name)
{
    lv_display_rotation_t rotation = lv_display_get_rotation(display);
    if (rotation == LV_DISPLAY_ROTATION_0)
    {
        // No rotation needed, use standard DMA path
        size_t transfer_size = lv_area_get_size(area) * sizeof(uint16_t);
        
        if (!smartdisplay_dma_should_use_for_size(transfer_size))
        {
            // Transfer too small for DMA, use direct transfer
            ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map));
            lv_display_flush_ready(display);
            return ESP_OK;
        }

        // Try DMA first, fall back to direct transfer if it fails
        esp_err_t ret = smartdisplay_dma_draw_bitmap(area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map, smartdisplay_dma_lvgl_flush_callback, display, false);
        if (ret == ESP_OK)
        {
            // DMA transfer initiated successfully, callback will handle flush_ready
            return ESP_OK;
        }
        
        // DMA failed, use direct transfer
        log_w("DMA transfer failed for %s, using direct transfer", panel_name);
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map));
        lv_display_flush_ready(display);
        return ESP_OK;
    }

    // Rotated - need to create rotation buffer
    int32_t w = lv_area_get_width(area);
    int32_t h = lv_area_get_height(area);
    lv_color_format_t cf = lv_display_get_color_format(display);
    uint32_t px_size = lv_color_format_get_size(cf);
    size_t buf_size = w * h * px_size;
    
    log_v("alloc rotation buffer to: %u bytes", buf_size);
    void *rotation_buffer = heap_caps_malloc(buf_size, LVGL_BUFFER_MALLOC_FLAGS);
    if (rotation_buffer == NULL) {
        log_e("Failed to allocate rotation buffer");
        return ESP_ERR_NO_MEM;
    }

    uint32_t w_stride = lv_draw_buf_width_to_stride(w, cf);
    uint32_t h_stride = lv_draw_buf_width_to_stride(h, cf);

    switch (rotation)
    {
    case LV_DISPLAY_ROTATION_90:
        lv_draw_sw_rotate(px_map, rotation_buffer, w, h, w_stride, h_stride, rotation, cf);
        
        // Try DMA first for rotated data
        if (smartdisplay_dma_should_use_for_size(buf_size))
        {
            rotation_callback_data_t *callback_data = heap_caps_malloc(sizeof(rotation_callback_data_t), MALLOC_CAP_DEFAULT);
            if (callback_data != NULL)
            {
                callback_data->display = display;
                callback_data->rotation_buffer = rotation_buffer;
                
                esp_err_t ret = smartdisplay_dma_draw_bitmap(area->y1, display->ver_res - area->x1 - w, area->y1 + h, display->ver_res - area->x1, rotation_buffer, smartdisplay_dma_rotation_callback, callback_data, false);
                if (ret == ESP_OK)
                {
                    // DMA transfer initiated, callback will free the buffer and handle completion
                    return ESP_OK;
                }
                free(callback_data);
            }
            log_w("DMA transfer failed for 90° rotation on %s, using direct transfer", panel_name);
        }
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, area->y1, display->ver_res - area->x1 - w, area->y1 + h, display->ver_res - area->x1, rotation_buffer));
        break;
        
    case LV_DISPLAY_ROTATION_180:
        lv_draw_sw_rotate(px_map, rotation_buffer, w, h, w_stride, w_stride, rotation, cf);
        
        // Try DMA first for rotated data
        if (smartdisplay_dma_should_use_for_size(buf_size))
        {
            rotation_callback_data_t *callback_data = heap_caps_malloc(sizeof(rotation_callback_data_t), MALLOC_CAP_DEFAULT);
            if (callback_data != NULL)
            {
                callback_data->display = display;
                callback_data->rotation_buffer = rotation_buffer;
                
                esp_err_t ret = smartdisplay_dma_draw_bitmap(display->hor_res - area->x1 - w, display->ver_res - area->y1 - h, display->hor_res - area->x1, display->ver_res - area->y1, rotation_buffer, smartdisplay_dma_rotation_callback, callback_data, false);
                if (ret == ESP_OK)
                {
                    // DMA transfer initiated, callback will free the buffer and handle completion
                    return ESP_OK;
                }
                free(callback_data);
            }
            log_w("DMA transfer failed for 180° rotation on %s, using direct transfer", panel_name);
        }
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, display->hor_res - area->x1 - w, display->ver_res - area->y1 - h, display->hor_res - area->x1, display->ver_res - area->y1, rotation_buffer));
        break;
        
    case LV_DISPLAY_ROTATION_270:
        lv_draw_sw_rotate(px_map, rotation_buffer, w, h, w_stride, h_stride, rotation, cf);
        
        // Try DMA first for rotated data
        if (smartdisplay_dma_should_use_for_size(buf_size))
        {
            rotation_callback_data_t *callback_data = heap_caps_malloc(sizeof(rotation_callback_data_t), MALLOC_CAP_DEFAULT);
            if (callback_data != NULL)
            {
                callback_data->display = display;
                callback_data->rotation_buffer = rotation_buffer;
                
                esp_err_t ret = smartdisplay_dma_draw_bitmap(display->hor_res - area->y2 - 1, area->x2 - w + 1, display->hor_res - area->y2 - 1 + h, area->x2 + 1, rotation_buffer, smartdisplay_dma_rotation_callback, callback_data, false);
                if (ret == ESP_OK)
                {
                    // DMA transfer initiated, callback will free the buffer and handle completion
                    return ESP_OK;
                }
                free(callback_data);
            }
            log_w("DMA transfer failed for 270° rotation on %s, using direct transfer", panel_name);
        }
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, display->hor_res - area->y2 - 1, area->x2 - w + 1, display->hor_res - area->y2 - 1 + h, area->x2 + 1, rotation_buffer));
        break;
        
    default:
        free(rotation_buffer);
        return ESP_ERR_INVALID_ARG;
    }

    // If we reach here, DMA was not used or failed, so we need to free the buffer and signal flush ready
    free(rotation_buffer);
    lv_display_flush_ready(display);
    return ESP_OK;
}
