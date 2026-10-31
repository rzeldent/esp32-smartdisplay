/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <esp32_smartdisplay_dma.h>
#include <esp32-hal-log.h>
#include <esp_heap_caps.h>
#include <esp_lcd_panel_io.h>
#include <esp_lcd_panel_ops.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <string.h>

#define TAG "smartdisplay_dma"

// Default DMA buffer size if not specified
#ifndef SMARTDISPLAY_DMA_BUFFER_SIZE
#define SMARTDISPLAY_DMA_BUFFER_SIZE (64 * 1024) // 64KB
#endif

// Maximum number of pending transfers in queue
#define SMARTDISPLAY_DMA_MAX_TRANSFERS 32

// Worker task stack size
#define SMARTDISPLAY_DMA_TASK_STACK_SIZE (4 * 1024)

// Worker task priority
#define SMARTDISPLAY_DMA_TASK_PRIORITY (configMAX_PRIORITIES - 2)

// Timeout for waiting on chunk done semaphore
#define SMARTDISPLAY_DMA_CHUNK_TIMEOUT_MS 1000

// Static DMA manager instance
static smartdisplay_dma_manager_t *s_dma_manager = NULL;

// Forward declarations
static void dma_worker_task(void *arg);
static esp_err_t dma_transfer_chunk(smartdisplay_dma_transfer_t *transfer);
static void dma_handle_transfer(smartdisplay_dma_transfer_t *transfer);

esp_err_t smartdisplay_dma_init(esp_lcd_panel_handle_t panel_handle, bool async_color_trans)
{
    if (s_dma_manager != NULL) {
        log_w("DMA manager already initialized");
        return ESP_OK;
    }

    if (panel_handle == NULL) {
        log_e("Invalid panel handle");
        return ESP_ERR_INVALID_ARG;
    }

    // Allocate DMA manager structure
    s_dma_manager = heap_caps_calloc(1, sizeof(smartdisplay_dma_manager_t), MALLOC_CAP_DEFAULT | MALLOC_CAP_8BIT);
    if (s_dma_manager == NULL) {
        log_e("Failed to allocate DMA manager");
        return ESP_ERR_NO_MEM;
    }

    // Create transfer queue
    s_dma_manager->transfer_queue = xQueueCreate(SMARTDISPLAY_DMA_MAX_TRANSFERS, sizeof(smartdisplay_dma_transfer_t));
    if (s_dma_manager->transfer_queue == NULL) {
        log_e("Failed to create transfer queue");
        goto cleanup;
    }

    // Create state mutex
    s_dma_manager->state_mutex = xSemaphoreCreateMutex();
    if (s_dma_manager->state_mutex == NULL) {
        log_e("Failed to create state mutex");
        goto cleanup;
    }

    // Create chunk done semaphore
    s_dma_manager->chunk_done_sem = xSemaphoreCreateBinary();
    if (s_dma_manager->chunk_done_sem == NULL) {
        log_e("Failed to create chunk done semaphore");
        goto cleanup;
    }

    // Allocate DMA-capable buffer
    s_dma_manager->dma_buffer_size = SMARTDISPLAY_DMA_BUFFER_SIZE;
    s_dma_manager->dma_buffer = heap_caps_malloc(s_dma_manager->dma_buffer_size, MALLOC_CAP_DMA | MALLOC_CAP_8BIT);
    if (s_dma_manager->dma_buffer == NULL) {
        log_e("Failed to allocate DMA buffer");
        goto cleanup;
    }

    s_dma_manager->panel_handle = panel_handle;
    s_dma_manager->async_color_trans = async_color_trans;
    s_dma_manager->state = SMARTDISPLAY_DMA_STATE_IDLE;

    // Create worker task
    BaseType_t ret = xTaskCreatePinnedToCore(
        dma_worker_task,
        "smartdisplay_dma",
        SMARTDISPLAY_DMA_TASK_STACK_SIZE,
        NULL,
        SMARTDISPLAY_DMA_TASK_PRIORITY,
        &s_dma_manager->worker_task,
        1 // Run on core 1
    );

    if (ret != pdPASS) {
        log_e("Failed to create DMA worker task");
        goto cleanup;
    }

    log_i("DMA manager initialized, buffer size: %zu bytes, async_color_trans: %d",
          s_dma_manager->dma_buffer_size, async_color_trans);

    return ESP_OK;

cleanup:
    smartdisplay_dma_deinit();
    return ESP_ERR_NO_MEM;
}

