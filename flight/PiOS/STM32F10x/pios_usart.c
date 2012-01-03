/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_USART USART Functions
 * @brief PIOS interface for USART port
 * @{
 *
 * @file       pios_usart.c   
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      USART commands. Inits USARTs, controls USARTs & Interupt handlers. (STM32 dependent)
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

#if defined(PIOS_INCLUDE_USART)

#include <pios_usart_priv.h>

/* Provide a COM driver */
static void PIOS_USART_ChangeBaud(uint32_t usart_id, uint32_t baud);
static void PIOS_USART_RegisterRxCallback(uint32_t usart_id, pios_com_callback rx_in_cb, uint32_t context);
static void PIOS_USART_RegisterTxCallback(uint32_t usart_id, pios_com_callback tx_out_cb, uint32_t context);
static void PIOS_USART_TxStart(uint32_t usart_id, uint16_t tx_bytes_avail);
static void PIOS_USART_RxStart(uint32_t usart_id, uint16_t rx_bytes_avail);

const struct pios_com_driver pios_usart_com_driver = {
	.set_baud   = PIOS_USART_ChangeBaud,
	.tx_start   = PIOS_USART_TxStart,
	.rx_start   = PIOS_USART_RxStart,
	.bind_tx_cb = PIOS_USART_RegisterTxCallback,
	.bind_rx_cb = PIOS_USART_RegisterRxCallback,
};

enum pios_usart_dev_magic {
	PIOS_USART_DEV_MAGIC = 0x11223344,
};

struct pios_usart_dev {
	enum pios_usart_dev_magic     magic;
	const struct pios_usart_cfg * cfg;

	pios_com_callback rx_in_cb;
	uint32_t rx_in_context;
	pios_com_callback tx_out_cb;
	uint32_t tx_out_context;

	uint32_t rx_dropped;
};

static bool PIOS_USART_validate(struct pios_usart_dev * usart_dev)
{
	return (usart_dev->magic == PIOS_USART_DEV_MAGIC);
}

#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_usart_dev * PIOS_USART_alloc(void)
{
	struct pios_usart_dev * usart_dev;

	usart_dev = (struct pios_usart_dev *)pvPortMalloc(sizeof(*usart_dev));
	if (!usart_dev) return(NULL);

	usart_dev->magic = PIOS_USART_DEV_MAGIC;
	return(usart_dev);
}
#else
static struct pios_usart_dev pios_usart_devs[PIOS_USART_MAX_DEVS];
static uint8_t pios_usart_num_devs;
static struct pios_usart_dev * PIOS_USART_alloc(void)
{
	struct pios_usart_dev * usart_dev;

	if (pios_usart_num_devs >= PIOS_USART_MAX_DEVS) {
		return (NULL);
	}

	usart_dev = &pios_usart_devs[pios_usart_num_devs++];
	usart_dev->magic = PIOS_USART_DEV_MAGIC;

	return (usart_dev);
}
#endif

/* Bind Interrupt Handlers
 *
 * Map all valid USART IRQs to the common interrupt handler
 * and provide storage for a 32-bit device id IRQ to map
 * each physical IRQ to a specific registered device instance.
 */
static void PIOS_USART_generic_irq_handler(uint32_t usart_id);

static uint32_t PIOS_USART_1_id;
void USART1_IRQHandler(void) __attribute__ ((alias ("PIOS_USART_1_irq_handler")));
static void PIOS_USART_1_irq_handler (void)
{
	PIOS_USART_generic_irq_handler (PIOS_USART_1_id);
}

static uint32_t PIOS_USART_2_id;
void USART2_IRQHandler(void) __attribute__ ((alias ("PIOS_USART_2_irq_handler")));
static void PIOS_USART_2_irq_handler (void)
{
	PIOS_USART_generic_irq_handler (PIOS_USART_2_id);
}

static uint32_t PIOS_USART_3_id;
void USART3_IRQHandler(void) __attribute__ ((alias ("PIOS_USART_3_irq_handler")));
static void PIOS_USART_3_irq_handler (void)
{
	PIOS_USART_generic_irq_handler (PIOS_USART_3_id);
}

/**
* Initialise a single USART device
*/
int32_t PIOS_USART_Init(uint32_t * usart_id, const struct pios_usart_cfg * cfg)
{
	PIOS_DEBUG_Assert(usart_id);
	PIOS_DEBUG_Assert(cfg);

	struct pios_usart_dev * usart_dev;

	usart_dev = (struct pios_usart_dev *) PIOS_USART_alloc();
	if (!usart_dev) goto out_fail;

	/* Bind the configuration to the device instance */
	usart_dev->cfg = cfg;

	/* Enable the USART Pins Software Remapping */
	if (usart_dev->cfg->remap) {
		GPIO_PinRemapConfig(usart_dev->cfg->remap, ENABLE);
	}

	/* Initialize the USART Rx and Tx pins */
	GPIO_Init(usart_dev->cfg->rx.gpio, &usart_dev->cfg->rx.init);
	GPIO_Init(usart_dev->cfg->tx.gpio, &usart_dev->cfg->tx.init);

	/* Enable USART clock */
	switch ((uint32_t)usart_dev->cfg->regs) {
	case (uint32_t)USART1:
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
		break;
	case (uint32_t)USART2:
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
		break;
	case (uint32_t)USART3:
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
		break;
	}
  
	/* Configure the USART */
	USART_Init(usart_dev->cfg->regs, &usart_dev->cfg->init);
  
	*usart_id = (uint32_t)usart_dev;

	/* Configure USART Interrupts */
	switch ((uint32_t)usart_dev->cfg->regs) {
	case (uint32_t)USART1:
		PIOS_USART_1_id = (uint32_t)usart_dev;
		break;
	case (uint32_t)USART2:
		PIOS_USART_2_id = (uint32_t)usart_dev;
		break;
	case (uint32_t)USART3:
		PIOS_USART_3_id = (uint32_t)usart_dev;
		break;
	}
	NVIC_Init(&usart_dev->cfg->irq.init);
	USART_ITConfig(usart_dev->cfg->regs, USART_IT_RXNE, ENABLE);
	USART_ITConfig(usart_dev->cfg->regs, USART_IT_TXE,  ENABLE);
  
	/* Enable USART */
	USART_Cmd(usart_dev->cfg->regs, ENABLE);

	return(0);

out_fail:
	return(-1);
}

static void PIOS_USART_RxStart(uint32_t usart_id, uint16_t rx_bytes_avail)
{
	struct pios_usart_dev * usart_dev = (struct pios_usart_dev *)usart_id;

	bool valid = PIOS_USART_validate(usart_dev);
	PIOS_Assert(valid);

	USART_ITConfig(usart_dev->cfg->regs, USART_IT_RXNE, ENABLE);
}
static void PIOS_USART_TxStart(uint32_t usart_id, uint16_t tx_bytes_avail)
{
	struct pios_usart_dev * usart_dev = (struct pios_usart_dev *)usart_id;

	bool valid = PIOS_USART_validate(usart_dev);
	PIOS_Assert(valid);

	USART_ITConfig(usart_dev->cfg->regs, USART_IT_TXE, ENABLE);
}

/**
* Changes the baud rate of the USART peripheral without re-initialising.
* \param[in] usart_id USART name (GPS, TELEM, AUX)
* \param[in] baud Requested baud rate
*/
static void PIOS_USART_ChangeBaud(uint32_t usart_id, uint32_t baud)
{
	struct pios_usart_dev * usart_dev = (struct pios_usart_dev *)usart_id;

	bool valid = PIOS_USART_validate(usart_dev);
	PIOS_Assert(valid);

	USART_InitTypeDef USART_InitStructure;

	/* Start with a copy of the default configuration for the peripheral */
	USART_InitStructure = usart_dev->cfg->init;

	/* Adjust the baud rate */
	USART_InitStructure.USART_BaudRate = baud;

	/* Write back the new configuration */
	USART_Init(usart_dev->cfg->regs, &USART_InitStructure);
}

static void PIOS_USART_RegisterRxCallback(uint32_t usart_id, pios_com_callback rx_in_cb, uint32_t context)
{
	struct pios_usart_dev * usart_dev = (struct pios_usart_dev *)usart_id;

	bool valid = PIOS_USART_validate(usart_dev);
	PIOS_Assert(valid);

	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	usart_dev->rx_in_context = context;
	usart_dev->rx_in_cb = rx_in_cb;
}

static void PIOS_USART_RegisterTxCallback(uint32_t usart_id, pios_com_callback tx_out_cb, uint32_t context)
{
	struct pios_usart_dev * usart_dev = (struct pios_usart_dev *)usart_id;

	bool valid = PIOS_USART_validate(usart_dev);
	PIOS_Assert(valid);

	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	usart_dev->tx_out_context = context;
	usart_dev->tx_out_cb = tx_out_cb;
}

static void PIOS_USART_generic_irq_handler(uint32_t usart_id)
{
	struct pios_usart_dev * usart_dev = (struct pios_usart_dev *)usart_id;

	bool valid = PIOS_USART_validate(usart_dev);
	PIOS_Assert(valid);

	/* Force read of dr after sr to make sure to clear error flags */
	volatile uint16_t sr = usart_dev->cfg->regs->SR;
	volatile uint8_t dr = usart_dev->cfg->regs->DR;

	/* Check if RXNE flag is set */
	bool rx_need_yield = false;
	if (sr & USART_SR_RXNE) {
		uint8_t byte = dr;
		if (usart_dev->rx_in_cb) {
			uint16_t rc;
			rc = (usart_dev->rx_in_cb)(usart_dev->rx_in_context, &byte, 1, NULL, &rx_need_yield);
			if (rc < 1) {
				/* Lost bytes on rx */
				usart_dev->rx_dropped += 1;
			}
		}
	}

	/* Check if TXE flag is set */
	bool tx_need_yield = false;
	if (sr & USART_SR_TXE) {
		if (usart_dev->tx_out_cb) {
			uint8_t b;
			uint16_t bytes_to_send;

			bytes_to_send = (usart_dev->tx_out_cb)(usart_dev->tx_out_context, &b, 1, NULL, &tx_need_yield);

			if (bytes_to_send > 0) {
				/* Send the byte we've been given */
				usart_dev->cfg->regs->DR = b;
			} else {
				/* No bytes to send, disable TXE interrupt */
				USART_ITConfig(usart_dev->cfg->regs, USART_IT_TXE, DISABLE);
			}
		} else {
			/* No bytes to send, disable TXE interrupt */
			USART_ITConfig(usart_dev->cfg->regs, USART_IT_TXE, DISABLE);
		}
	}

#if defined(PIOS_INCLUDE_FREERTOS)
	if (rx_need_yield || tx_need_yield) {
		vPortYieldFromISR();
	}
#endif	/* PIOS_INCLUDE_FREERTOS */
}

#endif

/**
  * @}
  * @}
  */
