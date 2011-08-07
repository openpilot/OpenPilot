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
 * 	        Parts by Thorsten Klose (tk@midibox.org) (tk@midibox.org)
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

/*
 * @todo This is virtually identical to the F1xx driver and should be merged.
 */

/* Project Includes */
#include "pios.h"

#if defined(PIOS_INCLUDE_USART)

#include <pios_usart_priv.h>

/* Provide a COM driver */
static void PIOS_USART_ChangeBaud(uint32_t usart_id, uint32_t baud);
static int32_t PIOS_USART_TxBufferPutMoreNonBlocking(uint32_t usart_id, const uint8_t *buffer, uint16_t len);
static int32_t PIOS_USART_TxBufferPutMore(uint32_t usart_id, const uint8_t *buffer, uint16_t len);
static int32_t PIOS_USART_RxBufferGet(uint32_t usart_id);
static int32_t PIOS_USART_RxBufferUsed(uint32_t usart_id);

const struct pios_com_driver pios_usart_com_driver = {
	.set_baud = PIOS_USART_ChangeBaud,
	.tx_nb    = PIOS_USART_TxBufferPutMoreNonBlocking,
	.tx       = PIOS_USART_TxBufferPutMore,
	.rx       = PIOS_USART_RxBufferGet,
	.rx_avail = PIOS_USART_RxBufferUsed,
};

enum pios_usart_dev_magic {
	PIOS_USART_DEV_MAGIC = 0x11223344,
};

struct pios_usart_dev {
	enum pios_usart_dev_magic     magic;
	const struct pios_usart_cfg * cfg;

	// align to 32-bit to try and provide speed improvement;
	uint8_t rx_buffer[PIOS_USART_RX_BUFFER_SIZE] __attribute__ ((aligned(4)));
	t_fifo_buffer rx;

	// align to 32-bit to try and provide speed improvement;
        uint8_t tx_buffer[PIOS_USART_TX_BUFFER_SIZE] __attribute__ ((aligned(4)));
	t_fifo_buffer tx;
};

static bool PIOS_USART_validate(struct pios_usart_dev * usart_dev)
{
	return (usart_dev->magic == PIOS_USART_DEV_MAGIC);
}

#if defined(PIOS_INCLUDE_FREERTOS) && 0
static struct pios_usart_dev * PIOS_USART_alloc(void)
{
	struct pios_usart_dev * usart_dev;

	usart_dev = (struct pios_usart_dev *)malloc(sizeof(*usart_dev));
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

#define USART_HANDLER(_n)									\
	static uint32_t PIOS_USART_ ## _n ## _id;				\
	void USART ## _n ## _IRQHandler(void) __attribute__ ((alias ("PIOS_USART_" #_n "_irq_handler"))); \
	static void PIOS_USART_ ## _n ## _irq_handler (void)	\
	{														\
		PIOS_USART_generic_irq_handler (PIOS_USART_ ## _n ## _id); \
	}														\
	struct hack

USART_HANDLER(1);
USART_HANDLER(2);
USART_HANDLER(3);
USART_HANDLER(4);
USART_HANDLER(5);
USART_HANDLER(6);

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

	/* Clear buffer counters */
	fifoBuf_init(&usart_dev->rx, usart_dev->rx_buffer, sizeof(usart_dev->rx_buffer));
	fifoBuf_init(&usart_dev->tx, usart_dev->tx_buffer, sizeof(usart_dev->tx_buffer));

	/* Map pins to USART function */
	/* note __builtin_ctz() due to the difference between GPIO_PinX and GPIO_PinSourceX */
	if (usart_dev->cfg->remap) {
		GPIO_PinAFConfig(usart_dev->cfg->rx.gpio,
				__builtin_ctz(usart_dev->cfg->rx.init.GPIO_Pin),
				usart_dev->cfg->remap);
		GPIO_PinAFConfig(usart_dev->cfg->tx.gpio,
				__builtin_ctz(usart_dev->cfg->tx.init.GPIO_Pin),
				usart_dev->cfg->remap);
	}

	/* Initialize the USART Rx and Tx pins */
	GPIO_Init(usart_dev->cfg->rx.gpio, (GPIO_InitTypeDef *)&usart_dev->cfg->rx.init);
	GPIO_Init(usart_dev->cfg->tx.gpio, (GPIO_InitTypeDef *)&usart_dev->cfg->tx.init);

	/* Configure the USART */
	USART_Init(usart_dev->cfg->regs, (USART_InitTypeDef *)&usart_dev->cfg->init);

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
	case (uint32_t)UART4:
		PIOS_USART_4_id = (uint32_t)usart_dev;
		break;
	case (uint32_t)UART5:
		PIOS_USART_5_id = (uint32_t)usart_dev;
		break;
	case (uint32_t)USART6:
		PIOS_USART_6_id = (uint32_t)usart_dev;
		break;
	}
	NVIC_Init((NVIC_InitTypeDef *)&(usart_dev->cfg->irq.init));
	USART_ITConfig(usart_dev->cfg->regs, USART_IT_RXNE, ENABLE);
	USART_ITConfig(usart_dev->cfg->regs, USART_IT_TXE,  ENABLE);

