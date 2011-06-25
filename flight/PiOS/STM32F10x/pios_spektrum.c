/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_SPEKTRUM Spektrum receiver functions
 * @brief Code to read Spektrum input
 * @{
 *
 * @file       pios_spektrum.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	        Parts by Thorsten Klose (tk@midibox.org) (tk@midibox.org)
 * @brief      USART commands. Inits USARTs, controls USARTs & Interrupt handlers. (STM32 dependent)
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
#include "pios_spektrum_priv.h"

#if defined(PIOS_INCLUDE_SPEKTRUM)
#if defined(PIOS_INCLUDE_PWM) || defined(PIOS_INCLUDE_SBUS)
#error "Both SPEKTRUM and either of PWM or SBUS inputs defined, choose only one"
#endif
#if defined(PIOS_COM_AUX)
#error "AUX com cannot be used with SPEKTRUM"
#endif

/**
 * @Note Framesyncing:
 * The code resets the watchdog timer whenever a single byte is received, so what watchdog code
 * is never called if regularly getting bytes.
 * RTC timer is running @625Hz, supervisor timer has divider 5 so frame sync comes every 1/125Hz=8ms.
 * Good for both 11ms and 22ms framecycles
 */

/* Global Variables */

/* Provide a RCVR driver */
static int32_t PIOS_SPEKTRUM_Get(uint32_t chan_id);

const struct pios_rcvr_driver pios_spektrum_rcvr_driver = {
	.read = PIOS_SPEKTRUM_Get,
};

/* Local Variables */
static uint16_t CaptureValue[PIOS_SPEKTRUM_NUM_INPUTS],CaptureValueTemp[PIOS_SPEKTRUM_NUM_INPUTS];
static uint8_t prev_byte = 0xFF, sync = 0, bytecount = 0, datalength=0, frame_error=0, byte_array[20] = { 0 };
uint8_t sync_of = 0;
uint16_t supv_timer=0;

/**
* Bind and Initialise Spektrum satellite receiver
*/
void PIOS_SPEKTRUM_Init(void)
{
	// TODO: need setting flag for bind on next powerup
	if (0) {
		PIOS_SPEKTRUM_Bind();
	}

	/* Init RTC supervisor timer interrupt */
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	/* Init RTC clock */
	PIOS_RTC_Init();
}

/**
* Get the value of an input channel
* \param[in] Channel Number of the channel desired
* \output -1 Channel not available
* \output >0 Channel value
*/
static int32_t PIOS_SPEKTRUM_Get(uint32_t chan_id)
{
	/* Return error if channel not available */
	if (chan_id >= PIOS_SPEKTRUM_NUM_INPUTS) {
		return -1;
	}
	return CaptureValue[chan_id];
}

/**
* Spektrum bind function
* \output 1 Successful bind
* \output 0 Bind failed
* \note Applications shouldn't call these functions directly
*/
uint8_t PIOS_SPEKTRUM_Bind(void)
{
	GPIO_InitTypeDef GPIO_InitStructure = pios_spektrum_cfg.gpio_init;
	GPIO_InitStructure.GPIO_Pin = pios_spektrum_cfg.pin;
	GPIO_Init(pios_spektrum_cfg.port, &GPIO_InitStructure);

	pios_spektrum_cfg.port->BRR = pios_spektrum_cfg.pin;
	//PIOS_DELAY_WaitmS(75);
	/* RX line, drive high for 10us */
	pios_spektrum_cfg.port->BSRR = pios_spektrum_cfg.pin;
	PIOS_DELAY_WaituS(10);
	/* RX line, drive low for 120us */
	pios_spektrum_cfg.port->BRR = pios_spektrum_cfg.pin;
	PIOS_DELAY_WaituS(120);
	/* RX line, drive high for 120us */
	pios_spektrum_cfg.port->BSRR = pios_spektrum_cfg.pin;
	PIOS_DELAY_WaituS(120);
	/* RX line, drive low for 120us */
	pios_spektrum_cfg.port->BRR = pios_spektrum_cfg.pin;
	PIOS_DELAY_WaituS(120);
	/* RX line, drive high for 120us */
	pios_spektrum_cfg.port->BSRR = pios_spektrum_cfg.pin;
	PIOS_DELAY_WaituS(120);
	/* RX line, drive low for 120us */
	pios_spektrum_cfg.port->BRR = pios_spektrum_cfg.pin;
	PIOS_DELAY_WaituS(120);
	/* RX line, drive high for 120us */
	pios_spektrum_cfg.port->BSRR = pios_spektrum_cfg.pin;
	PIOS_DELAY_WaituS(120);
	/* RX line, drive low for 120us */
	pios_spektrum_cfg.port->BRR = pios_spektrum_cfg.pin;
	PIOS_DELAY_WaituS(120);
	/* RX line, drive high for 120us */
	pios_spektrum_cfg.port->BSRR = pios_spektrum_cfg.pin;
	PIOS_DELAY_WaituS(120);
	/* RX line, set input and wait for data, PIOS_SPEKTRUM_Init */

	return 1;
}

/**
* Decodes a byte
* \param[in] b byte which should be spektrum decoded
* \return 0 if no error
* \return -1 if USART not available
* \return -2 if buffer full (retry)
* \note Applications shouldn't call these functions directly
*/
int32_t PIOS_SPEKTRUM_Decode(uint8_t b)
{
	static uint16_t channel = 0; /*, sync_word = 0;*/
	uint8_t channeln = 0, frame = 0;
	uint16_t data = 0;
	byte_array[bytecount] = b;
	bytecount++;
	if (sync == 0) {
		//sync_word = (prev_byte << 8) + b;
#if 0
		/* maybe create object to show this  data */
		if(bytecount==1)
		{
			/* record losscounter into channel8 */
			CaptureValueTemp[7]=b;
			/* instant write */
			CaptureValue[7]=b;
		}
#endif
		/* Known sync bytes, 0x01, 0x02, 0x12 */
		if (bytecount == 2) {
			if (b == 0x01) {
				datalength=0; // 10bit
				//frames=1;
				sync = 1;
				bytecount = 2;
			}
			else if(b == 0x02) {
				datalength=0; // 10bit
				//frames=2;
				sync = 1;
				bytecount = 2;
			}
			else if(b == 0x12) {
				datalength=1; // 11bit
				//frames=2;
				sync = 1;
				bytecount = 2;
			}
			else
			{
				bytecount = 0;
			}
		}
	} else {
		if ((bytecount % 2) == 0) {
			channel = (prev_byte << 8) + b;
			frame = channel >> 15;
			channeln = (channel >> (10+datalength)) & 0x0F;
			data = channel & (0x03FF+(0x0400*datalength));
			if(channeln==0 && data<10) // discard frame if throttle misbehaves
			{
				frame_error=1;
			}
			if (channeln < PIOS_SPEKTRUM_NUM_INPUTS && !frame_error)
				CaptureValueTemp[channeln] = data;
		}
	}
	if (bytecount == 16) {
		//PIOS_COM_SendBufferNonBlocking(PIOS_COM_TELEM_RF,byte_array,16); //00 2c 58 84 b0 dc ff
		bytecount = 0;
		sync = 0;
		sync_of = 0;
		if (!frame_error)
		{
			for(int i=0;i<PIOS_SPEKTRUM_NUM_INPUTS;i++)
			{
				CaptureValue[i] = CaptureValueTemp[i];
			}
		}
		frame_error=0;
	}
	prev_byte = b;
	return 0;
}

/* Interrupt handler for USART */
void PIOS_SPEKTRUM_irq_handler(uint32_t usart_id) {
	/* by always reading DR after SR make sure to clear any error interrupts */
	volatile uint16_t sr = pios_spektrum_cfg.pios_usart_spektrum_cfg->regs->SR;
	volatile uint8_t b = pios_spektrum_cfg.pios_usart_spektrum_cfg->regs->DR;
	
	/* check if RXNE flag is set */
	if (sr & USART_SR_RXNE) {
		if (PIOS_SPEKTRUM_Decode(b) < 0) {
			/* Here we could add some error handling */
		}
	} 

	if (sr & USART_SR_TXE) {	// check if TXE flag is set
		/* Disable TXE interrupt (TXEIE=0) */
		USART_ITConfig(pios_spektrum_cfg.pios_usart_spektrum_cfg->regs, USART_IT_TXE, DISABLE);
	}
	/* byte arrived so clear "watchdog" timer */
	supv_timer=0;
}

/**
 *@brief This function is called between frames and when a spektrum word hasnt been decoded for too long
 *@brief clears the channel values
 */
void PIOS_SPEKTRUMSV_irq_handler() {
	/* 125hz */
	supv_timer++;
	if(supv_timer > 5) {
		/* sync between frames */
		sync = 0;
		bytecount = 0;
		prev_byte = 0xFF;
		frame_error = 0;
		sync_of++;
		/* watchdog activated after 100ms silence */
		if (sync_of > 12) {
			/* signal lost */
			sync_of = 0;
			for (int i = 0; i < PIOS_SPEKTRUM_NUM_INPUTS; i++) {
				CaptureValue[i] = 0;
				CaptureValueTemp[i] = 0;
			}
		}
		supv_timer = 0;
	}
}

#endif

/** 
  * @}
  * @}
  */
