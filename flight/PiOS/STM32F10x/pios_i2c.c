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



#if defined(PIOS_INCLUDE_I2C)



/* Options */
//#define USE_DEBUG_PINS

#ifdef PIOS_INCLUDE_FREERTOS
	#define USE_FREERTOS
#endif


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
		unsigned INIRQ:1;
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

#ifdef USE_FREERTOS
	xSemaphoreHandle sem_readySignal;
	portBASE_TYPE xHigherPriorityTaskWoken;
#endif

	volatile uint8_t i2c_semaphore;
} I2CRecTypeDef;

/* Local Prototypes */
static void PIOS_I2C_InitPeripheral(void);
static void EV_IRQHandler(I2CRecTypeDef *i2cx);
static void ER_IRQHandler(I2CRecTypeDef *i2cx);

/* Local Variables */
static I2CRecTypeDef I2CRec;

/* Local Functions */
static void TransferStart(I2CRecTypeDef *i2cx);
static void TransferEnd(I2CRecTypeDef *i2cx);

/* Macros */
#ifdef USE_DEBUG_PINS
	#define	DEBUG_PIN_ISR	0
	#define DEBUG_PIN_BUSY	1
	#define DEBUG_PIN_WAIT	2
	#define DEBUG_PIN_ASSERT	7
	#define DebugPinHigh(x) PIOS_DEBUG_PinHigh(x)
	#define DebugPinLow(x)	PIOS_DEBUG_PinLow(x)
#else
	#define DebugPinHigh(x)
	#define DebugPinLow(x)
#endif

#define assert(exp) PIOS_DEBUG_Assert(exp)


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
#ifdef USE_FREERTOS
	vSemaphoreCreateBinary(I2CRec.sem_readySignal);
#endif

	TransferEnd(&I2CRec);

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

	DebugPinLow(2);

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
int32_t PIOS_I2C_LockDevice(I2CSemaphoreTypeDef semaphore_type)
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
	i2cx->transfer_error = 0;

	return status;
}


/**
* Semaphore handling: releases the IIC interface for other tasks
* \return < 0 on errors
*/
int32_t PIOS_I2C_UnlockDevice(void)
{
	I2CRec.i2c_semaphore = 0;

	/* No error */
	return 0;
}


/**
 * Internal function called at the start of a transfer
 */
static void TransferStart(I2CRecTypeDef *i2cx)
{
	assert(i2cx->transfer_state.BUSY == 0);

	DebugPinHigh(DEBUG_PIN_BUSY);
	i2cx->transfer_state.BUSY = 1;

	// Enable Interrupts: I2V2 event, buffer and error interrupt
	I2C_ITConfig(i2cx->base, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR, ENABLE);
}

/**
 * Internal function called at the end of a transfer
 */
static void TransferEnd(I2CRecTypeDef *i2cx)
{
	// Disable all interrupts
	I2C_ITConfig(i2cx->base, I2C_IT_EVT | I2C_IT_BUF | I2C_IT_ERR, DISABLE);

	DebugPinLow(DEBUG_PIN_BUSY);
	i2cx->transfer_state.BUSY = 0;

#ifdef USE_FREERTOS
	if (i2cx->transfer_state.INIRQ)
	{
		xSemaphoreGiveFromISR(i2cx->sem_readySignal, &i2cx->xHigherPriorityTaskWoken);
	}
	else
	{
		xSemaphoreGive(i2cx->sem_readySignal);
	}
#endif
}

/**
* Checks if transfer is finished
* \return 1 if ongoing transfer
* \return <=0 no transfer is busy; return value indicates error value of last transfer
* (PIOS_I2C_TransferBegin() has to be called again)
*/
int32_t PIOS_I2C_TransferCheck(void)
{
	I2CRecTypeDef *i2cx = &I2CRec;

	if(i2cx->transfer_state.BUSY)
		return 1;

	return i2cx->transfer_error;
}

/**
 * Stop the current transfer
 * \param error the error that must be reported
 */
void PIOS_I2C_TerminateTransfer(uint32_t error)
{
	I2CRecTypeDef *i2cx = &I2CRec;

	/* Send stop condition */
	I2C_GenerateSTOP(i2cx->base, ENABLE);

	/* Re-initialize peripheral */
	PIOS_I2C_InitPeripheral();

	i2cx->transfer_error = error;
}

