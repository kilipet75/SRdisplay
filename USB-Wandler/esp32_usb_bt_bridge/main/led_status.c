#include "led_status.h"
#include "driver/rmt_tx.h"
#include "esp_log.h"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "LED";

// -----------------------------------------------------------------------
// Konfiguration
// -----------------------------------------------------------------------
#define LED_GPIO_PIN        21          // DIN Pin GP21 (ESP32-S3 onboard)
#define LED_RMT_RESOLUTION  10000000    // 10 MHz → 100ns Auflösung
#define LED_BRIGHTNESS      50          // 0–255 (50 = gedimmt, schont die Augen)

// WS2812 Timing in Nanosekunden
#define WS2812_T0H_NS       400
#define WS2812_T0L_NS       850
#define WS2812_T1H_NS       800
#define WS2812_T1L_NS       450
#define WS2812_RESET_NS     55000       // > 50µs Reset-Pause

// -----------------------------------------------------------------------
// RMT Handles
// -----------------------------------------------------------------------
static rmt_channel_handle_t s_rmt_chan = NULL;
static rmt_encoder_handle_t s_encoder  = NULL;

// -----------------------------------------------------------------------
// Eigener RMT Bytes-Encoder für WS2812
// -----------------------------------------------------------------------
typedef struct {
    rmt_encoder_t    base;          // MUSS erstes Element sein
    rmt_encoder_t   *bytes_encoder;
    rmt_encoder_t   *copy_encoder;
    rmt_symbol_word_t reset_symbol;
    int              state;         // 0 = Daten, 1 = Reset
} ws2812_encoder_t;

// Vorwärts-Deklaration
static size_t ws2812_encode(rmt_encoder_t *encoder,
                            rmt_channel_handle_t channel,
                            const void *primary_data,
                            size_t data_size,
                            rmt_encode_state_t *ret_state);
static esp_err_t ws2812_del(rmt_encoder_t *encoder);
static esp_err_t ws2812_reset(rmt_encoder_t *encoder);

// -----------------------------------------------------------------------
// Encoder: encode()
// -----------------------------------------------------------------------
static size_t ws2812_encode(rmt_encoder_t *encoder,
                            rmt_channel_handle_t channel,
                            const void *primary_data,
                            size_t data_size,
                            rmt_encode_state_t *ret_state)
{
    ws2812_encoder_t *ws = __containerof(encoder, ws2812_encoder_t, base);

    rmt_encode_state_t session_state = RMT_ENCODING_RESET;
    rmt_encode_state_t state         = RMT_ENCODING_RESET;
    size_t encoded_symbols = 0;

    switch (ws->state) {
    case 0: // GRB Daten senden
        encoded_symbols += ws->bytes_encoder->encode(
            ws->bytes_encoder, channel,
            primary_data, data_size,
            &session_state);

        if (session_state & RMT_ENCODING_COMPLETE) {
            ws->state = 1; // weiter mit Reset
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            goto out;
        }
        // fallthrough
    case 1: // Reset-Symbol senden
        encoded_symbols += ws->copy_encoder->encode(
            ws->copy_encoder, channel,
            &ws->reset_symbol,
            sizeof(ws->reset_symbol),
            &session_state);

        if (session_state & RMT_ENCODING_COMPLETE) {
            ws->state = RMT_ENCODING_RESET; // fertig
            state |= RMT_ENCODING_COMPLETE;
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            goto out;
        }
    }

out:
    *ret_state = state;
    return encoded_symbols;
}

// -----------------------------------------------------------------------
// Encoder: del()
// -----------------------------------------------------------------------
static esp_err_t ws2812_del(rmt_encoder_t *encoder)
{
    ws2812_encoder_t *ws = __containerof(encoder, ws2812_encoder_t, base);
    rmt_del_encoder(ws->bytes_encoder);
    rmt_del_encoder(ws->copy_encoder);
    free(ws);
    return ESP_OK;
}

// -----------------------------------------------------------------------
// Encoder: reset()
// -----------------------------------------------------------------------
static esp_err_t ws2812_reset(rmt_encoder_t *encoder)
{
    ws2812_encoder_t *ws = __containerof(encoder, ws2812_encoder_t, base);
    rmt_encoder_reset(ws->bytes_encoder);
    rmt_encoder_reset(ws->copy_encoder);
    ws->state = 0;
    return ESP_OK;
}

