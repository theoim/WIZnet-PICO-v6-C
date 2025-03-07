/**
 * Copyright (c) 2024 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */
#include <stdio.h>

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "pico/critical_section.h"
#include "hardware/dma.h"
#include "hardware/spi.h"

#include "wizchip_conf.h"
#include "wizchip_spi.h"
#include "wizchip_pio_spi.h"
/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */

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
  //gpio_put(SPI_CS_PIN, 0);
  wizchip_pio_spi_frame_start();
}

static inline void wizchip_deselect(void)
{
  //gpio_put(SPI_CS_PIN, 1);
  wizchip_pio_spi_frame_end();
}

void wizchip_reset()
{
  wizchip_pio_spi_reset();
}

static uint8_t wizchip_read(void)
{
  uint8_t rx_data = 0;
  uint8_t tx_data = 0xFF;

  spi_read_blocking(SPI_PORT, tx_data, &rx_data, 1);

  return rx_data;
}

static void wizchip_write(uint8_t tx_data)
{
  spi_write_blocking(SPI_PORT, &tx_data, 1);
}

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

void wizchip_spi_init(uint32_t clock)
{
  //ToDo : clock
  wizchip_pio_spi_init(NULL);
}

void wizchip_cris_init(void)
{
  critical_section_init(&g_wizchip_cri_sec);
  reg_wizchip_cris_cbfunc(wizchip_critical_section_lock, wizchip_critical_section_unlock);
}

void wizchip_config(void)
{
  /* Deselect the FLASH : chip select high */
  wizchip_deselect();

#if _WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SPI_QSPI_
  reg_wizchip_cs_cbfunc(wizchip_pio_spi_frame_start, wizchip_pio_spi_frame_end);
  reg_wizchip_qspi_cbfunc(wizchip_pio_spi_read_byte, wizchip_pio_spi_write_byte);
#elif _WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SPI_
  /* CS function register */
  reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);

  /* SPI function register */
  reg_wizchip_spi_cbfunc(wizchip_read, wizchip_write);
#ifdef USE_SPI_DMA
  reg_wizchip_spiburst_cbfunc(wizchip_read_burst, wizchip_write_burst);
#endif
#endif
  /* Initialize wizchip  */
  uint8_t temp;
#if (_WIZCHIP_ == W5100S)
  uint8_t memsize[2][4] = {{2, 2, 2, 2}, {2, 2, 2, 2}};
#elif (_WIZCHIP_ == W5500)
  uint8_t memsize[2][8] = {{2, 2, 2, 2, 2, 2, 2, 2}, {2, 2, 2, 2, 2, 2, 2, 2}};
#elif (_WIZCHIP_ == W6300)
uint8_t memsize[2][8] = {{8, 8, 0, 0, 0, 0, 0, 0}, {8, 8, 0, 0, 0, 0, 0, 0}};
#endif

  if (ctlwizchip(CW_INIT_WIZCHIP, (void *)memsize) == -1)
  {
    printf(" Wizchip initialized fail\n");
    while(1){}

    return;
  }
  printf(" Wizchip initialized success\n");

#if 0
  /* Check PHY link status */
  do
  {
    if (ctlwizchip(CW_GET_PHYLINK, (void *)&temp) == -1)
    {
      printf(" Unknown PHY link status\n");

      return;
    }
  } while (temp == PHY_LINK_OFF);
  printf("PHY link on\n");
#endif
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
#elif (_WIZCHIP_ == W6300)
  printf(" ACCESS read value = 0x%02x\n", getCIDR());
  printf("VERSION(%04x) = %04x \r\n", _VER_, getVER());
  printf("System Status Register : %04x\r\n",getSYSR());
  printf("phy status regster : %04x\r\n",getPHYSR());
  setPHYRAR(0x05);
  printf("phy addr regster : %04x\r\n",getPHYRAR());
  setPHYRAR(0x1A);
  printf("phy status regster : %04x\r\n",getPHYRAR());

  for (uint8_t i = 0; i < 8; i++)
  {
    printf("%d : max size = %d k \r\n", i, getSn_TxMAX(i));
  }
#endif
}

void network_init(wiz_NetInfo net_info)
{
  uint8_t syslock = SYS_NET_LOCK;
  printf(" START IP          : %d.%d.%d.%d\n", net_info.ip[0], net_info.ip[1], net_info.ip[2], net_info.ip[3]);
  sleep_ms(10);
  ctlwizchip(CW_SYS_UNLOCK, &syslock);
  sleep_ms(20);
  printf(" Middle IP          : %d.%d.%d.%d\n", net_info.ip[0], net_info.ip[1], net_info.ip[2], net_info.ip[3]);
  ctlnetwork(CN_SET_NETINFO, (void *)&net_info);
  sleep_ms(30);
  printf(" END IP          : %d.%d.%d.%d\n", net_info.ip[0], net_info.ip[1], net_info.ip[2], net_info.ip[3]);
}

void print_network_information(wiz_NetInfo net_info)
{
  uint8_t tmp_str[8] = {
      0,
  };

  ctlnetwork(CN_GET_NETINFO, (void *)&net_info);
  ctlwizchip(CW_GET_ID, (void *)tmp_str);

  //if (net_info.dhcp == NETINFO_DHCP_ALL)
  //{
  //  printf("====================================================================================================\n");
  //  printf(" %s network configuration : DHCP\n\n", (char *)tmp_str);
 // } 
  //else
  //{
    printf("====================================================================================================\n");
    printf(" %s network configuration : static\n\n", (char *)tmp_str);
  //}

  printf(" MAC         : %02X:%02X:%02X:%02X:%02X:%02X\n", net_info.mac[0], net_info.mac[1], net_info.mac[2], net_info.mac[3], net_info.mac[4], net_info.mac[5]);
  printf(" IP          : %d.%d.%d.%d\n", net_info.ip[0], net_info.ip[1], net_info.ip[2], net_info.ip[3]);
  printf(" Subnet Mask : %d.%d.%d.%d\n", net_info.sn[0], net_info.sn[1], net_info.sn[2], net_info.sn[3]);
  printf(" Gateway     : %d.%d.%d.%d\n", net_info.gw[0], net_info.gw[1], net_info.gw[2], net_info.gw[3]);
  printf(" DNS         : %d.%d.%d.%d\n", net_info.dns[0], net_info.dns[1], net_info.dns[2], net_info.dns[3]);
  printf("====================================================================================================\n\n");
}
