/**
 ******************************************************************************
 *
 * @file       pios_i2c.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	        Parts by Thorsten Klose (tk@midibox.org) (tk@midibox.org)
 * @brief      I2C Enable/Disable routines
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_I2C I2C Functions
 * @{
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

#if !defined(PIOS_DONT_USE_I2C)

/* Options */
//#define USE_DEBUG_PINS

/* Global Variables */
volatile uint32_t PIOS_I2C_UnexpectedEvent;

/* Local types */
typedef union {
	struct {
		unsigned ALL:8;
	};
	struct {
		unsigned BUSY:1;
		unsigned STOP_REQUESTED:1;
		unsigned ABORT_IF_FIRST_BYTE_0:1;
		unsigned WRITE_WITHOUT_STOP:1;
	};
} TransferStateTypeDef;


typedef struct {
	I2C_TypeDef *base;

	uint8_t i2c_address;
	uint8_t *tx_buffer_ptr;
	uint8_t *rx_buffer_ptr;
	volatile uint16_t buffer_len;
	volatile uint16_t buffer_ix;

	volatile TransferStateTypeDef transfer_state;
	volatile int32_t transfer_error;
	volatile int32_t last_transfer_error;

	volatile uint8_t i2c_semaphore;
} I2CRecTypeDef;

/* Local Prototypes */
static void PIOS_I2C_InitPeripheral(void);
static void EV_IRQHandler(I2CRecTypeDef *i2cx);
static void ER_IRQHandler(I2CRecTypeDef *i2cx);

/* Local Variables */
static I2CRecTypeDef I2CRec;

/* Macros */
#ifdef USE_DEBUG_PINS
	#define	DEBUG_PIN_ISR	0
	#define DEBUG_PIN_BUSY	1
	#define DebugPinHigh(x) PIOS_DEBUG_PinHigh(x)
	#define DebugPinLow(x)	PIOS_DEBUG_PinLow(x)
#else
	#define DebugPinHigh(x)
	#define DebugPinLow(x)
#endif

