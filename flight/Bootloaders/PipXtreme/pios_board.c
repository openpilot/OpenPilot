/**
 ******************************************************************************
 *
 * @file       pios_board.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Defines board specific static initializers for hardware for the PipBee board.
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

/**
 * PIOS_Board_Init()
 * initializes all the core subsystems on this specific hardware
 * called from System/openpilot.c
 */
void PIOS_Board_Init(void) {


	/* Enable Prefetch Buffer */
	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

	/* Flash 2 wait state */
	FLASH_SetLatency(FLASH_Latency_2);

	/* Delay system */
	PIOS_DELAY_Init();	
	
	/* SPI Init */
	// PIOS_SPI_Init();


	/* Initialize the PiOS library */
	PIOS_COM_Init();
	PIOS_GPIO_Init();

#if defined(PIOS_INCLUDE_USB_HID)
	PIOS_USB_HID_Init(0);
#endif
	//PIOS_I2C_Init();
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);//TODO Tirar
}


// ***********************************************************************************
// SPI

#if defined(PIOS_INCLUDE_SPI)

#include <pios_spi_priv.h>

/* OP Interface
 *
 * NOTE: Leave this declared as const data so that it ends up in the
 * .rodata section (ie. Flash) rather than in the .bss section (RAM).
 */
void PIOS_SPI_port_irq_handler(void);
void DMA1_Channel5_IRQHandler() __attribute__ ((alias ("PIOS_SPI_port_irq_handler")));
void DMA1_Channel4_IRQHandler() __attribute__ ((alias ("PIOS_SPI_port_irq_handler")));

