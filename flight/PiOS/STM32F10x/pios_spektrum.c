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

#if defined(PIOS_INCLUDE_SPEKTRUM)

#include "pios_spektrum_priv.h"

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

enum pios_spektrum_dev_magic {
	PIOS_SPEKTRUM_DEV_MAGIC = 0xa9b9c9d9,
};

struct pios_spektrum_fsm {
	uint16_t channel;
	uint16_t CaptureValue[PIOS_SPEKTRUM_NUM_INPUTS];
	uint16_t CaptureValueTemp[PIOS_SPEKTRUM_NUM_INPUTS];
	uint8_t prev_byte;
	uint8_t sync;
	uint8_t bytecount;
	uint8_t datalength;
	uint8_t frame_error;
	uint8_t sync_of;
};

struct pios_spektrum_dev {
	enum pios_spektrum_dev_magic     magic;
	const struct pios_spektrum_cfg * cfg;

	struct pios_spektrum_fsm fsm;

	uint16_t supv_timer;
};

static bool PIOS_SPEKTRUM_validate(struct pios_spektrum_dev * spektrum_dev)
{
	return (spektrum_dev->magic == PIOS_SPEKTRUM_DEV_MAGIC);
}

#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_spektrum_dev * PIOS_SPEKTRUM_alloc(void)
{
	struct pios_spektrum_dev * spektrum_dev;

	spektrum_dev = (struct pios_spektrum_dev *)pvPortMalloc(sizeof(*spektrum_dev));
	if (!spektrum_dev) return(NULL);

	spektrum_dev->magic = PIOS_SPEKTRUM_DEV_MAGIC;
	return(spektrum_dev);
}
#else
static struct pios_spektrum_dev pios_spektrum_devs[PIOS_SPEKTRUM_MAX_DEVS];
static uint8_t pios_spektrum_num_devs;
static struct pios_spektrum_dev * PIOS_SPEKTRUM_alloc(void)
{
	struct pios_spektrum_dev * spektrum_dev;

	if (pios_spektrum_num_devs >= PIOS_SPEKTRUM_MAX_DEVS) {
		return (NULL);
	}

	spektrum_dev = &pios_spektrum_devs[pios_spektrum_num_devs++];
	spektrum_dev->magic = PIOS_SPEKTRUM_DEV_MAGIC;

	return (spektrum_dev);
}
#endif

static void PIOS_SPEKTRUM_Supervisor(uint32_t spektrum_id);
static bool PIOS_SPEKTRUM_Bind(const struct pios_spektrum_cfg * cfg, uint8_t bind);
static int32_t PIOS_SPEKTRUM_UpdateFSM(struct pios_spektrum_fsm * fsm, uint8_t b);

static uint16_t PIOS_SPEKTRUM_RxInCallback(uint32_t context, uint8_t * buf, uint16_t buf_len, uint16_t * headroom, bool * need_yield)
{
	struct pios_spektrum_dev * spektrum_dev = (struct pios_spektrum_dev *)context;

	bool valid = PIOS_SPEKTRUM_validate(spektrum_dev);
	PIOS_Assert(valid);

	/* process byte(s) and clear receive timer */
	for (uint8_t i = 0; i < buf_len; i++) {
		PIOS_SPEKTRUM_UpdateFSM(&(spektrum_dev->fsm), buf[i]);
		spektrum_dev->supv_timer = 0;
	}

	/* Always signal that we can accept another byte */
	if (headroom) {
		*headroom = 1;
	}

	/* We never need a yield */
	*need_yield = false;

	/* Always indicate that all bytes were consumed */
	return (buf_len);
}

static void PIOS_SPEKTRUM_ResetFSM(struct pios_spektrum_fsm * fsm)
{
	fsm->channel = 0;
	fsm->prev_byte = 0xFF;
	fsm->sync = 0;
	fsm->bytecount = 0;
	fsm->datalength = 0;
	fsm->frame_error = 0;
	fsm->sync_of = 0;
}

/**
* Decodes a byte
* \param[in] b byte which should be spektrum decoded
* \return 0 if no error
* \return -1 if USART not available
* \return -2 if buffer full (retry)
*/
static int32_t PIOS_SPEKTRUM_UpdateFSM(struct pios_spektrum_fsm * fsm, uint8_t b)
{
	fsm->bytecount++;
	if (fsm->sync == 0) {
		/* Known sync bytes, 0x01, 0x02, 0x12, 0xb2 */
		/* 0xb2 DX8 3bind pulses only */
		if (fsm->bytecount == 2) {
			if ((b == 0x01) || (b == 0x02) || (b == 0xb2)) {
				fsm->datalength=0; // 10bit
				fsm->sync = 1;
				fsm->bytecount = 2;
			}
			else if(b == 0x12) {
				fsm->datalength=1; // 11bit
				fsm->sync = 1;
				fsm->bytecount = 2;
			}
			else
			{
				fsm->bytecount = 0;
			}
		}
	} else {
		if ((fsm->bytecount % 2) == 0) {
			uint16_t data;
			uint8_t channeln;

			fsm->channel = (fsm->prev_byte << 8) + b;
			channeln = (fsm->channel >> (10+fsm->datalength)) & 0x0F;
			data = fsm->channel & (0x03FF+(0x0400*fsm->datalength));
			if(channeln==0 && data<10) // discard frame if throttle misbehaves
			{
				fsm->frame_error=1;
			}
			if (channeln < PIOS_SPEKTRUM_NUM_INPUTS && !fsm->frame_error)
				fsm->CaptureValueTemp[channeln] = data;
		}
	}
	if (fsm->bytecount == 16) {
		fsm->bytecount = 0;
		fsm->sync = 0;
		fsm->sync_of = 0;
		if (!fsm->frame_error)
		{
			for(int i=0;i<PIOS_SPEKTRUM_NUM_INPUTS;i++)
			{
				fsm->CaptureValue[i] = fsm->CaptureValueTemp[i];
			}
		}
		fsm->frame_error=0;
	}
	fsm->prev_byte = b;
	return 0;
}

