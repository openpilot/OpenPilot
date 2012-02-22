/* -*- Mode: c; c-basic-offset: 2; tab-width: 2; indent-tabs-mode: t -*- */
/**
 ******************************************************************************
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotCore OpenPilot Core
 * @{
 *
 * @file       pios_board.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Defines board specific static initializers for hardware for the OpenPilot board.
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

#include <pios.h>
#include <openpilot.h>
#include <uavobjectsinit.h>
#include <hwsettings.h>
#include <manualcontrolsettings.h>
#include <gcsreceiver.h>

/*
 * ADC system
 */
#include "pios_adc_priv.h"
extern void PIOS_ADC_handler(void);
void DMA1_Channel1_IRQHandler() __attribute__ ((alias("PIOS_ADC_handler")));
// Remap the ADC DMA handler to this one
static const struct pios_adc_cfg pios_adc_cfg = {
	.dma = {
		.ahb_clk  = RCC_AHBPeriph_DMA1,
		.irq = {
			.flags   = (DMA1_FLAG_TC1 | DMA1_FLAG_TE1 | DMA1_FLAG_HT1 | DMA1_FLAG_GL1),
			.init    = {
				.NVIC_IRQChannel                   = DMA1_Channel1_IRQn,
				.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
				.NVIC_IRQChannelSubPriority        = 0,
				.NVIC_IRQChannelCmd                = ENABLE,
			},
		},
		.rx = {
			.channel = DMA1_Channel1,
			.init    = {
				.DMA_PeripheralBaseAddr = (uint32_t) & ADC1->DR,
				.DMA_DIR                = DMA_DIR_PeripheralSRC,
				.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
				.DMA_MemoryInc          = DMA_MemoryInc_Enable,
				.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word,
				.DMA_MemoryDataSize     = DMA_MemoryDataSize_Word,
				.DMA_Mode               = DMA_Mode_Circular,
				.DMA_Priority           = DMA_Priority_High,
				.DMA_M2M                = DMA_M2M_Disable,
			},
		}
	}, 
	.half_flag = DMA1_IT_HT1,
	.full_flag = DMA1_IT_TC1,
};

struct pios_adc_dev pios_adc_devs[] = {
	{
		.cfg = &pios_adc_cfg,
		.callback_function = NULL,
	},
};

uint8_t pios_adc_num_devices = NELEMENTS(pios_adc_devs);

void PIOS_ADC_handler() {
	PIOS_ADC_DMA_Handler();
}

#if defined(PIOS_INCLUDE_USART)

#include "pios_usart_priv.h"

#if defined(PIOS_INCLUDE_TELEMETRY_RF)
/*
 * Telemetry USART
 */
static const struct pios_usart_cfg pios_usart_telem_main_cfg = {
  .regs  = USART1,
  .init = {
    .USART_BaudRate            = 57600,
    .USART_WordLength          = USART_WordLength_8b,
    .USART_Parity              = USART_Parity_No,
    .USART_StopBits            = USART_StopBits_1,
    .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
    .USART_Mode                = USART_Mode_Rx | USART_Mode_Tx,
  },
  .irq = {
    .init    = {
      .NVIC_IRQChannel                   = USART1_IRQn,
      .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
      .NVIC_IRQChannelSubPriority        = 0,
      .NVIC_IRQChannelCmd                = ENABLE,
    },
  },
  .rx   = {
#ifdef MOVE_CONTROLLER
    .gpio = GPIOB,
#else
    .gpio = GPIOA,
#endif
    .init = {
#ifdef MOVE_CONTROLLER
      .GPIO_Pin   = GPIO_Pin_7,
#else
      .GPIO_Pin   = GPIO_Pin_10,
#endif
      .GPIO_Speed = GPIO_Speed_2MHz,
      .GPIO_Mode  = GPIO_Mode_IPU,
    },
  },
  .tx   = {
#ifdef MOVE_CONTROLLER
    .gpio = GPIOB,
#else
    .gpio = GPIOA,
#endif
    .init = {
#ifdef MOVE_CONTROLLER
      .GPIO_Pin   = GPIO_Pin_6,
#else
      .GPIO_Pin   = GPIO_Pin_9,
#endif
      .GPIO_Speed = GPIO_Speed_2MHz,
      .GPIO_Mode  = GPIO_Mode_AF_PP,
    },
  },
#ifdef MOVE_CONTROLLER
  .remap = AFIO_MAPR_USART1_REMAP,
#endif
};