static const struct pios_spi_cfg pios_spi_port_cfg =
{
	.regs = SPI1,
//	.regs = SPI2,
//	.regs = SPI3,

	.init =
	{
		.SPI_Mode = SPI_Mode_Master,
//		.SPI_Mode = SPI_Mode_Slave,

		.SPI_Direction = SPI_Direction_2Lines_FullDuplex,
//		.SPI_Direction = SPI_Direction_2Lines_RxOnly,
//		.SPI_Direction = SPI_Direction_1Line_Rx,
//		.SPI_Direction = SPI_Direction_1Line_Tx,

//		.SPI_DataSize = SPI_DataSize_16b,
		.SPI_DataSize = SPI_DataSize_8b,

		.SPI_NSS = SPI_NSS_Soft,
//		.SPI_NSS = SPI_NSS_Hard,

		.SPI_FirstBit = SPI_FirstBit_MSB,
//		.SPI_FirstBit = SPI_FirstBit_LSB,

		.SPI_CRCPolynomial = 0,

		.SPI_CPOL = SPI_CPOL_Low,
//		.SPI_CPOL = SPI_CPOL_High,

		.SPI_CPHA = SPI_CPHA_1Edge,
//		.SPI_CPHA = SPI_CPHA_2Edge,

//		.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2,		// fastest SCLK
//		.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4,
//		.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8,
//		.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16,
//		.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32,
//		.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64,
//		.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128,
		.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256,		// slowest SCLK
	},
	.use_crc = FALSE,

	.dma =
	{
		.ahb_clk = RCC_AHBPeriph_DMA1,
		.irq =
		{
			.handler = PIOS_SPI_port_irq_handler,
		      .flags   = (DMA1_FLAG_TC2 | DMA1_FLAG_TE2 | DMA1_FLAG_HT2 | DMA1_FLAG_GL2),
		      .init    = {
			.NVIC_IRQChannel                   = DMA1_Channel2_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
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

	.ssel =
	{
		.gpio = GPIOA,
		.init =
		{
			.GPIO_Pin = GPIO_Pin_4,
			.GPIO_Speed = GPIO_Speed_10MHz,
			.GPIO_Mode = GPIO_Mode_Out_PP,
		},
	},
	.sclk =
	{
		.gpio = GPIOA,
		.init =
		{
			.GPIO_Pin = GPIO_Pin_5,
			.GPIO_Speed = GPIO_Speed_10MHz,
			.GPIO_Mode = GPIO_Mode_AF_PP,
		},
	},
	.miso =
	{
		.gpio = GPIOA,
		.init =
		{
			.GPIO_Pin = GPIO_Pin_6,
			.GPIO_Speed = GPIO_Speed_10MHz,
			.GPIO_Mode  = GPIO_Mode_IN_FLOATING,
//		    .GPIO_Mode = GPIO_Mode_IPU,
		},
	},
	.mosi =
	{
		.gpio = GPIOA,
		.init =
		{
			.GPIO_Pin = GPIO_Pin_7,
			.GPIO_Speed = GPIO_Speed_10MHz,
			.GPIO_Mode = GPIO_Mode_AF_PP,
		},
	},
};

/*
 * Board specific number of devices.
 */
struct pios_spi_dev pios_spi_devs[] =
{
	{
		.cfg = &pios_spi_port_cfg,
	},
};

uint8_t pios_spi_num_devices = NELEMENTS(pios_spi_devs);

void PIOS_SPI_port_irq_handler(void)
{
	/* Call into the generic code to handle the IRQ for this specific device */
	PIOS_SPI_IRQ_Handler(PIOS_SPI_PORT);
}

#endif /* PIOS_INCLUDE_SPI */

// ***********************************************************************************
// USART

#if defined(PIOS_INCLUDE_USART)
#include <pios_usart_priv.h>

/*
 * SERIAL USART
 */
void PIOS_USART_serial_irq_handler(void);
void USART1_IRQHandler() __attribute__ ((alias ("PIOS_USART_serial_irq_handler")));

const struct pios_usart_cfg pios_usart_serial_cfg =
{
	.regs = USART1,
	.init =
	{
		#if defined(PIOS_USART_BAUDRATE)
			.USART_BaudRate = PIOS_USART_BAUDRATE,
		#else
			.USART_BaudRate = 57600,
		#endif
		.USART_WordLength = USART_WordLength_8b,
		.USART_Parity = USART_Parity_No,
		.USART_StopBits = USART_StopBits_1,
		.USART_HardwareFlowControl = USART_HardwareFlowControl_None,
		.USART_Mode = USART_Mode_Rx | USART_Mode_Tx,
	},
	.irq =
	{
		.handler = PIOS_USART_serial_irq_handler,
		.init =
		{
			.NVIC_IRQChannel = USART1_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
	.rx =
	{
		.gpio = GPIOA,
		.init =
		{
			.GPIO_Pin = GPIO_Pin_10,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode = GPIO_Mode_IPU,
		},
	},
	.tx =
	{
		.gpio = GPIOA,
		.init =
		{
			.GPIO_Pin = GPIO_Pin_9,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode = GPIO_Mode_AF_PP,
		},
	},
};

/*
 * Board specific number of devices.
 */
struct pios_usart_dev pios_usart_devs[] =
{
	#define PIOS_USART_SERIAL    0
	{
		.cfg = &pios_usart_serial_cfg,
	},
};

uint8_t pios_usart_num_devices = NELEMENTS(pios_usart_devs);

void PIOS_USART_serial_irq_handler(void)
{
	PIOS_USART_IRQ_Handler(PIOS_USART_SERIAL);
}

#endif /* PIOS_INCLUDE_USART */

// ***********************************************************************************

#if defined(PIOS_INCLUDE_COM)
#include <pios_com_priv.h>

/*
 * COM devices
 */

/*
 * Board specific number of devices.
 */
extern const struct pios_com_driver pios_usart_com_driver;
#if defined(PIOS_INCLUDE_USB_HID)
	extern const struct pios_com_driver pios_usb_com_driver;
#endif

struct pios_com_dev pios_com_devs[] = { { .id = 0,
		.driver = &pios_usb_com_driver, },
#if defined(PIOS_INCLUDE_USB_HID)
		{
			.id = 0,
			.driver = &pios_usb_com_driver,
		},
#endif
		};

const uint8_t pios_com_num_devices = NELEMENTS(pios_com_devs);

#endif /* PIOS_INCLUDE_COM */

// ***********************************************************************************
