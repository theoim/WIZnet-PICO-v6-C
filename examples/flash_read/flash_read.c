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

#define DATA_SIZE 32768 //32KB

#define TEST_00 0
#define TEST_55 1 
#define TEST_AA 2 
#define TEST_11 3
#define TEST_NONE 4


//Select TEST for TX or RX
#define TX_BUFF_TEST
// #define RX_BUFF_TEST


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


uint8_t data_buff[2048];

int main()
{
  uint8_t write_data_register = 0xff;


  int select_data =  TEST_00;

  if(select_data == TEST_00)  write_data_register = 0x00;

  else if(select_data == TEST_55) write_data_register = 0x55;

  else if(select_data == TEST_AA) write_data_register = 0xaa;

  else if(select_data == TEST_11) write_data_register = 0xff;

  uint8_t WIZCHIP_WRITE_DATA[DATA_SIZE] = {0,};
  uint8_t WIZCHIP_READ_DATA[DATA_SIZE] = {0,};

  int unmatch_flag = 0;
  int unmatch_cnt = 0;
  int match_cnt = 0;

  set_clock_khz();
  stdio_init_all();
  sleep_ms(1000);

  printf("\n\n== Start ==\n");
  wizchip_spi_init(0);
  printf("wizchip_spi_init finished...\n");
  wizchip_cris_init();
  printf("wizchip_cris_init finished...\n");
  wizchip_reset();
  printf("wizchip_reset finished...\n");
  wizchip_config();
  printf("wizchip_config finished...\n");


  uint16_t rx_ptr = getSn_RX_RD(0);
  uint32_t rx_addrsel = 0;

  sleep_ms(35);

  //RX BUFF SET

  setSn_RX_BSR(0, 32);
  sleep_ms(35);
  printf("#%d socket RX buf Size : %dKB\r\n", 0 ,getSn_RXBUF_SIZE(0));

  sleep_ms(35);

  /////////////////
  /*WRITE RX BUFF*/
  /////////////////
  for(int i = 0; i < DATA_SIZE; i++){           
      rx_addrsel = ((uint32_t)rx_ptr << 8) + WIZCHIP_RXBUF_BLOCK(0);
      printf("[%d]write addr : %08X\r\n", i+1, rx_addrsel);
      WIZCHIP_WRITE_DATA[i] = write_data_register;
      WIZCHIP_WRITE(rx_addrsel, WIZCHIP_WRITE_DATA[i]);
      if(write_data_register <= 0x00) write_data_register = 0xff;
      write_data_register--;

      rx_ptr = rx_ptr + 1;
  }

      rx_ptr = getSn_RX_RD(0);

  ////////////////
  /*READ RX BUFF*/
  ////////////////
  for(int i = 0; i < DATA_SIZE; i++){
      rx_addrsel = ((uint32_t)rx_ptr << 8) + WIZCHIP_RXBUF_BLOCK(0);
      WIZCHIP_READ_DATA[i] = WIZCHIP_READ(rx_addrsel);
      
      printf("[%d]read addr : %08X\r\n", i+1, rx_addrsel);
      
      rx_ptr = rx_ptr + 1;
  }


#ifdef TX_BUFF_TEST
  
  sleep_ms(35);
  setSn_TXBUF_SIZE(0, 32);
  sleep_ms(35);
  printf("#%d socket TX buf Size : %dKB\r\n", 0 ,getSn_TXBUF_SIZE(0));
  sleep_ms(35);

  uint16_t tx_ptr = getSn_TX_WR(0);
  uint32_t tx_addrsel = 0;

  /////////////////
  /*WRITE RX BUFF*/
  /////////////////
  for(int i = 0; i < DATA_SIZE; i++){
    tx_addrsel = ((uint32_t)tx_ptr << 8) + WIZCHIP_TXBUF_BLOCK(0);
    // printf("[%d]write addr : %08X\r\n", i+1, tx_addrsel);
    
    WIZCHIP_WRITE_DATA[i] = write_data_register;


    WIZCHIP_WRITE(tx_addrsel, WIZCHIP_WRITE_DATA[i]);

    // if(write_data_register <= 0x00) write_data_register = 0xff;
    // write_data_register--;

    tx_ptr = tx_ptr + 1;
    setSn_TX_WR(0, tx_ptr);
  }
  
  tx_ptr = getSn_TX_WR(0);

  sleep_ms(50);

  /////////////////
  /*READ RX BUFF*/
  /////////////////
  for(int i = 0; i < DATA_SIZE; i++){
    tx_addrsel = ((uint32_t)tx_ptr << 8) + WIZCHIP_TXBUF_BLOCK(0);
    WIZCHIP_READ_DATA[i] = WIZCHIP_READ(tx_addrsel);
    // printf("[%d]read addr : %08X\r\n", i+1, tx_addrsel);
    tx_ptr = tx_ptr + 1;
    setSn_TX_WR(0, tx_ptr);
  }
#endif



  //////////////////////////////
  /*COMPARE R buff with W buff*/
  //////////////////////////////
  for(int i = 0; i < DATA_SIZE; i++){
    if(WIZCHIP_READ_DATA[i] != WIZCHIP_WRITE_DATA[i]){
      unmatch_cnt++;
      unmatch_flag = 1;
      printf("DATA UNMATCH!!\r\n");
      printf("[UNMATCH INDEX : %d]\r\n", i);
      printf("UNMATCH_WRITE_DATA : 0x%02x\r\n", WIZCHIP_WRITE_DATA[i]);
      printf("UNMATCH_READ_DATA : 0x%02x\r\n\r\n", WIZCHIP_READ_DATA[i]);   
    }
    else{
      match_cnt++;
    }
  }


  //print comparing random R&W data 
  if(unmatch_flag != 1){
    printf("ALL MATCH!!!!!!\r\n");
    printf("A NUMBER OF MATCH : %d\r\n", match_cnt);

    printf("8000 WRITE_DATA : 0x%02x\r\n", WIZCHIP_WRITE_DATA[8000]);
    printf("8000 READ_DATA : 0x%02x\r\n", WIZCHIP_READ_DATA[8000]); 

    printf("15000 WRITE_DATA : 0x%02x\r\n", WIZCHIP_WRITE_DATA[15000]);
    printf("15000 READ_DATA : 0x%02x\r\n", WIZCHIP_READ_DATA[15000]); 

    printf("16383 WRITE_DATA : 0x%02x\r\n", WIZCHIP_WRITE_DATA[16383]);
    printf("16383 READ_DATA : 0x%02x\r\n", WIZCHIP_READ_DATA[16383]); 

    printf("32766 WRITE_DATA : 0x%02x\r\n", WIZCHIP_WRITE_DATA[32766]);
    printf("32766 READ_DATA : 0x%02x\r\n", WIZCHIP_READ_DATA[32766]); 

    printf("32767 WRITE_DATA : 0x%02x\r\n", WIZCHIP_WRITE_DATA[32767]);
    printf("32767 READ_DATA : 0x%02x\r\n", WIZCHIP_READ_DATA[32767]);     


    //print overflow index
    printf("32768 WRITE_DATA : 0x%02x\r\n", WIZCHIP_WRITE_DATA[32768]);
    printf("32768 READ_DATA : 0x%02x\r\n", WIZCHIP_READ_DATA[32768]);  

    printf("32769 WRITE_DATA : 0x%02x\r\n", WIZCHIP_WRITE_DATA[32769]);
    printf("32769 READ_DATA : 0x%02x\r\n", WIZCHIP_READ_DATA[32769]); 

    printf("%d WRITE_DATA : 0x%02x\r\n", 32999, WIZCHIP_WRITE_DATA[32999]);
    printf("%d READ_DATA : 0x%02x\r\n", 32999, WIZCHIP_READ_DATA[32999]);   

    sleep_ms(1);
  }
  else{
    printf("OCCUR DATA UNMATCH!!\r\n");
    printf("A NUMBER OF UNMATCH : %d\r\n", unmatch_cnt);
    sleep_ms(1);
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
