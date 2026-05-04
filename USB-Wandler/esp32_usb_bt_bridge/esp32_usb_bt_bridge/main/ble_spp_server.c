#include "ble_spp_server.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"
#include "esp_bt_defs.h"
#include "ble_spp_server.h"

// -----------------------------------------------------------------------
// NEU: Verbindungs-Callbacks
// -----------------------------------------------------------------------
static ble_spp_connect_cb_t s_connect_cb    = NULL;  // <<< NEU
static ble_spp_connect_cb_t s_disconnect_cb = NULL;  // <<< NEU

static const char *TAG = "BLE_SPP";

// -----------------------------------------------------------------------
// UUIDs – wir verwenden Nordic UART Service (NUS) kompatible UUIDs
// Damit funktioniert die Bridge mit nRF Connect App und ähnlichen Tools
// -----------------------------------------------------------------------

// Service UUID: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
#define NUS_SERVICE_UUID  0x6E400001, 0xB5A3, 0xF393, 0xE0, 0xA9, \
                          0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E

// TX Characteristic (Notify): 6E400003-B5A3-F393-E0A9-E50E24DCCA9E
// (Daten fließen ESP32 → Client, daher "TX" aus Sicht des Servers)
#define NUS_TX_CHAR_UUID  0x6E400003, 0xB5A3, 0xF393, 0xE0, 0xA9, \
                          0xE5, 0x0E, 0x24, 0xDC, 0xCA, 0x9E

#define GATTS_APP_ID       0x55
#define DEVICE_NAME        "CDC-BLE-Bridge"
#define GATTS_NUM_HANDLE   8
#define BLE_MTU_SIZE       517  // max. MTU

// -----------------------------------------------------------------------
// Interne Zustandsvariablen
// -----------------------------------------------------------------------
static uint16_t s_gatts_if       = ESP_GATT_IF_NONE;
static uint16_t s_conn_id        = 0xFFFF;
static uint16_t s_service_handle = 0;
static uint16_t s_tx_char_handle = 0;
static uint16_t s_tx_cccd_handle = 0;  // Client Characteristic Config Descriptor
static bool     s_notifications_enabled = false;
static bool     s_connected      = false;
static uint16_t s_mtu            = 23; // Standard BLE MTU

// -----------------------------------------------------------------------
// BLE Advertising Konfiguration
// -----------------------------------------------------------------------

static esp_ble_adv_params_t s_adv_params = {
    .adv_int_min        = 0x20,   // 20ms
    .adv_int_max        = 0x40,   // 40ms
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

// Advertising Data
static uint8_t s_adv_data[] = {
    // Flags: LE General Discoverable, BR/EDR nicht unterstützt
    0x02, 0x01, 0x06,
    // Vollständiger Gerätename
    0x0F, 0x09, 'C','D','C','-','B','L','E','-','B','r','i','d','g','e',
};

// Scan Response: NUS Service UUID 128-bit
static uint8_t s_scan_rsp_data[] = {
    // 128-bit UUID list (NUS Service)
    0x11, 0x07,
    0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5,
    0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5,
    0x01, 0x00, 0x40, 0x6E,
};

// -----------------------------------------------------------------------
// Service / Characteristic Definitionen
// -----------------------------------------------------------------------

// Service UUID 128-bit (little endian für ESP-IDF API)
static uint8_t s_service_uuid[16] = {
    0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5,
    0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5,
    0x01, 0x00, 0x40, 0x6E
};

// TX Characteristic UUID 128-bit (little endian)
static uint8_t s_tx_char_uuid[16] = {
    0x9E, 0xCA, 0xDC, 0x24, 0x0E, 0xE5,
    0xA9, 0xE0, 0x93, 0xF3, 0xA3, 0xB5,
    0x03, 0x00, 0x40, 0x6E
};

// CCCD UUID (Standard 16-bit: 0x2902)
static uint16_t s_cccd_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;

// -----------------------------------------------------------------------
// Advertising neu starten (nach Disconnect)
// -----------------------------------------------------------------------
static void start_advertising(void)
{
    esp_ble_gap_set_device_name(DEVICE_NAME);

    esp_ble_gap_config_adv_data_raw(s_adv_data, sizeof(s_adv_data));
    esp_ble_gap_config_scan_rsp_data_raw(s_scan_rsp_data, sizeof(s_scan_rsp_data));
}

// -----------------------------------------------------------------------
// GAP Callback
// -----------------------------------------------------------------------
static void gap_event_handler(esp_gap_ble_cb_event_t event,
                              esp_ble_gap_cb_param_t *param)
{
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
            // Advertising starten sobald Daten gesetzt wurden
            esp_ble_gap_start_advertising(&s_adv_params);
            break;

        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            if (param->adv_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG, "Advertising gestartet – sichtbar als \"%s\"",
                         DEVICE_NAME);
            } else {
                ESP_LOGE(TAG, "Advertising Start fehlgeschlagen");
            }
            break;

        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            ESP_LOGI(TAG, "Advertising gestoppt");
            break;

        case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
            ESP_LOGI(TAG, "Verbindungsparameter aktualisiert: "
                     "interval=%d, latency=%d, timeout=%d",
                     param->update_conn_params.conn_int,
                     param->update_conn_params.latency,
                     param->update_conn_params.timeout);
            break;

        default:
            break;
    }
}

