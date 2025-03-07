/**
 * Copyright (c) 2024 WIZnet Co.,Ltd
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _WIZCHIP_GPIO_IRQ_H_
#define _WIZCHIP_GPIO_IRQ_H_

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
#define PIN_INT 21

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
/*! \brief Initialize wizchip gpio interrupt callback function
 *  \ingroup wizchip_gpio_irq
 *
 *  Add a wizchip interrupt callback.
 *
 *  \param socket Socket number
 *  \param callback The gpio interrupt callback function
 */
void wizchip_gpio_interrupt_init(uint8_t socket, void (*callback)(void));

/*! \brief Assign gpio interrupt callback function
 *  \ingroup wizchip_gpio_irq
 *
 *  GPIO interrupt callback function.
 *
 *  \param gpio Which GPIO caused this interrupt
 *  \param events Which events caused this interrupt. See \ref gpio_set_irq_enabled for details
 */
static void wizchip_gpio_interrupt_callback(uint gpio, uint32_t events);

#ifdef __cplusplus
}
#endif

#endif /* _WIZCHIP_GPIO_IRQ_H_ */