// -----------------------------------------------------------------------
// Encoder erstellen
// -----------------------------------------------------------------------
static esp_err_t create_ws2812_encoder(rmt_encoder_handle_t *ret_encoder)
{
    ws2812_encoder_t *ws = calloc(1, sizeof(ws2812_encoder_t));
    if (!ws) return ESP_ERR_NO_MEM;

    ws->base.encode = ws2812_encode;
    ws->base.del    = ws2812_del;
    ws->base.reset  = ws2812_reset;

    // Bytes-Encoder für WS2812 Bit-Timing
    rmt_bytes_encoder_config_t bytes_cfg = {
        .bit0 = {
            .level0    = 1,
            .duration0 = WS2812_T0H_NS / (1000000000 / LED_RMT_RESOLUTION),
            .level1    = 0,
            .duration1 = WS2812_T0L_NS / (1000000000 / LED_RMT_RESOLUTION),
        },
        .bit1 = {
            .level0    = 1,
            .duration0 = WS2812_T1H_NS / (1000000000 / LED_RMT_RESOLUTION),
            .level1    = 0,
            .duration1 = WS2812_T1L_NS / (1000000000 / LED_RMT_RESOLUTION),
        },
        .flags.msb_first = 1,  // WS2812 erwartet MSB zuerst
    };
    ESP_ERROR_CHECK(rmt_new_bytes_encoder(&bytes_cfg, &ws->bytes_encoder));

    // Copy-Encoder für das Reset-Symbol
    rmt_copy_encoder_config_t copy_cfg = {};
    ESP_ERROR_CHECK(rmt_new_copy_encoder(&copy_cfg, &ws->copy_encoder));

    // Reset-Symbol: Low für > 50µs
    uint32_t reset_ticks = (uint32_t)(
        (uint64_t)LED_RMT_RESOLUTION * WS2812_RESET_NS / 1000000000
    );
    ws->reset_symbol = (rmt_symbol_word_t){
        .level0    = 0,
        .duration0 = reset_ticks / 2,
        .level1    = 0,
        .duration1 = reset_ticks / 2,
    };

    *ret_encoder = &ws->base;
    return ESP_OK;
}

// -----------------------------------------------------------------------
// LED mit GRB-Farbe setzen
// (WS2812 erwartet Byte-Reihenfolge: G, R, B)
// -----------------------------------------------------------------------
static void led_set_grb(uint8_t red, uint8_t green, uint8_t blue)
{
    uint8_t grb[3] = {
        (uint8_t)(green * LED_BRIGHTNESS / 255),
        (uint8_t)(red   * LED_BRIGHTNESS / 255),
        (uint8_t)(blue  * LED_BRIGHTNESS / 255),
    };

    rmt_transmit_config_t tx_cfg = {
        .loop_count = 0,  // einmalig senden
    };

    esp_err_t err = rmt_transmit(s_rmt_chan, s_encoder, grb, sizeof(grb), &tx_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "rmt_transmit fehlgeschlagen: %s", esp_err_to_name(err));
        return;
    }

    // Warten bis Übertragung abgeschlossen (max 100ms)
    rmt_tx_wait_all_done(s_rmt_chan, pdMS_TO_TICKS(100));
}

// -----------------------------------------------------------------------
// Public API
// -----------------------------------------------------------------------
void led_status_init(void)
{
    ESP_LOGI(TAG, "WS2812 LED initialisieren (Pin %d)", LED_GPIO_PIN);

    // RMT TX Kanal erstellen
    rmt_tx_channel_config_t chan_cfg = {
        .gpio_num            = LED_GPIO_PIN,
        .clk_src             = RMT_CLK_SRC_DEFAULT,
        .resolution_hz       = LED_RMT_RESOLUTION,
        .mem_block_symbols   = 64,
        .trans_queue_depth   = 4,
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&chan_cfg, &s_rmt_chan));

    // WS2812-Encoder erstellen
    ESP_ERROR_CHECK(create_ws2812_encoder(&s_encoder));

    // Kanal aktivieren
    ESP_ERROR_CHECK(rmt_enable(s_rmt_chan));

    // Startzustand: Rot (Boot)
    led_status_set(LED_STATE_BOOT);
    ESP_LOGI(TAG, "LED bereit");
}

void led_status_set(led_state_t state)
{
    switch (state) {
    case LED_STATE_BOOT:
        ESP_LOGI(TAG, "LED → ROT (Boot)");
        led_set_grb(255, 0, 0);
        break;

    case LED_STATE_USB_CONNECTED:
        ESP_LOGI(TAG, "LED → GRÜN (USB verbunden)");
        led_set_grb(0, 255, 0);
        break;

    case LED_STATE_BLE_CONNECTED:
        ESP_LOGI(TAG, "LED → BLAU (BLE verbunden)");
        led_set_grb(0, 0, 255);
        break;

    default:
        ESP_LOGW(TAG, "Unbekannter LED-Zustand: %d", state);
        led_set_grb(0, 0, 0); // aus
        break;
    }
}