/**
 ******************************************************************************
 *
 * @file       pios_board.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Defines board specific static initializers for hardware for the AHRS board.
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

#if defined(PIOS_INCLUDE_SPI)

#include <pios_spi_priv.h>

/* OP Interface
 * 
 * NOTE: Leave this declared as const data so that it ends up in the 
 * .rodata section (ie. Flash) rather than in the .bss section (RAM).
 */
void PIOS_SPI_op_irq_handler(void);
void DMA1_Channel5_IRQHandler() __attribute__ ((alias ("PIOS_SPI_op_irq_handler")));
void DMA1_Channel4_IRQHandler() __attribute__ ((alias ("PIOS_SPI_op_irq_handler")));
static const struct pios_spi_cfg pios_spi_op_cfg = {
  .regs   = SPI2,
  .init   = {
    .SPI_Mode              = SPI_Mode_Slave,
    .SPI_Direction         = SPI_Direction_2Lines_FullDuplex,
    .SPI_DataSize          = SPI_DataSize_8b,
    .SPI_NSS               = SPI_NSS_Hard,
    .SPI_FirstBit          = SPI_FirstBit_MSB,
    .SPI_CRCPolynomial     = 7,
    .SPI_CPOL              = SPI_CPOL_High,
    .SPI_CPHA              = SPI_CPHA_2Edge,
  },
  .use_crc = TRUE,
  .dma = {
    .ahb_clk  = RCC_AHBPeriph_DMA1,
    
    .irq = {
      .handler = PIOS_SPI_op_irq_handler,
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
	.DMA_Priority           = DMA_Priority_Medium,
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
	.DMA_Priority           = DMA_Priority_Medium,
	.DMA_M2M                = DMA_M2M_Disable,
      },
    },
  },
  .ssel = {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_12,
      .GPIO_Speed = GPIO_Speed_50MHz,
      .GPIO_Mode  = GPIO_Mode_IN_FLOATING,
    },
  },
  .sclk = {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_13,
      .GPIO_Speed = GPIO_Speed_50MHz,
      .GPIO_Mode  = GPIO_Mode_IN_FLOATING,
    },
  },
  .miso = {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_14,
      .GPIO_Speed = GPIO_Speed_50MHz,
      .GPIO_Mode  = GPIO_Mode_AF_PP,
    },
  },
  .mosi = {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_15,
      .GPIO_Speed = GPIO_Speed_50MHz,
      .GPIO_Mode  = GPIO_Mode_IN_FLOATING,
    },
  },
};

/*
 * Board specific number of devices.
 */
struct pios_spi_dev pios_spi_devs[] = {
  {
    .cfg = &pios_spi_op_cfg,
  },
};

uint8_t pios_spi_num_devices = NELEMENTS(pios_spi_devs);

void PIOS_SPI_op_irq_handler(void)
{
  /* Call into the generic code to handle the IRQ for this specific device */
  PIOS_SPI_IRQ_Handler(PIOS_SPI_OP);
}

#endif /* PIOS_INCLUDE_SPI */

#if defined(PIOS_INCLUDE_USART)
#include <pios_usart_priv.h>

/*
 * AUX USART
 */
