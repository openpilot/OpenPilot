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
#include "manualcontrolsettings.h"

//#define I2C_DEBUG_PIN			0
//#define USART_GPS_DEBUG_PIN		1

#if defined(PIOS_INCLUDE_SPI)

#include <pios_spi_priv.h>

/* MicroSD Interface
 * 
 * NOTE: Leave this declared as const data so that it ends up in the 
 * .rodata section (ie. Flash) rather than in the .bss section (RAM).
 */
void PIOS_SPI_sdcard_irq_handler(void);
void DMA1_Channel2_IRQHandler() __attribute__ ((alias ("PIOS_SPI_sdcard_irq_handler")));
void DMA1_Channel3_IRQHandler() __attribute__ ((alias ("PIOS_SPI_sdcard_irq_handler")));
static const struct pios_spi_cfg pios_spi_sdcard_cfg = {
  .regs   = SPI1,
  .init   = {
    .SPI_Mode              = SPI_Mode_Master,
    .SPI_Direction         = SPI_Direction_2Lines_FullDuplex,
    .SPI_DataSize          = SPI_DataSize_8b,
    .SPI_NSS               = SPI_NSS_Soft,
    .SPI_FirstBit          = SPI_FirstBit_MSB,
    .SPI_CRCPolynomial     = 7,
    .SPI_CPOL              = SPI_CPOL_High,
    .SPI_CPHA              = SPI_CPHA_2Edge,
    .SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256, /* Maximum divider (ie. slowest clock rate) */
  },
  .dma = {
    .ahb_clk  = RCC_AHBPeriph_DMA1,
    
    .irq = {
      .flags   = (DMA1_FLAG_TC2 | DMA1_FLAG_TE2 | DMA1_FLAG_HT2 | DMA1_FLAG_GL2),
      .init    = {
	.NVIC_IRQChannel                   = DMA1_Channel2_IRQn,
	.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
	.NVIC_IRQChannelSubPriority        = 0,
	.NVIC_IRQChannelCmd                = ENABLE,
      },
    },
    
    .rx = {
      .channel = DMA1_Channel2,
      .init    = {
	.DMA_PeripheralBaseAddr = (uint32_t)&(SPI1->DR),
	.DMA_DIR                = DMA_DIR_PeripheralSRC,
	.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
	.DMA_MemoryInc          = DMA_MemoryInc_Enable,
	.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
	.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
	.DMA_Mode               = DMA_Mode_Normal,
	.DMA_Priority           = DMA_Priority_Medium,
	.DMA_M2M                = DMA_M2M_Disable,
      },
    },
    .tx = {
      .channel = DMA1_Channel3,
      .init    = {
	.DMA_PeripheralBaseAddr = (uint32_t)&(SPI1->DR),
	.DMA_DIR                = DMA_DIR_PeripheralDST,
	.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
	.DMA_MemoryInc          = DMA_MemoryInc_Enable,
	.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
	.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
	.DMA_Mode               = DMA_Mode_Normal,
	.DMA_Priority           = DMA_Priority_Medium,
	.DMA_M2M                = DMA_M2M_Disable,
      },
    },
  },
  .ssel = {
    .gpio = GPIOA,
    .init = {
      .GPIO_Pin   = GPIO_Pin_4,
      .GPIO_Speed = GPIO_Speed_50MHz,
      .GPIO_Mode  = GPIO_Mode_Out_PP,
    },
  },
  .sclk = {
    .gpio = GPIOA,
    .init = {
      .GPIO_Pin   = GPIO_Pin_5,
      .GPIO_Speed = GPIO_Speed_50MHz,
      .GPIO_Mode  = GPIO_Mode_AF_PP,
    },
  },
  .miso = {
    .gpio = GPIOA,
    .init = {
      .GPIO_Pin   = GPIO_Pin_6,
      .GPIO_Speed = GPIO_Speed_50MHz,
      .GPIO_Mode  = GPIO_Mode_IPU,
    },
  },
  .mosi = {
    .gpio = GPIOA,
    .init = {
      .GPIO_Pin   = GPIO_Pin_7,
      .GPIO_Speed = GPIO_Speed_50MHz,
      .GPIO_Mode  = GPIO_Mode_AF_PP,
    },
  },
};

/* AHRS Interface
 * 
 * NOTE: Leave this declared as const data so that it ends up in the 
 * .rodata section (ie. Flash) rather than in the .bss section (RAM).
 */
