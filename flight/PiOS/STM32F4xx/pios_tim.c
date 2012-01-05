/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_TIM Timer Functions
 * @brief PIOS Timer control code
 * @{
 *
 * @file       pios_tim.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Sets up timers and ways to register callbacks on them
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


#include "pios.h"

#include "pios_tim.h"
#include "pios_tim_priv.h"

enum pios_tim_dev_magic {
	PIOS_TIM_DEV_MAGIC = 0x87654098,
};

struct pios_tim_dev {
	enum pios_tim_dev_magic     magic;

	const struct pios_tim_channel * channels;
	uint8_t num_channels;

	const struct pios_tim_callbacks * callbacks;
	uint32_t context;
};

#if 0
static bool PIOS_TIM_validate(struct pios_tim_dev * tim_dev)
{
	return (tim_dev->magic == PIOS_TIM_DEV_MAGIC);
}
#endif

#if defined(PIOS_INCLUDE_FREERTOS) && 0
static struct pios_tim_dev * PIOS_TIM_alloc(void)
{
	struct pios_tim_dev * tim_dev;

	tim_dev = (struct pios_tim_dev *)malloc(sizeof(*tim_dev));
	if (!tim_dev) return(NULL);

	tim_dev->magic = PIOS_TIM_DEV_MAGIC;
	return(tim_dev);
}
#else
static struct pios_tim_dev pios_tim_devs[PIOS_TIM_MAX_DEVS];
static uint8_t pios_tim_num_devs;
static struct pios_tim_dev * PIOS_TIM_alloc(void)
{
	struct pios_tim_dev * tim_dev;

	if (pios_tim_num_devs >= PIOS_TIM_MAX_DEVS) {
		return (NULL);
	}

	tim_dev = &pios_tim_devs[pios_tim_num_devs++];
	tim_dev->magic = PIOS_TIM_DEV_MAGIC;

	return (tim_dev);
}
#endif




int32_t PIOS_TIM_InitClock(const struct pios_tim_clock_cfg * cfg)
{
	PIOS_DEBUG_Assert(cfg);

	/* Enable appropriate clock to timer module */
	switch((uint32_t) cfg->timer) {
		case (uint32_t)TIM1:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
			break;
		case (uint32_t)TIM2:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
			break;
		case (uint32_t)TIM3:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
			break;
		case (uint32_t)TIM4:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
			break;
#ifdef STM32F10X_HD
		case (uint32_t)TIM5:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
			break;
		case (uint32_t)TIM6:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
			break;
		case (uint32_t)TIM7:
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
			break;
		case (uint32_t)TIM8:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);
			break;
		case (uint32_t)TIM9:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);
			break;
		case (uint32_t)TIM10:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, ENABLE);
			break;
		case (uint32_t)TIM11:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM11, ENABLE);
			break;
#endif
	}

	/* Configure the dividers for this timer */
	TIM_TimeBaseInit(cfg->timer, cfg->time_base_init);

	/* Configure internal timer clocks */
	TIM_InternalClockConfig(cfg->timer);

	/* Enable timers */
	TIM_Cmd(cfg->timer, ENABLE);

	/* Enable Interrupts */
	NVIC_Init(&cfg->irq.init);

	return 0;
}

int32_t PIOS_TIM_InitChannels(uint32_t * tim_id, const struct pios_tim_channel * channels, uint8_t num_channels, const struct pios_tim_callbacks * callbacks, uint32_t context)
{
	PIOS_Assert(channels);
	PIOS_Assert(num_channels);

	struct pios_tim_dev * tim_dev;
	tim_dev = (struct pios_tim_dev *) PIOS_TIM_alloc();
	if (!tim_dev) goto out_fail;

	/* Bind the configuration to the device instance */
	tim_dev->channels = channels;
	tim_dev->num_channels = num_channels;
	tim_dev->callbacks = callbacks;
	tim_dev->context = context;

	/* Configure the pins */
	for (uint8_t i = 0; i < num_channels; i++) {
		const struct pios_tim_channel * chan = &(channels[i]);

		/* Enable the peripheral clock for the GPIO */
/*		switch ((uint32_t)chan->pin.gpio) {
		case (uint32_t) GPIOA:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			break;
		case (uint32_t) GPIOB:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
			break;
		case (uint32_t) GPIOC:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
			break;
		default:
			PIOS_Assert(0);
			break;
		}
*/ // commented out for now as f4 starts all clocks
		GPIO_Init(chan->pin.gpio, &chan->pin.init);

		PIOS_DEBUG_Assert(chan->remaP);
			
		// Second parameter should technically be PinSource but they are numerically the same
		GPIO_PinAFConfig(chan->pin.gpio, chan->pin.pin_source,chan->remap);
	}

	*tim_id = (uint32_t)tim_dev;

	return(0);

out_fail:
	return(-1);
}

