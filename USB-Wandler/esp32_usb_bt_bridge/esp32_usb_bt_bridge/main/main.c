#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "nvs_flash.h"

#include "usb_cdc_host.h"
#include "ble_spp_server.h"

#include "led_status.h"

static const char *TAG = "MAIN";

// -----------------------------------------------------------------------
// Datenpuffer für USB → BLE Transfer
// Queue damit USB-Callback nicht blockiert
// -----------------------------------------------------------------------

#define DATA_QUEUE_LEN      32
#define DATA_CHUNK_MAX_LEN  512

typedef struct {
    uint8_t data[DATA_CHUNK_MAX_LEN];
    size_t  len;
} data_chunk_t;

static QueueHandle_t s_data_queue;

// -----------------------------------------------------------------------
// USB-CDC Daten Callback → in Queue einreihen
// -----------------------------------------------------------------------
static void on_usb_data_received(const uint8_t *data, size_t len)
{
    if (len == 0 || len > DATA_CHUNK_MAX_LEN) {
        ESP_LOGW(TAG, "Ungültige Datenlänge: %zu", len);
        return;
    }

    data_chunk_t chunk;
    memcpy(chunk.data, data, len);
    chunk.len = len;

    // Nicht-blockierend in Queue schreiben
    if (xQueueSend(s_data_queue, &chunk, 0) != pdTRUE) {
        ESP_LOGW(TAG, "Queue voll, Daten verworfen!");
    }
}

// -----------------------------------------------------------------------
// USB-CDC Verbindungs-Callbacks                           // <<< NEU
// -----------------------------------------------------------------------
static void on_usb_connected(void)                        // <<< NEU
{
    ESP_LOGI(TAG, "USB-CDC Gerät verbunden");
    led_status_set(LED_STATE_USB_CONNECTED);
}

static void on_usb_disconnected(void)                     // <<< NEU
{
    ESP_LOGI(TAG, "USB-CDC Gerät getrennt");
    led_status_set(LED_STATE_BOOT);
}

// -----------------------------------------------------------------------
// BLE Callbacks
// -----------------------------------------------------------------------
static void on_ble_connected(void)
{
    ESP_LOGI(TAG, "BLE Client verbunden → LED blau");
    led_status_set(LED_STATE_BLE_CONNECTED);
}

static void on_ble_disconnected(void)
{
    ESP_LOGI(TAG, "BLE Client getrennt → LED je nach USB-Zustand");
    // USB könnte noch verbunden sein → grün, sonst rot
    // Da wir keinen globalen USB-Status haben, grün als Fallback
    // (USB-Task würde bei echtem Disconnect on_usb_disconnected rufen)
    led_status_set(LED_STATE_USB_CONNECTED);
}

// -----------------------------------------------------------------------
// BLE Sende-Task: liest aus Queue und sendet per BLE
// -----------------------------------------------------------------------
static void ble_send_task(void *arg)
{
    data_chunk_t chunk;
    bool was_connected = false;

    ESP_LOGI(TAG, "BLE Send Task gestartet");

    while (1) {
        // Auf Daten warten (max 1s Timeout für Status-Log)
        if (xQueueReceive(s_data_queue, &chunk, pdMS_TO_TICKS(1000)) == pdTRUE) {
            bool is_connected = ble_spp_is_connected();

            // <<< NEU: LED-Zustand bei Änderung aktualisieren
            if (is_connected && !was_connected) {
                ESP_LOGI(TAG, "BLE verbunden → LED blau");
                led_status_set(LED_STATE_BLE_CONNECTED);
                was_connected = true;
            } else if (!is_connected && was_connected) {
                ESP_LOGI(TAG, "BLE getrennt → LED grün");
                led_status_set(LED_STATE_USB_CONNECTED);
                was_connected = false;
            }
            if (is_connected) {
                // Nullterminierung für Logging (temporär)
                char preview[33] = {0};
                size_t plen = chunk.len < 32 ? chunk.len : 32;
                memcpy(preview, chunk.data, plen);

                ESP_LOGI(TAG, "Sende %zu Bytes: \"%s%s\"",
                         chunk.len, preview,
                         chunk.len > 32 ? "..." : "");

                bool ok = ble_spp_send(chunk.data, chunk.len);
                if (!ok) {
                    ESP_LOGW(TAG, "BLE Senden fehlgeschlagen");
                }
            } else {
                // BLE nicht verbunden: Daten verwerfen mit Warnung
                static uint32_t dropped = 0;
                dropped++;
                if (dropped % 10 == 1) {
                    ESP_LOGW(TAG, "BLE nicht verbunden, %lu Pakete verworfen",
                             dropped);
                }
            }
        } else {
            // <<< NEU: Auch bei Queue-Timeout BLE-Zustand prüfen
            //          (für den Fall dass BLE sich trennt ohne Datentransfer)
            bool is_connected = ble_spp_is_connected();
            if (is_connected && !was_connected) {
                led_status_set(LED_STATE_BLE_CONNECTED);
                was_connected = true;
            } else if (!is_connected && was_connected) {
                led_status_set(LED_STATE_USB_CONNECTED);
                was_connected = false;
            }
        }
        // Andernfalls: Queue war leer, Loop weiter
    } //while(1)
}