static const struct pios_usart_cfg pios_usart_usart2_cfg = {
  .regs  = USART2,
  .init = {
    .USART_BaudRate            = 57600,
    .USART_WordLength          = USART_WordLength_8b,
    .USART_Parity              = USART_Parity_No,
    .USART_StopBits            = USART_StopBits_1,
    .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
    .USART_Mode                = USART_Mode_Rx | USART_Mode_Tx,
  },
  .irq = {
    .init    = {
      .NVIC_IRQChannel                   = USART2_IRQn,
      .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
      .NVIC_IRQChannelSubPriority        = 0,
      .NVIC_IRQChannelCmd                = ENABLE,
    },
  },
  .rx   = {
    .gpio = GPIOA,
    .init = {
      .GPIO_Pin   = GPIO_Pin_3,
      .GPIO_Speed = GPIO_Speed_2MHz,
      .GPIO_Mode  = GPIO_Mode_IPU,
    },
  },
  .tx   = {
    .gpio = GPIOA,
    .init = {
      .GPIO_Pin   = GPIO_Pin_2,
      .GPIO_Speed = GPIO_Speed_2MHz,
      .GPIO_Mode  = GPIO_Mode_AF_PP,
    },
  },
};

static const struct pios_usart_cfg pios_usart_telem_flexi_cfg = {
  .regs  = USART3,
  .init = {
    .USART_BaudRate            = 57600,
    .USART_WordLength          = USART_WordLength_8b,
    .USART_Parity              = USART_Parity_No,
    .USART_StopBits            = USART_StopBits_1,
    .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
    .USART_Mode                = USART_Mode_Rx | USART_Mode_Tx,
  },
  .irq = {
    .init    = {
      .NVIC_IRQChannel                   = USART3_IRQn,
      .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
      .NVIC_IRQChannelSubPriority        = 0,
      .NVIC_IRQChannelCmd                = ENABLE,
    },
  },
  .rx   = {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_11,
      .GPIO_Speed = GPIO_Speed_2MHz,
      .GPIO_Mode  = GPIO_Mode_IPU,
    },
  },
  .tx   = {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_10,
      .GPIO_Speed = GPIO_Speed_2MHz,
      .GPIO_Mode  = GPIO_Mode_AF_PP,
    },
  },
};
#endif /* PIOS_INCLUDE_TELEMETRY_RF */

#endif  /* PIOS_INCLUDE_USART */

#if defined(PIOS_INCLUDE_COM)

#include "pios_com_priv.h"

#define PIOS_COM_TELEM_RF_RX_BUF_LEN 192
#define PIOS_COM_TELEM_RF_TX_BUF_LEN 192

#define PIOS_COM_GPS_RX_BUF_LEN 96

#define PIOS_COM_TELEM_USB_RX_BUF_LEN 192
#define PIOS_COM_TELEM_USB_TX_BUF_LEN 192

#endif	/* PIOS_INCLUDE_COM */

#if defined(PIOS_INCLUDE_RTC)
/*
 * Realtime Clock (RTC)
 */
#include <pios_rtc_priv.h>

void PIOS_RTC_IRQ_Handler (void);
void RTC_IRQHandler() __attribute__ ((alias ("PIOS_RTC_IRQ_Handler")));
static const struct pios_rtc_cfg pios_rtc_main_cfg = {
	.clksrc = RCC_RTCCLKSource_HSE_Div128,
	.prescaler = 100,
	.irq = {
		.init = {
			.NVIC_IRQChannel                   = RTC_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		  },
	},
};

void PIOS_RTC_IRQ_Handler (void)
{
	PIOS_RTC_irq_handler ();
}

#endif

#include "pios_tim_priv.h"

static const TIM_TimeBaseInitTypeDef tim_1_2_3_4_time_base = {
	.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1,
	.TIM_ClockDivision = TIM_CKD_DIV1,
	.TIM_CounterMode = TIM_CounterMode_Up,
	.TIM_Period = ((1000000 / PIOS_SERVO_UPDATE_HZ) - 1),
	.TIM_RepetitionCounter = 0x0000,
};

static const struct pios_tim_clock_cfg tim_1_cfg = {
	.timer = TIM1,
	.time_base_init = &tim_1_2_3_4_time_base,
	.irq = {
		.init = {
			.NVIC_IRQChannel                   = TIM1_CC_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		},
	},
};