void PIOS_SPI_ahrs_irq_handler(void);
void DMA1_Channel4_IRQHandler() __attribute__ ((alias ("PIOS_SPI_ahrs_irq_handler")));
void DMA1_Channel5_IRQHandler() __attribute__ ((alias ("PIOS_SPI_ahrs_irq_handler")));
static const struct pios_spi_cfg pios_spi_ahrs_cfg = {
  .regs   = SPI2,
  .init   = {
    .SPI_Mode              = SPI_Mode_Master,
    .SPI_Direction         = SPI_Direction_2Lines_FullDuplex,
    .SPI_DataSize          = SPI_DataSize_8b,
    .SPI_NSS               = SPI_NSS_Soft,
    .SPI_FirstBit          = SPI_FirstBit_MSB,
    .SPI_CRCPolynomial     = 7,
    .SPI_CPOL              = SPI_CPOL_High,
    .SPI_CPHA              = SPI_CPHA_2Edge,
    .SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16, 
  },
  .use_crc = TRUE,
  .dma = {
    .ahb_clk  = RCC_AHBPeriph_DMA1,
    
    .irq = {
      .flags   = (DMA1_FLAG_TC4 | DMA1_FLAG_TE4 | DMA1_FLAG_HT4 | DMA1_FLAG_GL4),
      .init    = {
	.NVIC_IRQChannel                   = DMA1_Channel4_IRQn,
	.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
	.NVIC_IRQChannelSubPriority        = 0,
	.NVIC_IRQChannelCmd                = ENABLE,
      },
    },

    .rx = {
      .channel = DMA1_Channel4,
      .init    = {
	.DMA_PeripheralBaseAddr = (uint32_t)&(SPI2->DR),
	.DMA_DIR                = DMA_DIR_PeripheralSRC,
	.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
	.DMA_MemoryInc          = DMA_MemoryInc_Enable,
	.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
	.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
	.DMA_Mode               = DMA_Mode_Normal,
	.DMA_Priority           = DMA_Priority_High,
	.DMA_M2M                = DMA_M2M_Disable,
      },
    },
    .tx = {
      .channel = DMA1_Channel5,
      .init    = {
	.DMA_PeripheralBaseAddr = (uint32_t)&(SPI2->DR),
	.DMA_DIR                = DMA_DIR_PeripheralDST,
	.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
	.DMA_MemoryInc          = DMA_MemoryInc_Enable,
	.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
	.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
	.DMA_Mode               = DMA_Mode_Normal,
	.DMA_Priority           = DMA_Priority_High,
	.DMA_M2M                = DMA_M2M_Disable,
      },
    },
  },
  .ssel = {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_12,
      .GPIO_Speed = GPIO_Speed_10MHz,
      .GPIO_Mode  = GPIO_Mode_Out_PP,
    },
  },
  .sclk = {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_13,
      .GPIO_Speed = GPIO_Speed_10MHz,
      .GPIO_Mode  = GPIO_Mode_AF_PP,
    },
  },
  .miso = {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_14,
      .GPIO_Speed = GPIO_Speed_10MHz,
      .GPIO_Mode  = GPIO_Mode_IN_FLOATING,
    },
  },
  .mosi = {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_15,
      .GPIO_Speed = GPIO_Speed_10MHz,
      .GPIO_Mode  = GPIO_Mode_AF_PP,
    },
  },
};

static uint32_t pios_spi_sdcard_id;
void PIOS_SPI_sdcard_irq_handler(void)
{
  /* Call into the generic code to handle the IRQ for this specific device */
	PIOS_SPI_IRQ_Handler(pios_spi_sdcard_id);
}

uint32_t pios_spi_ahrs_id;
void PIOS_SPI_ahrs_irq_handler(void)
{
  /* Call into the generic code to handle the IRQ for this specific device */
	PIOS_SPI_IRQ_Handler(pios_spi_ahrs_id);
}

#endif /* PIOS_INCLUDE_SPI */

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
				.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_LOW,
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
				.DMA_Priority           = DMA_Priority_Low,
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

/*
 * Telemetry USART
 */
