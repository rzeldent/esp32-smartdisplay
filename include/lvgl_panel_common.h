#pragma once

#include <esp32_smartdisplay.h>
#include <esp_lcd_panel_io.h>
#if SOC_LCD_RGB_SUPPORTED
#include <esp_lcd_panel_rgb.h>
#endif

static inline lv_display_t* lvgl_create_display()
{
    lv_display_t *display = lv_display_create(DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lv_color_format_t cf = lv_display_get_color_format(display);
    uint32_t px_size = lv_color_format_get_size(cf);
    uint32_t drawBufferSize = px_size * LVGL_BUFFER_PIXELS;
    void *drawBuffer = heap_caps_malloc(drawBufferSize, LVGL_BUFFER_MALLOC_FLAGS);
    lv_display_set_buffers(display, drawBuffer, NULL, drawBufferSize, LV_DISPLAY_RENDER_MODE_PARTIAL);
    return display;
}

#if SOC_LCD_RGB_SUPPORTED
static inline bool lvgl_panel_frame_trans_done(esp_lcd_panel_handle_t panel, esp_lcd_rgb_panel_event_data_t *edata, void *user_ctx)
{
    lv_display_t *display = (lv_display_t *)user_ctx;
    lv_display_flush_ready(display);
    return false;
}
#endif

static inline bool lvgl_panel_color_trans_done(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_display_t *display = (lv_display_t *)user_ctx;
    lv_display_flush_ready(display);
    return false;
}

// Hardware rotation is supported
// Uses a static staging buffer to avoid modifying the LVGL draw buffer
// while SPI transfer is in progress (causing display corruption)
static inline void lv_flush_hardware(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    esp_lcd_panel_handle_t panel_handle = display->user_data;
    
    // Calculate buffer size needed
    uint32_t pixels = lv_area_get_size(area);
    size_t buf_size = pixels * sizeof(uint16_t);
    
    // Use a persistent static buffer to avoid repeated allocations
    // This buffer is allocated once and reused for each frame
    static uint8_t *staging_buf = NULL;
    static size_t staging_buf_size = 0;
    
    // Allocate or reallocate if size changed
    if (staging_buf == NULL || buf_size > staging_buf_size) {
        if (staging_buf != NULL) {
            free(staging_buf);
        }
        staging_buf = (uint8_t *)heap_caps_malloc(buf_size, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT);
        if (staging_buf == NULL) {
            log_e("Failed to allocate staging buffer for flush (size: %u)", buf_size);
            return;
        }
        staging_buf_size = buf_size;
        log_d("Allocated staging buffer: %u bytes", buf_size);
    }
    
    // Copy and byte-swap to staging buffer instead of modifying LVGL's buffer
    // This prevents the corruption that occurs when LVGL reuses the buffer
    // while SPI is still reading it
    uint16_t *src = (uint16_t *)px_map;
    uint16_t *dst = (uint16_t *)staging_buf;
    for (uint32_t i = 0; i < pixels; i++) {
        *dst++ = __builtin_bswap16(*src++);
    }

    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, staging_buf));
};

// Hardware rotation is not supported
static inline void lv_flush_software(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    const esp_lcd_panel_handle_t panel_handle = display->user_data;

    lv_display_rotation_t rotation = lv_display_get_rotation(display);
    if (rotation == LV_DISPLAY_ROTATION_0)
    {
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map));
        return;
    }

    // Rotated
    int32_t w = lv_area_get_width(area);
    int32_t h = lv_area_get_height(area);
    lv_color_format_t cf = lv_display_get_color_format(display);
    uint32_t px_size = lv_color_format_get_size(cf);
    size_t buf_size = w * h * px_size;
    log_v("alloc rotation buffer to: %u bytes", buf_size);
    void *rotation_buffer = heap_caps_malloc(buf_size, LVGL_BUFFER_MALLOC_FLAGS);
    assert(rotation_buffer != NULL);

    uint32_t w_stride = lv_draw_buf_width_to_stride(w, cf);
    uint32_t h_stride = lv_draw_buf_width_to_stride(h, cf);

    switch (rotation)
    {
    case LV_DISPLAY_ROTATION_90:
        lv_draw_sw_rotate(px_map, rotation_buffer, w, h, w_stride, h_stride, rotation, cf);
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, area->y1, display->ver_res - area->x1 - w, area->y1 + h, display->ver_res - area->x1, rotation_buffer));
        break;
    case LV_DISPLAY_ROTATION_180:
        lv_draw_sw_rotate(px_map, rotation_buffer, w, h, w_stride, w_stride, rotation, cf);
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, display->hor_res - area->x1 - w, display->ver_res - area->y1 - h, display->hor_res - area->x1, display->ver_res - area->y1, rotation_buffer));
        break;
    case LV_DISPLAY_ROTATION_270:
        lv_draw_sw_rotate(px_map, rotation_buffer, w, h, w_stride, h_stride, rotation, cf);
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel_handle, display->hor_res - area->y2 - 1, area->x1, display->hor_res - area->y2 - 1 + h, area->x2 + 1, rotation_buffer));
        break;
    default:
        assert(false);
        break;
    }

    free(rotation_buffer);
};

static inline void lvgl_setup_panel(esp_lcd_panel_handle_t panel_handle)
{
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
#ifdef DISPLAY_IPS
    // If LCD is IPS invert the colors
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
#endif
#if (DISPLAY_SWAP_XY)
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, DISPLAY_SWAP_XY));
#endif
#if (DISPLAY_MIRROR_X || DISPLAY_MIRROR_Y)
    ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y));
#endif
#if (DISPLAY_GAP_X || DISPLAY_GAP_Y)
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, DISPLAY_GAP_X, DISPLAY_GAP_Y));
#endif
    // Turn display on
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));
};