#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// Callback-Typ: wird aufgerufen bei BLE Verbindungs-Events  // <<< NEU
typedef void (*ble_spp_connect_cb_t)(void);

/**
 * @brief BLE SPP Server initialisieren
 * @param connect_cb    Callback wenn Client verbunden    (darf NULL sein)
 * @param disconnect_cb Callback wenn Client getrennt     (darf NULL sein)
 */
void ble_spp_server_init(ble_spp_connect_cb_t connect_cb,      // <<< NEU
                         ble_spp_connect_cb_t disconnect_cb);  // <<< NEU

/**
 * @brief Daten per BLE Notification senden
 * @param data Zeiger auf die Daten
 * @param len  Datenlänge (max. ~500 Bytes, wird ggf. aufgeteilt)
 * @return true wenn gesendet, false wenn kein Client verbunden
 */
bool ble_spp_send(const uint8_t *data, size_t len);

/**
 * @brief Gibt an ob ein BLE-Client verbunden ist
 */
bool ble_spp_is_connected(void);