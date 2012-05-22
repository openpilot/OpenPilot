/**
 ******************************************************************************
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotCore OpenPilot Core
 * @{
 *
 * @file       board_hw_defs.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
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
#include <pios_config.h>

#if defined(PIOS_INCLUDE_LED)

#include <pios_led_priv.h>
static const struct pios_led pios_leds[] = {
	[PIOS_LED_HEARTBEAT] = {
		.pin = {
			.gpio = GPIOC,
			.init = {
				.GPIO_Pin   = GPIO_Pin_5,
				.GPIO_Speed = GPIO_Speed_50MHz,
				.GPIO_Mode  = GPIO_Mode_OUT,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd = GPIO_PuPd_UP
			},
		},
	},
	[PIOS_LED_ALARM] = {
		.pin = {
			.gpio = GPIOC,
			.init = {
				.GPIO_Pin   = GPIO_Pin_4,
				.GPIO_Speed = GPIO_Speed_50MHz,
				.GPIO_Mode  = GPIO_Mode_OUT,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd = GPIO_PuPd_UP
			},
		},
	},
};

static const struct pios_led_cfg pios_led_cfg = {
	.leds     = pios_leds,
	.num_leds = NELEMENTS(pios_leds),
};

#endif	/* PIOS_INCLUDE_LED */

#include <pios_usart_priv.h>

#if defined(PIOS_INCLUDE_GPS)
/*
 * GPS USART
 */
static const struct pios_usart_cfg pios_usart_gps_cfg = {
	.regs = USART1,
	.remap = GPIO_AF_USART1,
	.init = {
		.USART_BaudRate = 38400,
		.USART_WordLength = USART_WordLength_8b,
		.USART_Parity = USART_Parity_No,
		.USART_StopBits = USART_StopBits_1,
		.USART_HardwareFlowControl =
		USART_HardwareFlowControl_None,
		.USART_Mode = USART_Mode_Rx | USART_Mode_Tx,
	},
	.irq = {
		.init = {
			.NVIC_IRQChannel = USART1_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_LOW,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
	.rx = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_10,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode  = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd  = GPIO_PuPd_UP
		},
	},
	.tx = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_9,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode  = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd  = GPIO_PuPd_UP
		},
	},
};

#endif /* PIOS_INCLUDE_GPS */

#ifdef PIOS_INCLUDE_COM_AUX
/*
 * AUX USART
 */
static const struct pios_usart_cfg pios_usart_aux_cfg = {
	.regs = USART2,
	.remap = GPIO_AF_USART2,
	.init = {
		.USART_BaudRate = 230400,
		.USART_WordLength = USART_WordLength_8b,
		.USART_Parity = USART_Parity_No,
		.USART_StopBits = USART_StopBits_1,
		.USART_HardwareFlowControl =
		USART_HardwareFlowControl_None,
		.USART_Mode = USART_Mode_Rx | USART_Mode_Tx,
	},
	.irq = {
		.init = {
			.NVIC_IRQChannel = USART2_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
	.rx = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_3,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode  = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd  = GPIO_PuPd_UP
		},
	},
	.tx = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_2,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode  = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd  = GPIO_PuPd_UP
		},
	},
};

#endif /* PIOS_COM_AUX */

#ifdef PIOS_INCLUDE_COM_TELEM
/*
 * Telemetry on main USART
 */
static const struct pios_usart_cfg pios_usart_telem_main_cfg = {
	.regs = UART4,
	.remap = GPIO_AF_UART4,
	.init = {
		.USART_BaudRate = 57600,
		.USART_WordLength = USART_WordLength_8b,
		.USART_Parity = USART_Parity_No,
		.USART_StopBits = USART_StopBits_1,
		.USART_HardwareFlowControl =
		USART_HardwareFlowControl_None,
		.USART_Mode = USART_Mode_Rx | USART_Mode_Tx,
	},
	.irq = {
		.init = {
			.NVIC_IRQChannel = UART4_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
	.rx = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_1,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode  = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd  = GPIO_PuPd_UP
		},
	},
	.tx = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_0,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode  = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd  = GPIO_PuPd_UP
		},
	},
};

