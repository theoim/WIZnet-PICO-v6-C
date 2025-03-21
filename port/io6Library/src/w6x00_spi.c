/**
 * Copyright (c) 2022 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include <stdio.h>

#include "port_common.h"

#include "wizchip_conf.h"
#include "w6x00_spi.h"

#if (_WIZCHIP_ == W6300)
#include "wizchip_pio_spi.h"
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/critical_section.h"
#include "hardware/dma.h"
#endif

/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
static critical_section_t g_wizchip_cri_sec;

#ifdef USE_SPI_DMA
static uint dma_tx;
static uint dma_rx;
static dma_channel_config dma_channel_config_tx;
static dma_channel_config dma_channel_config_rx;
#endif

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
static inline void wizchip_select(void)
{
#ifdef USE_SPI_PIO
  wizchip_pio_spi_frame_start();
#else
    gpio_put(PIN_CS, 0);
#endif

}

static inline void wizchip_deselect(void)
{
#ifdef USE_SPI_PIO
  wizchip_pio_spi_frame_end();
#else
    gpio_put(PIN_CS, 1);
#endif

#ifdef USE_SPI_PIO

#else

#endif

}

void wizchip_reset()
{
#ifdef USE_SPI_PIO
    wizchip_pio_spi_reset();
#else
    gpio_init(PIN_RST);
    gpio_set_dir(PIN_RST, GPIO_OUT);

    gpio_put(PIN_RST, 0);
    sleep_ms(100);

    gpio_put(PIN_RST, 1);
    sleep_ms(100);

    bi_decl(bi_1pin_with_name(PIN_RST, "W6x00 RESET"));
#endif

}

#ifndef USE_SPI_PIO
static uint8_t wizchip_read(void)
{
    uint8_t rx_data = 0;
    uint8_t tx_data = 0xFF;

    spi_read_blocking(SPI_PORT, tx_data, &rx_data, 1);

    return rx_data;
}

static void wizchip_read_buf(uint8_t* rx_data, datasize_t len)
{
    uint8_t tx_data = 0xFF;

    spi_read_blocking(SPI_PORT, tx_data, rx_data, len);
}

static void wizchip_write(uint8_t tx_data)
{
    spi_write_blocking(SPI_PORT, &tx_data, 1);
}

static void wizchip_write_buf(uint8_t* tx_data, datasize_t len)
{
    spi_write_blocking(SPI_PORT, tx_data, len);
}
#endif

#ifdef USE_SPI_DMA
static void wizchip_read_burst(uint8_t *pBuf, uint16_t len)
{
    uint8_t dummy_data = 0xFF;

    channel_config_set_read_increment(&dma_channel_config_tx, false);
    channel_config_set_write_increment(&dma_channel_config_tx, false);
    dma_channel_configure(dma_tx, &dma_channel_config_tx,
                          &spi_get_hw(SPI_PORT)->dr, // write address
                          &dummy_data,               // read address
                          len,                       // element count (each element is of size transfer_data_size)
                          false);                    // don't start yet

    channel_config_set_read_increment(&dma_channel_config_rx, false);
    channel_config_set_write_increment(&dma_channel_config_rx, true);
    dma_channel_configure(dma_rx, &dma_channel_config_rx,
                          pBuf,                      // write address
                          &spi_get_hw(SPI_PORT)->dr, // read address
                          len,                       // element count (each element is of size transfer_data_size)
                          false);                    // don't start yet

    dma_start_channel_mask((1u << dma_tx) | (1u << dma_rx));
    dma_channel_wait_for_finish_blocking(dma_rx);
}

static void wizchip_write_burst(uint8_t *pBuf, uint16_t len)
{
    uint8_t dummy_data;

    channel_config_set_read_increment(&dma_channel_config_tx, true);
    channel_config_set_write_increment(&dma_channel_config_tx, false);
    dma_channel_configure(dma_tx, &dma_channel_config_tx,
                          &spi_get_hw(SPI_PORT)->dr, // write address
                          pBuf,                      // read address
                          len,                       // element count (each element is of size transfer_data_size)
                          false);                    // don't start yet

    channel_config_set_read_increment(&dma_channel_config_rx, false);
    channel_config_set_write_increment(&dma_channel_config_rx, false);
    dma_channel_configure(dma_rx, &dma_channel_config_rx,
                          &dummy_data,               // write address
                          &spi_get_hw(SPI_PORT)->dr, // read address
                          len,                       // element count (each element is of size transfer_data_size)
                          false);                    // don't start yet

    dma_start_channel_mask((1u << dma_tx) | (1u << dma_rx));
    dma_channel_wait_for_finish_blocking(dma_rx);
}
#endif

static void wizchip_critical_section_lock(void)
{
    critical_section_enter_blocking(&g_wizchip_cri_sec);
}

static void wizchip_critical_section_unlock(void)
{
    critical_section_exit(&g_wizchip_cri_sec);
}

void wizchip_spi_initialize(void)
{
#ifdef USE_SPI_PIO
    //ToDo : clock
    wizchip_pio_spi_init(NULL);
#else
    // this example will use SPI0 at 5MHz
    spi_init(SPI_PORT, 5000 * 1000);

    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);

    // make the SPI pins available to picotool
    bi_decl(bi_3pins_with_func(PIN_MISO, PIN_MOSI, PIN_SCK, GPIO_FUNC_SPI));

    // chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    // make the SPI pins available to picotool
    bi_decl(bi_1pin_with_name(PIN_CS, "W6x00 CHIP SELECT"));

    #ifdef USE_SPI_DMA
        dma_tx = dma_claim_unused_channel(true);
        dma_rx = dma_claim_unused_channel(true);

        dma_channel_config_tx = dma_channel_get_default_config(dma_tx);
        channel_config_set_transfer_data_size(&dma_channel_config_tx, DMA_SIZE_8);
        channel_config_set_dreq(&dma_channel_config_tx, DREQ_SPI0_TX);

        // We set the inbound DMA to transfer from the SPI receive FIFO to a memory buffer paced by the SPI RX FIFO DREQ
        // We coinfigure the read address to remain unchanged for each element, but the write
        // address to increment (so data is written throughout the buffer)
        dma_channel_config_rx = dma_channel_get_default_config(dma_rx);
        channel_config_set_transfer_data_size(&dma_channel_config_rx, DMA_SIZE_8);
        channel_config_set_dreq(&dma_channel_config_rx, DREQ_SPI0_RX);
        channel_config_set_read_increment(&dma_channel_config_rx, false);
        channel_config_set_write_increment(&dma_channel_config_rx, true);
    #endif
#endif

}

void wizchip_cris_initialize(void)
{
    critical_section_init(&g_wizchip_cri_sec);
    reg_wizchip_cris_cbfunc(wizchip_critical_section_lock, wizchip_critical_section_unlock);
}

void wizchip_initialize(void)
{
    /* Deselect the FLASH : chip select high */
    wizchip_deselect();

