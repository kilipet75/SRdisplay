#pragma once

#include <stdint.h>

// LED Farb-Zustände
typedef enum {
    LED_STATE_BOOT        = 0,  // Rot  - Einschalten
    LED_STATE_USB_CONNECTED,    // Grün - USB-CDC verbunden
    LED_STATE_BLE_CONNECTED,    // Blau - Bluetooth verbunden
} led_state_t;

void led_status_init(void);
void led_status_set(led_state_t state);