static const struct pios_usart_cfg pios_usart_telem_cfg = {
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

/*
 * GPS USART
 */
static const struct pios_usart_cfg pios_usart_gps_cfg = {
  .regs = USART3,
  .remap = GPIO_PartialRemap_USART3,
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
    .gpio = GPIOC,
    .init = {
      .GPIO_Pin   = GPIO_Pin_11,
      .GPIO_Speed = GPIO_Speed_2MHz,
      .GPIO_Mode  = GPIO_Mode_IPU,
    },
  },
  .tx   = {
    .gpio = GPIOC,
    .init = {
      .GPIO_Pin   = GPIO_Pin_10,
      .GPIO_Speed = GPIO_Speed_2MHz,
      .GPIO_Mode  = GPIO_Mode_AF_PP,
    },
  },
};

#ifdef PIOS_COM_AUX
/*
 * AUX USART
 */
static const struct pios_usart_cfg pios_usart_aux_cfg = {
  .regs = USART1,
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
  .remap = GPIO_Remap_USART1,
  .rx   = {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_7,
      .GPIO_Speed = GPIO_Speed_2MHz,
      .GPIO_Mode  = GPIO_Mode_IPU,
    },
  },
  .tx   = {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_6,
      .GPIO_Speed = GPIO_Speed_2MHz,
      .GPIO_Mode  = GPIO_Mode_AF_PP,
    },
  },
};
#endif

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

#if defined(PIOS_INCLUDE_SPEKTRUM)
/*
 * SPEKTRUM USART
 */
#include <pios_spektrum_priv.h>

static const struct pios_usart_cfg pios_usart_spektrum_cfg = {
  .regs = USART1,
  .init = {
    .USART_BaudRate            = 115200,
    .USART_WordLength          = USART_WordLength_8b,
    .USART_Parity              = USART_Parity_No,
    .USART_StopBits            = USART_StopBits_1,
    .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
    .USART_Mode                = USART_Mode_Rx,
  },
  .irq = {
    .init    = {
      .NVIC_IRQChannel                   = USART1_IRQn,
      .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
      .NVIC_IRQChannelSubPriority        = 0,
      .NVIC_IRQChannelCmd                = ENABLE,
    },
  },
  .rx   = {
    .gpio = GPIOA,
    .init = {
      .GPIO_Pin   = GPIO_Pin_10,
      .GPIO_Speed = GPIO_Speed_2MHz,
      .GPIO_Mode  = GPIO_Mode_IPU,
    },
  },
  .tx   = {
    .gpio = GPIOA,
    .init = {
      .GPIO_Pin   = GPIO_Pin_9,
      .GPIO_Speed = GPIO_Speed_2MHz,
      .GPIO_Mode  = GPIO_Mode_IN_FLOATING,
    },
  },
};

static const struct pios_spektrum_cfg pios_spektrum_cfg = {
	.bind = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_10,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode  = GPIO_Mode_Out_PP,
		},
	},
	.remap = 0,
};

#endif	/* PIOS_COM_SPEKTRUM */

#if defined(PIOS_INCLUDE_SBUS)
#error PIOS_INCLUDE_SBUS not implemented
#endif	/* PIOS_INCLUDE_SBUS */

#endif /* PIOS_INCLUDE_USART */

#if defined(PIOS_INCLUDE_COM)

#include "pios_com_priv.h"

#define PIOS_COM_TELEM_RF_RX_BUF_LEN 192
#define PIOS_COM_TELEM_RF_TX_BUF_LEN 192

#define PIOS_COM_GPS_RX_BUF_LEN 96

#define PIOS_COM_TELEM_USB_RX_BUF_LEN 192
#define PIOS_COM_TELEM_USB_TX_BUF_LEN 192

#endif	/* PIOS_INCLUDE_COM */

/**
 * Pios servo configuration structures
 */
#include <pios_servo_priv.h>
static const struct pios_servo_channel pios_servo_channels[] = {
	{
		.timer = TIM4,
		.port = GPIOB,
		.channel = TIM_Channel_1,
		.pin = GPIO_Pin_6,
	}, 
	{
		.timer = TIM4,
		.port = GPIOB,
		.channel = TIM_Channel_2,
		.pin = GPIO_Pin_7,
	}, 
	{
		.timer = TIM4,
		.port = GPIOB,
		.channel = TIM_Channel_3,
		.pin = GPIO_Pin_8,
	}, 
	{
		.timer = TIM4,
		.port = GPIOB,
		.channel = TIM_Channel_4,
		.pin = GPIO_Pin_9,
	}, 
	{
		.timer = TIM8,
		.port = GPIOC,
		.channel = TIM_Channel_1,
		.pin = GPIO_Pin_6,
	}, 
	{
		.timer = TIM8,
		.port = GPIOC,
		.channel = TIM_Channel_2,
		.pin = GPIO_Pin_7,
	}, 
	{
		.timer = TIM8,
		.port = GPIOC,
		.channel = TIM_Channel_3,
		.pin = GPIO_Pin_8,
	}, 
	{
		.timer = TIM8,
		.port = GPIOC,
		.channel = TIM_Channel_4,
		.pin = GPIO_Pin_9,
	}, 	
};