	// FIXME XXX Clear / reset uart here - sends NUL char else

	/* Enable USART */
	USART_Cmd(usart_dev->cfg->regs, ENABLE);

	return(0);

out_fail:
	return(-1);
}

const struct pios_usart_cfg * PIOS_USART_GetConfig(uint32_t usart_id)
{
	struct pios_usart_dev * usart_dev = (struct pios_usart_dev *)usart_id;

	bool valid = PIOS_USART_validate(usart_dev);

	if (!valid) {
		return (NULL);
	}

	return usart_dev->cfg;
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

/**
* Returns number of used bytes in receive buffer
* \param[in] USART USART name
* \return > 0: number of used bytes
* \return 0 if USART not available
*/
static int32_t PIOS_USART_RxBufferUsed(uint32_t usart_id)
{
	struct pios_usart_dev * usart_dev = (struct pios_usart_dev *)usart_id;

	bool valid = PIOS_USART_validate(usart_dev);
	PIOS_Assert(valid)

	return (fifoBuf_getUsed(&usart_dev->rx));
}

/**
* Gets a byte from the receive buffer
* \param[in] USART USART name
* \return -1 if no new byte available
* \return >= 0: actual byte received
*/
static int32_t PIOS_USART_RxBufferGet(uint32_t usart_id)
{
	struct pios_usart_dev * usart_dev = (struct pios_usart_dev *)usart_id;

	bool valid = PIOS_USART_validate(usart_dev);
	PIOS_Assert(valid)

	if (fifoBuf_getUsed(&usart_dev->rx) == 0) {
		/* Nothing new in the buffer */
		return -1;
	}

	/* get byte - this operation should be atomic! */
	uint8_t b = fifoBuf_getByte(&usart_dev->rx);

	/* Return received byte */
	return b;
}

/**
* puts a byte onto the receive buffer
* \param[in] USART USART name
* \param[in] b byte which should be put into Rx buffer
* \return 0 if no error
* \return -1 if buffer full (retry)
*/
static int32_t PIOS_USART_RxBufferPut(uint32_t usart_id, uint8_t b)
{
	struct pios_usart_dev * usart_dev = (struct pios_usart_dev *)usart_id;

	bool valid = PIOS_USART_validate(usart_dev);
	PIOS_Assert(valid)

	if (fifoBuf_getFree(&usart_dev->rx) < 1) {
		/* Buffer full (retry) */
		return -1;
	}

	/* Copy received byte into receive buffer */
	/* This operation should be atomic! */
	fifoBuf_putByte(&usart_dev->rx, b);

	/* No error */
	return 0;
}

/**
* returns number of used bytes in transmit buffer
* \param[in] USART USART name
* \return number of used bytes
* \return 0 if USART not available
*/
static int32_t PIOS_USART_TxBufferUsed(uint32_t usart_id)
{
	struct pios_usart_dev * usart_dev = (struct pios_usart_dev *)usart_id;

	bool valid = PIOS_USART_validate(usart_dev);
	PIOS_Assert(valid)

	return (fifoBuf_getUsed(&usart_dev->tx));
}

/**
* gets a byte from the transmit buffer
* \param[in] USART USART name
* \return -1 if no new byte available
* \return >= 0: transmitted byte
*/
static int32_t PIOS_USART_TxBufferGet(uint32_t usart_id)
{
	struct pios_usart_dev * usart_dev = (struct pios_usart_dev *)usart_id;

	bool valid = PIOS_USART_validate(usart_dev);
	PIOS_Assert(valid)

	if (fifoBuf_getUsed(&usart_dev->tx) == 0) {
		/* Nothing new in the buffer */
		return -1;
	}

	/* get byte - this operation should be atomic! */
	PIOS_IRQ_Disable();
	uint8_t b = fifoBuf_getByte(&usart_dev->tx);
	PIOS_IRQ_Enable();
  
	/* Return received byte */
	return b;
}

/**
* puts more than one byte onto the transmit buffer (used for atomic sends)
* \param[in] USART USART name
* \param[in] *buffer pointer to buffer to be sent
* \param[in] len number of bytes to be sent
* \return 0 if no error
* \return -1 if buffer full or cannot get all requested bytes (retry)
*/
static int32_t PIOS_USART_TxBufferPutMoreNonBlocking(uint32_t usart_id, const uint8_t *buffer, uint16_t len)
{
	struct pios_usart_dev * usart_dev = (struct pios_usart_dev *)usart_id;

	bool valid = PIOS_USART_validate(usart_dev);
	PIOS_Assert(valid)

	if (len >= fifoBuf_getFree(&usart_dev->tx)) {
		/* Buffer cannot accept all requested bytes (retry) */
		return -1;
	}

	/* Copy bytes to be transmitted into transmit buffer */
	/* This operation should be atomic! */
	PIOS_IRQ_Disable();

	uint16_t used = fifoBuf_getUsed(&usart_dev->tx);
	fifoBuf_putData(&usart_dev->tx,buffer,len);
	
	if (used == 0) {
		/* enable sending when buffer was previously empty */
		USART_ITConfig(usart_dev->cfg->regs, USART_IT_TXE, ENABLE);
	}

	PIOS_IRQ_Enable();

	/* No error */
	return 0;
}

/**
* puts more than one byte onto the transmit buffer (used for atomic sends)<BR>
* (blocking function)
* \param[in] USART USART name
* \param[in] *buffer pointer to buffer to be sent
* \param[in] len number of bytes to be sent
* \return 0 if no error
*/
static int32_t PIOS_USART_TxBufferPutMore(uint32_t usart_id, const uint8_t *buffer, uint16_t len)
{
	int32_t rc;

	while ((rc = PIOS_USART_TxBufferPutMoreNonBlocking(usart_id, buffer, len)) == -1);

	return rc;
}

static void PIOS_USART_generic_irq_handler(uint32_t usart_id)
{
	struct pios_usart_dev * usart_dev = (struct pios_usart_dev *)usart_id;

	bool valid = PIOS_USART_validate(usart_dev);
	PIOS_Assert(valid);

	/* Call any user provided callback function instead of processing
	 * the interrupt ourselves.
	 */
	if (usart_dev->cfg->irq.handler) {
		(usart_dev->cfg->irq.handler)(usart_id);
		return;
	}

	/* Force read of dr after sr to make sure to clear error flags */
	volatile uint16_t sr = usart_dev->cfg->regs->SR;
	volatile uint8_t dr = usart_dev->cfg->regs->DR;

	/* Check if RXNE flag is set */
	if (sr & USART_SR_RXNE) {
		if (PIOS_USART_RxBufferPut(usart_id, dr) < 0) {
			/* Here we could add some error handling */
		}
	}

	/* Check if TXE flag is set */
	if (sr & USART_SR_TXE) {
		if (PIOS_USART_TxBufferUsed(usart_id) > 0) {
			int32_t b = PIOS_USART_TxBufferGet(usart_id);

			if (b < 0) {
				/* Here we could add some error handling */
				usart_dev->cfg->regs->DR = 0xff;
			} else {
				usart_dev->cfg->regs->DR = b & 0xff;
			}
		} else {
			/* Disable TXE interrupt (TXEIE=0) */
			USART_ITConfig(usart_dev->cfg->regs, USART_IT_TXE, DISABLE);
		}
	}
}

#endif

/**
  * @}
  * @}
  */
