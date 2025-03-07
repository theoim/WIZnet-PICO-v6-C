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
/* Clock */
#define PLL_SYS_KHZ (133 * 1000)

/* Buffer */
#define ETHERNET_BUF_MAX_SIZE (1024 * 2)

/* Socket */
#define SOCKET_TCP_SERVER 0
#define SOCKET_TCP_CLIENT 1
#define SOCKET_UDP 2
#define SOCKET_TCP_SERVER6 3
#define SOCKET_TCP_CLIENT6 4
#define SOCKET_UDP6 5
#define SOCKET_TCP_SERVER_DUAL 6
#define SOCKET_DHCP 7

/* Port */
#define PORT_TCP_SERVER 5000
#define PORT_TCP_CLIENT 5001
#define PORT_TCP_CLIENT_DEST    5002
#define PORT_UDP 5003

#define PORT_TCP_SERVER6 5004
#define PORT_TCP_CLIENT6 5005
#define PORT_TCP_CLIENT6_DEST 5006
#define PORT_UDP6 5007

#define PORT_TCP_SERVER_DUAL 5008

#define IPV4
#define IPV6

#ifdef IPV4
#define TCP_SERVER
#define TCP_CLIENT
#define UDP
#endif

#ifdef IPV6
#define TCP_SERVER6
#define TCP_CLIENT6
#define UDP6
#endif

#if defined IPV4 && defined IPV6
#define TCP_SERVER_DUAL
#endif

#define RETRY_CNT   10000

static void set_clock_khz(void);

static wiz_NetInfo g_net_info =
    {
        .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56}, // MAC address
        .ip = {192, 168, 11, 20},                     // IP address
        .sn = {255, 255, 255, 0},                    // Subnet Mask
        .gw = {192, 168, 11, 1},                     // Gateway
        .dns = {8, 8, 8, 8},                         // DNS server
        .lla = {0xfe, 0x80, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x02, 0x08, 0xdc, 0xff,
                0xfe, 0x57, 0x57, 0x25},             // Link Local Address
        .gua = {0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00},             // Global Unicast Address
        .sn6 = {0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00},             // IPv6 Prefix
        .gw6 = {0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00},             // Gateway IPv6 Address
        .dns6 = {0x20, 0x01, 0x48, 0x60,
                0x48, 0x60, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x88, 0x88},             // DNS6 server
        .ipmode = NETINFO_STATIC_ALL
};

int main() {

    set_clock_khz();

    stdio_init_all();

    sleep_ms(1000);
    gpio_init(25);
    gpio_set_dir(25, GPIO_OUT);

     printf(" IP          : %d.%d.%d.%d\n", g_net_info.ip[0], g_net_info.ip[1], g_net_info.ip[2], g_net_info.ip[3]);


    while (true) {
        // LED 토글
        gpio_put(25, 1);  // LED ON
        sleep_ms(300);    // 300ms 대기
        gpio_put(25, 0);  // LED OFF
        sleep_ms(300);    // 300ms 대기
    }

    return 0;
}

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