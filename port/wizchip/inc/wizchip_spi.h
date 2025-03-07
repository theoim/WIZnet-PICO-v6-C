/**
 * Copyright (c) 2024 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _WIZCHIP_SPI_H_
#define _WIZCHIP_SPI_H_

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * ----------------------------------------------------------------------------------------------------
 * Includes
 * ----------------------------------------------------------------------------------------------------
 */

/**
 * ----------------------------------------------------------------------------------------------------
 * Macros
 * ----------------------------------------------------------------------------------------------------
 */
#define SPI_PORT spi0

#define SPI_SCK_PIN 18
#define SPI_MOSI_PIN 19
#define SPI_MISO_PIN 16
#define SPI_CS_PIN 17
#define RST_PIN 20

//#define USE_SPI_DMA // if you want to use SPI DMA, uncomment.

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
/*! \brief Set CS pin
 *  \ingroup wizchip_spi
 *
 *  Set chip select pin of spi0 to low(active low).
 *
 *  \param none
 */
static inline void wizchip_select(void);

/*! \brief Set CS pin
 *  \ingroup wizchip_spi
 *
 *  Set chip select pin of spi0 to high(inactive high).
 *
 *  \param none
 */
static inline void wizchip_deselect(void);

/*! \brief Read from an SPI device, blocking
 *  \ingroup wizchip_spi
 *
 *  Set spi_read_blocking function.
 *  Read byte from SPI to rx_data buffer.
 *  Blocks until all data is transferred. No timeout, as SPI hardware always transfers at a known data rate.
 *
 *  \param none
 */
static uint8_t wizchip_read(void);

/*! \brief Write to an SPI device, blocking
 *  \ingroup wizchip_spi
 *
 *  Set spi_write_blocking function.
 *  Write byte from tx_data buffer to SPI device.
 *  Blocks until all data is transferred. No timeout, as SPI hardware always transfers at a known data rate.
 *
 *  \param tx_data Buffer of data to write
 */
static void wizchip_write(uint8_t tx_data);

#ifdef USE_SPI_DMA
/*! \brief Configure all DMA parameters and optionally start transfer
 *  \ingroup wizchip_spi
 *
 *  Configure all DMA parameters and read from DMA.
 *
 *  \param pBuf Buffer of data to read
 *  \param len Element count (each element is of size transfer_data_size)
 */
static void wizchip_read_burst(uint8_t *pBuf, uint16_t len);

/*! \brief Configure all DMA parameters and optionally start transfer
 *  \ingroup wizchip_spi
 *
 *  Configure all DMA parameters and write to DMA.
 *
 *  \param pBuf Buffer of data to write
 *  \param len Element count (each element is of size transfer_data_size)
 */
static void wizchip_write_burst(uint8_t *pBuf, uint16_t len);
#endif

/*! \brief Enter a critical section
 *  \ingroup wizchip_spi
 *
 *  Set ciritical section enter blocking function.
 *  If the spin lock associated with this critical section is in use, then this
 *  method will block until it is released.
 *
 *  \param none
 */
static void wizchip_critical_section_lock(void);

/*! \brief Release a critical section
 *  \ingroup wizchip_spi
 *
 *  Set ciritical section exit function.
 *  Release a critical section.
 *
 *  \param none
 */
static void wizchip_critical_section_unlock(void);

/*! \brief Initialize SPI instances and Set DMA channel
 *  \ingroup wizchip_spi
 *
 *  Set GPIO to spi0.
 *  Puts the SPI into a known state, and enable it.
 *  Set DMA channel completion channel.
 *
 *  \param clock SPI clock
 */
void wizchip_spi_init(uint32_t clock);

/*! \brief Initialize a critical section structure
 *  \ingroup wizchip_spi
 *
 *  The critical section is initialized ready for use.
 *  Registers callback function for critical section for chip.
 *
 *  \param none
 */
void wizchip_cris_init(void);

/*! \brief Reset wizchip
 *  \ingroup wizchip_spi
 *
 *  Set a reset pin and reset.
 *
 *  \param none
 */
void wizchip_reset(void);

/*! \brief Configure chip
 *  \ingroup wizchip_spi
 *
 *  Set callback function to read/write byte using SPI.
 *  Set callback function for chip select/deselect.
 *  Set memory size of wizchip and monitor PHY link status.
 *
 *  \param none
 */
void wizchip_config(void);

/*! \brief Check chip version
 *  \ingroup wizchip_spi
 *
 *  Get version information.
 *
 *  \param none
 */
void wizchip_check(void);

/*! \brief Initialize network
 *  \ingroup wizchip_spi
 *
 *  Set network information.
 *
 *  \param net_info Network information
 */
void network_init(wiz_NetInfo net_info);

/*! \brief Print network information
 *  \ingroup wizchip_spi
 *
 *  Print network information about MAC address, IP address, Subnet mask, Gateway, DHCP and DNS address.
 *
 *  \param net_info Network information
 */
void print_network_information(wiz_NetInfo net_info);

#ifdef __cplusplus
}
#endif

#endif /* _WIZCHIP_SPI_H_ */