const struct pios_servo_cfg pios_servo_cfg = {
	.tim_base_init = {
		.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1,
		.TIM_ClockDivision = TIM_CKD_DIV1,
		.TIM_CounterMode = TIM_CounterMode_Up,
		.TIM_Period = ((1000000 / PIOS_SERVO_UPDATE_HZ) - 1),
		.TIM_RepetitionCounter = 0x0000,
	},
	.tim_oc_init = {
		.TIM_OCMode = TIM_OCMode_PWM1,
		.TIM_OutputState = TIM_OutputState_Enable,
		.TIM_OutputNState = TIM_OutputNState_Disable,
		.TIM_Pulse = PIOS_SERVOS_INITIAL_POSITION,		
		.TIM_OCPolarity = TIM_OCPolarity_High,
		.TIM_OCNPolarity = TIM_OCPolarity_High,
		.TIM_OCIdleState = TIM_OCIdleState_Reset,
		.TIM_OCNIdleState = TIM_OCNIdleState_Reset,
	},
	.gpio_init = {
		.GPIO_Mode = GPIO_Mode_AF_PP,
		.GPIO_Speed = GPIO_Speed_2MHz,
	},
	.remap = 0,
	.channels = pios_servo_channels,
	.num_channels = NELEMENTS(pios_servo_channels),
};


/* 
 * PWM Inputs 
 */
#if defined(PIOS_INCLUDE_PWM)
#include <pios_pwm_priv.h>
static const struct pios_pwm_channel pios_pwm_channels[] = {
	{
		.timer = TIM1,
		.port = GPIOA,
		.ccr = TIM_IT_CC2,
		.channel = TIM_Channel_2,
		.pin = GPIO_Pin_9,
	}, 
	{
		.timer = TIM1,
		.port = GPIOA,
		.ccr = TIM_IT_CC3,
		.channel = TIM_Channel_3,
		.pin = GPIO_Pin_10,
	}, 
	{
		.timer = TIM5,
		.port = GPIOA,
		.ccr = TIM_IT_CC1,
		.channel = TIM_Channel_1,
		.pin = GPIO_Pin_0
	}, 
	{
		.timer = TIM1,
		.port = GPIOA,
		.ccr = TIM_IT_CC1,
		.channel = TIM_Channel_1,
		.pin = GPIO_Pin_8,
	}, 
	{ 
		.timer = TIM3,
		.port = GPIOB,
		.ccr = TIM_IT_CC4,
		.channel = TIM_Channel_4,
		.pin = GPIO_Pin_1,
	},  	
	{
		.timer = TIM3,
		.port = GPIOB,
		.ccr = TIM_IT_CC3,
		.channel = TIM_Channel_3,
		.pin = GPIO_Pin_0,
	}, 		
	{
		.timer = TIM3,
		.port = GPIOB,
		.ccr = TIM_IT_CC1,
		.channel = TIM_Channel_1,
		.pin = GPIO_Pin_4,
	}, 		
	{
		.timer = TIM3,
		.port = GPIOB,
		.ccr = TIM_IT_CC2,
		.channel = TIM_Channel_2,
		.pin = GPIO_Pin_5,
	}, 		
};