static void PIOS_TIM_generic_irq_handler(TIM_TypeDef * timer)
{
	/* Iterate over all registered clients of the TIM layer to find channels on this timer */
	for (uint8_t i = 0; i < pios_tim_num_devs; i++) {
		const struct pios_tim_dev * tim_dev = &pios_tim_devs[i];

		if (!tim_dev->channels || tim_dev->num_channels == 0) {
			/* No channels to process on this client */
			continue;
		}

		/* Check for an overflow event on this timer */
		bool overflow_event;
		uint16_t overflow_count;
		if (TIM_GetITStatus(timer, TIM_IT_Update) == SET) {
			TIM_ClearITPendingBit(timer, TIM_IT_Update);
			overflow_count = timer->ARR;
			overflow_event = true;
		} else {
			overflow_count = 0;
			overflow_event = false;
		}

		for (uint8_t j = 0; j < tim_dev->num_channels; j++) {
			const struct pios_tim_channel * chan = &tim_dev->channels[j];

			if (chan->timer != timer) {
				/* channel is not on this timer */
				continue;
			}

			/* Figure out which interrupt bit we should be looking at */
			uint16_t timer_it;
			switch (chan->timer_chan) {
			case TIM_Channel_1:
				timer_it = TIM_IT_CC1;
				break;
			case TIM_Channel_2:
				timer_it = TIM_IT_CC2;
				break;
			case TIM_Channel_3:
				timer_it = TIM_IT_CC3;
				break;
			case TIM_Channel_4:
				timer_it = TIM_IT_CC4;
				break;
			default:
				PIOS_Assert(0);
				break;
			}

			bool edge_event;
			uint16_t edge_count;
			if (TIM_GetITStatus(chan->timer, timer_it) == SET) {
				TIM_ClearITPendingBit(chan->timer, timer_it);

				/* Read the current counter */
				switch(chan->timer_chan) {
				case TIM_Channel_1:
					edge_count = TIM_GetCapture1(chan->timer);
					break;
				case TIM_Channel_2:
					edge_count = TIM_GetCapture2(chan->timer);
					break;
				case TIM_Channel_3:
					edge_count = TIM_GetCapture3(chan->timer);
					break;
				case TIM_Channel_4:
					edge_count = TIM_GetCapture4(chan->timer);
					break;
				default:
					PIOS_Assert(0);
					break;
				}
				edge_event = true;
			} else {
				edge_event = false;
				edge_count = 0;
			}

			if (!tim_dev->callbacks) {
				/* No callbacks registered, we're done with this channel */
				continue;
			}

			/* Generate the appropriate callbacks */
			if (overflow_event & edge_event) {
				/*
				 * When both edge and overflow happen in the same interrupt, we
				 * need a heuristic to determine the order of the edge and overflow
				 * events so that the callbacks happen in the right order.  If we
				 * get the order wrong, our pulse width calculations could be off by up
				 * to ARR ticks.  That could be bad.
				 *
				 * Heuristic: If the edge_count is < 16 ticks above zero then we assume the
				 *            edge happened just after the overflow.
				 */

				if (edge_count < 16) {
					/* Call the overflow callback first */
					if (tim_dev->callbacks->overflow) {
						(*tim_dev->callbacks->overflow)((uint32_t)tim_dev,
									tim_dev->context,
									j,
									overflow_count);
					}
					/* Call the edge callback second */
					if (tim_dev->callbacks->edge) {
						(*tim_dev->callbacks->edge)((uint32_t)tim_dev,
									tim_dev->context,
									j,
									edge_count);
					}
				} else {
					/* Call the edge callback first */
					if (tim_dev->callbacks->edge) {
						(*tim_dev->callbacks->edge)((uint32_t)tim_dev,
									tim_dev->context,
									j,
									edge_count);
					}
					/* Call the overflow callback second */
					if (tim_dev->callbacks->overflow) {
						(*tim_dev->callbacks->overflow)((uint32_t)tim_dev,
									tim_dev->context,
									j,
									overflow_count);
					}
				}
			} else if (overflow_event && tim_dev->callbacks->overflow) {
				(*tim_dev->callbacks->overflow)((uint32_t)tim_dev,
								tim_dev->context,
								j,
								overflow_count);
			} else if (edge_event && tim_dev->callbacks->edge) {
				(*tim_dev->callbacks->edge)((uint32_t)tim_dev,
							tim_dev->context,
							j,
							edge_count);
			}
		}
	}
}
#if 0
	uint16_t val = 0;
	for(uint8_t i = 0; i < pios_pwm_cfg.num_channels; i++) {
		struct pios_pwm_channel channel = pios_pwm_cfg.channels[i];
		if ((channel.timer == timer) && (TIM_GetITStatus(channel.timer, channel.ccr) == SET)) {
			
			TIM_ClearITPendingBit(channel.timer, channel.ccr);
			
			switch(channel.channel) {
				case TIM_Channel_1:
					val = TIM_GetCapture1(channel.timer);
					break;
				case TIM_Channel_2:
					val = TIM_GetCapture2(channel.timer);
					break;
				case TIM_Channel_3:
					val = TIM_GetCapture3(channel.timer);
					break;
				case TIM_Channel_4:
					val = TIM_GetCapture4(channel.timer);
					break;					
			}
			
			if (CaptureState[i] == 0) {
				RiseValue[i] = val; 
			} else {
				FallValue[i] = val;
			}
			
			// flip state machine and capture value here
			/* Simple rise or fall state machine */
			TIM_ICInitTypeDef TIM_ICInitStructure = pios_pwm_cfg.tim_ic_init;
			if (CaptureState[i] == 0) {
				/* Switch states */
				CaptureState[i] = 1;
				
				/* Switch polarity of input capture */
				TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
				TIM_ICInitStructure.TIM_Channel = channel.channel;
				TIM_ICInit(channel.timer, &TIM_ICInitStructure);				
			} else {
				/* Capture computation */
				if (FallValue[i] > RiseValue[i]) {
					CaptureValue[i] = (FallValue[i] - RiseValue[i]);
				} else {
					CaptureValue[i] = ((channel.timer->ARR - RiseValue[i]) + FallValue[i]);
				}
				
				/* Switch states */
				CaptureState[i] = 0;
				
				/* Increase supervisor counter */
				CapCounter[i]++;
				
				/* Switch polarity of input capture */
				TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
				TIM_ICInitStructure.TIM_Channel = channel.channel;
				TIM_ICInit(channel.timer, &TIM_ICInitStructure);
			}
		}		
	}