// -----------------------------------------------------------------------
// Bluetooth Stack initialisieren
// -----------------------------------------------------------------------
static esp_err_t bluetooth_init(void)
{
    // Classic BT Speicher freigeben (wir nutzen nur BLE)
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));

    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    ESP_LOGI(TAG, "Bluetooth Stack initialisiert");
    return ESP_OK;
}

static void keepalive_task(void *arg)
{
    uint32_t count = 0;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000)); // 10 Sekunden

        char msg[32];
        int len = snprintf(msg, sizeof(msg), "keepalive:%lu\n", count++);

        if (ble_spp_send((uint8_t *)msg, len)) {
            ESP_LOGI("KEEPALIVE", "Gesendet: %s", msg);
        } else {
            ESP_LOGI("KEEPALIVE", "Kein Client verbunden");
        }
    }
}

// -----------------------------------------------------------------------
// app_main
// -----------------------------------------------------------------------
void app_main(void)
{
    ESP_LOGI(TAG, "=== USB-CDC → BLE Bridge startet ===");

    // 1) LED zuerst initialisieren → sofort ROT                // <<< NEU
    led_status_init();

    // 2) NVS initialisieren (wird von BT benötigt)
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
    ESP_LOGI(TAG, "NVS initialisiert");

    // 3) Daten-Queue erstellen
    s_data_queue = xQueueCreate(DATA_QUEUE_LEN, sizeof(data_chunk_t));
    assert(s_data_queue != NULL);
    ESP_LOGI(TAG, "Daten-Queue erstellt (%d Slots)", DATA_QUEUE_LEN);

    // 4) Bluetooth initialisieren
    ESP_ERROR_CHECK(bluetooth_init());

    // 5) BLE SPP Server starten
    ble_spp_server_init(on_ble_connected, on_ble_disconnected);
    ESP_LOGI(TAG, "BLE Server gestartet");

    xTaskCreate(keepalive_task, "keepalive", 2048, NULL, 3, NULL);

    // 6) BLE Send Task starten
    xTaskCreate(ble_send_task, "ble_send",
                4096, NULL, 4, NULL);

    // 7) Kurz warten damit BLE sich stabilisiert
    vTaskDelay(pdMS_TO_TICKS(500));

    // 8) USB CDC Host starten
    usb_cdc_host_init(on_usb_data_received,
                      on_usb_connected,
                      on_usb_disconnected);
    ESP_LOGI(TAG, "USB CDC Host gestartet");

    ESP_LOGI(TAG, "System bereit!");
    ESP_LOGI(TAG, "Verbinde ein USB-CDC Gerät und öffne BLE-Verbindung");
    ESP_LOGI(TAG, "BLE Name: 'CDC-BLE-Bridge'");

    // app_main darf enden – alle Tasks laufen im Hintergrund
}

