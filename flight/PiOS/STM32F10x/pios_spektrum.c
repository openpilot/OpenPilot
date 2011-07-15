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

/**
 * @Note Framesyncing:
 * The code resets the watchdog timer whenever a single byte is received, so what watchdog code
 * is never called if regularly getting bytes.
 * RTC timer is running @625Hz, supervisor timer has divider 5 so frame sync comes every 1/125Hz=8ms.
 * Good for both 11ms and 22ms framecycles
 */

/* Global Variables */

/* Provide a RCVR driver */
static int32_t PIOS_SPEKTRUM_Get(uint32_t rcvr_id, uint8_t channel);

const struct pios_rcvr_driver pios_spektrum_rcvr_driver = {
	.read = PIOS_SPEKTRUM_Get,
};

/* Local Variables */
static uint16_t CaptureValue[PIOS_SPEKTRUM_NUM_INPUTS],CaptureValueTemp[PIOS_SPEKTRUM_NUM_INPUTS];
static uint8_t prev_byte = 0xFF, sync = 0, bytecount = 0, datalength=0, frame_error=0, byte_array[20] = { 0 };
uint8_t sync_of = 0;
uint16_t supv_timer=0;

static void PIOS_SPEKTRUM_Supervisor(uint32_t spektrum_id);
static bool PIOS_SPEKTRUM_Bind(const struct pios_spektrum_cfg * cfg);

/**
* Bind and Initialise Spektrum satellite receiver
*/
void PIOS_SPEKTRUM_Init(const struct pios_spektrum_cfg * cfg, bool bind)
{
	// TODO: need setting flag for bind on next powerup
	if (bind) {
		PIOS_SPEKTRUM_Bind(cfg);
	}

	if (!PIOS_RTC_RegisterTickCallback(PIOS_SPEKTRUM_Supervisor, 0)) {
		PIOS_DEBUG_Assert(0);
	}
}

/**
* Get the value of an input channel
* \param[in] Channel Number of the channel desired
* \output -1 Channel not available
* \output >0 Channel value
*/
static int32_t PIOS_SPEKTRUM_Get(uint32_t rcvr_id, uint8_t channel)
{
	/* Return error if channel not available */
	if (channel >= PIOS_SPEKTRUM_NUM_INPUTS) {
		return -1;
	}
	return CaptureValue[channel];
}

/**
* Spektrum bind function
* \output true Successful bind
* \output false Bind failed
*/
static bool PIOS_SPEKTRUM_Bind(const struct pios_spektrum_cfg * cfg)
{
	GPIO_Init(cfg->bind.gpio, &cfg->bind.init);

	GPIO_ResetBits(cfg->bind.gpio, cfg->bind.init.GPIO_Pin);
	//PIOS_DELAY_WaitmS(75);
	/* RX line, drive high for 10us */
	GPIO_SetBits(cfg->bind.gpio, cfg->bind.init.GPIO_Pin);
	PIOS_DELAY_WaituS(10);
	/* RX line, drive low for 120us */
	GPIO_ResetBits(cfg->bind.gpio, cfg->bind.init.GPIO_Pin);
	PIOS_DELAY_WaituS(120);
	/* RX line, drive high for 120us */
	GPIO_SetBits(cfg->bind.gpio, cfg->bind.init.GPIO_Pin);
	PIOS_DELAY_WaituS(120);
	/* RX line, drive low for 120us */
	GPIO_ResetBits(cfg->bind.gpio, cfg->bind.init.GPIO_Pin);
	PIOS_DELAY_WaituS(120);
	/* RX line, drive high for 120us */
	GPIO_SetBits(cfg->bind.gpio, cfg->bind.init.GPIO_Pin);
	PIOS_DELAY_WaituS(120);
	/* RX line, drive low for 120us */
	GPIO_ResetBits(cfg->bind.gpio, cfg->bind.init.GPIO_Pin);
	PIOS_DELAY_WaituS(120);
	/* RX line, drive high for 120us */
	GPIO_SetBits(cfg->bind.gpio, cfg->bind.init.GPIO_Pin);
	PIOS_DELAY_WaituS(120);
	/* RX line, drive low for 120us */
	GPIO_ResetBits(cfg->bind.gpio, cfg->bind.init.GPIO_Pin);
	PIOS_DELAY_WaituS(120);
	/* RX line, drive high for 120us */
	GPIO_SetBits(cfg->bind.gpio, cfg->bind.init.GPIO_Pin);
	PIOS_DELAY_WaituS(120);
	/* RX line, set input and wait for data, PIOS_SPEKTRUM_Init */

	return true;
}

/**
* Decodes a byte
* \param[in] b byte which should be spektrum decoded
* \return 0 if no error
* \return -1 if USART not available
* \return -2 if buffer full (retry)
* \note Applications shouldn't call these functions directly
*/
static int32_t PIOS_SPEKTRUM_Decode(uint8_t b)
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

/* Custom interrupt handler for USART */
void PIOS_SPEKTRUM_irq_handler(uint32_t usart_id) {
	/* Grab the config for this device from the underlying USART device */
	const struct pios_usart_cfg * cfg;
	cfg = PIOS_USART_GetConfig(usart_id);
	PIOS_Assert(cfg);
  
	/* by always reading DR after SR make sure to clear any error interrupts */
	volatile uint16_t sr = cfg->regs->SR;
	volatile uint8_t b = cfg->regs->DR;
	
	/* check if RXNE flag is set */
	if (sr & USART_SR_RXNE) {
		if (PIOS_SPEKTRUM_Decode(b) < 0) {
			/* Here we could add some error handling */
		}
	} 

	if (sr & USART_SR_TXE) {	// check if TXE flag is set
		/* Disable TXE interrupt (TXEIE=0) */
		USART_ITConfig(cfg->regs, USART_IT_TXE, DISABLE);
	}
	/* byte arrived so clear "watchdog" timer */
	supv_timer=0;
}

/**
 *@brief This function is called between frames and when a spektrum word hasnt been decoded for too long
 *@brief clears the channel values
 */
static void PIOS_SPEKTRUM_Supervisor(uint32_t spektrum_id) {
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
