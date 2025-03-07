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

#include "port_common.h"

#include "wizchip_conf.h"
#include "wizchip_spi.h"
#include "wizchip_pio_spi.h"
#include "wizchip_timer.h"

#include "loopback.h"


/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
#define PLL_SYS_KHZ (133 * 1000)
#define CLK_PERI_SPI0 (5 * 1000 * 1000)

#define ETHERNET_BUF_MAX_SIZE (1024 * 2)

#define SOCKET_LOOPBACK 0

#define PORT_LOOPBACK 5000
/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */
wiz_NetInfo gWIZNETINFO = {.mac = {0x00, 0x08, 0xdc, 0x12, 0x34, 0x45},
                           .ip = {192, 168, 11, 20}, //{192, 168, 15, 100},
                           .sn = {255, 255, 255, 0},
                           .gw = {192, 168, 11, 1}, //{192, 168, 15, 1},
                           .dns = {8, 8, 8, 8},
                           .lla = {0xfe, 0x80, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00,
                                   0x02, 0x08, 0xdc, 0xff,
                                   0xfe, 0xff, 0xff, 0xff},
                           .gua = {0x20, 0x01, 0x02, 0xb8,
                                   0x00, 0x10, 0x00, 0x01,
                                   0x02, 0x08, 0xdc, 0xff,
                                   0xfe, 0xff, 0xff, 0xff},
                           .sn6 = {0xff, 0xff, 0xff, 0xff,
                                   0xff, 0xff, 0xff, 0xff,
                                   0x00, 0x00, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00},
                           .gw6 = {0xfe, 0x80, 0x00, 0x00,
                                   0x00, 0x00, 0x00, 0x00,
                                   0x02, 0x00, 0x87, 0xff,
                                   0xfe, 0x08, 0x4c, 0x81}};

static uint8_t g_loopback_buf[ETHERNET_BUF_MAX_SIZE] = {
    0,
};

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
static void set_clock_khz(void);

/**
 * ----------------------------------------------------------------------------------------------------
 * Main
 * ----------------------------------------------------------------------------------------------------
 */
#define TEST
#if 1
uint8_t data_buff[2048];

int main()

{
  uint16_t i = 0;

  set_clock_khz();

  stdio_init_all();
  sleep_ms(3000);

  printf("\n\n== Start ==\n");

#if 1

#ifdef TEST
  gpio_init(2);
  gpio_set_dir(2, GPIO_OUT);
  gpio_put(2, false);
#endif

  wizchip_spi_init(0);
  printf("wizchip_spi_init finished...\n");

#if 0//def TEST
  wizchip_check();

  while(1){}
#endif

  wizchip_cris_init();
  printf("wizchip_cris_init finished...\n");

  wizchip_reset();
  printf("wizchip_reset finished...\n");

  wizchip_config();
  printf("wizchip_config finished...\n");

  wizchip_check();

  network_init(gWIZNETINFO);

 // wizchip_check();

  /* Get network information */
  print_network_information(gWIZNETINFO);

  while(1)
  {
    loopback_tcps(0, data_buff, 5000, AS_IPV4);
  }

#endif
}
#endif

/**
 * ----------------------------------------------------------------------------------------------------
 * Functions
 * ----------------------------------------------------------------------------------------------------
 */
static void set_clock_khz(void)
{
  // set a system clock frequency in khz
  set_sys_clock_khz(PLL_SYS_KHZ, true);

  // configure the specified clock
  clock_configure(
      clk_peri,
      0,                                                // No glitchless mux
      CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, // System PLL on AUX mux
      PLL_SYS_KHZ * 1000,                               // Input frequency
      PLL_SYS_KHZ * 1000                                // Output (must be same as no divider)
  );
}
