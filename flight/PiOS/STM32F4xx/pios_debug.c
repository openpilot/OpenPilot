/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @defgroup   PIOS_DEBUG Debugging Functions
 * @brief Debugging functionality
 * @{
 *
 * @file       pios_debug.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Debugging Functions
 * @see        The GNU Public License (GPL) Version 3
 * 
 *****************************************************************************/
/* 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 3 of the License, or 
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License along 
 * with this program; if not, write to the Free Software Foundation, Inc., 
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* Project Includes */
#include "pios.h"

// Global variables
const char *PIOS_DEBUG_AssertMsg = "ASSERT FAILED";

#ifdef PIOS_ENABLE_DEBUG_PINS
static const struct pios_tim_channel * debug_channels;
static uint8_t debug_num_channels;
#endif	/* PIOS_ENABLE_DEBUG_PINS */

/**
* Initialise Debug-features
*/
void PIOS_DEBUG_Init(const struct pios_tim_channel * channels, uint8_t num_channels)
{
#ifdef PIOS_ENABLE_DEBUG_PINS
	PIOS_Assert(channels);
	PIOS_Assert(num_channels);

	/* Store away the GPIOs we've been given */
	debug_channels = channels;
	debug_num_channels = num_channels;

	/* Configure the GPIOs we've been given */
	for (uint8_t i = 0; i < num_channels; i++) {
		const struct pios_tim_channel * chan = &channels[i];

		// Initialise pins as standard output pins
		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_StructInit(&GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Pin = chan->init->GPIO_Pin;

		/* Initialize the GPIO */
		GPIO_Init(chan->init->port, &GPIO_InitStructure);

		/* Set the pin low */
		GPIO_WriteBit(chan->init->port, chan->init->GPIO_Pin, Bit_RESET);
	}
#endif // PIOS_ENABLE_DEBUG_PINS
}

/**
* Set debug-pin high
* \param pin 0 for S1 output
*/
void PIOS_DEBUG_PinHigh(uint8_t pin)
{
#ifdef PIOS_ENABLE_DEBUG_PINS
	if (!debug_channels || pin >= debug_num_channels) {
		return;
	}

	const struct pios_tim_channel * chan = &debug_channels[pin];

	GPIO_WriteBit(chan->init->port, chan->init->GPIO_Pin, Bit_Set);

#endif // PIOS_ENABLE_DEBUG_PINS
}

/**
* Set debug-pin low
* \param pin 0 for S1 output
*/
void PIOS_DEBUG_PinLow(uint8_t pin)
{
#ifdef PIOS_ENABLE_DEBUG_PINS
	if (!debug_channels || pin >= debug_num_channels) {
		return;
	}

	const struct pios_tim_channel * chan = &debug_channels[pin];

	GPIO_WriteBit(chan->init->port, chan->init->GPIO_Pin, Bit_RESET);

#endif // PIOS_ENABLE_DEBUG_PINS
}


void PIOS_DEBUG_PinValue8Bit(uint8_t value)
{
#ifdef PIOS_ENABLE_DEBUG_PINS
	if (!debug_channels) {
		return;
	}

	uint32_t bsrr_l = ( ((~value)&0x0F)<<(16+6)   ) | ((value & 0x0F)<<6);
	uint32_t bsrr_h = ( ((~value)&0xF0)<<(16+6-4) ) | ((value & 0xF0)<<(6-4));

	PIOS_IRQ_Disable();

	/* 
	 * This is sketchy since it assumes a particular ordering
	 * and bitwise layout of the channels provided to the debug code.
	 */
	debug_channels[0].init.port->BSRR = bsrr_l;
	debug_channels[4].init.port->BSRR = bsrr_h;

	PIOS_IRQ_Enable();
#endif // PIOS_ENABLE_DEBUG_PINS
}

void PIOS_DEBUG_PinValue4BitL(uint8_t value)
{
#ifdef PIOS_ENABLE_DEBUG_PINS
	if (!debug_channels) {
		return;
	}

	/* 
	 * This is sketchy since it assumes a particular ordering
	 * and bitwise layout of the channels provided to the debug code.
	 */
	uint32_t bsrr_l = ((~(value & 0x0F)<<(16+6))) | ((value & 0x0F)<<6);
	debug_channels[0].init.port->BSRR = bsrr_l;
#endif // PIOS_ENABLE_DEBUG_PINS
}


/**
 * Report a serious error and halt
 */
void PIOS_DEBUG_Panic(const char *msg)
{
#ifdef PIOS_COM_DEBUG
	register int *lr asm("lr");	// Link-register holds the PC of the caller
	PIOS_COM_SendFormattedStringNonBlocking(PIOS_COM_DEBUG, "\r%s @0x%x\r", msg, lr);
#endif

	// Stay put
	while (1) ;
}

/**
  * @}
  * @}
  */