// -----------------------------------------------------------------------
// GATTS Callback
// -----------------------------------------------------------------------
static void gatts_event_handler(esp_gatts_cb_event_t event,
                                esp_gatt_if_t gatts_if,
                                esp_ble_gatts_cb_param_t *param)
{
    // Interface speichern sobald App registriert
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            s_gatts_if = gatts_if;
        } else {
            ESP_LOGE(TAG, "GATTS App Registrierung fehlgeschlagen: %d",
                     param->reg.app_id);
            return;
        }
    }

    // Nur unsere App-Events verarbeiten
    if (gatts_if != ESP_GATT_IF_NONE && gatts_if != s_gatts_if) {
        return;
    }

    switch (event) {

        // ----------------------------------------------------------------
        case ESP_GATTS_REG_EVT: {
            ESP_LOGI(TAG, "GATTS App registriert, erstelle Service...");

            // Service erstellen
            esp_gatt_srvc_id_t service_id = {
                .is_primary = true,
                .id = {
                    .inst_id = 0,
                    .uuid = {
                        .len = ESP_UUID_LEN_128,
                    }
                }
            };
            memcpy(service_id.id.uuid.uuid.uuid128,
                   s_service_uuid, ESP_UUID_LEN_128);

            esp_ble_gatts_create_service(gatts_if, &service_id,
                                         GATTS_NUM_HANDLE);
            break;
        }

        // ----------------------------------------------------------------
        case ESP_GATTS_CREATE_EVT: {
            if (param->create.status != ESP_GATT_OK) {
                ESP_LOGE(TAG, "Service Erstellung fehlgeschlagen");
                break;
            }
            s_service_handle = param->create.service_handle;
            ESP_LOGI(TAG, "Service erstellt, Handle: %d", s_service_handle);

            // Service starten
            esp_ble_gatts_start_service(s_service_handle);

            // TX Characteristic hinzufügen (Notify)
            esp_bt_uuid_t tx_uuid = {
                .len = ESP_UUID_LEN_128,
            };
            memcpy(tx_uuid.uuid.uuid128, s_tx_char_uuid, ESP_UUID_LEN_128);

            esp_attr_value_t char_val = {
                .attr_max_len = BLE_MTU_SIZE,
                .attr_len     = 0,
                .attr_value   = NULL,
            };

            esp_ble_gatts_add_char(
                s_service_handle,
                &tx_uuid,
                ESP_GATT_PERM_READ,
                ESP_GATT_CHAR_PROP_BIT_NOTIFY,
                &char_val,
                NULL
            );
            break;
        }

        // ----------------------------------------------------------------
        case ESP_GATTS_ADD_CHAR_EVT: {
            if (param->add_char.status != ESP_GATT_OK) {
                ESP_LOGE(TAG, "Characteristic hinzufügen fehlgeschlagen");
                break;
            }
            s_tx_char_handle = param->add_char.attr_handle;
            ESP_LOGI(TAG, "TX Characteristic hinzugefügt, Handle: %d",
                     s_tx_char_handle);

            // CCCD (Client Characteristic Config Descriptor) hinzufügen
            // Damit der Client Notifications aktivieren kann
            esp_bt_uuid_t cccd_uuid = {
                .len = ESP_UUID_LEN_16,
                .uuid.uuid16 = s_cccd_uuid,
            };

            uint8_t cccd_val[2] = {0x00, 0x00};
            esp_attr_value_t cccd_attr_val = {
                .attr_max_len = 2,
                .attr_len     = 2,
                .attr_value   = cccd_val,
            };

            esp_ble_gatts_add_char_descr(
                s_service_handle,
                &cccd_uuid,
                ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                &cccd_attr_val,
                NULL
            );
            break;
        }

        // ----------------------------------------------------------------
        case ESP_GATTS_ADD_CHAR_DESCR_EVT: {
            if (param->add_char_descr.status == ESP_GATT_OK) {
                s_tx_cccd_handle = param->add_char_descr.attr_handle;
                ESP_LOGI(TAG, "CCCD hinzugefügt, Handle: %d",
                         s_tx_cccd_handle);
            }
            // Advertising starten
            start_advertising();
            break;
        }

        // ----------------------------------------------------------------
        case ESP_GATTS_CONNECT_EVT: {
            s_conn_id   = param->connect.conn_id;
            s_connected = true;
            s_notifications_enabled = false;
            ESP_LOGI(TAG, "BLE Client verbunden, conn_id=%d", s_conn_id);

            // Verbindungsparameter optimieren (schnelleres Interval)
            esp_ble_conn_update_params_t conn_params = {
                .latency  = 0,
                .max_int  = 0x10,  // 20ms
                .min_int  = 0x08,  // 10ms
                .timeout  = 400,   // 4s
            };
            memcpy(conn_params.bda, param->connect.remote_bda,
                   sizeof(esp_bd_addr_t));
            esp_ble_gap_update_conn_params(&conn_params);

            // MTU negotiation anfordern
            //esp_ble_gap_set_local_mtu(BLE_MTU_SIZE);
            break;
        }

        // ----------------------------------------------------------------
        case ESP_GATTS_DISCONNECT_EVT: {
            s_connected = false;
            s_notifications_enabled = false;
            s_conn_id   = 0xFFFF;
            ESP_LOGW(TAG, "BLE Client getrennt, starte Advertising neu...");
            
            // <<< NEU: Disconnect-Callback aufrufen
            if (s_disconnect_cb) {
                s_disconnect_cb();
            }

            start_advertising();
            break;
        }

        // ----------------------------------------------------------------
        case ESP_GATTS_WRITE_EVT: {
            // Prüfen ob Client Notifications aktiviert/deaktiviert
            if (param->write.handle == s_tx_cccd_handle && param->write.len == 2) {
                uint16_t cccd_val = param->write.value[0] |
                                    (param->write.value[1] << 8);

                if (cccd_val == 0x0001) {
                    s_notifications_enabled = true;
                    ESP_LOGI(TAG, "Notifications aktiviert vom Client");

                    // <<< NEU: Connect-Callback erst wenn Notifications
                    //          aktiv sind (Client ist wirklich bereit)
                    if (s_connect_cb) {
                        s_connect_cb();
                    }

                } else {
                    s_notifications_enabled = false;
                    ESP_LOGI(TAG, "Notifications deaktiviert");

                    // <<< NEU: Auch hier Disconnect signalisieren
                    if (s_disconnect_cb) {
                        s_disconnect_cb();
                    }

                }

                // Response senden wenn nötig
                if (param->write.need_rsp) {
                    esp_ble_gatts_send_response(gatts_if, param->write.conn_id,
                                                param->write.trans_id,
                                                ESP_GATT_OK, NULL);
                }
            }
            break;
        }

        // ----------------------------------------------------------------
        case ESP_GATTS_MTU_EVT:
            s_mtu = param->mtu.mtu;
            ESP_LOGI(TAG, "MTU ausgehandelt: %d Bytes", s_mtu);
            break;

        case ESP_GATTS_START_EVT:
            ESP_LOGI(TAG, "Service gestartet");
            break;

        case ESP_GATTS_CONF_EVT:
            // Confirmation für Indication (wir nutzen Notify, daher ignoriert)
            break;

        default:
            break;
    }
}