/**
* Bind and Initialise Spektrum satellite receiver
*/
int32_t PIOS_SPEKTRUM_Init(uint32_t * spektrum_id, const struct pios_spektrum_cfg *cfg, const struct pios_com_driver * driver, uint32_t lower_id, uint8_t bind)
{
	PIOS_DEBUG_Assert(spektrum_id);
	PIOS_DEBUG_Assert(cfg);
	PIOS_DEBUG_Assert(driver);

	struct pios_spektrum_dev * spektrum_dev;

	spektrum_dev = (struct pios_spektrum_dev *) PIOS_SPEKTRUM_alloc();
	if (!spektrum_dev) goto out_fail;

	/* Bind the configuration to the device instance */
	spektrum_dev->cfg = cfg;

	if (bind) {
		PIOS_SPEKTRUM_Bind(cfg,bind);
	}

	PIOS_SPEKTRUM_ResetFSM(&(spektrum_dev->fsm));

	*spektrum_id = (uint32_t)spektrum_dev;

	(driver->bind_rx_cb)(lower_id, PIOS_SPEKTRUM_RxInCallback, *spektrum_id);

	if (!PIOS_RTC_RegisterTickCallback(PIOS_SPEKTRUM_Supervisor, *spektrum_id)) {
		PIOS_DEBUG_Assert(0);
	}

	return (0);

out_fail:
	return(-1);
}

/**
* Get the value of an input channel
* \param[in] Channel Number of the channel desired
* \output -1 Channel not available
* \output >0 Channel value
*/
static int32_t PIOS_SPEKTRUM_Get(uint32_t rcvr_id, uint8_t channel)
{
	struct pios_spektrum_dev * spektrum_dev = (struct pios_spektrum_dev *)rcvr_id;

	if(!PIOS_SPEKTRUM_validate(spektrum_dev))
		return PIOS_RCVR_INVALID;

	/* Return error if channel not available */
	if (channel >= PIOS_SPEKTRUM_NUM_INPUTS) {
		return PIOS_RCVR_INVALID;
	}
	return spektrum_dev->fsm.CaptureValue[channel];
}

/**
* Spektrum bind function
* \output true Successful bind
* \output false Bind failed
*/
static bool PIOS_SPEKTRUM_Bind(const struct pios_spektrum_cfg * cfg, uint8_t bind)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = cfg->bind.init.GPIO_Pin;
	GPIO_InitStructure.GPIO_Speed = cfg->bind.init.GPIO_Speed;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;

	/* just to limit bind pulses */
	bind=(bind<=10)?bind:10;

	GPIO_Init(cfg->bind.gpio, &cfg->bind.init);
	/* RX line, set high */
	GPIO_SetBits(cfg->bind.gpio, cfg->bind.init.GPIO_Pin);

	/* on CC works upto 140ms, I guess bind window is around 20-140ms after powerup */
	PIOS_DELAY_WaitmS(60);

	for (int i = 0; i < bind ; i++) {
		/* RX line, drive low for 120us */
		GPIO_ResetBits(cfg->bind.gpio, cfg->bind.init.GPIO_Pin);
		PIOS_DELAY_WaituS(120);
		/* RX line, drive high for 120us */
		GPIO_SetBits(cfg->bind.gpio, cfg->bind.init.GPIO_Pin);
		PIOS_DELAY_WaituS(120);
	}
	/* RX line, set input and wait for data, PIOS_SPEKTRUM_Init */
	GPIO_Init(cfg->bind.gpio, &GPIO_InitStructure);

	return true;
}

/**
 *@brief This function is called between frames and when a spektrum word hasnt been decoded for too long
 *@brief clears the channel values
 */
static void PIOS_SPEKTRUM_Supervisor(uint32_t spektrum_id)
{
	struct pios_spektrum_dev * spektrum_dev = (struct pios_spektrum_dev *)spektrum_id;

	bool valid = PIOS_SPEKTRUM_validate(spektrum_dev);
	PIOS_Assert(valid);

	/* 625hz */
	spektrum_dev->supv_timer++;
	if(spektrum_dev->supv_timer > 4) {
		/* sync between frames */
		struct pios_spektrum_fsm * fsm = &(spektrum_dev->fsm);

		fsm->sync = 0;
		fsm->bytecount = 0;
		fsm->prev_byte = 0xFF;
		fsm->frame_error = 0;
		fsm->sync_of++;
		/* watchdog activated after 200ms silence */
		if (fsm->sync_of > 30) {

			/* signal lost */
			fsm->sync_of = 0;
			for (int i = 0; i < PIOS_SPEKTRUM_NUM_INPUTS; i++) {
				fsm->CaptureValue[i] = PIOS_RCVR_TIMEOUT;
				fsm->CaptureValueTemp[i] = PIOS_RCVR_TIMEOUT;
			}
		}
		spektrum_dev->supv_timer = 0;
	}
}

#endif

/** 
  * @}
  * @}
  */