#endif /* PIOS_COM_TELEM */


#if 0
/*
 * COTelemetry on main USART
 */
static const struct pios_usart_cfg pios_usart_cotelem_main_cfg = {
	.regs = UART4,
	.remap = GPIO_AF_UART4,
	.init = {
		.USART_BaudRate = 57600,
		.USART_WordLength = USART_WordLength_8b,
		.USART_Parity = USART_Parity_No,
		.USART_StopBits = USART_StopBits_1,
		.USART_HardwareFlowControl =
		USART_HardwareFlowControl_None,
		.USART_Mode = USART_Mode_Rx | USART_Mode_Tx,
	},
	.irq = {
		.init = {
			.NVIC_IRQChannel = UART4_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
	.rx = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_1,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode  = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd  = GPIO_PuPd_UP
		},
	},
	.tx = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_0,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode  = GPIO_Mode_AF,
			.GPIO_OType = GPIO_OType_PP,
			.GPIO_PuPd  = GPIO_PuPd_UP
		},
	},
};

#endif /* PIOS_COM_COTELEM */


#if defined(PIOS_INCLUDE_COM)

#include <pios_com_priv.h>

#endif /* PIOS_INCLUDE_COM */


#if defined(PIOS_INCLUDE_RTC)
/*
 * Realtime Clock (RTC)
 */
#include <pios_rtc_priv.h>