esp_err_t smartdisplay_dma_init_with_logging(esp_lcd_panel_handle_t panel_handle, const char *panel_name, bool async_color_trans)
{
    if (panel_handle == NULL) {
        log_e("Invalid panel handle");
        return ESP_ERR_INVALID_ARG;
    }

    log_i("Initializing DMA for panel: %s, async_color_trans: %d", panel_name ? panel_name : "unknown", async_color_trans);

    esp_err_t ret = smartdisplay_dma_init(panel_handle, async_color_trans);
    if (ret != ESP_OK) {
        log_e("Failed to initialize DMA for panel %s: 0x%x", panel_name ? panel_name : "unknown", ret);
    } else {
        log_i("DMA initialized successfully for panel: %s", panel_name ? panel_name : "unknown");
    }

    return ret;
}

esp_err_t smartdisplay_dma_deinit()
{
    if (s_dma_manager == NULL) {
        return ESP_OK;
    }

    // Signal worker task to exit
    if (s_dma_manager->worker_task != NULL) {
        smartdisplay_dma_transfer_t exit_transfer = {0};
        exit_transfer.src_data = (void *)1; // Special marker for exit
        xQueueSend(s_dma_manager->transfer_queue, &exit_transfer, portMAX_DELAY);
        vTaskDelete(s_dma_manager->worker_task);
        s_dma_manager->worker_task = NULL;
    }

    // Clean up queue
    if (s_dma_manager->transfer_queue != NULL) {
        vQueueDelete(s_dma_manager->transfer_queue);
        s_dma_manager->transfer_queue = NULL;
    }

    // Clean up mutex
    if (s_dma_manager->state_mutex != NULL) {
        vSemaphoreDelete(s_dma_manager->state_mutex);
        s_dma_manager->state_mutex = NULL;
    }

    // Clean up semaphore
    if (s_dma_manager->chunk_done_sem != NULL) {
        vSemaphoreDelete(s_dma_manager->chunk_done_sem);
        s_dma_manager->chunk_done_sem = NULL;
    }

    // Free DMA buffer
    if (s_dma_manager->dma_buffer != NULL) {
        heap_caps_free(s_dma_manager->dma_buffer);
        s_dma_manager->dma_buffer = NULL;
    }

    heap_caps_free(s_dma_manager);
    s_dma_manager = NULL;

    log_i("DMA manager deinitialized");
    return ESP_OK;
}