/**
* Initializes IIC driver
* \param[in] mode currently only mode 0 supported
* \return < 0 if initialisation failed
*/
int32_t PIOS_I2C_Init(void)
{
	/* Configure IIC pins in open drain mode */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;

	GPIO_InitStructure.GPIO_Pin = PIOS_I2C_SCL_PIN;
	GPIO_Init(PIOS_I2C_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = PIOS_I2C_SDA_PIN;
	GPIO_Init(PIOS_I2C_GPIO_PORT, &GPIO_InitStructure);

	PIOS_I2C_InitPeripheral();

	/* Now accessible for other tasks */
	I2CRec.i2c_semaphore = 0;
	I2CRec.last_transfer_error = 0;

	/* Configure and enable I2C2 interrupts */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = I2C2_EV_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_I2C_IRQ_EV_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = I2C2_ER_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_I2C_IRQ_ER_PRIORITY;
	NVIC_Init(&NVIC_InitStructure);

	/* No error */
	return 0;
}


/**
* Internal function to (re-)initialize the I2C peripheral
*/
static void PIOS_I2C_InitPeripheral(void)
{
	I2C_InitTypeDef  I2C_InitStructure;
	I2CRecTypeDef *i2cx = &I2CRec;

	/* Prepare I2C init-struct */
	I2C_StructInit(&I2C_InitStructure);
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_OwnAddress1 = 0;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

	/* Define base address */
	i2cx->base = I2C2;

	/* enable peripheral clock of I2C */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

	/* Set I2C clock bus clock params */
	/* Note that the STM32 driver handles value <= 100kHz differently! (duty cycle always 1:1) */
	/* Important: bus frequencies > 400kHz don't work stable */
	I2C_InitStructure.I2C_DutyCycle = PIOS_I2C_DUTY_CYCLE;
	I2C_InitStructure.I2C_ClockSpeed = PIOS_I2C_BUS_FREQ;

	/* Trigger software reset via I2C_DeInit */
	I2C_DeInit(i2cx->base);

	/* Clear transfer state and error value */
	i2cx->transfer_state.ALL = 0;
	PIOS_DEBUG_PinLow(0);
	i2cx->transfer_error = 0;

	/* Configure I2C peripheral */
	I2C_Init(i2cx->base, &I2C_InitStructure);
}


/**
* Semaphore handling: requests the IIC interface
* \param[in] semaphore_type is either IIC_Blocking or IIC_Non_Blocking
* \return Non_Blocking: returns -1 to request a retry
* \return 0 if IIC interface free
*/
int32_t PIOS_I2C_TransferBegin(I2CSemaphoreTypeDef semaphore_type)
{
	volatile I2CRecTypeDef *i2cx = &I2CRec;
	int32_t status = -1;

	do {
		PIOS_IRQ_Disable();
		if(!i2cx->i2c_semaphore) {
			i2cx->i2c_semaphore = 1;
			status = 0;
		}
		PIOS_IRQ_Enable();
	} while(semaphore_type == I2C_Blocking && status != 0);

	/* Clear transfer errors of last transmission */
	i2cx->last_transfer_error = 0;
	i2cx->transfer_error = 0;

	return status;
}


/**
* Semaphore handling: releases the IIC interface for other tasks
* \return < 0 on errors
*/
int32_t PIOS_I2C_TransferFinished(void)
{
	I2CRec.i2c_semaphore = 0;

	/* No error */
	return 0;
}

/**
* Returns the last transfer error<BR>
* Will be updated by PIOS_I2C_TransferCheck(), so that the error status
* doesn't get lost (the check function will return 0 when called again)<BR>
* Will be cleared when a new transfer has been started successfully
* \return last error status
*/
int32_t PIOS_IIC_LastErrorGet(void)
{
	return I2CRec.last_transfer_error;
}


/**
* Checks if transfer is finished
* \return 0 if no ongoing transfer
* \return 1 if ongoing transfer
* \return < 0 if error during transfer
* \note Note that the semaphore will be released automatically after an error
* (PIOS_I2C_TransferBegin() has to be called again)
*/
int32_t PIOS_I2C_TransferCheck(void)
{
	I2CRecTypeDef *i2cx = &I2CRec;

	/* Ongoing transfer? */
	if(i2cx->transfer_state.BUSY) {
		return 1;
	}

	/* Error during transfer? */
	/* (must be done *after* BUSY check to avoid race conditon!) */
	if(i2cx->transfer_error) {
		/* Store error status for PIOS_IIC_LastErrorGet() function */
		i2cx->last_transfer_error = i2cx->transfer_error;
		/* Clear current error status */
		i2cx->transfer_error = 0;
		/* Release semaphore for easier programming at user level */
		i2cx->i2c_semaphore = 0;
		/* And exit */
		return i2cx->last_transfer_error;
	}

	/* No transfer */
	return 0;
}


/**
* Waits until transfer is finished
* \return 0 if no ongoing transfer
* \return < 0 if error during transfer
* \note Note that the semaphore will be released automatically after an error
* (PIOS_I2C_TransferBegin() has to be called again)
*/
int32_t PIOS_I2C_TransferWait(void)
{
	I2CRecTypeDef *i2cx = &I2CRec;
	
	uint32_t repeat_ctr = PIOS_I2C_TIMEOUT_VALUE;
	uint16_t last_buffer_ix = i2cx->buffer_ix;

	while(--repeat_ctr > 0) {
		/* Check if buffer index has changed - if so, reload repeat counter */
		if(i2cx->buffer_ix != last_buffer_ix) {
			repeat_ctr = PIOS_I2C_TIMEOUT_VALUE;
			last_buffer_ix = i2cx->buffer_ix;
		}
		
		/* Get transfer state */
		int32_t check_state = PIOS_I2C_TransferCheck();
		
		/* Exit if transfer finished or error detected */
		if(check_state <= 0) {
			if(check_state < 0) {
				/* Release semaphore for easier programming at user level */
				i2cx->i2c_semaphore = 0;
			}
			return check_state;
		}
	}

	/* Timeout error - something is stalling... */

	/* Send stop condition */
	I2C_GenerateSTOP(i2cx->base, ENABLE);

	/* Re-initialize peripheral */
	PIOS_I2C_InitPeripheral();

	/* Release semaphore (!) */
	i2cx->i2c_semaphore = 0;

	return (i2cx->last_transfer_error = I2C_ERROR_TIMEOUT);
}


/**
* Starts a new transfer. If this function is called during an ongoing
* transfer, we wait until it has been finished and setup the new transfer
* \param[in] transfer type:<BR>
* <UL>
*   <LI>I2C_Read: a common Read transfer
*   <LI>I2C_Write: a common Write transfer
*   <LI>I2C_Write_WithoutStop: don't send stop condition after transfer to allow
*         a restart condition (e.g. used to access EEPROMs)
* \param[in] address of I2C device (bit 0 always cleared)
* \param[in] *buffer pointer to transmit/receive buffer
* \param[in] len number of bytes which should be transmitted/received
* \return 0 no error
* \return < 0 on errors, if PIOS_I2C_ERROR_PREV_OFFSET is added, the previous
*      transfer got an error (the previous task didn't use \ref PIOS_I2C_TransferWait
*      to poll the transfer state)
* \note Note that the semaphore will be released automatically after an error
* (PIOS_I2C_TransferBegin() has to be called again)
*/
int32_t PIOS_I2C_Transfer(I2CTransferTypeDef transfer, uint8_t address, uint8_t *buffer, uint16_t len)
{
	I2CRecTypeDef *i2cx = &I2CRec;
	int32_t error;

	/* Wait until previous transfer finished */
	if((error = PIOS_I2C_TransferWait())) {
		/* Transmission error during previous transfer */
		
		/* Release semaphore for easier programming at user level */
		i2cx->i2c_semaphore = 0;
		return error + I2C_ERROR_PREV_OFFSET;
	}

	DebugPinLow(DEBUG_PIN_BUSY);
	i2cx->transfer_state.BUSY = 0;

	/* Disable interrupts */
	I2C_ITConfig(i2cx->base, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR, DISABLE);

	/* Clear transfer state and error value */
	i2cx->transfer_state.ALL = 0;
	i2cx->transfer_error = 0;

	/* Set buffer length and start index */
	i2cx->buffer_len = len;
	i2cx->buffer_ix = 0;

	/* Branch depending on read/write */
	if(transfer == I2C_Read) {
		/* Take new address/buffer/len */
		/* Set bit 0 for read operation */
		i2cx->i2c_address = address | 1;
		/* Ensure that previous TX buffer won't be accessed */
		i2cx->tx_buffer_ptr = NULL;
		i2cx->rx_buffer_ptr = buffer;
	} else if(transfer == I2C_Write || transfer == I2C_Write_WithoutStop) {
		/* Take new address/buffer/len */
		/* Clear bit 0 for write operation */
		i2cx->i2c_address = address & 0xfe;
		i2cx->tx_buffer_ptr = buffer;
		/* Ensure that nothing will be received */
		i2cx->rx_buffer_ptr = NULL;
		/* Option to skip stop-condition generation after successful write */
		i2cx->transfer_state.WRITE_WITHOUT_STOP = transfer == I2C_Write_WithoutStop ? 1 : 0;
	} else {
		/* Release semaphore for easier programming at user level */
		i2cx->i2c_semaphore = 0;
		return (i2cx->last_transfer_error=I2C_ERROR_UNSUPPORTED_TRANSFER_TYPE);
	}

	/* Start with ACK */
	I2C_AcknowledgeConfig(i2cx->base, ENABLE);

	/* Clear last error status */
	i2cx->last_transfer_error = 0;

	/* Notify that transfer has started */
	DebugPinHigh(DEBUG_PIN_BUSY);
	i2cx->transfer_state.BUSY = 1;

	/* Send start condition */
	I2C_GenerateSTART(i2cx->base, ENABLE);

	/* Enable I2V2 event, buffer and error interrupt */
	/* This must be done *after* GenerateStart, for the case last transfer was WRITE_WITHOUT_STOP. */
	/* In this case, start was already generated at the end of the last communication! */
	I2C_ITConfig(i2cx->base, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR, ENABLE);

	/* No error */
	return 0;
}

/**
* Internal function for handling IIC event interrupts
*/
static void EV_IRQHandler(I2CRecTypeDef *i2cx)
{
	uint8_t b;

	DebugPinHigh(DEBUG_PIN_ISR);

	/* Read SR1 and SR2 at the beginning (if not done so, flags may get lost) */
	uint32_t event = I2C_GetLastEvent(i2cx->base);

	/* The order of the handling blocks is chosen by test results @ 1MHZ */
	/* Don't change this order */

	/* RxNE set, will be cleared by reading/writing DR */
	/* Note: also BTF will be reset after a read of SR1 (TxE flag) followed by either read/write DR */
	/* Or a START or STOP condition generated */
	/* Failsave: really requested a receive transfer? If not, continue to check TXE flag, if not set, */
	/* We'll end up in the unexpected event handler. */
	if(event & I2C_FLAG_RXNE && i2cx->rx_buffer_ptr != NULL) {
		/* Get received data */
		b = I2C_ReceiveData(i2cx->base);
		
		/* Failsave: still place in buffer? */
		if(i2cx->buffer_ix < i2cx->buffer_len) {
			i2cx->rx_buffer_ptr[i2cx->buffer_ix++] = b;
		}
		
		/* Last byte received, disable interrupts and return. */
		if(i2cx->transfer_state.STOP_REQUESTED) {
			/* Transfer finished */
			DebugPinLow(DEBUG_PIN_BUSY);
			i2cx->transfer_state.BUSY = 0;
			/* Disable all interrupts */
			I2C_ITConfig(i2cx->base, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR, DISABLE);
			DebugPinLow(DEBUG_PIN_ISR);
			return;
		}
		
		/* Request NAK and stop condition before receiving last data */
		if(i2cx->buffer_ix >= i2cx->buffer_len-1) {
			/* Request NAK */
			I2C_AcknowledgeConfig(i2cx->base, DISABLE);
			/* Request stop condition */
			I2C_GenerateSTOP(i2cx->base, ENABLE);
			i2cx->transfer_state.STOP_REQUESTED = 1;
		}
		DebugPinLow(DEBUG_PIN_ISR);
		return;
	}

	/* ADDR set, TRA flag not set (indicates transmitter/receiver mode). */
	/* ADDR will be cleared by a read of SR1 followed by a read of SR2 (done by I2C_GetLastEvent) */
	/* If transmitter mode is selected (TRA set), we go on, TXE will be catched to send the first byte */
	if((event & I2C_FLAG_ADDR) && !(event & I2C_FLAG_TRA)) {
		/* Address sent (receiver mode), receiving first byte - check if we already have to request NAK/Stop */
		if(i2cx->buffer_len == 1) {
			/* Request NAK */
			I2C_AcknowledgeConfig(i2cx->base, DISABLE);
			/* Request stop condition */
			I2C_GenerateSTOP(i2cx->base, ENABLE);
			i2cx->transfer_state.STOP_REQUESTED = 1;
		}
		DebugPinLow(DEBUG_PIN_ISR);
		return;
	}

	/* TxE set, will be cleared by writing DR, or after START or STOP was generated */
	/* This handling also applies for BTF, as TXE will alway be set if BTF is. */
	/* Note: also BTF will be reset after a read of SR1 (TxE flag) followed by either read/write DR */
	/* Or a START or STOP condition generated */
	if(event & I2C_FLAG_TXE) {
		/* Last byte already sent, disable interrupts and return. */
		if(i2cx->transfer_state.STOP_REQUESTED) {
			/* Transfer finished */
			DebugPinLow(DEBUG_PIN_BUSY);
			i2cx->transfer_state.BUSY = 0;
			/* Disable all interrupts */
			I2C_ITConfig(i2cx->base, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR, DISABLE);
			DebugPinLow(DEBUG_PIN_ISR);
			return;
		}
		
		if(i2cx->buffer_ix < i2cx->buffer_len) {
			/* Checking tx_buffer_ptr for NULL is a failsafe measure. */
			I2C_SendData(i2cx->base, (i2cx->tx_buffer_ptr == NULL) ? 0 : i2cx->tx_buffer_ptr[i2cx->buffer_ix++]);
			DebugPinLow(DEBUG_PIN_ISR);
			return;
		} 
		
		/* Peripheral is transfering last byte, request stop condition / */
		/* On write-without-stop transfer-type, request start condition instead */
		if(!i2cx->transfer_state.WRITE_WITHOUT_STOP) {
			DebugPinHigh(2);
			I2C_GenerateSTOP(i2cx->base, ENABLE);
			i2cx->transfer_state.STOP_REQUESTED = 1;
			DebugPinLow(2);
		} else {
			I2C_GenerateSTART(i2cx->base, ENABLE);
			i2cx->transfer_state.STOP_REQUESTED = 1;
		}
		
		if(i2cx->buffer_len == 0) {
			/* Transfer finished */
			i2cx->transfer_state.BUSY = 0;
			DebugPinLow(DEBUG_PIN_BUSY);
			/* Disable all interrupts */
			I2C_ITConfig(i2cx->base, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR, DISABLE);
		} else {
			/* Disable the I2C_IT_BUF interrupt after sending the last buffer data  */
			/* (last EV8) to not allow a new interrupt just with TxE - only BTF will generate it */
			/* If this is not done, BUSY will be cleared before the transfer is finished */
			I2C_ITConfig(i2cx->base, I2C_IT_BUF, DISABLE);
		}
		DebugPinLow(DEBUG_PIN_ISR);
		return;
	}

	/* SB set, cleared by reading SR1 (done by I2C_GetLastEvent) followed by writing DR register */
	if(event & I2C_FLAG_SB) {
		/* Don't send address if stop was requested (WRITE_WITHOUT_STOP - mode, start condition was sent) */
		/* We have to wait for the application to start the next transfer */
		if(i2cx->transfer_state.STOP_REQUESTED) {
			/* Transfer finished */
			i2cx->transfer_state.BUSY = 0;
			DebugPinLow(DEBUG_PIN_BUSY);
			/* Disable all interrupts */
			I2C_ITConfig(i2cx->base, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR, DISABLE);
			DebugPinLow(DEBUG_PIN_ISR);
			return;
		}
		
		/* Send IIC address */
		I2C_Send7bitAddress(i2cx->base, i2cx->i2c_address, 
		(i2cx->i2c_address & 1)
		? I2C_Direction_Receiver
		: I2C_Direction_Transmitter);
		DebugPinLow(DEBUG_PIN_ISR);
		return;
	}

	/* This code is only reached if something got wrong, e.g. interrupt handler is called too late, */
	/* The device reset itself (while testing, it was always event 0x00000000). we have to stop the transfer, */
	/* Else read/write of corrupt data may be the result. */
	I2C_ITConfig(i2cx->base, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR, DISABLE);
	
	/* Notify error */
	PIOS_I2C_UnexpectedEvent = event;
	i2cx->transfer_error = I2C_ERROR_UNEXPECTED_EVENT;
	i2cx->transfer_state.BUSY = 0;
	DebugPinLow(DEBUG_PIN_BUSY);
	
	/* Do dummy read to send NAK + STOP condition */
	I2C_AcknowledgeConfig(i2cx->base, DISABLE);
	b = I2C_ReceiveData(i2cx->base);
	I2C_GenerateSTOP(i2cx->base, ENABLE);

	DebugPinLow(DEBUG_PIN_ISR);
}


/**
* Internal function for handling IIC error interrupts
*/
static void ER_IRQHandler(I2CRecTypeDef *i2cx)
{
	/* Read SR1 and SR2 at the beginning (if not done so, flags may get lost) */
	uint32_t event = I2C_GetLastEvent(i2cx->base);

	/* Note that only one error number is available */
	/* The order of these checks defines the priority */

	/* Bus error (start/stop condition during read */
	/* Unlikely, should only be relevant for slave mode?) */
	if(event & I2C_FLAG_BERR) {
		I2C_ClearITPendingBit(i2cx->base, I2C_IT_BERR);
		i2cx->transfer_error = I2C_ERROR_BUS;
	}

	/* Arbitration lost */
	if(event & I2C_FLAG_ARLO) {
		I2C_ClearITPendingBit(i2cx->base, I2C_IT_ARLO);
		i2cx->transfer_error = I2C_ERROR_ARBITRATION_LOST;
	}

	/* No acknowledge received from slave (e.g. slave not connected) */
	if(event & I2C_FLAG_AF) {
		I2C_ClearITPendingBit(i2cx->base, I2C_IT_AF);
		i2cx->transfer_error = I2C_ERROR_SLAVE_NOT_CONNECTED;
		/* Send stop condition to release bus */
		I2C_GenerateSTOP(i2cx->base, ENABLE);
	}

	/* Disable interrupts */
	I2C_ITConfig(i2cx->base, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR, DISABLE);

	/* Notify that transfer has finished (due to the error) */
	i2cx->transfer_state.BUSY = 0;
	DebugPinLow(DEBUG_PIN_BUSY);
}


/* Interrupt vectors */
void I2C2_EV_IRQHandler(void)
{
  EV_IRQHandler((I2CRecTypeDef *)&I2CRec);
}

void I2C2_ER_IRQHandler(void)
{
  ER_IRQHandler((I2CRecTypeDef *)&I2CRec);
}

#endif
