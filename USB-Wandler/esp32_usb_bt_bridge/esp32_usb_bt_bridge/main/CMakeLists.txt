idf_component_register(
    SRCS
        "main.c"
        "led_status.c"
        "usb_cdc_host.c"
        "ble_spp_server.c"
    INCLUDE_DIRS "."
    REQUIRES
        usb
        bt
        driver
        freertos
        log
        nvs_flash
        esp_event
        esp_driver_rmt       
)