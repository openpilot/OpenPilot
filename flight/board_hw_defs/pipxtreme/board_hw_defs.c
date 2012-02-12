#include <pios_config.h>

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

	.init =
	{
		.SPI_Mode = SPI_Mode_Master,
		.SPI_Direction = SPI_Direction_2Lines_FullDuplex,
		.SPI_DataSize = SPI_DataSize_8b,
		.SPI_NSS = SPI_NSS_Soft,
		.SPI_FirstBit = SPI_FirstBit_MSB,
		.SPI_CRCPolynomial = 0,
		.SPI_CPOL = SPI_CPOL_Low,
		.SPI_CPHA = SPI_CPHA_1Edge,
		.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256,		// slowest SCLK
	},
	.use_crc = FALSE,

	.dma =
	{
		.ahb_clk = RCC_AHBPeriph_DMA1,
		.irq =
		{
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

uint32_t pios_spi_port_id;
void PIOS_SPI_port_irq_handler(void)
{
	/* Call into the generic code to handle the IRQ for this specific device */
	PIOS_SPI_IRQ_Handler(pios_spi_port_id);
}

#endif /* PIOS_INCLUDE_SPI */

#if defined(PIOS_INCLUDE_ADC)

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

#endif	/* PIOS_INCLUDE_ADC */

#if defined(PIOS_INCLUDE_USART)

#include <pios_usart_priv.h>

/*
 * SERIAL USART
 */
static const struct pios_usart_cfg pios_usart_serial_cfg =
{
	.regs = USART1,
	.init =
	{
		.USART_BaudRate = 57600,
		.USART_WordLength = USART_WordLength_8b,
		.USART_Parity = USART_Parity_No,
		.USART_StopBits = USART_StopBits_1,
		.USART_HardwareFlowControl = USART_HardwareFlowControl_None,
		.USART_Mode = USART_Mode_Rx | USART_Mode_Tx,
	},
	.irq =
	{
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

#endif /* PIOS_INCLUDE_USART */

#if defined(PIOS_INCLUDE_COM)

#include <pios_com_priv.h>

#endif /* PIOS_INCLUDE_COM */

// ***********************************************************************************

#if defined(PIOS_INCLUDE_USB)
#include "pios_usb_priv.h"

static const struct pios_usb_cfg pios_usb_main_cfg = {
  .irq = {
    .init    = {
      .NVIC_IRQChannel                   = USB_LP_CAN1_RX0_IRQn,
      .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_LOW,
      .NVIC_IRQChannelSubPriority        = 0,
      .NVIC_IRQChannelCmd                = ENABLE,
    },
  },
};

#include "pios_usb_board_data_priv.h"
#include "pios_usb_desc_hid_only_priv.h"

#endif	/* PIOS_INCLUDE_USB */

#if defined(PIOS_INCLUDE_USB_HID)
#include <pios_usb_hid_priv.h>

const struct pios_usb_hid_cfg pios_usb_hid_cfg = {
	.data_if = 0,
	.data_rx_ep = 1,
	.data_tx_ep = 1,
};

#endif	/* PIOS_INCLUDE_USB_HID */