void TIM1_CC_IRQHandler();
void TIM3_IRQHandler();
void TIM5_IRQHandler();
void TIM1_CC_IRQHandler() __attribute__ ((alias ("PIOS_TIM1_CC_irq_handler")));
void TIM3_IRQHandler() __attribute__ ((alias ("PIOS_TIM3_irq_handler")));
void TIM5_IRQHandler() __attribute__ ((alias ("PIOS_TIM5_irq_handler")));
const struct pios_pwm_cfg pios_pwm_cfg = {
	.tim_base_init = {
		.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1,
		.TIM_ClockDivision = TIM_CKD_DIV1,
		.TIM_CounterMode = TIM_CounterMode_Up,
		.TIM_Period = 0xFFFF,
		.TIM_RepetitionCounter = 0x0000,
	},
	.tim_ic_init = {
		.TIM_ICPolarity = TIM_ICPolarity_Rising,
		.TIM_ICSelection = TIM_ICSelection_DirectTI,
		.TIM_ICPrescaler = TIM_ICPSC_DIV1,
		.TIM_ICFilter = 0x0,		
	},
	.gpio_init = {
		.GPIO_Mode = GPIO_Mode_IPD,
		.GPIO_Speed = GPIO_Speed_2MHz,
	},
	.remap = GPIO_PartialRemap_TIM3,
	.irq = {
		.init    = {
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		},
	},
	.channels = pios_pwm_channels,
	.num_channels = NELEMENTS(pios_pwm_channels),
};
void PIOS_TIM1_CC_irq_handler()
{
	PIOS_PWM_irq_handler(TIM1);
}
void PIOS_TIM3_irq_handler()
{
	PIOS_PWM_irq_handler(TIM3);
}
void PIOS_TIM5_irq_handler()
{
	PIOS_PWM_irq_handler(TIM5);
}
#endif

/*
 * PPM Input
 */
#if defined(PIOS_INCLUDE_PPM)
#include <pios_ppm_priv.h>
void TIM6_IRQHandler();
void TIM6_IRQHandler() __attribute__ ((alias ("PIOS_TIM6_irq_handler")));
static const struct pios_ppmsv_cfg pios_ppmsv_cfg = {
	.tim_base_init = {
		.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1,	/* For 1 uS accuracy */
		.TIM_ClockDivision = TIM_CKD_DIV1,
		.TIM_CounterMode = TIM_CounterMode_Up,
		.TIM_Period = ((1000000 / 25) - 1), /* 25 Hz */
		.TIM_RepetitionCounter = 0x0000,
	},
	.irq = {
		.init    = {
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		},
	},
	.timer = TIM6,
	.ccr = TIM_IT_Update,
};

void PIOS_TIM6_irq_handler(void)
{
	PIOS_PPMSV_irq_handler();
}

void TIM1_CC_IRQHandler();
void TIM1_CC_IRQHandler() __attribute__ ((alias ("PIOS_TIM1_CC_irq_handler")));
static const struct pios_ppm_cfg pios_ppm_cfg = {
	.tim_base_init = {
		.TIM_Prescaler = (PIOS_MASTER_CLOCK / 1000000) - 1,	/* For 1 uS accuracy */
		.TIM_ClockDivision = TIM_CKD_DIV1,
		.TIM_CounterMode = TIM_CounterMode_Up,
		.TIM_Period = 0xFFFF,
		.TIM_RepetitionCounter = 0x0000,
	},
	.tim_ic_init = {
			.TIM_ICPolarity = TIM_ICPolarity_Rising,
			.TIM_ICSelection = TIM_ICSelection_DirectTI,
			.TIM_ICPrescaler = TIM_ICPSC_DIV1,
			.TIM_ICFilter = 0x0,
			.TIM_Channel = TIM_Channel_2,
	},
	.gpio_init = {
			.GPIO_Mode = GPIO_Mode_IPD,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Pin = GPIO_Pin_9,
	},
	.remap = 0,
	.irq = {
		.init    = {
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
			.NVIC_IRQChannel = TIM1_CC_IRQn,
		},
	},
	.timer = TIM1,
	.port = GPIOA,
	.ccr = TIM_IT_CC2,
};

void PIOS_TIM1_CC_irq_handler(void)
{
	PIOS_PPM_irq_handler();
}

#endif //PPM

#if defined(PIOS_INCLUDE_I2C)

#include <pios_i2c_priv.h>

/*
 * I2C Adapters
 */

void PIOS_I2C_main_adapter_ev_irq_handler(void);
void PIOS_I2C_main_adapter_er_irq_handler(void);
void I2C2_EV_IRQHandler() __attribute__ ((alias ("PIOS_I2C_main_adapter_ev_irq_handler")));
void I2C2_ER_IRQHandler() __attribute__ ((alias ("PIOS_I2C_main_adapter_er_irq_handler")));