/**
* Waits until transfer is finished.
* \return error value of the transfer
*/
int32_t PIOS_I2C_TransferWait(void)
{
	I2CRecTypeDef *i2cx = &I2CRec;
	
	DebugPinHigh(DEBUG_PIN_WAIT);

#ifdef USE_FREERTOS

	if (i2cx->transfer_state.BUSY)
	{
		// Wait until we see the ready signal
		if (xSemaphoreTake(i2cx->sem_readySignal, PIOS_I2C_TIMEOUT_VALUE/portTICK_RATE_MS) == pdTRUE)
		{
			// OK, got the semaphore, release it again
			assert(i2cx->transfer_state.BUSY == 0);
		}
		else
		{
			PIOS_I2C_TerminateTransfer(I2C_ERROR_TIMEOUT);
		}
	}

#else
	uint32_t repeat_ctr = PIOS_I2C_TIMEOUT_VALUE;
	uint16_t last_buffer_ix = i2cx->buffer_ix;

	if (i2cx->transfer_state.BUSY)
	{
		while(--repeat_ctr > 0)
		{
			/* Check if buffer index has changed - if so, reload repeat counter */
			if(i2cx->buffer_ix != last_buffer_ix) {
				repeat_ctr = PIOS_I2C_TIMEOUT_VALUE;
				last_buffer_ix = i2cx->buffer_ix;
			}

			/* Get transfer state */
			int32_t check_state = PIOS_I2C_TransferCheck();

			/* Exit if transfer finished */
			if(check_state <= 0)
			{
				DebugPinLow(DEBUG_PIN_WAIT);
				return check_state;
			}
		}

		/* Timeout error - something is stalling... */
		PIOS_I2C_TerminateTransfer(I2C_ERROR_TIMEOUT);
	}
#endif

	DebugPinLow(DEBUG_PIN_WAIT);

	return i2cx->transfer_error;
}

/**
* Perform a transfer. No previous transfer should be ongoing when this function is called.
* When the function returns, the transfer is finished.
* See PIOS_I2C_StartTransfer() for details on the parameters.
* \return 0 no error
* \return < 0 on errors
*
*/
int32_t PIOS_I2C_Transfer(I2CTransferTypeDef transfer, uint8_t address, uint8_t *buffer, uint16_t len)
{
	PIOS_I2C_StartTransfer(transfer, address, buffer, len);
	return PIOS_I2C_TransferWait();
}

/**
* Starts a new transfer. No previous transfer should be ongoing when this function is called.
* When this function returns, the new transfer is ongoing. PIOS_I2C_TransferWait() should be called
* to wait for the transfer to finish and to retrieve the result code.
* \param[in] transfer type:<BR>
* <UL>
*   <LI>I2C_Read: a common Read transfer
*   <LI>I2C_Write: a common Write transfer
*   <LI>I2C_Write_WithoutStop: don't send stop condition after transfer to allow
*         a restart condition (e.g. used to access EEPROMs)
* \param[in] address of I2C device (bit 0 always cleared)
* \param[in] *buffer pointer to transmit/receive buffer
* \param[in] len number of bytes which should be transmitted/received
*/
void PIOS_I2C_StartTransfer(I2CTransferTypeDef transfer, uint8_t address, uint8_t *buffer, uint16_t len)
{
	I2CRecTypeDef *i2cx = &I2CRec;

	// Should not be busy
	if (i2cx->transfer_state.BUSY)
	{
		assert(0);
		return;
	}

#ifdef USE_FREERTOS
	// Consume Ready semaphore in case it would be there for some reason
	xSemaphoreTake(i2cx->sem_readySignal, 0);
	assert(xSemaphoreTake(i2cx->sem_readySignal, 0) == pdFALSE);
#endif

	// Clear state
	i2cx->transfer_state.ALL = 0;
	i2cx->transfer_error = 0;

	// Set buffer length and start index
	i2cx->buffer_len = len;
	i2cx->buffer_ix = 0;

	if(transfer == I2C_Read)
	{
		/* Take new address/buffer/len */
		/* Set bit 0 for read operation */
		i2cx->i2c_address = address | 1;
		/* Ensure that previous TX buffer won't be accessed */
		i2cx->tx_buffer_ptr = NULL;
		i2cx->rx_buffer_ptr = buffer;
		// Ack the bytes we will be getting
		I2C_AcknowledgeConfig(i2cx->base, ENABLE);
	}
	else if(transfer == I2C_Write || transfer == I2C_Write_WithoutStop)
	{
		/* Take new address/buffer/len */
		/* Clear bit 0 for write operation */
		i2cx->i2c_address = address & 0xfe;
		i2cx->tx_buffer_ptr = buffer;
		/* Ensure that nothing will be received */
		i2cx->rx_buffer_ptr = NULL;
		/* Option to skip stop-condition generation after successful write */
		i2cx->transfer_state.WRITE_WITHOUT_STOP = transfer == I2C_Write_WithoutStop ? 1 : 0;
	}
	else
	{
		i2cx->transfer_error = I2C_ERROR_UNSUPPORTED_TRANSFER_TYPE;
		return;
	}

	// Start the transfer
	I2C_GenerateSTART(i2cx->base, ENABLE);
	TransferStart(i2cx);
}

