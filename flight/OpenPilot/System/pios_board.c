/**
 ******************************************************************************
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
#include <pios_spi_priv.h>

/* MicroSD Interface
 * 
 * NOTE: Leave this declared as const static data so that it ends up in the 
 * .rodata section (ie. Flash) rather than in the .bss section (RAM).
 */
void PIOS_SPI_sdcard_irq_handler(void);
const struct pios_spi_cfg pios_spi_sdcard_cfg = {
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
    .SPI_BaudRatePrescaler = 7 << 3, /* Maximum divider (ie. slowest clock rate) */
  },
  .dma = {
    .ahb_clk  = RCC_AHBPeriph_DMA1,
    
    .irq = {
      .handler = PIOS_SPI_sdcard_irq_handler,
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
 * NOTE: Leave this declared as const static data so that it ends up in the 
 * .rodata section (ie. Flash) rather than in the .bss section (RAM).
 */
void PIOS_SPI_ahrs_irq_handler(void);
const struct pios_spi_cfg pios_spi_ahrs_cfg = {
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
    .SPI_BaudRatePrescaler = 7 << 3, /* Maximum divider (ie. slowest clock rate) */
  },
  .dma = {
    .ahb_clk  = RCC_AHBPeriph_DMA1,
    
    .irq = {
      .handler = PIOS_SPI_ahrs_irq_handler,
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
      .GPIO_Mode  = GPIO_Mode_Out_PP,
    },
  },
  .sclk = {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_13,
      .GPIO_Speed = GPIO_Speed_50MHz,
      .GPIO_Mode  = GPIO_Mode_AF_PP,
    },
  },
  .miso = {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_14,
      .GPIO_Speed = GPIO_Speed_50MHz,
      .GPIO_Mode  = GPIO_Mode_IPU,
    },
  },
  .mosi = {
    .gpio = GPIOB,
    .init = {
      .GPIO_Pin   = GPIO_Pin_15,
      .GPIO_Speed = GPIO_Speed_50MHz,
      .GPIO_Mode  = GPIO_Mode_AF_PP,
    },
  },
};

/*
 * Board specific number of devices.
 */
struct pios_spi_dev pios_spi_devs[] = {
  {
    .cfg = &pios_spi_sdcard_cfg,
  },
  {
    .cfg = &pios_spi_ahrs_cfg,
  },
};

uint8_t pios_spi_num_devices = NELEMENTS(pios_spi_devs);

void PIOS_SPI_sdcard_irq_handler(void)
{
  /* Call into the generic code to handle the IRQ for this specific device */
  PIOS_SPI_IRQ_Handler(PIOS_SDCARD_SPI);
}

void PIOS_SPI_ahrs_irq_handler(void)
{
  /* Call into the generic code to handle the IRQ for this specific device */
  PIOS_SPI_IRQ_Handler(PIOS_OPAHRS_SPI);
}
