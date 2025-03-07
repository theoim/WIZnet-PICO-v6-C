set(TARGET_NAME W6300_Test)

add_executable(${TARGET_NAME}
        ${TARGET_NAME}.c
        )

target_link_libraries(${TARGET_NAME}
        ETHERNET_FILES
        WIZCHIP_FILES
        LOOPBACK_FILES
        PORT_COMMON_FILES
        W6300_FILES
        )

pico_enable_stdio_usb(${TARGET_NAME} 1)
pico_enable_stdio_uart(${TARGET_NAME} 0)

pico_add_extra_outputs(${TARGET_NAME})