#ifdef USE_SPI_PIO
    reg_wizchip_cs_cbfunc(wizchip_pio_spi_frame_start, wizchip_pio_spi_frame_end);
    reg_wizchip_qspi_cbfunc(wizchip_pio_spi_read_byte, wizchip_pio_spi_write_byte);
#else
    /* CS function register */
    reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);

    /* SPI function register */
    reg_wizchip_spi_cbfunc(wizchip_read, wizchip_write, wizchip_read_buf, wizchip_write_buf);
    #ifdef USE_SPI_DMA
        reg_wizchip_spiburst_cbfunc(wizchip_read_burst, wizchip_write_burst);
    #endif
#endif


    /* W6x00 initialize */
    uint8_t temp;
#if (_WIZCHIP_ == W5100S)
    uint8_t memsize[2][4] = {{2, 2, 2, 2}, {2, 2, 2, 2}};
#elif (_WIZCHIP_ == W5500)
    uint8_t memsize[2][8] = {{2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 2, 2, 2, 2, 2}};
#elif (_WIZCHIP_ == W6100)
    uint8_t memsize[2][8] = {{2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 2, 2, 2, 2, 2}};
#elif (_WIZCHIP_ == W6300)
uint8_t memsize[2][8] = {{16, 0, 0, 0, 0, 0, 0, 0}, {16, 0, 0, 0, 0, 0, 0, 0}};
#endif

    if (ctlwizchip(CW_INIT_WIZCHIP, (void *)memsize) == -1)
    {
        printf(" W6x00 initialized fail\n");

        return;
    }

    /* Check PHY link status */
    do
    {
        if (ctlwizchip(CW_GET_PHYLINK, (void *)&temp) == -1)
        {
            printf(" Unknown PHY link status\n");

            return;
        }
    } while (temp == PHY_LINK_OFF);
}