static const struct pios_tim_clock_cfg tim_2_cfg = {
	.timer = TIM2,
	.time_base_init = &tim_1_2_3_4_time_base,
	.irq = {
		.init = {
			.NVIC_IRQChannel                   = TIM2_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		},
	},
};

static const struct pios_tim_clock_cfg tim_3_cfg = {
	.timer = TIM3,
	.time_base_init = &tim_1_2_3_4_time_base,
	.irq = {
		.init = {
			.NVIC_IRQChannel                   = TIM3_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		},
	},
};

static const struct pios_tim_clock_cfg tim_4_cfg = {
	.timer = TIM4,
	.time_base_init = &tim_1_2_3_4_time_base,
	.irq = {
		.init = {
			.NVIC_IRQChannel                   = TIM4_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		},
	},
};

static const struct pios_tim_channel pios_tim_rcvrport_all_channels[] = {
#ifdef MOVE_CONTROLLER
	{
		.timer = TIM4,
		.timer_chan = TIM_Channel_1,
		.pin = {
			.gpio = GPIOE,
			.init = {
				.GPIO_Pin   = GPIO_Pin_12,
				.GPIO_Mode  = GPIO_Mode_IPD,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	},
#else
	{
		.timer = TIM4,
		.timer_chan = TIM_Channel_1,
		.pin = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin   = GPIO_Pin_6,
				.GPIO_Mode  = GPIO_Mode_IPD,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	},
#endif
	{
		.timer = TIM3,
		.timer_chan = TIM_Channel_2,
		.pin = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin   = GPIO_Pin_5,
				.GPIO_Mode  = GPIO_Mode_IPD,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
		.remap = GPIO_PartialRemap_TIM3,
	},
	{
		.timer = TIM3,
		.timer_chan = TIM_Channel_3,
		.pin = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin   = GPIO_Pin_0,
				.GPIO_Mode  = GPIO_Mode_IPD,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	}, 
	{
		.timer = TIM3,
		.timer_chan = TIM_Channel_4,
		.pin = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin   = GPIO_Pin_1,
				.GPIO_Mode  = GPIO_Mode_IPD,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	}, 
	{ 
		.timer = TIM2,
		.timer_chan = TIM_Channel_1,
		.pin = {
			.gpio = GPIOA,
			.init = {
				.GPIO_Pin   = GPIO_Pin_0,
				.GPIO_Mode  = GPIO_Mode_IPD,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	},  	
	{
		.timer = TIM2,
		.timer_chan = TIM_Channel_2,
		.pin = {
			.gpio = GPIOA,
			.init = {
				.GPIO_Pin   = GPIO_Pin_1,
				.GPIO_Mode  = GPIO_Mode_IPD,
				.GPIO_Speed = GPIO_Speed_2MHz,
			},
		},
	}, 		
};

/* PPM */
#include <pios_ppm_priv.h>

const struct pios_ppm_cfg pios_ppm_cfg = {
	.tim_ic_init = {
		.TIM_ICPolarity = TIM_ICPolarity_Rising,
		.TIM_ICSelection = TIM_ICSelection_DirectTI,
		.TIM_ICPrescaler = TIM_ICPSC_DIV1,
		.TIM_ICFilter = 0x0,
	},
	/* Use only the first channel for ppm */
	.channels = &pios_tim_rcvrport_all_channels[0],
	.num_channels = 1,
};

#include "pios_rcvr_priv.h"

/* One slot per selectable receiver group.
 *  eg. PWM, PPM, GCS, DSMMAINPORT, DSMFLEXIPORT, SBUS
 * NOTE: No slot in this map for NONE.
 */
uint32_t pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE];

#include <pios_pwm_priv.h>

static const struct pios_tim_channel pios_tim_rssi_pwm_channel = {
#if defined(ANALOG_TRANSMITTER)
	.timer = TIM3,
	.timer_chan = TIM_Channel_2,
	.pin = {
		.gpio = GPIOB,
		.init = {
			.GPIO_Pin   = GPIO_Pin_5,
			.GPIO_Mode  = GPIO_Mode_IPD,
			.GPIO_Speed = GPIO_Speed_2MHz,
		},
	},
	.remap = GPIO_PartialRemap_TIM3,
#else
	.timer = TIM2,
	.timer_chan = TIM_Channel_2,
	.pin = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_1,
			.GPIO_Mode  = GPIO_Mode_IPD,
			.GPIO_Speed = GPIO_Speed_2MHz,
		},
	},
#endif
};

const struct pios_pwm_cfg pios_pwm_cfg = {
	.tim_ic_init = {
		.TIM_ICPolarity = TIM_ICPolarity_Rising,
		.TIM_ICSelection = TIM_ICSelection_DirectTI,
		.TIM_ICPrescaler = TIM_ICPSC_DIV1,
		.TIM_ICFilter = 0x0,
	},
	.channels = &pios_tim_rssi_pwm_channel,
	.num_channels = 1,
};

#if defined(PIOS_INCLUDE_I2C)


#include <pios_i2c_priv.h>

/*
 * I2C Adapters
 */

void PIOS_I2C_flexi_adapter_ev_irq_handler(void);
void PIOS_I2C_flexi_adapter_er_irq_handler(void);
void I2C2_EV_IRQHandler() __attribute__ ((alias ("PIOS_I2C_flexi_adapter_ev_irq_handler")));
void I2C2_ER_IRQHandler() __attribute__ ((alias ("PIOS_I2C_flexi_adapter_er_irq_handler")));

static const struct pios_i2c_adapter_cfg pios_i2c_flexi_adapter_cfg = {
  .regs = I2C2,
  .init = {
    .I2C_Mode                = I2C_Mode_I2C,
    .I2C_OwnAddress1         = 0,
    .I2C_Ack                 = I2C_Ack_Enable,
    .I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit,
    .I2C_DutyCycle           = I2C_DutyCycle_2,
    .I2C_ClockSpeed          = 400000,	/* bits/s */
  },
  .transfer_timeout_ms = 50,
  .scl = {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_10,
      .GPIO_Speed = GPIO_Speed_10MHz,
      .GPIO_Mode  = GPIO_Mode_AF_OD,
    },
  },
  .sda = {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_11,
      .GPIO_Speed = GPIO_Speed_10MHz,
      .GPIO_Mode  = GPIO_Mode_AF_OD,
    },
  },
  .event = {
    .flags   = 0,		/* FIXME: check this */
    .init = {
      .NVIC_IRQChannel                   = I2C2_EV_IRQn,
      .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGHEST,
      .NVIC_IRQChannelSubPriority        = 0,
      .NVIC_IRQChannelCmd                = ENABLE,
    },
  },
  .error = {
    .flags   = 0,		/* FIXME: check this */
    .init = {
      .NVIC_IRQChannel                   = I2C2_ER_IRQn,
      .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGHEST,
      .NVIC_IRQChannelSubPriority        = 0,
      .NVIC_IRQChannelCmd                = ENABLE,
    },
  },
};

