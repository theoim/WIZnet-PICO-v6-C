# wizchip
add_library(WIZCHIP_FILES INTERFACE)

pico_generate_pio_header(WIZCHIP_FILES ${PORT_DIR}/wizchip/src/wizchip_pio_spi.pio)

target_sources(WIZCHIP_FILES INTERFACE
        ${PORT_DIR}/wizchip/src/wizchip_gpio_irq.c
        ${PORT_DIR}/wizchip/src/wizchip_pio_spi.c
        ${PORT_DIR}/wizchip/src/wizchip_spi.c
        ${PORT_DIR}/wizchip/src/wizchip_timer.c
        )

target_include_directories(WIZCHIP_FILES INTERFACE
        ${WIZNET_DIR}/Ethernet
        ${PORT_DIR}/wizchip/inc
        )

target_link_libraries(WIZCHIP_FILES INTERFACE
        pico_stdlib
        hardware_dma
        hardware_gpio
        hardware_pio
        hardware_spi
        )

# port_common
add_library(PORT_COMMON_FILES INTERFACE)

target_include_directories(PORT_COMMON_FILES INTERFACE
        ${PORT_DIR}
        )

target_link_libraries(PORT_COMMON_FILES INTERFACE
        pico_stdlib
        hardware_clocks
        )