void wizchip_check(void)
{
#if (_WIZCHIP_ == W5100S)
    /* Read version register */
    if (getVER() != 0x51)
    {
        printf(" ACCESS ERR : VERSION != 0x51, read value = 0x%02x\n", getVER());

        while (1)
            ;
    }
#elif (_WIZCHIP_ == W5500)
    /* Read version register */
    if (getVERSIONR() != 0x04)
    {
        printf(" ACCESS ERR : VERSION != 0x04, read value = 0x%02x\n", getVERSIONR());

        while (1)
            ;
    }
#elif (_WIZCHIP_ == W6100)
    /* Read version register */
    if (getCIDR() != 0x6100)
    {
        printf(" ACCESS ERR : VERSION != 0x6100, read value = 0x%02x\n", getCIDR());

        while (1)
            ;
    }
#elif (_WIZCHIP_ == W6300)
  printf(" ACCESS read value = 0x%02x\n", getCIDR());
  printf("VERSION(%04x) = %04x \r\n", _VER_, getVER());
  // printf("System Status Register : %04x\r\n",getSYSR());
  // printf("phy status regster : %04x\r\n",getPHYSR());
  setPHYRAR(0x05);
  // printf("phy addr regster : %04x\r\n",getPHYRAR());
  setPHYRAR(0x1A);
  // printf("phy status regster : %04x\r\n",getPHYRAR());

  // for (uint8_t i = 0; i < 8; i++)
  // {
  //   printf("%d : max size = %d k \r\n", i, getSn_TxMAX(i));
  // }
#endif
}

/* Network */
void network_initialize(wiz_NetInfo net_info)
{
    uint8_t syslock = SYS_NET_LOCK;
    ctlwizchip(CW_SYS_UNLOCK, &syslock);
    ctlnetwork(CN_SET_NETINFO, (void *)&net_info);
}

void print_network_information(wiz_NetInfo net_info)
{
    uint8_t tmp_str[8] = {
        0,
    };

    ctlnetwork(CN_GET_NETINFO, (void *)&net_info);
    ctlwizchip(CW_GET_ID, (void *)tmp_str);

    printf("==========================================================\n");
    printf(" %s network configuration\n\n", (char *)tmp_str);

    printf(" MAC         : %02X:%02X:%02X:%02X:%02X:%02X\n", net_info.mac[0], net_info.mac[1], net_info.mac[2], net_info.mac[3], net_info.mac[4], net_info.mac[5]);
    printf(" IP          : %d.%d.%d.%d\n", net_info.ip[0], net_info.ip[1], net_info.ip[2], net_info.ip[3]);
    printf(" Subnet Mask : %d.%d.%d.%d\n", net_info.sn[0], net_info.sn[1], net_info.sn[2], net_info.sn[3]);
    printf(" Gateway     : %d.%d.%d.%d\n", net_info.gw[0], net_info.gw[1], net_info.gw[2], net_info.gw[3]);
    printf(" DNS         : %d.%d.%d.%d\n", net_info.dns[0], net_info.dns[1], net_info.dns[2], net_info.dns[3]);
    print_ipv6_addr(" GW6 ", net_info.gw6);
	print_ipv6_addr(" LLA ", net_info.lla);
	print_ipv6_addr(" GUA ", net_info.gua);
	print_ipv6_addr(" SUB6", net_info.sn6);
    print_ipv6_addr(" DNS6", net_info.dns6);
    printf("==========================================================\n\n");
}

void print_ipv6_addr(uint8_t* name, uint8_t* ip6addr)
{
	printf("%s        : ", name);
	printf("%04X:%04X", ((uint16_t)ip6addr[0] << 8) | ((uint16_t)ip6addr[1]), ((uint16_t)ip6addr[2] << 8) | ((uint16_t)ip6addr[3]));
	printf(":%04X:%04X", ((uint16_t)ip6addr[4] << 8) | ((uint16_t)ip6addr[5]), ((uint16_t)ip6addr[6] << 8) | ((uint16_t)ip6addr[7]));
	printf(":%04X:%04X", ((uint16_t)ip6addr[8] << 8) | ((uint16_t)ip6addr[9]), ((uint16_t)ip6addr[10] << 8) | ((uint16_t)ip6addr[11]));
	printf(":%04X:%04X\r\n", ((uint16_t)ip6addr[12] << 8) | ((uint16_t)ip6addr[13]), ((uint16_t)ip6addr[14] << 8) | ((uint16_t)ip6addr[15]));
}