uint32_t pios_i2c_flexi_adapter_id;
void PIOS_I2C_flexi_adapter_ev_irq_handler(void)
{
  /* Call into the generic code to handle the IRQ for this specific device */
  PIOS_I2C_EV_IRQ_Handler(pios_i2c_flexi_adapter_id);
}

void PIOS_I2C_flexi_adapter_er_irq_handler(void)
{
  /* Call into the generic code to handle the IRQ for this specific device */
  PIOS_I2C_ER_IRQ_Handler(pios_i2c_flexi_adapter_id);
}

#endif /* PIOS_INCLUDE_I2C */

uint32_t rssi_pwm_id;

uint32_t pios_com_usart1_id;
#ifndef MOVE_CONTROLLER
uint32_t pios_com_usart2_id;
uint32_t pios_com_usart3_id;
#endif

/**
 * PIOS_Board_Init()
 * initializes all the core subsystems on this specific hardware
 * called from System/openpilot.c
 */
void PIOS_Board_Init(void) {

	/* Delay system */
	PIOS_DELAY_Init();

	/* Initialize UAVObject libraries */
	EventDispatcherInitialize();
	UAVObjInitialize();

	HwSettingsInitialize();

#ifndef ERASE_FLASH
	/* Initialize watchdog as early as possible to catch faults during init */
	PIOS_WDG_Init();
#endif

	/* Initialize the alarms library */
	AlarmsInitialize();

	/* Initialize IAP */
	PIOS_IAP_Init();

#if defined(PIOS_INCLUDE_RTC)
	/* Initialize the real-time clock and its associated tick */
	PIOS_RTC_Init(&pios_rtc_main_cfg);
#endif

	/* Initialize the task monitor library */
	TaskMonitorInitialize();

	/* Set up pulse timers */
	PIOS_TIM_InitClock(&tim_1_cfg);
	PIOS_TIM_InitClock(&tim_2_cfg);
	PIOS_TIM_InitClock(&tim_3_cfg);
	PIOS_TIM_InitClock(&tim_4_cfg);

#ifdef TRANSMITTER_BOX
	/* Configure the rcvr port */
	{
		uint32_t pios_ppm_id;
		PIOS_PPM_Init(&pios_ppm_id, &pios_ppm_cfg);

		uint32_t pios_ppm_rcvr_id;
		if (PIOS_RCVR_Init(&pios_ppm_rcvr_id, &pios_ppm_rcvr_driver, pios_ppm_id)) {
			PIOS_Assert(0);
		}
		pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_PPM] = pios_ppm_rcvr_id;
	}
