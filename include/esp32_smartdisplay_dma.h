#ifndef ESP32_SMARTDISPLAY_DMA_H
#define ESP32_SMARTDISPLAY_DMA_H

#include <esp_err.h>
#include <esp_lcd_panel_ops.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <lvgl.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // DMA transfer states
    typedef enum
    {
        SMARTDISPLAY_DMA_STATE_IDLE = 0,
        SMARTDISPLAY_DMA_STATE_BUSY,
        SMARTDISPLAY_DMA_STATE_ERROR
    } smartdisplay_dma_state_t;

    // DMA transfer completion callback
    typedef void (*smartdisplay_dma_callback_t)(bool success, void *user_data);

    // DMA transfer descriptor
    typedef struct
    {
        const void *src_data;                 // Source data pointer
        size_t data_len;                      // Data length in bytes
        int x_start, y_start;                 // Display coordinates
        int x_end, y_end;                     // Display coordinates
        smartdisplay_dma_callback_t callback; // Completion callback
        void *user_data;                      // User data for callback
        bool high_priority;                   // High priority transfer
    } smartdisplay_dma_transfer_t;

    // DMA manager structure
    typedef struct
    {
        QueueHandle_t transfer_queue;        // Queue for pending transfers
        SemaphoreHandle_t state_mutex;       // Mutex for state protection
        SemaphoreHandle_t chunk_done_sem;    // Signalled by the panel IO's on_color_trans_done callback once a queued chunk has actually finished transferring
        TaskHandle_t worker_task;            // DMA worker task handle
        smartdisplay_dma_state_t state;      // Current DMA state
        void *dma_buffer;                    // DMA-capable buffer
        size_t dma_buffer_size;              // DMA buffer size
        esp_lcd_panel_handle_t panel_handle; // LCD panel handle
        bool async_color_trans;              // True if the panel IO completes color transfers asynchronously and signals chunk_done_sem via smartdisplay_dma_notify_chunk_done()
        uint32_t active_transfers;           // Number of active transfers
        uint32_t completed_transfers;        // Total completed transfers
        uint32_t failed_transfers;           // Total failed transfers
    } smartdisplay_dma_manager_t;

    /**
     * @brief Initialize DMA manager for display transfers
     *
     * @param panel_handle LCD panel handle
     * @param async_color_trans Set to true when the panel IO used by panel_handle
     *        completes esp_lcd_panel_draw_bitmap() asynchronously (e.g. SPI/I80
     *        panels driven through esp_lcd_panel_io, which queue the transaction
     *        and return before the wire transfer finishes) AND its
     *        on_color_trans_done callback calls smartdisplay_dma_notify_chunk_done().
     *        Set to false for panels whose draw_bitmap() already blocks until the
     *        data has physically been written (e.g. RGB parallel panels, or SPI
     *        panels driven with spi_device_polling_transmit()) - for those, no
     *        chunk_done_sem signal will ever arrive, so the DMA worker must not
     *        wait for one.
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t smartdisplay_dma_init(esp_lcd_panel_handle_t panel_handle, bool async_color_trans);

    /**
     * @brief Deinitialize DMA manager
     *
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t smartdisplay_dma_deinit();

    /**
     * @brief Queue a bitmap transfer with DMA optimization
     *
     * @param x_start Start X coordinate
     * @param y_start Start Y coordinate
     * @param x_end End X coordinate
     * @param y_end End Y coordinate
     * @param color_data Pixel data to transfer
     * @param callback Completion callback (optional)
     * @param user_data User data for callback (optional)
     * @param high_priority High priority transfer flag
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t smartdisplay_dma_draw_bitmap(int x_start, int y_start, int x_end, int y_end, const void *color_data, smartdisplay_dma_callback_t callback, void *user_data, bool high_priority);

    /**
     * @brief Check if DMA transfer is recommended for given size
     *
     * @param data_len Data length in bytes
     * @return true if DMA should be used
     */
    bool smartdisplay_dma_should_use_dma(size_t data_len);

    /**
     * @brief Wait for all pending DMA transfers to complete
     *
     * @param timeout_ms Timeout in milliseconds
     * @return esp_err_t ESP_OK on success, ESP_ERR_TIMEOUT on timeout
     */
    esp_err_t smartdisplay_dma_wait_all_done(uint32_t timeout_ms);

    /**
     * @brief Get DMA manager statistics
     *
     * @param active_transfers Number of active transfers
     * @param completed_transfers Total completed transfers
     * @param failed_transfers Total failed transfers
     * @return esp_err_t ESP_OK on success
     */
    esp_err_t smartdisplay_dma_get_stats(uint32_t *active_transfers, uint32_t *completed_transfers, uint32_t *failed_transfers);

    /**
     * @brief Flush LVGL display with DMA optimization
     *
     * @param display LVGL display handle
     * @param area Display area to flush
     * @param px_map Pixel data
     */
    void smartdisplay_dma_lvgl_flush(lv_display_t *display, const lv_area_t *area, uint8_t *px_map);

    /**
     * @brief Notify the DMA manager that a queued panel IO color transfer has
     *        actually completed on the wire.
     *
     * Call this from a panel's on_color_trans_done callback (which runs in ISR
     * context). It gives the semaphore that smartdisplay_dma_transfer_chunk()
     * waits on before reusing the DMA buffer for the next chunk or letting the
     * caller treat the transfer as finished, so lv_display_flush_ready() is
     * only ever called once the SPI/I80 transfer that produced the current
     * framebuffer contents has really finished (not just been queued).
     *
     * @return true if a higher-priority task was woken and the ISR should
     *         request a context switch before it returns
     */
    bool smartdisplay_dma_notify_chunk_done(void);

#ifdef __cplusplus
}
#endif

#endif // ESP32_SMARTDISPLAY_DMA_H
