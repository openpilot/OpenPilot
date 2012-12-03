/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_SERVO RC Servo Functions
 * @brief Code to do set RC servo output
 * @{
 *
 * @file       pios_servo.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      RC Servo routines (STM32 dependent)
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
#include "pios_stepper_priv.h"
#include "pios_tim_priv.h"
#include "steppercommand.h"

/* Private Function Prototypes */

struct pios_stepper_out_dev {
	const struct pios_stepper_out_cfg * cfg;
	uint32_t Channel[3];//3 stepper
	int32_t StepRemaining[3];//3 first axes
	uint32_t OldPosition[3];//3 first axes
	uint32_t OldServoPosition[3];//3 first axes
	bool Direction[3];
 	uint32_t SpeedHz[3];
 	bool Enable[3];
 	bool Start[3];
 	bool End[3];
 	bool Set;
 	 	        /*uint32_t end_period;
 	 	        int8_t NumChannels;
 	 	        uint8_t NumChannelCounter;
 	 	        uint32_t ChannelValue[PIOS_PPM_OUT_MAX_CHANNELS];

 	 	        uint8_t supv_timer;
 	 	        bool Tracking;
 	 	        bool Fresh;*/
	};

static struct pios_stepper_out_dev * servo_cfg;

static struct pios_stepper_out_dev * PIOS_STEPPER_OUT_alloc(void)
{
	struct pios_stepper_out_dev * stepper_dev;

	stepper_dev = (struct pios_stepper_out_dev *)pvPortMalloc(sizeof(*stepper_dev));
	if (!stepper_dev) return(NULL);

	//ppm_dev->magic = PIOS_PPM_DEV_MAGIC;
	return(stepper_dev);
}


static struct pios_stepper_out_dev pios_stepper_out_devs[3];
static uint8_t pios_stepper_out_num_devs;
static struct pios_stepper_out_dev * PIOS_STEPPER_alloc(void)
{
	struct pios_stepper_out_dev * stepper_dev;

	if (pios_stepper_out_num_devs >= 7) {
		return (NULL);
	}

	stepper_dev = &pios_stepper_out_devs[pios_stepper_out_num_devs++];
	//ppm_dev->magic = PIOS_PPM_DEV_MAGIC;

	return (stepper_dev);
}

static void PIOS_STEPPER_tim_edge_cb (uint32_t id, uint32_t context, uint8_t channel, uint16_t count);
const static struct pios_tim_callbacks tim_callbacks = {
	.overflow = NULL,
	.edge     = PIOS_STEPPER_tim_edge_cb,
};


/**
* Initialise Stepper
*/
int32_t PIOS_STEPPER_Out_Init(uint32_t * pios_stepper_id, const struct pios_stepper_out_cfg * cfg)
{
	StepperCommandInitialize();
	PIOS_DEBUG_Assert(pios_stepper_id);
	PIOS_DEBUG_Assert(cfg);

	struct pios_stepper_out_dev * stepper_dev;

	stepper_dev = (struct pios_stepper_out_dev *) PIOS_STEPPER_OUT_alloc();
	if (!stepper_dev) goto out_fail;

	/* Bind the configuration to the device instance */
	stepper_dev->cfg = cfg;

	 	 	                /* Set up the state variables */
	stepper_dev->Channel[0]=0;
	stepper_dev->Channel[1]=0;
	stepper_dev->Channel[2]=0;
	stepper_dev->SpeedHz[0]=1200;
	stepper_dev->SpeedHz[1]=1200;
	stepper_dev->SpeedHz[2]=1200;
	stepper_dev->StepRemaining[0]=0;
	stepper_dev->StepRemaining[1]=0;
	stepper_dev->StepRemaining[2]=0;
	stepper_dev->Enable[0]=1;
	stepper_dev->Enable[1]=1;
	stepper_dev->Enable[2]=1;
	stepper_dev->OldPosition[0]=0;
	stepper_dev->OldPosition[1]=0;
	stepper_dev->OldPosition[2]=0;
	stepper_dev->OldServoPosition[0]=0;
	stepper_dev->OldServoPosition[1]=0;
	stepper_dev->OldServoPosition[2]=0;
	stepper_dev->Start[0]=0;
	stepper_dev->Start[1]=0;
	stepper_dev->Start[2]=0;
	stepper_dev->End[0]=0;
	stepper_dev->End[1]=0;
	stepper_dev->End[2]=0;

	uint32_t tim_id;

	if (PIOS_TIM_InitChannels(&tim_id, cfg->channels, cfg->num_channels, &tim_callbacks, (uint32_t)stepper_dev)) {
		return -1;
	}

	/* Configure the channels to be in output compare mode */
	for (uint8_t i = 0; i < cfg->num_channels; i++) {
		const struct pios_tim_channel * chan = &cfg->channels[i];

		/* Set up for output compare function */
		switch(chan->timer_chan) {
		case TIM_Channel_1:
			TIM_OC1Init(chan->timer, &cfg->tim_oc_init);
			TIM_OC1PreloadConfig(chan->timer, TIM_OCPreload_Enable);
			TIM_ITConfig(chan->timer, TIM_IT_CC1 | TIM_IT_Update, ENABLE);
			break;
		case TIM_Channel_2:
			TIM_OC2Init(chan->timer, &cfg->tim_oc_init);
			TIM_OC2PreloadConfig(chan->timer, TIM_OCPreload_Enable);
			TIM_ITConfig(chan->timer, TIM_IT_CC2 | TIM_IT_Update, ENABLE);
			break;
		case TIM_Channel_3:
			TIM_OC3Init(chan->timer, &cfg->tim_oc_init);
			TIM_OC3PreloadConfig(chan->timer, TIM_OCPreload_Enable);
			TIM_ITConfig(chan->timer, TIM_IT_CC3 | TIM_IT_Update, ENABLE);
			break;
		case TIM_Channel_4:
			TIM_OC4Init(chan->timer, &cfg->tim_oc_init);
			TIM_OC4PreloadConfig(chan->timer, TIM_OCPreload_Enable);
			TIM_ITConfig(chan->timer, TIM_IT_CC4 | TIM_IT_Update, ENABLE);
			break;
		}

		TIM_ARRPreloadConfig(chan->timer, ENABLE);
		TIM_CtrlPWMOutputs(chan->timer, ENABLE);
		TIM_Cmd(chan->timer, ENABLE);
	}

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = cfg->tim_base_init;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1;

	for(uint8_t i = 0; (i < 3) ; i++) {
		const struct pios_tim_channel * chan = &cfg->channels[i];

		TIM_TimeBaseStructure.TIM_Period = ((1000000 / stepper_dev->SpeedHz[i]) - 1);
		TIM_TimeBaseInit(chan->timer, &TIM_TimeBaseStructure);

		switch(chan->timer_chan) {
		case TIM_Channel_1:
			TIM_SetCompare1(chan->timer, stepper_dev->Channel[i]);
			break;
		case TIM_Channel_2:
			TIM_SetCompare2(chan->timer, stepper_dev->Channel[i]);
			break;
		case TIM_Channel_3:
			TIM_SetCompare3(chan->timer, stepper_dev->Channel[i]);
			break;
		case TIM_Channel_4:
			TIM_SetCompare4(chan->timer, stepper_dev->Channel[i]);
			break;
		}

		GPIO_InitTypeDef GPIO_InitStructure;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
		GPIO_InitStructure.GPIO_Pin = stepper_dev->cfg->directions[i].pin.init.GPIO_Pin;
		GPIO_Init(stepper_dev->cfg->directions[i].pin.gpio, &GPIO_InitStructure);

			/* GPIO's Off */
		stepper_dev->cfg->directions[i].pin.gpio->BRR = stepper_dev->cfg->directions[i].pin.init.GPIO_Pin;
	}
	stepper_dev->Set=1;
	/* Store away the requested configuration */
			servo_cfg = stepper_dev;
	return 0;
	out_fail:
	return(-1);
}

/**
* Set the servo update rate (Max 500Hz)
* \param[in] array of rates in Hz
* \param[in] maximum number of banks
*/
void PIOS_Stepper_SetHz(const uint16_t * speeds, uint8_t banks)
{
/*	if (!servo_cfg) {
		return;
	}

	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure = servo_cfg->tim_base_init;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1;

	uint8_t set = 0;

	for(uint8_t i = 0; (i < servo_cfg->num_channels) && (set < banks); i++) {
		bool new = true;
		const struct pios_tim_channel * chan = &servo_cfg->channels[i];

		for(uint8_t j = 0; (j < i) && new; j++)
			new &= chan->timer != servo_cfg->channels[j].timer;

		if(new) {
			TIM_TimeBaseStructure.TIM_Period = ((1000000 / speeds[set]) - 1);
			TIM_TimeBaseInit(chan->timer, &TIM_TimeBaseStructure);
			set++;
		}
	}*/
}

/**
* Set servo position
* \param[in] Servo Servo number (0-7)
* \param[in] Position Servo position in microseconds
*/
void PIOS_Stepper_Set(uint8_t servo, uint16_t position)
{
	int32_t remain;
	servo--;
	if(servo<3 && servo_cfg->Start[servo]==0)
	{

		remain=position-servo_cfg->OldServoPosition[servo];//-servo_cfg->OldPosition[servo];
		if(remain!=0)
			{
			//if(servo_cfg->Enable[servo]==0)
			{
				const struct pios_tim_channel * chan = &servo_cfg->cfg->channels[servo];
				switch(chan->timer_chan) {
				case TIM_Channel_1:
					TIM_SetCompare1(chan->timer, 200);
					break;
				case TIM_Channel_2:
					TIM_SetCompare2(chan->timer, 200);
					break;
				case TIM_Channel_3:
					TIM_SetCompare3(chan->timer, 200);
					break;
				case TIM_Channel_4:
					TIM_SetCompare4(chan->timer, 200);
					break;
				}
				servo_cfg->Enable[servo]=1;
			}
			if(remain<=0)
			{
				servo_cfg->StepRemaining[servo]=-remain;
				servo_cfg->Direction[servo]=0;
				servo_cfg->cfg->directions[servo].pin.gpio->BRR = servo_cfg->cfg->directions[servo].pin.init.GPIO_Pin;
			}
			else
			{
				servo_cfg->StepRemaining[servo]=remain;
				servo_cfg->Direction[servo]=1;
				servo_cfg->cfg->directions[servo].pin.gpio->BSRR = servo_cfg->cfg->directions[servo].pin.init.GPIO_Pin;
				//GPIO_ResetBits(servo_cfg->cfg->directions[servo].pin.gpio, servo_cfg->cfg->directions[servo].pin.init.GPIO_Pin);
			}
			StepperCommandChannelSet((int32_t*)servo_cfg->StepRemaining);
			servo_cfg->Start[servo]=1;
			}
		servo_cfg->OldServoPosition[servo]=position;
	}
}

static void PIOS_STEPPER_tim_edge_cb (uint32_t tim_id, uint32_t context, uint8_t chan_idx, uint16_t count)
{
	/* Recover our device context */
	//struct pios_stepper_out_dev * stepper_dev = (struct pios_stepper_out_dev *)context;
	//switch(chan_idx)
	//chan_idx=0;
	if(servo_cfg->Start[chan_idx]==1)
	{
		if(servo_cfg->StepRemaining[chan_idx]>=0)
		{
			/*if(servo_cfg->Direction==1)
			{
				servo_cfg->OldPosition[chan_idx]++;
			}
			else
			{
				servo_cfg->OldPosition[chan_idx]--;
			}*/
			servo_cfg->StepRemaining[chan_idx]--;
		}
		else
		{
			//if(servo_cfg->Enable[chan_idx]==1)
			{
			const struct pios_tim_channel * chan = &servo_cfg->cfg->channels[chan_idx];
			//TIM_Cmd(chan->timer, DISABLE);
			// Update the position
				switch(chan->timer_chan) {
					case TIM_Channel_1:
						TIM_SetCompare1(chan->timer, 0);
						break;
					case TIM_Channel_2:
						TIM_SetCompare2(chan->timer, 0);
						break;
					case TIM_Channel_3:
						TIM_SetCompare3(chan->timer, 0);
						break;
					case TIM_Channel_4:
						TIM_SetCompare4(chan->timer, 0);
						break;
				}
			servo_cfg->Start[chan_idx]=0;
			//servo_cfg->Enable[chan_idx]=0;
			}
		}
	}
}
