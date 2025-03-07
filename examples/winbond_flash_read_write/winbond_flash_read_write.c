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
#include "w6300.h"

#include "loopback.h"


/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
#define PLL_SYS_KHZ (133 * 1000)
#define CLK_PERI_SPI0 (5 * 1000 * 1000)



/**
 * ----------------------------------------------------------------------------------------------------
 * Variables
 * ----------------------------------------------------------------------------------------------------
 */

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

int main()
{
  uint8_t op_code_flash = 0;
  uint8_t addr_flash = 0x00;

  uint8_t rx_data[2] = {0, };

  set_clock_khz();
  stdio_init_all();
  sleep_ms(1000);

  op_code_flash = 0x90;
  // addr_flash[0] = 0x0;
  // addr_flash[1] = 0x0;
  // addr_flash[2] = 0x0;
  


  printf("\n\n== Start ==\n");
  wizchip_spi_init(0);
  printf("wizchip_spi_init finished...\n");
  wizchip_cris_init();

  printf("wizchip_cris_init finished...\n");

  wizchip_reset();
  printf("wizchip_reset finished...\n");

  wizchip_config();
  printf("wizchip_config finished...\n");

  while (1)
  {   
  // wizchip_pio_spi_frame_start();
  wizchip_pio_spi_read_flash(op_code_flash, addr_flash, rx_data, 2);
  // wizchip_pio_spi_frame_start();
    printf("read > 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",\
              rx_data[0], rx_data[1], rx_data[2], rx_data[3], \
              rx_data[4], rx_data[5], rx_data[6], rx_data[7]);
              
    sleep_ms(1000);
  }
}



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