void PIOS_RTC_IRQ_Handler (void);
void RTC_WKUP_IRQHandler() __attribute__ ((alias ("PIOS_RTC_IRQ_Handler")));
static const struct pios_rtc_cfg pios_rtc_main_cfg = {
	.clksrc = RCC_RTCCLKSource_HSE_Div8, // Divide 8 Mhz crystal down to 1
	// For some reason it's acting like crystal is 16 Mhz.  This clock is then divided
	// by another 16 to give a nominal 62.5 khz clock
	.prescaler = 100, // Every 100 cycles gives 625 Hz
	.irq = {
		.init = {
			.NVIC_IRQChannel                   = RTC_WKUP_IRQn,
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


#if defined(PIOS_INCLUDE_USB)
#include "pios_usb_priv.h"

static const struct pios_usb_cfg pios_usb_main_cfg = {
	.irq = {
		.init    = {
			.NVIC_IRQChannel                   = OTG_FS_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_LOW,
			.NVIC_IRQChannelSubPriority        = 0,
			.NVIC_IRQChannelCmd                = ENABLE,
		},
	},
	.vsense = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin   = GPIO_Pin_15,
			.GPIO_Speed = GPIO_Speed_25MHz,
			.GPIO_Mode  = GPIO_Mode_IN,
			.GPIO_OType = GPIO_OType_OD,
		},
	}
};

#include "pios_usb_board_data_priv.h"
#include "pios_usb_desc_hid_cdc_priv.h"
#include "pios_usb_desc_hid_only_priv.h"
#include "pios_usbhook.h"

#endif	/* PIOS_INCLUDE_USB */

#if defined(PIOS_INCLUDE_COM_MSG)

#include <pios_com_msg_priv.h>

#endif /* PIOS_INCLUDE_COM_MSG */

#if defined(PIOS_INCLUDE_USB_HID)
#include <pios_usb_hid_priv.h>

const struct pios_usb_hid_cfg pios_usb_hid_cfg = {
	.data_if = 0,
	.data_rx_ep = 1,
	.data_tx_ep = 1,
};
#endif /* PIOS_INCLUDE_USB_HID */

#if defined(PIOS_INCLUDE_USB_CDC)
#include <pios_usb_cdc_priv.h>

const struct pios_usb_cdc_cfg pios_usb_cdc_cfg = {
	.ctrl_if = 1,
	.ctrl_tx_ep = 2,

	.data_if = 2,
	.data_rx_ep = 3,
	.data_tx_ep = 3,
};
#endif	/* PIOS_INCLUDE_USB_CDC */

#if defined(PIOS_INCLUDE_VIDEO)

#include <pios_video.h>
static const struct pios_exti_cfg pios_exti_hsync_cfg __exti_config = {
	.vector = PIOS_Hsync_ISR,
	.line = EXTI_Line7,
	.pin = {
		.gpio = GPIOB,
		.init = {
			.GPIO_Pin = GPIO_Pin_7,
			.GPIO_Speed = GPIO_Speed_100MHz,
			.GPIO_Mode = GPIO_Mode_IN,
			.GPIO_OType = GPIO_OType_OD,
			.GPIO_PuPd = GPIO_PuPd_NOPULL,
		},
	},
	.irq = {
		.init = {
			.NVIC_IRQChannel = EXTI9_5_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGHEST,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
	.exti = {
		.init = {
			.EXTI_Line = EXTI_Line7, // matches above GPIO pin
			.EXTI_Mode = EXTI_Mode_Interrupt,
			//.EXTI_Trigger = EXTI_Trigger_Rising_Falling,
			.EXTI_Trigger = EXTI_Trigger_Falling,
			.EXTI_LineCmd = ENABLE,
		},
	},
};
static const struct pios_exti_cfg pios_exti_vsync_cfg __exti_config = {
		.vector = PIOS_Vsync_ISR,
		.line = EXTI_Line5,
		.pin = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin = GPIO_Pin_5,
				.GPIO_Speed = GPIO_Speed_100MHz,
				.GPIO_Mode = GPIO_Mode_IN,
				.GPIO_OType = GPIO_OType_OD,
				.GPIO_PuPd = GPIO_PuPd_NOPULL,
			},
		},
		.irq = {
			.init = {
				.NVIC_IRQChannel = EXTI9_5_IRQn,
				.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
				.NVIC_IRQChannelSubPriority = 0,
				.NVIC_IRQChannelCmd = ENABLE,
			},
		},
		.exti = {
			.init = {
				.EXTI_Line = EXTI_Line5, // matches above GPIO pin
				.EXTI_Mode = EXTI_Mode_Interrupt,
				.EXTI_Trigger = EXTI_Trigger_Falling,
				.EXTI_LineCmd = ENABLE,
			},
		},
};


static const struct pios_video_cfg pios_video_cfg = {
	.mask = {
		.regs = SPI3,
		.remap = GPIO_AF_SPI3,
		.init = {
			.SPI_Mode              = SPI_Mode_Slave,
			.SPI_Direction         = SPI_Direction_1Line_Tx,
			.SPI_DataSize          = SPI_DataSize_16b,
			.SPI_NSS               = SPI_NSS_Soft,
			.SPI_FirstBit          = SPI_FirstBit_MSB,
			.SPI_CRCPolynomial     = 7,
			.SPI_CPOL              = SPI_CPOL_Low,
			.SPI_CPHA              = SPI_CPHA_2Edge,
			.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4,
		},
		.use_crc = false,
		.dma = {
			.irq = {
				// Note this is the stream ID that triggers interrupts (in this case RX)
				.flags = (DMA_IT_TCIF7),
				.init = {
					.NVIC_IRQChannel = DMA1_Stream7_IRQn,
					.NVIC_IRQChannelPreemptionPriority = 0,
					.NVIC_IRQChannelSubPriority = 0,
					.NVIC_IRQChannelCmd = ENABLE,
				},
			},

			.rx = {
				//not used
				.channel = DMA1_Stream4,
				.init = {
					.DMA_Channel            = DMA_Channel_0,
					.DMA_PeripheralBaseAddr = (uint32_t) & (SPI3->DR),
					.DMA_DIR                = DMA_DIR_PeripheralToMemory,
					.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
					.DMA_MemoryInc          = DMA_MemoryInc_Enable,
					.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
					.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord,
					.DMA_Mode               = DMA_Mode_Normal,
					.DMA_Priority           = DMA_Priority_Medium,
					//TODO: Enable FIFO
					.DMA_FIFOMode           = DMA_FIFOMode_Disable,
					.DMA_FIFOThreshold      = DMA_FIFOThreshold_Full,
					.DMA_MemoryBurst        = DMA_MemoryBurst_Single,
					.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single,
				},
			},
			.tx = {
				.channel = DMA1_Stream7,
				.init = {
					.DMA_Channel            = DMA_Channel_0,
					.DMA_PeripheralBaseAddr = (uint32_t) & (SPI3->DR),
					.DMA_DIR                = DMA_DIR_MemoryToPeripheral,
					.DMA_BufferSize 		= BUFFER_LINE_LENGTH,
					.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
					.DMA_MemoryInc          = DMA_MemoryInc_Enable,
					.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
					.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord,
					.DMA_Mode               = DMA_Mode_Normal,
					.DMA_Priority           = DMA_Priority_High,
					.DMA_FIFOMode           = DMA_FIFOMode_Disable,
					.DMA_FIFOThreshold      = DMA_FIFOThreshold_Full,
					.DMA_MemoryBurst        = DMA_MemoryBurst_Single,
					.DMA_PeripheralBurst    = DMA_PeripheralBurst_Single,
				},
			},
		},
		.sclk = {
			.gpio = GPIOC,
			.init = {
				.GPIO_Pin = GPIO_Pin_10,
				.GPIO_Speed = GPIO_Speed_100MHz,
				.GPIO_Mode = GPIO_Mode_AF,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd = GPIO_PuPd_NOPULL
			},
		},
		.miso = {
			.gpio = GPIOC,
			.init = {
				.GPIO_Pin = GPIO_Pin_11,
				.GPIO_Speed = GPIO_Speed_50MHz,
				.GPIO_Mode = GPIO_Mode_AF,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd = GPIO_PuPd_NOPULL
			},
		},
		.mosi = {
			.gpio = GPIOC,
			.init = {
				.GPIO_Pin = GPIO_Pin_12,
				.GPIO_Speed = GPIO_Speed_50MHz,
				.GPIO_Mode = GPIO_Mode_AF,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd = GPIO_PuPd_NOPULL
			},
		},
		.slave_count = 1,
	},
	.level = {
			.regs = SPI1,
			.remap = GPIO_AF_SPI1,
			.init   = {
				.SPI_Mode              = SPI_Mode_Slave,
				.SPI_Direction         = SPI_Direction_1Line_Tx,
				.SPI_DataSize          = SPI_DataSize_16b,
				.SPI_NSS               = SPI_NSS_Soft,
				.SPI_FirstBit          = SPI_FirstBit_MSB,
				.SPI_CRCPolynomial     = 7,
				.SPI_CPOL              = SPI_CPOL_Low,
				.SPI_CPHA              = SPI_CPHA_2Edge,
				.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2,
			},
			.use_crc = false,
			.dma = {
				.irq = {
					.flags   = (DMA_IT_TCIF5),
					.init    = {
						.NVIC_IRQChannel                   = DMA2_Stream5_IRQn,
						.NVIC_IRQChannelPreemptionPriority = 0,
						.NVIC_IRQChannelSubPriority        = 0,
						.NVIC_IRQChannelCmd                = ENABLE,
					},
				},

				.rx = {
					//not used
					.channel = DMA2_Stream0,
					.init    = {
		                .DMA_Channel            = DMA_Channel_3,
						.DMA_PeripheralBaseAddr = (uint32_t)&(SPI1->DR),
						.DMA_DIR                = DMA_DIR_PeripheralToMemory,
						.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
						.DMA_MemoryInc          = DMA_MemoryInc_Enable,
						.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
						.DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
						.DMA_Mode               = DMA_Mode_Normal,
						.DMA_Priority           = DMA_Priority_Medium,
						.DMA_FIFOMode           = DMA_FIFOMode_Disable,
		                /* .DMA_FIFOThreshold */
		                .DMA_MemoryBurst        = DMA_MemoryBurst_Single,
		                .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single,
					},
				},
				.tx = {
					.channel = DMA2_Stream5,
					.init    = {
		                .DMA_Channel            = DMA_Channel_3,
						.DMA_PeripheralBaseAddr = (uint32_t)&(SPI1->DR),
						.DMA_DIR                = DMA_DIR_MemoryToPeripheral,
						.DMA_BufferSize 		= BUFFER_LINE_LENGTH,
						.DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
						.DMA_MemoryInc          = DMA_MemoryInc_Enable,
						.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord,
						.DMA_MemoryDataSize     = DMA_MemoryDataSize_HalfWord,
						.DMA_Mode               = DMA_Mode_Normal,
						.DMA_Priority           = DMA_Priority_High,
						.DMA_FIFOMode           = DMA_FIFOMode_Disable,
						.DMA_FIFOThreshold      = DMA_FIFOThreshold_Full,
		                .DMA_MemoryBurst        = DMA_MemoryBurst_Single,
		                .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single,
					},
				},
			},
			.sclk = {
				.gpio = GPIOB,
				.init = {
					.GPIO_Pin   = GPIO_Pin_3,
					.GPIO_Speed = GPIO_Speed_100MHz,
					.GPIO_Mode  = GPIO_Mode_AF,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_UP
				},
			},
			.miso = {
				.gpio = GPIOB,
				.init = {
					.GPIO_Pin   = GPIO_Pin_4,
					.GPIO_Speed = GPIO_Speed_50MHz,
					.GPIO_Mode  = GPIO_Mode_AF,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_UP
				},
			},
			.mosi = {
				.gpio = GPIOB,
				.init = {
					.GPIO_Pin   = GPIO_Pin_5,
					.GPIO_Speed = GPIO_Speed_50MHz,
					.GPIO_Mode  = GPIO_Mode_AF,
					.GPIO_OType = GPIO_OType_PP,
					.GPIO_PuPd = GPIO_PuPd_UP
				},
			},
			.slave_count = 1,

	},

	.hsync = &pios_exti_hsync_cfg,
	.vsync = &pios_exti_vsync_cfg,
	
	.pixel_timer = {
		.timer = TIM4,
		.timer_chan = TIM_Channel_1,
		.pin = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin = GPIO_Pin_6,
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Mode  = GPIO_Mode_AF,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd  = GPIO_PuPd_UP
			},
			.pin_source = GPIO_PinSource6,
		},
		.remap = GPIO_AF_TIM4,
	},
	.hsync_capture = {
		.timer = TIM4,
		.timer_chan = TIM_Channel_2,
		.pin = {
			.gpio = GPIOB,
			.init = {
				.GPIO_Pin = GPIO_Pin_7,
				.GPIO_Speed = GPIO_Speed_2MHz,
				.GPIO_Mode  = GPIO_Mode_AF,
				.GPIO_OType = GPIO_OType_PP,
				.GPIO_PuPd  = GPIO_PuPd_UP
			},
			.pin_source = GPIO_PinSource7,
		},
		.remap = GPIO_AF_TIM4,
	},
	.tim_oc_init = {
		.TIM_OCMode = TIM_OCMode_PWM1,
		.TIM_OutputState = TIM_OutputState_Enable,
		.TIM_OutputNState = TIM_OutputNState_Disable,
		.TIM_Pulse = 1,
		.TIM_OCPolarity = TIM_OCPolarity_High,
		.TIM_OCNPolarity = TIM_OCPolarity_High,
		.TIM_OCIdleState = TIM_OCIdleState_Reset,
		.TIM_OCNIdleState = TIM_OCNIdleState_Reset,
	},
};





#endif