// -----------------------------------------------------------------------
// Öffentliche Funktionen
// -----------------------------------------------------------------------

void ble_spp_server_init(ble_spp_connect_cb_t connect_cb,
                         ble_spp_connect_cb_t disconnect_cb)
{
    s_connect_cb    = connect_cb;    // <<< NEU
    s_disconnect_cb = disconnect_cb; // <<< NEU

    ESP_LOGI(TAG, "BLE SPP Server initialisiert");
    
    // GAP und GATTS Callbacks registrieren
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(gap_event_handler));
    ESP_ERROR_CHECK(esp_ble_gatts_register_callback(gatts_event_handler));

    // App registrieren → löst ESP_GATTS_REG_EVT aus
    ESP_ERROR_CHECK(esp_ble_gatts_app_register(GATTS_APP_ID));
}

bool ble_spp_is_connected(void)
{
    return s_connected && s_notifications_enabled;
}

bool ble_spp_send(const uint8_t *data, size_t len)
{
    if (!ble_spp_is_connected()) {
        return false;
    }

    // MTU - 3 Bytes Overhead = max Nutzdaten pro Paket
    size_t max_chunk = s_mtu - 3;
    size_t offset    = 0;

    while (offset < len) {
        size_t chunk_len = len - offset;
        if (chunk_len > max_chunk) {
            chunk_len = max_chunk;
        }

        esp_err_t err = esp_ble_gatts_send_indicate(
            s_gatts_if,
            s_conn_id,
            s_tx_char_handle,
            chunk_len,
            (uint8_t *)(data + offset),
            false   // false = Notification (kein ACK nötig)
        );

        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Senden fehlgeschlagen: %s", esp_err_to_name(err));
            return false;
        }

        offset += chunk_len;

        // Kurze Pause zwischen Chunks damit BLE Stack nicht überflutet wird
        if (offset < len) {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }

    return true;
}