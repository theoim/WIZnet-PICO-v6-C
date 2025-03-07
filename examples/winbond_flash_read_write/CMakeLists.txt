set(TARGET_NAME winbond_flash_read_write)

add_executable(${TARGET_NAME}
        ${TARGET_NAME}.c
        )

target_link_libraries(${TARGET_NAME}
        ETHERNET_FILES
        WIZCHIP_FILES
        LOOPBACK_FILES
        PORT_COMMON_FILES
        )

pico_enable_stdio_usb(${TARGET_NAME} 1)
pico_enable_stdio_uart(${TARGET_NAME} 0)

pico_add_extra_outputs(${TARGET_NAME})