esp_err_t smartdisplay_dma_draw_bitmap(int x_start, int y_start, int x_end, int y_end,
                                       const void *color_data, smartdisplay_dma_callback_t callback,
                                       void *user_data, bool high_priority)
{
    if (s_dma_manager == NULL) {
        log_e("DMA manager not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (color_data == NULL) {
        log_e("Invalid color data");
        return ESP_ERR_INVALID_ARG;
    }

    smartdisplay_dma_transfer_t transfer = {
        .src_data = color_data,
        .data_len = (x_end - x_start) * (y_end - y_start) * sizeof(uint16_t),
        .x_start = x_start,
        .y_start = y_start,
        .x_end = x_end,
        .y_end = y_end,
        .callback = callback,
        .user_data = user_data,
        .high_priority = high_priority
    };

    // Check if we should use DMA for this transfer size
    if (!smartdisplay_dma_should_use_dma(transfer.data_len)) {
        log_v("Transfer too small for DMA (%zu bytes), using direct transfer", transfer.data_len);
        return ESP_ERR_NOT_SUPPORTED;
    }

    // Check if data fits in DMA buffer
    if (transfer.data_len > s_dma_manager->dma_buffer_size) {
        log_w("Transfer size (%zu) exceeds DMA buffer (%zu), using direct transfer",
              transfer.data_len, s_dma_manager->dma_buffer_size);
        return ESP_ERR_NOT_SUPPORTED;
    }

    // Copy data to DMA buffer
    memcpy(s_dma_manager->dma_buffer, color_data, transfer.data_len);
    transfer.src_data = s_dma_manager->dma_buffer;

    // Send to queue
    TickType_t timeout = high_priority ? 0 : portMAX_DELAY;
    BaseType_t ret = xQueueSend(s_dma_manager->transfer_queue, &transfer, timeout);
    if (ret != pdPASS) {
        log_e("Failed to queue DMA transfer");
        return ESP_ERR_TIMEOUT;
    }

    // Update state
    if (xSemaphoreTake(s_dma_manager->state_mutex, portMAX_DELAY) == pdTRUE) {
        s_dma_manager->state = SMARTDISPLAY_DMA_STATE_BUSY;
        s_dma_manager->active_transfers++;
        xSemaphoreGive(s_dma_manager->state_mutex);
    }

    return ESP_OK;
}

bool smartdisplay_dma_should_use_dma(size_t data_len)
{
    // Use DMA for transfers larger than 1KB
    return data_len >= 1024;
}

esp_err_t smartdisplay_dma_wait_all_done(uint32_t timeout_ms)
{
    if (s_dma_manager == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    TickType_t start = xTaskGetTickCount();
    TickType_t timeout_ticks = pdMS_TO_TICKS(timeout_ms);

    while (true) {
        if (xSemaphoreTake(s_dma_manager->state_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            bool idle = (s_dma_manager->state == SMARTDISPLAY_DMA_STATE_IDLE);
            xSemaphoreGive(s_dma_manager->state_mutex);
            if (idle) {
                return ESP_OK;
            }
        }

        if (timeout_ms != portMAX_DELAY) {
            TickType_t elapsed = xTaskGetTickCount() - start;
            if (elapsed >= timeout_ticks) {
                return ESP_ERR_TIMEOUT;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

esp_err_t smartdisplay_dma_get_stats(uint32_t *active_transfers, uint32_t *completed_transfers, uint32_t *failed_transfers)
{
    if (s_dma_manager == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    if (xSemaphoreTake(s_dma_manager->state_mutex, portMAX_DELAY) == pdTRUE) {
        if (active_transfers) *active_transfers = s_dma_manager->active_transfers;
        if (completed_transfers) *completed_transfers = s_dma_manager->completed_transfers;
        if (failed_transfers) *failed_transfers = s_dma_manager->failed_transfers;
        xSemaphoreGive(s_dma_manager->state_mutex);
        return ESP_OK;
    }

    return ESP_ERR_TIMEOUT;
}

bool smartdisplay_dma_notify_chunk_done(void)
{
    if (s_dma_manager == NULL || s_dma_manager->chunk_done_sem == NULL) {
        return false;
    }

    BaseType_t higher_priority_task_woken = pdFALSE;
    xSemaphoreGiveFromISR(s_dma_manager->chunk_done_sem, &higher_priority_task_woken);
    return higher_priority_task_woken == pdTRUE;
}

// Worker task implementation
static void dma_worker_task(void *arg)
{
    (void)arg;
    smartdisplay_dma_transfer_t transfer;

    log_i("DMA worker task started");

    while (true) {
        // Wait for transfer
        if (xQueueReceive(s_dma_manager->transfer_queue, &transfer, portMAX_DELAY) != pdPASS) {
            continue;
        }

        // Check for exit signal
        if (transfer.src_data == (void *)1) {
            log_i("DMA worker task exiting");
            break;
        }

        // Process transfer
        dma_handle_transfer(&transfer);
    }

    vTaskDelete(NULL);
}

static void dma_handle_transfer(smartdisplay_dma_transfer_t *transfer)
{
    esp_err_t err = ESP_OK;

    // Update state to busy
    if (xSemaphoreTake(s_dma_manager->state_mutex, portMAX_DELAY) == pdTRUE) {
        s_dma_manager->state = SMARTDISPLAY_DMA_STATE_BUSY;
        xSemaphoreGive(s_dma_manager->state_mutex);
    }

    // Perform the transfer in chunks if needed
    err = dma_transfer_chunk(transfer);

    // Update statistics
    if (xSemaphoreTake(s_dma_manager->state_mutex, portMAX_DELAY) == pdTRUE) {
        if (err == ESP_OK) {
            s_dma_manager->completed_transfers++;
        } else {
            s_dma_manager->failed_transfers++;
            s_dma_manager->state = SMARTDISPLAY_DMA_STATE_ERROR;
        }
        s_dma_manager->active_transfers--;
        if (s_dma_manager->active_transfers == 0) {
            s_dma_manager->state = SMARTDISPLAY_DMA_STATE_IDLE;
        }
        xSemaphoreGive(s_dma_manager->state_mutex);
    }

    // Call completion callback
    if (transfer->callback != NULL) {
        transfer->callback(err == ESP_OK, transfer->user_data);
    }
}

static esp_err_t dma_transfer_chunk(smartdisplay_dma_transfer_t *transfer)
{
    // For SPI/I80 panels, we use esp_lcd_panel_draw_bitmap which internally uses DMA
    // The async_color_trans flag determines if we need to wait for chunk completion

    esp_err_t err = esp_lcd_panel_draw_bitmap(s_dma_manager->panel_handle,
                                              transfer->x_start, transfer->y_start,
                                              transfer->x_end, transfer->y_end,
                                              transfer->src_data);

    if (err != ESP_OK) {
        log_e("Panel draw bitmap failed: 0x%x", err);
        return err;
    }

    // If async_color_trans is enabled, wait for the chunk done signal from ISR
    if (s_dma_manager->async_color_trans) {
        if (xSemaphoreTake(s_dma_manager->chunk_done_sem, pdMS_TO_TICKS(SMARTDISPLAY_DMA_CHUNK_TIMEOUT_MS)) != pdTRUE) {
            log_e("Timeout waiting for chunk done");
            return ESP_ERR_TIMEOUT;
        }
    }

    return ESP_OK;
}