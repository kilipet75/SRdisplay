#pragma once
#include <stdint.h>
#include <stddef.h>

// Callback-Typ: wird aufgerufen wenn Daten vom USB-CDC-Gerät ankommen
typedef void (*usb_cdc_data_cb_t)(const uint8_t *data, size_t len);

// Callback-Typ: wird aufgerufen bei Verbindungs-Events
typedef void (*usb_cdc_connect_cb_t)(void);

/**
 * @brief USB CDC Host initialisieren und Task starten
 * @param data_cb       Callback für empfangene Daten
 * @param connect_cb    Callback wenn Gerät verbunden (darf NULL sein)
 * @param disconnect_cb Callback wenn Gerät getrennt  (darf NULL sein)
 */
void usb_cdc_host_init(usb_cdc_data_cb_t    data_cb,
                       usb_cdc_connect_cb_t connect_cb,
                       usb_cdc_connect_cb_t disconnect_cb);