void PIOS_USART_aux_irq_handler(void);
void USART3_IRQHandler() __attribute__ ((alias ("PIOS_USART_aux_irq_handler")));
const struct pios_usart_cfg pios_usart_aux_cfg = {
  .regs = USART3,
  .init = {
	#if defined (PIOS_USART_BAUDRATE)
		  .USART_BaudRate            = PIOS_USART_BAUDRATE,
	#else
		  .USART_BaudRate            = 57600,
	#endif
    .USART_WordLength          = USART_WordLength_8b,
    .USART_Parity              = USART_Parity_No,
    .USART_StopBits            = USART_StopBits_1,
    .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
    .USART_Mode                = USART_Mode_Rx | USART_Mode_Tx,
  },
  .irq = {
    .handler = PIOS_USART_aux_irq_handler,
    .init    = {
      .NVIC_IRQChannel                   = USART3_IRQn,
      .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
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

/*
 * Board specific number of devices.
 */
struct pios_usart_dev pios_usart_devs[] = {
#define PIOS_USART_AUX    0
  {
    .cfg = &pios_usart_aux_cfg,
  },
};

uint8_t pios_usart_num_devices = NELEMENTS(pios_usart_devs);

void PIOS_USART_aux_irq_handler(void)
{
  PIOS_USART_IRQ_Handler(PIOS_USART_AUX);
}

#endif /* PIOS_INCLUDE_USART */

#if defined(PIOS_INCLUDE_COM)
#include <pios_com_priv.h>

/*
 * COM devices
 */

/*
 * Board specific number of devices.
 */
extern const struct pios_com_driver pios_usart_com_driver;

struct pios_com_dev pios_com_devs[] = {
#if defined(PIOS_INCLUDE_USART)	
  {
    .id     = PIOS_USART_AUX,
    .driver = &pios_usart_com_driver,
  },
#endif
};

const uint8_t pios_com_num_devices = NELEMENTS(pios_com_devs);

#endif /* PIOS_INCLUDE_COM */

#if defined(PIOS_INCLUDE_I2C)
#include <pios_i2c_priv.h>

/*
 * I2C Adapters
 */

void PIOS_I2C_main_adapter_ev_irq_handler(void);
void PIOS_I2C_main_adapter_er_irq_handler(void);
void I2C1_EV_IRQHandler() __attribute__ ((alias ("PIOS_I2C_main_adapter_ev_irq_handler")));
void I2C1_ER_IRQHandler() __attribute__ ((alias ("PIOS_I2C_main_adapter_er_irq_handler")));

const struct pios_i2c_adapter_cfg pios_i2c_main_adapter_cfg = {
  .regs = I2C1,
  .init = {
    .I2C_Mode                = I2C_Mode_I2C,
    .I2C_OwnAddress1         = 0,
    .I2C_Ack                 = I2C_Ack_Enable,
    .I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit,
    .I2C_DutyCycle           = I2C_DutyCycle_2,
    .I2C_ClockSpeed          = 200000,	/* bits/s */
  },
  .transfer_timeout_ms = 50,
  .scl = {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_6,
      .GPIO_Speed = GPIO_Speed_10MHz,
      .GPIO_Mode  = GPIO_Mode_AF_OD,
    },
  },
  .sda = {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_7,
      .GPIO_Speed = GPIO_Speed_10MHz,
      .GPIO_Mode  = GPIO_Mode_AF_OD,
    },
  },
  .event = {
    .handler = PIOS_I2C_main_adapter_ev_irq_handler,
    .flags   = 0,		/* FIXME: check this */
    .init = {
      .NVIC_IRQChannel                   = I2C1_EV_IRQn,
      //.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGHEST,
      .NVIC_IRQChannelPreemptionPriority = 2,
      .NVIC_IRQChannelSubPriority        = 0,
      .NVIC_IRQChannelCmd                = ENABLE,
    },
  },
  .error = {
    .handler = PIOS_I2C_main_adapter_er_irq_handler,
    .flags   = 0,		/* FIXME: check this */
    .init = {
      .NVIC_IRQChannel                   = I2C1_ER_IRQn,
      //.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGHEST,
      .NVIC_IRQChannelPreemptionPriority = 2,
      .NVIC_IRQChannelSubPriority        = 0,
      .NVIC_IRQChannelCmd                = ENABLE,
    },
  },
};

/*
 * Board specific number of devices.
 */
struct pios_i2c_adapter pios_i2c_adapters[] = {
  {
    .cfg = &pios_i2c_main_adapter_cfg,
  },
};

uint8_t pios_i2c_num_adapters = NELEMENTS(pios_i2c_adapters);

void PIOS_I2C_main_adapter_ev_irq_handler(void)
{
  /* Call into the generic code to handle the IRQ for this specific device */
  PIOS_I2C_EV_IRQ_Handler(PIOS_I2C_MAIN_ADAPTER);
}

void PIOS_I2C_main_adapter_er_irq_handler(void)
{
  /* Call into the generic code to handle the IRQ for this specific device */
  PIOS_I2C_ER_IRQ_Handler(PIOS_I2C_MAIN_ADAPTER);
}

#endif /* PIOS_INCLUDE_I2C */