#endif

/* Bind Interrupt Handlers
 *
 * Map all valid TIM IRQs to the common interrupt handler
 * and give it enough context to properly demux the various timers
 */
void TIM1_CC_IRQHandler(void) __attribute__ ((alias ("PIOS_TIM_1_CC_irq_handler")));
static void PIOS_TIM_1_CC_irq_handler (void)
{
	PIOS_TIM_generic_irq_handler (TIM1);
}

// The rest of TIM1 interrupts are overlapped
void TIM1_BRK_TIM9_IRQHandler(void) __attribute__ ((alias ("PIOS_TIM_9_CC_irq_handler")));
static void PIOS_TIM_9_CC_irq_handler (void)
{
	// TODO: Check for TIM1_BRK
	PIOS_TIM_generic_irq_handler (TIM9);
}

void TIM1_UP_TIM10_IRQHandler(void) __attribute__ ((alias ("PIOS_TIM_10_CC_irq_handler")));
static void PIOS_TIM_10_CC_irq_handler (void)
{
	if (TIM_GetITStatus(TIM1, TIM_IT_Update)) {
		PIOS_TIM_generic_irq_handler(TIM1);
	} else if (TIM_GetITStatus(TIM10, TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4)) {
		PIOS_TIM_generic_irq_handler (TIM10);
	}
}

void TIM1_TRG_COM_TIM11_IRQHandler(void) __attribute__ ((alias ("PIOS_TIM_11_CC_irq_handler")));
static void PIOS_TIM_11_CC_irq_handler (void)
{
	// TODO: Check for TIM1_TRG
	PIOS_TIM_generic_irq_handler (TIM11);
}

void TIM2_IRQHandler(void) __attribute__ ((alias ("PIOS_TIM_2_irq_handler")));
static void PIOS_TIM_2_irq_handler (void)
{
	PIOS_TIM_generic_irq_handler (TIM2);
}

void TIM3_IRQHandler(void) __attribute__ ((alias ("PIOS_TIM_3_irq_handler")));
static void PIOS_TIM_3_irq_handler (void)
{
	PIOS_TIM_generic_irq_handler (TIM3);
}

void TIM4_IRQHandler(void) __attribute__ ((alias ("PIOS_TIM_4_irq_handler")));
static void PIOS_TIM_4_irq_handler (void)
{
	PIOS_TIM_generic_irq_handler (TIM4);
}

void TIM5_IRQHandler(void) __attribute__ ((alias ("PIOS_TIM_5_irq_handler")));
static void PIOS_TIM_5_irq_handler (void)
{
	PIOS_TIM_generic_irq_handler (TIM5);
}

void TIM6_IRQHandler(void) __attribute__ ((alias ("PIOS_TIM_6_irq_handler")));
static void PIOS_TIM_6_irq_handler (void)
{
	PIOS_TIM_generic_irq_handler (TIM6);
}

void TIM7_IRQHandler(void) __attribute__ ((alias ("PIOS_TIM_7_irq_handler")));
static void PIOS_TIM_7_irq_handler (void)
{
	PIOS_TIM_generic_irq_handler (TIM7);
}

void TIM8_UP_IRQHandler(void) __attribute__ ((alias ("PIOS_TIM_8_UP_irq_handler")));
static void PIOS_TIM_8_UP_irq_handler (void)
{
	PIOS_TIM_generic_irq_handler (TIM8);
}

void TIM8_CC_IRQHandler(void) __attribute__ ((alias ("PIOS_TIM_8_CC_irq_handler")));
static void PIOS_TIM_8_CC_irq_handler (void)
{
	PIOS_TIM_generic_irq_handler (TIM8);
}

/**
 * @}
 * @}
 */