static const struct pios_i2c_adapter_cfg pios_i2c_main_adapter_cfg = {
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

uint32_t pios_i2c_main_adapter_id;
void PIOS_I2C_main_adapter_ev_irq_handler(void)
{
#ifdef I2C_DEBUG_PIN
	PIOS_DEBUG_PinHigh(I2C_DEBUG_PIN);
#endif
	/* Call into the generic code to handle the IRQ for this specific device */
	PIOS_I2C_EV_IRQ_Handler(pios_i2c_main_adapter_id);
#ifdef I2C_DEBUG_PIN
	PIOS_DEBUG_PinLow(I2C_DEBUG_PIN);
#endif
}

void PIOS_I2C_main_adapter_er_irq_handler(void)
{
  /* Call into the generic code to handle the IRQ for this specific device */
  PIOS_I2C_ER_IRQ_Handler(pios_i2c_main_adapter_id);
}

#endif /* PIOS_INCLUDE_I2C */

#if defined(PIOS_ENABLE_DEBUG_PINS)

static const struct stm32_gpio pios_debug_pins[] = {
  #define PIOS_DEBUG_PIN_SERVO_1 0
  {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_6,
      .GPIO_Speed = GPIO_Speed_50MHz,
      .GPIO_Mode  = GPIO_Mode_Out_PP,
    },
  },
  #define PIOS_DEBUG_PIN_SERVO_2 1
  {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_7,
      .GPIO_Speed = GPIO_Speed_50MHz,
      .GPIO_Mode  = GPIO_Mode_Out_PP,
    },
  },
  #define PIOS_DEBUG_PIN_SERVO_3 2
  {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_8,
      .GPIO_Speed = GPIO_Speed_50MHz,
      .GPIO_Mode  = GPIO_Mode_Out_PP,
    },
  },
  #define PIOS_DEBUG_PIN_SERVO_4 3
  {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_9,
      .GPIO_Speed = GPIO_Speed_50MHz,
      .GPIO_Mode  = GPIO_Mode_Out_PP,
    },
  },
  #define PIOS_DEBUG_PIN_SERVO_5 4
  {
    .gpio = GPIOC,
    .init = {
      .GPIO_Pin   = GPIO_Pin_6,
      .GPIO_Speed = GPIO_Speed_50MHz,
      .GPIO_Mode  = GPIO_Mode_Out_PP,
    },
  },
  #define PIOS_DEBUG_PIN_SERVO_6 5
  {
    .gpio = GPIOC,
    .init = {
      .GPIO_Pin   = GPIO_Pin_7,
      .GPIO_Speed = GPIO_Speed_50MHz,
      .GPIO_Mode  = GPIO_Mode_Out_PP,
    },
  },
  #define PIOS_DEBUG_PIN_SERVO_7 6
  {
    .gpio = GPIOC,
    .init = {
      .GPIO_Pin   = GPIO_Pin_8,
      .GPIO_Speed = GPIO_Speed_50MHz,
      .GPIO_Mode  = GPIO_Mode_Out_PP,
    },
  },
  #define PIOS_DEBUG_PIN_SERVO_8 7
  {
    .gpio = GPIOC,
    .init = {
      .GPIO_Pin   = GPIO_Pin_9,
      .GPIO_Speed = GPIO_Speed_50MHz,
      .GPIO_Mode  = GPIO_Mode_Out_PP,
    },
  },
};

#endif /* PIOS_ENABLE_DEBUG_PINS */

#if defined(PIOS_INCLUDE_RCVR)
#include "pios_rcvr_priv.h"

/* One slot per selectable receiver group.
 *  eg. PWM, PPM, GCS, SPEKTRUM1, SPEKTRUM2, SBUS
 * NOTE: No slot in this map for NONE.
 */
uint32_t pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_NONE];

#endif /* PIOS_INCLUDE_RCVR */

#if defined(PIOS_INCLUDE_USB_HID)
#include "pios_usb_hid_priv.h"

static const struct pios_usb_hid_cfg pios_usb_hid_main_cfg = {
  .irq = {
    .init    = {
      .NVIC_IRQChannel                   = USB_LP_CAN1_RX0_IRQn,
      .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_LOW,
      .NVIC_IRQChannelSubPriority        = 0,
      .NVIC_IRQChannelCmd                = ENABLE,
    },
  },
};
#endif	/* PIOS_INCLUDE_USB_HID */

extern const struct pios_com_driver pios_usb_com_driver;

