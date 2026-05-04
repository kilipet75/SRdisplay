#include "usb_cdc_host.h"

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "usb/usb_host.h"
#include "usb/cdc_acm_host.h"

static const char *TAG = "USB_CDC";

// -----------------------------------------------------------------------
// Interne Variablen
// -----------------------------------------------------------------------
static usb_cdc_data_cb_t s_data_callback = NULL;
static usb_cdc_connect_cb_t s_connect_callback    = NULL;  // <<< NEU
static usb_cdc_connect_cb_t s_disconnect_callback = NULL;  // <<< NEU
static SemaphoreHandle_t s_device_disconnected_sem;

// -----------------------------------------------------------------------
// USB-CDC Callbacks
// -----------------------------------------------------------------------

/**
 * Wird aufgerufen wenn Daten vom CDC-Gerät empfangen werden.
 * Gibt Daten direkt an den registrierten Callback weiter.
 */
static bool usb_rx_callback(const uint8_t *data, size_t data_len, void *arg)
{
    if (s_data_callback && data_len > 0) {
        s_data_callback(data, data_len);
    }
    return true; // true = Puffer wird freigegeben
}

/**
 * Wird aufgerufen bei Verbindungs-Events (connect/disconnect/error).
 */
static void usb_event_callback(const cdc_acm_host_dev_event_data_t *event, void *user_ctx)
{
    switch (event->type) {

        case CDC_ACM_HOST_ERROR:
            ESP_LOGE(TAG, "CDC-ACM Fehler: %d", event->data.error);
            // Durchfall zu DISCONNECTED gewollt
            __attribute__((fallthrough));

        case CDC_ACM_HOST_DEVICE_DISCONNECTED:
            ESP_LOGW(TAG, "USB-CDC Gerät getrennt");
            // <<< NEU: Disconnect-Callback aufrufen
            if (s_disconnect_callback) {
                s_disconnect_callback();
            }
            xSemaphoreGive(s_device_disconnected_sem);
            break;

        case CDC_ACM_HOST_SERIAL_STATE:
            ESP_LOGI(TAG, "Serial IOCTL Event");
            break;

        default:
            ESP_LOGW(TAG, "Unbekanntes CDC Event: %d", event->type);
            break;
    }
}

// -----------------------------------------------------------------------
// USB Host Library Callbacks
// -----------------------------------------------------------------------

/**
 * Callback für USB Host Library Events (wird in eigenem Task benötigt).
 */
static void usb_host_lib_event_cb(void *arg)
{
    while (1) {
        uint32_t event_flags;
        usb_host_lib_handle_events(portMAX_DELAY, &event_flags);

        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) {
            ESP_LOGI(TAG, "Keine USB-Clients mehr");
            usb_host_device_free_all();
        }
        if (event_flags & USB_HOST_LIB_EVENT_FLAGS_ALL_FREE) {
            ESP_LOGI(TAG, "Alle USB-Geräte freigegeben");
        }
    }
}

// -----------------------------------------------------------------------
// Haupt-Task: Geräteerkennung und Verbindung
// -----------------------------------------------------------------------

static void usb_cdc_host_task(void *arg)
{
    ESP_LOGI(TAG, "USB CDC Host Task gestartet");

    // 1) USB Host Library initialisieren
    const usb_host_config_t host_config = {
        .skip_phy_setup = false,
        .intr_flags     = ESP_INTR_FLAG_LEVEL1,
    };
    ESP_ERROR_CHECK(usb_host_install(&host_config));

    // 2) Eigener Task für USB Host Library Events
    xTaskCreate(usb_host_lib_event_cb, "usb_host_events",
                4096, NULL, 4, NULL);

    // 3) CDC-ACM Treiber initialisieren
    const cdc_acm_host_driver_config_t cdc_driver_config = {
        .driver_task_stack_size = 4096,
        .driver_task_priority   = 5,
        .xCoreID                = 0,
        .new_dev_cb             = NULL, // wir pollen selbst
    };
    ESP_ERROR_CHECK(cdc_acm_host_install(&cdc_driver_config));

    // 4) Semaphor für Disconnect-Erkennung
    s_device_disconnected_sem = xSemaphoreCreateBinary();
    assert(s_device_disconnected_sem);

    // 5) Hauptschleife: immer wieder auf Gerät warten → verbinden
    while (1) {
        ESP_LOGI(TAG, "Warte auf USB-CDC Gerät...");

        // CDC-ACM Gerät öffnen (blockiert bis Gerät gefunden)
        cdc_acm_dev_hdl_t cdc_dev = NULL;

        const cdc_acm_host_device_config_t dev_config = {
            .connection_timeout_ms = 10000,  // 10s Timeout
            .out_buffer_size       = 512,
            .in_buffer_size        = 512,
            .event_cb              = usb_event_callback,
            .data_cb               = usb_rx_callback,
            .user_arg              = NULL,
        };

        // Verbindung herstellen (VID=0, PID=0 → erstes gefundenes Gerät)
        esp_err_t err = cdc_acm_host_open(
            0x0000,         // VID: 0 = beliebig
            0x0000,         // PID: 0 = beliebig
            0,              // Interface Nummer
            &dev_config,
            &cdc_dev
        );

        if (err != ESP_OK || cdc_dev == NULL) {
            ESP_LOGW(TAG, "Gerät nicht gefunden oder Fehler (%s), erneut versuchen...",
                     esp_err_to_name(err));
            vTaskDelay(pdMS_TO_TICKS(2000));
            continue;
        }

        ESP_LOGI(TAG, "USB-CDC Gerät verbunden!");

        // <<< NEU: Connect-Callback aufrufen
        if (s_connect_callback) {
            s_connect_callback();
        }

        // 6) Baudrate und Linecoding setzen (115200, 8N1)
        cdc_acm_line_coding_t line_coding = {
            .dwDTERate   = 115200,
            .bCharFormat = 0,  // 1 Stoppbit
            .bParityType = 0,  // Keine Parität
            .bDataBits   = 8,
        };
        err = cdc_acm_host_line_coding_set(cdc_dev, &line_coding);
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "Line Coding konnte nicht gesetzt werden: %s",
                     esp_err_to_name(err));
        }

        // 7) RTS/DTR setzen (manche Geräte brauchen das zum Starten)
        cdc_acm_host_set_control_line_state(cdc_dev, true, true);

        ESP_LOGI(TAG, "Serielle Schnittstelle offen: 115200 8N1");

        // 8) Warten bis Gerät getrennt wird
        xSemaphoreTake(s_device_disconnected_sem, portMAX_DELAY);

        // 9) Aufräumen
        cdc_acm_host_close(cdc_dev);
        ESP_LOGW(TAG, "Verbindung getrennt, warte auf neues Gerät...");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Wird nie erreicht
    vTaskDelete(NULL);
}

// -----------------------------------------------------------------------
// Öffentliche Initialisierungsfunktion
// -----------------------------------------------------------------------

void usb_cdc_host_init(usb_cdc_data_cb_t data_cb,
                       usb_cdc_connect_cb_t connect_cb,
                       usb_cdc_connect_cb_t disconnect_cb)
{
    s_data_callback = data_cb;
    s_connect_callback    = connect_cb;    // <<< NEU
    s_disconnect_callback = disconnect_cb; // <<< NEU

    xTaskCreate(usb_cdc_host_task, "usb_cdc_host",
                8192, NULL, 5, NULL);
}