#endif

	/* Configure USART1 */
	{
		uint32_t pios_usart1_id;
		if (PIOS_USART_Init(&pios_usart1_id, &pios_usart_telem_main_cfg)) {
			PIOS_Assert(0);
		}

		uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_RX_BUF_LEN);
		uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_TX_BUF_LEN);
		PIOS_Assert(rx_buffer);
		PIOS_Assert(tx_buffer);
		if (PIOS_COM_Init(&pios_com_usart1_id, &pios_usart_com_driver, pios_usart1_id,
											rx_buffer, PIOS_COM_TELEM_RF_RX_BUF_LEN,
											tx_buffer, PIOS_COM_TELEM_RF_TX_BUF_LEN)) {
			PIOS_Assert(0);
		}
	}

#ifndef MOVE_CONTROLLER
	/* Configure USART2 */
	{
		uint32_t pios_usart2_id;
		if (PIOS_USART_Init(&pios_usart2_id, &pios_usart_usart2_cfg)) {
			PIOS_Assert(0);
		}

		uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_RX_BUF_LEN);
		uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_TX_BUF_LEN);
		PIOS_Assert(rx_buffer);
		PIOS_Assert(tx_buffer);
		if (PIOS_COM_Init(&pios_com_usart2_id, &pios_usart_com_driver, pios_usart2_id,
											rx_buffer, PIOS_COM_TELEM_RF_RX_BUF_LEN,
											tx_buffer, PIOS_COM_TELEM_RF_TX_BUF_LEN)) {
			PIOS_Assert(0);
		}
	}

	/* Configure USART3 */
	{
		uint32_t pios_usart3_id;
		if (PIOS_USART_Init(&pios_usart3_id, &pios_usart_telem_flexi_cfg)) {
			PIOS_Assert(0);
		}
		uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_RX_BUF_LEN);
		uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_TX_BUF_LEN);
		PIOS_Assert(rx_buffer);
		PIOS_Assert(tx_buffer);
		if (PIOS_COM_Init(&pios_com_usart3_id, &pios_usart_com_driver, pios_usart3_id,
											rx_buffer, PIOS_COM_TELEM_RF_RX_BUF_LEN,
											tx_buffer, PIOS_COM_TELEM_RF_TX_BUF_LEN)) {
			PIOS_Assert(0);
		}
	}
#endif
	PIOS_COM_SendString(PIOS_COM_DEBUG, "Hello Debug\n\r");
	//PIOS_COM_SendString(PIOS_COM_TELEM_GCS, "Hello GCS\n\r");
	//PIOS_COM_SendString(PIOS_COM_TELEM_OUT, "Hello CC\n\r");

#if defined(PIOS_INCLUDE_I2C)
	if (PIOS_I2C_Init(&pios_i2c_flexi_adapter_id, &pios_i2c_flexi_adapter_cfg)) {
		PIOS_Assert(0);
	}
#endif  /* PIOS_INCLUDE_AK8974 */