uint32_t pios_com_telem_rf_id;
uint32_t pios_com_telem_usb_id;
uint32_t pios_com_gps_id;
uint32_t pios_com_aux_id;
uint32_t pios_com_spektrum_id;

#include "ahrs_spi_comm.h"

/**
 * PIOS_Board_Init()
 * initializes all the core subsystems on this specific hardware
 * called from System/openpilot.c
 */
void PIOS_Board_Init(void) {

	/* Remap AFIO pin */
	//GPIO_PinRemapConfig( GPIO_Remap_SWJ_NoJTRST, ENABLE);

	/* Debug services */
	PIOS_DEBUG_Init();

	/* Delay system */
	PIOS_DELAY_Init();	
	
#if defined(PIOS_INCLUDE_SPI)	
	/* Set up the SPI interface to the SD card */
	if (PIOS_SPI_Init(&pios_spi_sdcard_id, &pios_spi_sdcard_cfg)) {
		PIOS_Assert(0);
	}

	/* Enable and mount the SDCard */
	PIOS_SDCARD_Init(pios_spi_sdcard_id);
	PIOS_SDCARD_MountFS(0);
#endif /* PIOS_INCLUDE_SPI */

	/* Initialize UAVObject libraries */
	EventDispatcherInitialize();
	UAVObjInitialize();
	UAVObjectsInitializeAll();

#if defined(PIOS_INCLUDE_RTC)
	/* Initialize the real-time clock and its associated tick */
	PIOS_RTC_Init(&pios_rtc_main_cfg);
#endif

	/* Initialize the alarms library */
	AlarmsInitialize();

	/* Initialize the task monitor library */
	TaskMonitorInitialize();

	/* Prepare the AHRS Comms upper layer protocol */
	AhrsInitComms();

	/* Set up the SPI interface to the AHRS */
	if (PIOS_SPI_Init(&pios_spi_ahrs_id, &pios_spi_ahrs_cfg)) {
		PIOS_Assert(0);
	}

	/* Bind the AHRS comms layer to the AHRS SPI link */
	AhrsConnect(pios_spi_ahrs_id);

	/* Configure the main IO port */
	uint8_t hwsettings_op_mainport;
	HwSettingsOP_MainPortGet(&hwsettings_op_mainport);

	switch (hwsettings_op_mainport) {
	case HWSETTINGS_OP_MAINPORT_DISABLED:
		break;
	case HWSETTINGS_OP_MAINPORT_TELEMETRY:
#if defined(PIOS_INCLUDE_TELEMETRY_RF)
		{
			uint32_t pios_usart_telem_rf_id;
			if (PIOS_USART_Init(&pios_usart_telem_rf_id, &pios_usart_telem_cfg)) {
				PIOS_Assert(0);
			}

			uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_RX_BUF_LEN);
			uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_RF_TX_BUF_LEN);
			PIOS_Assert(rx_buffer);
			PIOS_Assert(tx_buffer);
			if (PIOS_COM_Init(&pios_com_telem_rf_id, &pios_usart_com_driver, pios_usart_telem_rf_id,
							  rx_buffer, PIOS_COM_TELEM_RF_RX_BUF_LEN,
							  tx_buffer, PIOS_COM_TELEM_RF_TX_BUF_LEN)) {
				PIOS_Assert(0);
			}
		}
#endif /* PIOS_INCLUDE_TELEMETRY_RF */
		break;
	}

	/* Configure the flexi port */
	uint8_t hwsettings_op_flexiport;
	HwSettingsOP_FlexiPortGet(&hwsettings_op_flexiport);

	switch (hwsettings_op_flexiport) {
	case HWSETTINGS_OP_FLEXIPORT_DISABLED:
		break;
	case HWSETTINGS_OP_FLEXIPORT_GPS:
#if defined(PIOS_INCLUDE_GPS)
		{
			uint32_t pios_usart_gps_id;
			if (PIOS_USART_Init(&pios_usart_gps_id, &pios_usart_gps_cfg)) {
				PIOS_Assert(0);
			}
			uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_GPS_RX_BUF_LEN);
			PIOS_Assert(rx_buffer);
			if (PIOS_COM_Init(&pios_com_gps_id, &pios_usart_com_driver, pios_usart_gps_id,
					  rx_buffer, PIOS_COM_GPS_RX_BUF_LEN,
					  NULL, 0)) {
				PIOS_Assert(0);
			}
		}