/**
* Internal function for handling IIC event interrupts
*/
static void EV_IRQHandler(I2CRecTypeDef *i2cx)
{
	uint8_t b;
	uint32_t event;

	DebugPinHigh(DEBUG_PIN_ISR);

	// Update state
	i2cx->transfer_state.INIRQ = 1;
#ifdef USE_FREERTOS
	i2cx->xHigherPriorityTaskWoken = pdFALSE;
#endif

	/* Read SR1 and SR2 at the beginning (if not done so, flags may get lost) */
	event = I2C_GetLastEvent(i2cx->base);

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
			TransferEnd(i2cx);
			goto isr_return;
		}
		
		/* Request NAK and stop condition before receiving last data */
		if(i2cx->buffer_ix >= i2cx->buffer_len-1) {
			/* Request NAK */
			I2C_AcknowledgeConfig(i2cx->base, DISABLE);
			/* Request stop condition */
			I2C_GenerateSTOP(i2cx->base, ENABLE);
			i2cx->transfer_state.STOP_REQUESTED = 1;
		}
		goto isr_return;
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
		goto isr_return;
	}

	/* TxE set, will be cleared by writing DR, or after START or STOP was generated */
	/* This handling also applies for BTF, as TXE will alway be set if BTF is. */
	/* Note: also BTF will be reset after a read of SR1 (TxE flag) followed by either read/write DR */
	/* Or a START or STOP condition generated */
	if(event & I2C_FLAG_TXE) {
		/* Last byte already sent, disable interrupts and return. */
		if(i2cx->transfer_state.STOP_REQUESTED) {
			TransferEnd(i2cx);
			goto isr_return;
		}
		
		if(i2cx->buffer_ix < i2cx->buffer_len) {
			/* Checking tx_buffer_ptr for NULL is a failsafe measure. */
			I2C_SendData(i2cx->base, (i2cx->tx_buffer_ptr == NULL) ? 0 : i2cx->tx_buffer_ptr[i2cx->buffer_ix++]);
			goto isr_return;
		} 
		
		/* Peripheral is transfering last byte, request stop condition / */
		/* On write-without-stop transfer-type, request start condition instead */
		i2cx->transfer_state.STOP_REQUESTED = 1;
		if(!i2cx->transfer_state.WRITE_WITHOUT_STOP)
		{
			I2C_GenerateSTOP(i2cx->base, ENABLE);
		}
		else
		{
			DebugPinHigh(2);
		}
		
		if(i2cx->buffer_len == 0) {
			TransferEnd(i2cx);
		} else {
			/* Disable the I2C_IT_BUF interrupt after sending the last buffer data  */
			/* (last EV8) to not allow a new interrupt just with TxE - only BTF will generate it */
			/* If this is not done, BUSY will be cleared before the transfer is finished */
			I2C_ITConfig(i2cx->base, I2C_IT_BUF, DISABLE);
		}
		goto isr_return;
	}

	// Send address
	/* SB set, cleared by reading SR1 (done by I2C_GetLastEvent) followed by writing DR register */
	if(event & I2C_FLAG_SB) {
		/* Don't send address if stop was requested (WRITE_WITHOUT_STOP - mode, start condition was sent) */
		/* We have to wait for the application to start the next transfer */
		if(i2cx->transfer_state.STOP_REQUESTED) {
			TransferEnd(i2cx);
			DebugPinLow(DEBUG_PIN_ISR);
			return;
		}
		
		/* Send IIC address */
		I2C_Send7bitAddress(i2cx->base, i2cx->i2c_address, 
		(i2cx->i2c_address & 1)
		? I2C_Direction_Receiver
		: I2C_Direction_Transmitter);
		goto isr_return;
	}
	
	DebugPinHigh(DEBUG_PIN_ASSERT);DebugPinLow(DEBUG_PIN_ASSERT);

	//
	// FredericG: Despite the comments below, it seems to me that this situation can happen and can
	// be ignored without ill effects...
	// For now this condition does not stop the transfer, but further investigation in needed
	//

//	assert(0);
//
//	/* This code is only reached if something got wrong, e.g. interrupt handler is called too late, */
//	/* The device reset itself (while testing, it was always event 0x00000000). we have to stop the transfer, */
//	/* Else read/write of corrupt data may be the result. */
//
//	/* Notify error */
//	PIOS_I2C_UnexpectedEvent = event;
//	i2cx->transfer_error = I2C_ERROR_UNEXPECTED_EVENT;
//
//	TransferEnd(i2cx);
//
//	/* Do dummy read to send NAK + STOP condition */
//	I2C_AcknowledgeConfig(i2cx->base, DISABLE);
//	b = I2C_ReceiveData(i2cx->base);
//	I2C_GenerateSTOP(i2cx->base, ENABLE);

isr_return:
	// Cause task-switch when needed
#ifdef USE_FREERTOS
	portEND_SWITCHING_ISR(i2cx->xHigherPriorityTaskWoken);
#endif

	// Update state
	i2cx->transfer_state.INIRQ = 0;
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

	/* Notify that transfer has finished (due to the error) */
	TransferEnd(i2cx);
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