#if defined(ANALOG_TRANSMITTER)
	// Initialize switch GPIO pins
	{
		GPIO_TypeDef *GPIO_GPIOxs[] = {
			GPIOA, GPIOB, GPIOB, GPIOB, GPIOB, GPIOB
		};
		GPIO_InitTypeDef GPIO_InitStructures[] = {
			{
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Pin = GPIO_Pin_8,
				.GPIO_Mode = GPIO_Mode_IN_FLOATING,
			},
			{
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Pin = GPIO_Pin_7,
				.GPIO_Mode = GPIO_Mode_IN_FLOATING,
			},
			{
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Pin = GPIO_Pin_14,
				.GPIO_Mode = GPIO_Mode_IN_FLOATING,
			},
			{
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Pin = GPIO_Pin_13,
				.GPIO_Mode = GPIO_Mode_IN_FLOATING,
			},
			{
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Pin = GPIO_Pin_15,
				.GPIO_Mode = GPIO_Mode_IN_FLOATING,
			},
			{
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Pin = GPIO_Pin_9,
				.GPIO_Mode = GPIO_Mode_Out_OD,
			},
		};
		unsigned int i;
		for (i = 0; i < sizeof(GPIO_GPIOxs) / sizeof(GPIO_TypeDef); ++i)
			GPIO_Init(GPIO_GPIOxs[i], GPIO_InitStructures + i);
	}

	// Initialize the XBee RSSI PWM input.
	PIOS_PWM_Init(&rssi_pwm_id, &pios_pwm_cfg);

#elif defined(MAPLE_MINI)

	// Initialize switch GPIO pins
	{
		GPIO_TypeDef *GPIO_GPIOxs[] = {
			GPIOB, GPIOB, GPIOB
		};
		GPIO_InitTypeDef GPIO_InitStructures[] = {
			{
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Pin = GPIO_Pin_0,
				.GPIO_Mode = GPIO_Mode_IPU,
			},
			{
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Pin = GPIO_Pin_2,
				.GPIO_Mode = GPIO_Mode_IPU,
			},
			{
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Pin = GPIO_Pin_9,
				.GPIO_Mode = GPIO_Mode_Out_OD,
			},
		};
		unsigned int i;
		for (i = 0; i < sizeof(GPIO_GPIOxs) / sizeof(GPIO_TypeDef); ++i)
			GPIO_Init(GPIO_GPIOxs[i], GPIO_InitStructures + i);
	}

	// Initialize the XBee RSSI PWM input.
	PIOS_PWM_Init(&rssi_pwm_id, &pios_pwm_cfg);

	// Turn on the USB port on the Maple Mini */
	GPIO_ResetBits(GPIOB, GPIO_Pin_9);

#elif defined(MOVE_CONTROLLER)

	// Initialize switch GPIO pins
	// PA0 - PS Button
	// PB8 - Select Button
	// PB9 - Start Button
	// PD5 - Square Button
	// PC13 - Circle Button
	// PC14 - X Button
	// PC15 - Triangle Button
	// PE0 - Move Button
	{
		GPIO_TypeDef *GPIO_GPIOxs[] = {
			GPIOA, GPIOB, GPIOB, GPIOD, GPIOC, GPIOC, GPIOC, GPIOE, GPIOD, GPIOD
		};
		GPIO_InitTypeDef GPIO_InitStructures[] = {
			{
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Pin = GPIO_Pin_0,
				.GPIO_Mode = GPIO_Mode_IPD,
			},
			{
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Pin = GPIO_Pin_8,
				.GPIO_Mode = GPIO_Mode_IPD,
			},
			{
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Pin = GPIO_Pin_9,
				.GPIO_Mode = GPIO_Mode_IPD,
			},
			{
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Pin = GPIO_Pin_5,
				.GPIO_Mode = GPIO_Mode_IPD,
			},
			{
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Pin = GPIO_Pin_13,
				.GPIO_Mode = GPIO_Mode_IPD,
			},
			{
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Pin = GPIO_Pin_14,
				.GPIO_Mode = GPIO_Mode_IPD,
			},
			{
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Pin = GPIO_Pin_15,
				.GPIO_Mode = GPIO_Mode_IPD,
			},
			{
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Pin = GPIO_Pin_0,
				.GPIO_Mode = GPIO_Mode_IPD,
			},
			{
				.GPIO_Speed = GPIO_Speed_50MHz,
				.GPIO_Pin = GPIO_Pin_0,
				.GPIO_Mode = GPIO_Mode_Out_PP,
			},
		};
		unsigned int i;
		for (i = 0; i < sizeof(GPIO_GPIOxs) / sizeof(GPIO_TypeDef); ++i)
			GPIO_Init(GPIO_GPIOxs[i], GPIO_InitStructures + i);

		// Set the referece voltage for T button potentiometer
		GPIO_ResetBits(GPIOD, GPIO_Pin_0);
	}
#endif

	/* Remap AFIO pin */
	GPIO_PinRemapConfig( GPIO_Remap_SWJ_NoJTRST, ENABLE);

	PIOS_ADC_Init();
	PIOS_GPIO_Init();
}

/**
 * @}
 */
