#include <esp32_smartdisplay_dma.h>
#include <esp32-hal-log.h>
#include <esp_heap_caps.h>
#include <esp_lcd_panel_io.h>
#include <string.h>

// Global DMA manager instance
static smartdisplay_dma_manager_t *g_dma_manager = NULL;

#ifndef _min
#define _min(a, b) ((a) < (b) ? (a) : (b))
#endif

bool smartdisplay_dma_should_use_dma(size_t data_len)
{
    return g_dma_manager != NULL && data_len >= SMARTDISPLAY_DMA_CHUNK_THRESHOLD;
}

esp_err_t smartdisplay_dma_draw_bitmap(int x_start, int y_start, int x_end, int y_end, const void *color_data, smartdisplay_dma_callback_t callback, void *user_data, bool high_priority)
{
    if (g_dma_manager == NULL)
    {
        log_e("DMA manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (color_data == NULL)
    {
        log_e("Invalid color data");
        return ESP_ERR_INVALID_ARG;
    }

    // Calculate transfer size
    const size_t width = x_end - x_start;
    const size_t height = y_end - y_start;
    const size_t data_len = width * height * sizeof(uint16_t); // Assuming RGB565

    // For small transfers, use direct transfer
    if (!smartdisplay_dma_should_use_dma(data_len))
    {
        const esp_err_t ret = esp_lcd_panel_draw_bitmap(g_dma_manager->panel_handle, x_start, y_start, x_end, y_end, color_data);
        if (callback != NULL)
            callback(ret == ESP_OK, user_data);

        return ret;
    }

    // Create transfer descriptor using compound literal
    const smartdisplay_dma_transfer_t transfer = {
        .src_data = color_data,
        .data_len = data_len,
        .x_start = x_start,
        .y_start = y_start,
        .x_end = x_end,
        .y_end = y_end,
        .callback = callback,
        .user_data = user_data,
        .high_priority = high_priority};

    // Queue transfer
    const BaseType_t queue_result = high_priority ? xQueueSendToFront(g_dma_manager->transfer_queue, &transfer, 0) : xQueueSend(g_dma_manager->transfer_queue, &transfer, 0);
    if (queue_result != pdPASS)
    {
        log_w("Transfer queue full, falling back to direct transfer");
        esp_err_t ret = esp_lcd_panel_draw_bitmap(g_dma_manager->panel_handle, x_start, y_start, x_end, y_end, color_data);
        if (callback != NULL)
            callback(ret == ESP_OK, user_data);

        return ret;
    }

    // Update statistics
    if (xSemaphoreTake(g_dma_manager->state_mutex, pdMS_TO_TICKS(10)) == pdTRUE)
    {
        g_dma_manager->active_transfers++;
        xSemaphoreGive(g_dma_manager->state_mutex);
    }

    return ESP_OK;
}

esp_err_t smartdisplay_dma_wait_all_done(uint32_t timeout_ms)
{
    if (g_dma_manager == NULL)
        return ESP_ERR_INVALID_STATE;

    const uint32_t start_time = xTaskGetTickCount();
    const uint32_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);

    while ((xTaskGetTickCount() - start_time) < timeout_ticks)
    {
        if (xSemaphoreTake(g_dma_manager->state_mutex, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            const bool all_done = (g_dma_manager->active_transfers == 0 && uxQueueMessagesWaiting(g_dma_manager->transfer_queue) == 0);
            xSemaphoreGive(g_dma_manager->state_mutex);

            if (all_done)
                return ESP_OK;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    return ESP_ERR_TIMEOUT;
}

esp_err_t smartdisplay_dma_get_stats(uint32_t *active_transfers, uint32_t *completed_transfers, uint32_t *failed_transfers)
{
    if (g_dma_manager == NULL)
        return ESP_ERR_INVALID_STATE;

    if (xSemaphoreTake(g_dma_manager->state_mutex, pdMS_TO_TICKS(100)) != pdTRUE)
        return ESP_ERR_TIMEOUT;

    if (active_transfers)
        *active_transfers = g_dma_manager->active_transfers;

    if (completed_transfers)
        *completed_transfers = g_dma_manager->completed_transfers;

    if (failed_transfers)
        *failed_transfers = g_dma_manager->failed_transfers;

    xSemaphoreGive(g_dma_manager->state_mutex);
    return ESP_OK;
}

// DMA completion callback for LVGL
static void lvgl_dma_callback(bool success, void *user_data)
{
    // user_data is the lv_display_t pointer directly
    lv_display_t *display = (lv_display_t *)user_data;
    if (!success)
        log_e("DMA transfer failed for LVGL flush");

    lv_display_flush_ready(display);
}

void smartdisplay_dma_lvgl_flush(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    if (g_dma_manager == NULL)
    {
        // Fallback to default flush using panel handle from display user data
        esp_lcd_panel_handle_t panel = (esp_lcd_panel_handle_t)lv_display_get_user_data(display);
        if (panel)
            esp_lcd_panel_draw_bitmap(panel, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map);

        lv_display_flush_ready(display);
        return;
    }

    // Queue DMA transfer - pass display pointer directly as user data. No byte order is swapped for SPI
    esp_err_t ret = smartdisplay_dma_draw_bitmap(area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map, lvgl_dma_callback, display, false);
    if (ret != ESP_OK)
    {
        log_w("Failed to queue DMA transfer, using direct transfer");
        esp_lcd_panel_handle_t panel = (esp_lcd_panel_handle_t)lv_display_get_user_data(display);
        if (panel)
            esp_lcd_panel_draw_bitmap(panel, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map);

        lv_display_flush_ready(display);
    }
}

static esp_err_t smartdisplay_dma_copy_to_buffer(const void *src, size_t len, void **dest)
{
    if (src == NULL || len == 0 || dest == NULL)
        return ESP_ERR_INVALID_ARG;

    if (len > g_dma_manager->dma_buffer_size)
    {
        log_e("Data size (%d) exceeds DMA buffer size (%d)", len, g_dma_manager->dma_buffer_size);
        return ESP_ERR_INVALID_SIZE;
    }

    // Check if source data is already DMA-capable
    if (esp_ptr_dma_capable(src))
    {
        *dest = (void *)src;
        return ESP_OK;
    }

    // Copy to DMA buffer
    memcpy(g_dma_manager->dma_buffer, src, len);
    *dest = g_dma_manager->dma_buffer;

    return ESP_OK;
}

static esp_err_t smartdisplay_dma_transfer_chunk(const smartdisplay_dma_transfer_t *transfer)
{
    if (transfer == NULL || transfer->src_data == NULL)
        return ESP_ERR_INVALID_ARG;

    size_t remaining = transfer->data_len;
    const uint8_t *src_ptr = (const uint8_t *)transfer->src_data;
    const size_t pixels_per_row = transfer->x_end - transfer->x_start;
    const size_t bytes_per_pixel = sizeof(uint16_t); // RGB565
    const size_t bytes_per_row = pixels_per_row * bytes_per_pixel;

    int current_y = transfer->y_start;
    while (remaining > 0 && current_y < transfer->y_end)
    {
        // Calculate chunk size (limit to DMA buffer size)
        size_t chunk_rows = _min(remaining / bytes_per_row, g_dma_manager->dma_buffer_size / bytes_per_row);
        if (chunk_rows == 0)
            chunk_rows = 1; // At least one row

        const size_t chunk_size = _min(chunk_rows * bytes_per_row, remaining);
        // Copy data to DMA buffer
        void *dma_data;
        const esp_err_t copy_result = smartdisplay_dma_copy_to_buffer(src_ptr, chunk_size, &dma_data);
        if (copy_result != ESP_OK)
        {
            log_e("Failed to copy data to DMA buffer");
            return copy_result;
        }

        // Perform DMA transfer
        const int chunk_y_end = current_y + chunk_rows;
        const esp_err_t transfer_result = esp_lcd_panel_draw_bitmap(g_dma_manager->panel_handle, transfer->x_start, current_y, transfer->x_end, chunk_y_end, dma_data);
        if (transfer_result != ESP_OK)
        {
            log_e("LCD panel transfer failed: %s", esp_err_to_name(transfer_result));
            return transfer_result;
        }

        // Update pointers
        src_ptr += chunk_size;
        remaining -= chunk_size;
        current_y = chunk_y_end;

        // Small delay to prevent overwhelming the system
        vTaskDelay(1);
    }

    return ESP_OK;
}

// DMA worker task implementation
static void smartdisplay_dma_worker_task(void *pvParameters)
{
    log_i("DMA worker task started");

    smartdisplay_dma_transfer_t transfer;

    while (1)
    {
        // Wait for transfer request
        if (xQueueReceive(g_dma_manager->transfer_queue, &transfer, portMAX_DELAY) == pdTRUE)
        {
            // Update state
            if (xSemaphoreTake(g_dma_manager->state_mutex, portMAX_DELAY) == pdTRUE)
            {
                g_dma_manager->state = SMARTDISPLAY_DMA_STATE_BUSY;
                xSemaphoreGive(g_dma_manager->state_mutex);
            }

            // Perform transfer
            const esp_err_t result = smartdisplay_dma_transfer_chunk(&transfer);
            const bool success = result == ESP_OK;

            // Update statistics
            if (xSemaphoreTake(g_dma_manager->state_mutex, portMAX_DELAY) == pdTRUE)
            {
                g_dma_manager->active_transfers--;
                if (success)
                    g_dma_manager->completed_transfers++;
                else
                    g_dma_manager->failed_transfers++;

                g_dma_manager->state = SMARTDISPLAY_DMA_STATE_IDLE;
                xSemaphoreGive(g_dma_manager->state_mutex);
            }

            // Call completion callback
            if (transfer.callback != NULL)
                transfer.callback(success, transfer.user_data);

            log_d("Transfer completed: %s (%d bytes)", success ? "SUCCESS" : "FAILED", transfer.data_len);
        }
    }
}

esp_err_t smartdisplay_dma_init(esp_lcd_panel_handle_t panel_handle)
{
    if (g_dma_manager != NULL)
    {
        log_w("DMA manager already initialized");
        return ESP_OK;
    }

    if (panel_handle == NULL)
    {
        log_e("Invalid panel handle");
        return ESP_ERR_INVALID_ARG;
    }

    // Allocate DMA manager
    g_dma_manager = heap_caps_calloc(1, sizeof(smartdisplay_dma_manager_t), MALLOC_CAP_DEFAULT);
    if (g_dma_manager == NULL)
    {
        log_e("Failed to allocate DMA manager");
        return ESP_ERR_NO_MEM;
    }

    // Allocate DMA-capable buffer
    g_dma_manager->dma_buffer = heap_caps_malloc(SMARTDISPLAY_DMA_BUFFER_SIZE, MALLOC_CAP_DMA | MALLOC_CAP_32BIT);
    if (g_dma_manager->dma_buffer == NULL)
    {
        log_e("Failed to allocate DMA buffer");
        free(g_dma_manager);
        g_dma_manager = NULL;
        return ESP_ERR_NO_MEM;
    }

    g_dma_manager->dma_buffer_size = SMARTDISPLAY_DMA_BUFFER_SIZE;

    // Create transfer queue
    g_dma_manager->transfer_queue = xQueueCreate(SMARTDISPLAY_DMA_QUEUE_SIZE, sizeof(smartdisplay_dma_transfer_t));
    if (g_dma_manager->transfer_queue == NULL)
    {
        log_e("Failed to create transfer queue");
        smartdisplay_dma_deinit();
        return ESP_ERR_NO_MEM;
    }

    // Create state mutex
    g_dma_manager->state_mutex = xSemaphoreCreateMutex();
    if (g_dma_manager->state_mutex == NULL)
    {
        log_e("Failed to create state mutex");
        smartdisplay_dma_deinit();
        return ESP_ERR_NO_MEM;
    }

    // Initialize state
    *g_dma_manager = (smartdisplay_dma_manager_t){
        .panel_handle = panel_handle,
        .state = SMARTDISPLAY_DMA_STATE_IDLE,
        .active_transfers = 0,
        .completed_transfers = 0,
        .failed_transfers = 0,
        .transfer_queue = g_dma_manager->transfer_queue,
        .state_mutex = g_dma_manager->state_mutex,
        .dma_buffer = g_dma_manager->dma_buffer,
        .dma_buffer_size = g_dma_manager->dma_buffer_size};

    // Create worker task
    const BaseType_t task_result = xTaskCreatePinnedToCore(
        smartdisplay_dma_worker_task,
        "dma_worker",
        4096, // Stack size
        NULL,
        5, // Priority (higher than LVGL)
        &g_dma_manager->worker_task,
        1 // Pin to core 1
    );

    if (task_result != pdPASS)
    {
        log_e("Failed to create DMA worker task");
        smartdisplay_dma_deinit();
        return ESP_ERR_NO_MEM;
    }

    log_i("DMA manager initialized with %d KB buffer", SMARTDISPLAY_DMA_BUFFER_SIZE / 1024);
    return ESP_OK;
}

esp_err_t smartdisplay_dma_deinit()
{
    if (g_dma_manager == NULL)
        return ESP_OK;

    // Wait for all transfers to complete
    smartdisplay_dma_wait_all_done(SMARTDISPLAY_DMA_TIMEOUT_MS);

    // Delete worker task
    if (g_dma_manager->worker_task != NULL)
    {
        vTaskDelete(g_dma_manager->worker_task);
        g_dma_manager->worker_task = NULL;
    }

    // Delete queue
    if (g_dma_manager->transfer_queue != NULL)
    {
        vQueueDelete(g_dma_manager->transfer_queue);
        g_dma_manager->transfer_queue = NULL;
    }

    // Delete mutex
    if (g_dma_manager->state_mutex != NULL)
    {
        vSemaphoreDelete(g_dma_manager->state_mutex);
        g_dma_manager->state_mutex = NULL;
    }

    // Free DMA buffer
    if (g_dma_manager->dma_buffer != NULL)
    {
        heap_caps_free(g_dma_manager->dma_buffer);
        g_dma_manager->dma_buffer = NULL;
    }

    // Free manager
    free(g_dma_manager);
    g_dma_manager = NULL;

    log_i("DMA manager deinitialized");
    return ESP_OK;
}