#endif	/* PIOS_INCLUDE_GPS */
		break;
	}

	PIOS_Servo_Init();
	PIOS_ADC_Init();
	PIOS_GPIO_Init();

	/* Configure the aux port */
	uint8_t hwsettings_op_auxport;
	HwSettingsOP_AuxPortGet(&hwsettings_op_auxport);

	switch (hwsettings_op_auxport) {
	case HWSETTINGS_OP_AUXPORT_DISABLED:
		break;
	case HWSETTINGS_OP_AUXPORT_DEBUG:
		/* Not supported yet */
		break;
	case HWSETTINGS_OP_AUXPORT_SPEKTRUM1:
#if defined(PIOS_INCLUDE_SPEKTRUM)
		{
			uint32_t pios_usart_spektrum_id;
			if (PIOS_USART_Init(&pios_usart_spektrum_id, &pios_usart_spektrum_cfg)) {
				PIOS_Assert(0);
			}

			uint32_t pios_spektrum_id;
			if (PIOS_SPEKTRUM_Init(&pios_spektrum_id, &pios_spektrum_cfg, &pios_usart_com_driver, pios_usart_spektrum_id, false)) {
				PIOS_Assert(0);
			}

			uint32_t pios_spektrum_rcvr_id;
			if (PIOS_RCVR_Init(&pios_spektrum_rcvr_id, &pios_spektrum_rcvr_driver, pios_spektrum_id)) {
				PIOS_Assert(0);
			}
			pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_SPEKTRUM1] = pios_spektrum_rcvr_id;
		}
#endif
		break;
	}

	/* Configure the rcvr port */
	uint8_t hwsettings_rcvrport;
	HwSettingsRcvrPortGet(&hwsettings_rcvrport);

	switch (hwsettings_rcvrport) {
	case HWSETTINGS_RCVRPORT_DISABLED:
		break;
	case HWSETTINGS_RCVRPORT_PWM:
#if defined(PIOS_INCLUDE_PWM)
		PIOS_PWM_Init();
		uint32_t pios_pwm_rcvr_id;
		if (PIOS_RCVR_Init(&pios_pwm_rcvr_id, &pios_pwm_rcvr_driver, 0)) {
			PIOS_Assert(0);
		}
		pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_PWM] = pios_pwm_rcvr_id;
#endif	/* PIOS_INCLUDE_PWM */
		break;
	case HWSETTINGS_RCVRPORT_PPM:
#if defined(PIOS_INCLUDE_PPM)
		PIOS_PPM_Init();
		uint32_t pios_ppm_rcvr_id;
		if (PIOS_RCVR_Init(&pios_ppm_rcvr_id, &pios_ppm_rcvr_driver, 0)) {
			PIOS_Assert(0);
		}
		pios_rcvr_group_map[MANUALCONTROLSETTINGS_CHANNELGROUPS_PPM] = pios_ppm_rcvr_id;
#endif	/* PIOS_INCLUDE_PPM */
		break;
	}

#if defined(PIOS_INCLUDE_USB_HID)
	uint32_t pios_usb_hid_id;
	PIOS_USB_HID_Init(&pios_usb_hid_id, &pios_usb_hid_main_cfg);
#if defined(PIOS_INCLUDE_COM)
	uint8_t * rx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_USB_RX_BUF_LEN);
	uint8_t * tx_buffer = (uint8_t *) pvPortMalloc(PIOS_COM_TELEM_USB_TX_BUF_LEN);
	PIOS_Assert(rx_buffer);
	PIOS_Assert(tx_buffer);
	if (PIOS_COM_Init(&pios_com_telem_usb_id, &pios_usb_com_driver, pios_usb_hid_id,
			  rx_buffer, PIOS_COM_TELEM_USB_RX_BUF_LEN,
			  tx_buffer, PIOS_COM_TELEM_USB_TX_BUF_LEN)) {
		PIOS_Assert(0);
	}
#endif	/* PIOS_INCLUDE_COM */
#endif	/* PIOS_INCLUDE_USB_HID */

#if defined(PIOS_INCLUDE_I2C)
	if (PIOS_I2C_Init(&pios_i2c_main_adapter_id, &pios_i2c_main_adapter_cfg)) {
		PIOS_Assert(0);
	}
#endif	/* PIOS_INCLUDE_I2C */
	PIOS_IAP_Init();
	PIOS_WDG_Init();
}

/**
 * @}
 */
