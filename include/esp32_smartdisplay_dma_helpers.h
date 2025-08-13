#pragma once

#include <esp32_smartdisplay_dma.h>
#include <lvgl.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Common DMA flush callback for LVGL displays
     * @param success Whether the DMA transfer was successful
     * @param user_data Pointer to lv_display_t
     */
    void smartdisplay_dma_lvgl_flush_callback(bool success, void *user_data);

    /**
     * @brief Optimized flush function for SPI/I80/QSPI panels with byte swapping
     * @param display LVGL display object
     * @param area Area to flush
     * @param px_map Pixel data buffer
     * @param panel_handle ESP LCD panel handle for fallback
     * @param panel_name Panel name for logging
     * @return ESP_OK if handled, ESP_FAIL if fallback needed
     */
    esp_err_t smartdisplay_dma_flush_with_byteswap(lv_display_t *display, const lv_area_t *area, uint8_t *px_map, esp_lcd_panel_handle_t panel_handle, const char *panel_name);

    /**
     * @brief Check if DMA should be used for a given transfer size
     * @param transfer_size Size of transfer in bytes
     * @return true if DMA should be used, false otherwise
     */
    bool smartdisplay_dma_should_use_for_size(size_t transfer_size);

    /**
     * @brief Initialize DMA for a panel with standardized logging
     * @param panel_handle ESP LCD panel handle
     * @param panel_name Panel name for logging
     * @return ESP_OK on success, error code otherwise
     */
    esp_err_t smartdisplay_dma_init_with_logging(esp_lcd_panel_handle_t panel_handle, const char *panel_name);

    /**
     * @brief Structure to pass both display and buffer to rotation callback
     */
    typedef struct
    {
        lv_display_t *display;
        void *rotation_buffer;
    } rotation_callback_data_t;

    /**
     * @brief DMA completion callback for rotated displays
     * @param success Whether the DMA transfer was successful
     * @param user_data Pointer to rotation_callback_data_t
     */
    void smartdisplay_dma_rotation_callback(bool success, void *user_data);

    /**
     * @brief Optimized flush function for parallel panels with software rotation
     * @param display LVGL display object
     * @param area Area to flush
     * @param px_map Pixel data buffer
     * @param panel_handle ESP LCD panel handle for fallback
     * @param panel_name Panel name for logging
     * @return ESP_OK if handled, ESP_FAIL if fallback needed
     */
    esp_err_t smartdisplay_dma_flush_with_rotation(lv_display_t *display, const lv_area_t *area, uint8_t *px_map, esp_lcd_panel_handle_t panel_handle, const char *panel_name);

#ifdef __cplusplus
}
#endif
