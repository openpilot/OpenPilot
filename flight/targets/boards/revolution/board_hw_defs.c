/**
 ******************************************************************************
 * @file       board_hw_defs.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @author     PhoenixPilot, http://github.com/PhoenixPilot, Copyright (C) 2012
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotCore OpenPilot Core
 * @{
 * @brief Defines board specific static initializers for hardware for the Revolution board.
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

#if defined(PIOS_INCLUDE_LED)

#include <pios_led_priv.h>
static const struct pios_gpio pios_leds[] = {
    [PIOS_LED_HEARTBEAT] = {
        .pin                =             {
            .gpio = GPIOB,
            .init =             {
                .GPIO_Pin   = GPIO_Pin_12,
                .GPIO_Speed = GPIO_Speed_50MHz,
                .GPIO_Mode  = GPIO_Mode_OUT,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
        },
        .active_low         = true
    },
    [PIOS_LED_ALARM] =     {
        .pin                =             {
            .gpio = GPIOB,
            .init =             {
                .GPIO_Pin   = GPIO_Pin_6,
                .GPIO_Speed = GPIO_Speed_50MHz,
                .GPIO_Mode  = GPIO_Mode_OUT,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
        },
        .active_low         = true
    },
#ifdef PIOS_RFM22B_DEBUG_ON_TELEM
    [PIOS_LED_D1] =        {
        .pin                =             {
            .gpio = GPIOC,
            .init =             {
                .GPIO_Pin   = GPIO_Pin_6,
                .GPIO_Speed = GPIO_Speed_50MHz,
                .GPIO_Mode  = GPIO_Mode_OUT,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
        },
    },
    [PIOS_LED_D2] =        {
        .pin                =             {
            .gpio = GPIOC,
            .init =             {
                .GPIO_Pin   = GPIO_Pin_7,
                .GPIO_Speed = GPIO_Speed_50MHz,
                .GPIO_Mode  = GPIO_Mode_OUT,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
        },
    },
    [PIOS_LED_D3] =        {
        .pin                =             {
            .gpio = GPIOC,
            .init =             {
                .GPIO_Pin   = GPIO_Pin_8,
                .GPIO_Speed = GPIO_Speed_50MHz,
                .GPIO_Mode  = GPIO_Mode_OUT,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
        },
    },
    [PIOS_LED_D4] =        {
        .pin                =             {
            .gpio = GPIOC,
            .init =             {
                .GPIO_Pin   = GPIO_Pin_9,
                .GPIO_Speed = GPIO_Speed_50MHz,
                .GPIO_Mode  = GPIO_Mode_OUT,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
        },
    },
#endif /* ifdef PIOS_RFM22B_DEBUG_ON_TELEM */
};

static const struct pios_gpio_cfg pios_led_cfg = {
    .gpios     = pios_leds,
    .num_gpios = NELEMENTS(pios_leds),
};

static const struct pios_gpio pios_leds_v2[] = {
    [PIOS_LED_HEARTBEAT] = {
        .pin                =             {
            .gpio = GPIOB,
            .init =             {
                .GPIO_Pin   = GPIO_Pin_5,
                .GPIO_Speed = GPIO_Speed_50MHz,
                .GPIO_Mode  = GPIO_Mode_OUT,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
        },
        .active_low         = true
    },
    [PIOS_LED_ALARM] =     {
        .pin                =             {
            .gpio = GPIOB,
            .init =             {
                .GPIO_Pin   = GPIO_Pin_4,
                .GPIO_Speed = GPIO_Speed_50MHz,
                .GPIO_Mode  = GPIO_Mode_OUT,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
        },
        .active_low         = true
    },
#ifdef PIOS_RFM22B_DEBUG_ON_TELEM
    [PIOS_LED_D1] =        {
        .pin                =             {
            .gpio = GPIOB,
            .init =             {
                .GPIO_Pin   = GPIO_Pin_13,
                .GPIO_Speed = GPIO_Speed_50MHz,
                .GPIO_Mode  = GPIO_Mode_OUT,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
        },
    },
    [PIOS_LED_D2] =        {
        .pin                =             {
            .gpio = GPIOB,
            .init =             {
                .GPIO_Pin   = GPIO_Pin_14,
                .GPIO_Speed = GPIO_Speed_50MHz,
                .GPIO_Mode  = GPIO_Mode_OUT,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
        },
    },
    [PIOS_LED_D3] =        {
        .pin                =             {
            .gpio = GPIOB,
            .init =             {
                .GPIO_Pin   = GPIO_Pin_15,
                .GPIO_Speed = GPIO_Speed_50MHz,
                .GPIO_Mode  = GPIO_Mode_OUT,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
        },
    },
    [PIOS_LED_D4] =        {
        .pin                =             {
            .gpio = GPIOC,
            .init =             {
                .GPIO_Pin   = GPIO_Pin_6,
                .GPIO_Speed = GPIO_Speed_50MHz,
                .GPIO_Mode  = GPIO_Mode_OUT,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
        },
    },
#endif /* ifdef PIOS_RFM22B_DEBUG_ON_TELEM */
};

static const struct pios_gpio_cfg pios_led_v2_cfg = {
    .gpios     = pios_leds_v2,
    .num_gpios = NELEMENTS(pios_leds_v2),
};

const struct pios_gpio_cfg *PIOS_BOARD_HW_DEFS_GetLedCfg(uint32_t board_revision)
{
    switch (board_revision) {
    case 2:
        return &pios_led_cfg;

        break;
    case 3:
        return &pios_led_v2_cfg;

        break;
    default:
        PIOS_DEBUG_Assert(0);
    }
    return NULL;
}

#endif /* PIOS_INCLUDE_LED */

#if defined(PIOS_INCLUDE_SPI)
#include <pios_spi_priv.h>

#if defined(PIOS_OVERO_SPI)
/*      SPI2 Interface
 *      - Used for Flexi/IO/Overo communications
        3: PB12 = SPI2 NSS, CAN2 RX
        4: PB13 = SPI2 SCK, CAN2 TX, USART3 CTS
        5: PB14 = SPI2 MISO, TIM12 CH1, USART3 RTS
        6: PB15 = SPI2 MOSI, TIM12 CH2
 */
#include <pios_overo_priv.h>
void PIOS_OVERO_irq_handler(void);
void DMA1_Stream7_IRQHandler(void) __attribute__((alias("PIOS_OVERO_irq_handler")));
static const struct pios_overo_cfg pios_overo_cfg = {
    .regs  = SPI2,
    .remap = GPIO_AF_SPI2,
    .init  = {
        .SPI_Mode              = SPI_Mode_Slave,
        .SPI_Direction         = SPI_Direction_2Lines_FullDuplex,
        .SPI_DataSize          = SPI_DataSize_8b,
        .SPI_NSS                                   = SPI_NSS_Hard,
        .SPI_FirstBit          = SPI_FirstBit_MSB,
        .SPI_CRCPolynomial     = 7,
        .SPI_CPOL              = SPI_CPOL_High,
        .SPI_CPHA              = SPI_CPHA_2Edge,
        .SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2,
    },
    .use_crc = false,
    .dma     = {
        .irq                                       = {
            // Note this is the stream ID that triggers interrupts (in this case TX)
            .flags = (DMA_IT_TCIF7),
            .init  = {
                .NVIC_IRQChannel    = DMA1_Stream7_IRQn,
                .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
                .NVIC_IRQChannelSubPriority        = 0,
                .NVIC_IRQChannelCmd = ENABLE,
            },
        },

        .rx                                        = {
            .channel = DMA1_Stream0,
            .init    = {
                .DMA_Channel            = DMA_Channel_0,
                .DMA_PeripheralBaseAddr = (uint32_t)&(SPI2->DR),
                .DMA_DIR                = DMA_DIR_PeripheralToMemory,
                .DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
                .DMA_MemoryInc          = DMA_MemoryInc_Enable,
                .DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
                .DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
                .DMA_Mode               = DMA_Mode_Circular,
                .DMA_Priority           = DMA_Priority_Medium,
                // TODO: Enable FIFO
                .DMA_FIFOMode           = DMA_FIFOMode_Disable,
                .DMA_FIFOThreshold      = DMA_FIFOThreshold_Full,
                .DMA_MemoryBurst        = DMA_MemoryBurst_Single,
                .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single,
            },
        },
        .tx                                        = {
            .channel = DMA1_Stream7,
            .init    = {
                .DMA_Channel            = DMA_Channel_0,
                .DMA_PeripheralBaseAddr = (uint32_t)&(SPI2->DR),
                .DMA_DIR                = DMA_DIR_MemoryToPeripheral,
                .DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
                .DMA_MemoryInc          = DMA_MemoryInc_Enable,
                .DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
                .DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
                .DMA_Mode               = DMA_Mode_Circular,
                .DMA_Priority           = DMA_Priority_Medium,
                .DMA_FIFOMode           = DMA_FIFOMode_Disable,
                .DMA_FIFOThreshold      = DMA_FIFOThreshold_Full,
                .DMA_MemoryBurst        = DMA_MemoryBurst_Single,
                .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single,
            },
        },
    },
    .sclk                                          = {
        .gpio = GPIOB,
        .init = {
            .GPIO_Pin   = GPIO_Pin_13,
            .GPIO_Speed = GPIO_Speed_100MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL
        },
    },
    .miso                                          = {
        .gpio = GPIOB,
        .init = {
            .GPIO_Pin   = GPIO_Pin_14,
            .GPIO_Speed = GPIO_Speed_50MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL
        },
    },
    .mosi                                          = {
        .gpio = GPIOB,
        .init = {
            .GPIO_Pin   = GPIO_Pin_15,
            .GPIO_Speed = GPIO_Speed_50MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL
        },
    },
    .slave_count                                   = 1,
    .ssel                                          = {
        {
            .gpio = GPIOB,
            .init = {
                .GPIO_Pin   = GPIO_Pin_12,
                .GPIO_Speed = GPIO_Speed_50MHz,
                .GPIO_Mode  = GPIO_Mode_OUT,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
        }
    },
};
uint32_t pios_overo_id = 0;
void PIOS_OVERO_irq_handler(void)
{
    /* Call into the generic code to handle the IRQ for this specific device */
    PIOS_OVERO_DMA_irq_handler(pios_overo_id);
}

#endif /* PIOS_OVERO_SPI */

/*
 * SPI1 Interface
 * Used for MPU6000 gyro and accelerometer
 */
void PIOS_SPI_gyro_irq_handler(void);
void DMA2_Stream0_IRQHandler(void) __attribute__((alias("PIOS_SPI_gyro_irq_handler")));
void DMA2_Stream3_IRQHandler(void) __attribute__((alias("PIOS_SPI_gyro_irq_handler")));
static const struct pios_spi_cfg pios_spi_gyro_cfg = {
    .regs  = SPI1,
    .remap = GPIO_AF_SPI1,
    .init  = {
        .SPI_Mode              = SPI_Mode_Master,
        .SPI_Direction         = SPI_Direction_2Lines_FullDuplex,
        .SPI_DataSize          = SPI_DataSize_8b,
        .SPI_NSS                                   = SPI_NSS_Soft,
        .SPI_FirstBit          = SPI_FirstBit_MSB,
        .SPI_CRCPolynomial     = 7,
        .SPI_CPOL              = SPI_CPOL_High,
        .SPI_CPHA              = SPI_CPHA_2Edge,
        .SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16,
    },
    .use_crc = false,
    .dma     = {
        .irq                                       = {
            .flags = (DMA_IT_TCIF0 | DMA_IT_TEIF0 | DMA_IT_HTIF0),
            .init  = {
                .NVIC_IRQChannel    = DMA2_Stream0_IRQn,
                .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
                .NVIC_IRQChannelSubPriority        = 0,
                .NVIC_IRQChannelCmd = ENABLE,
            },
        },

        .rx                                        = {
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
        .tx                                        = {
            .channel = DMA2_Stream3,
            .init    = {
                .DMA_Channel            = DMA_Channel_3,
                .DMA_PeripheralBaseAddr = (uint32_t)&(SPI1->DR),
                .DMA_DIR                = DMA_DIR_MemoryToPeripheral,
                .DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
                .DMA_MemoryInc          = DMA_MemoryInc_Enable,
                .DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
                .DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
                .DMA_Mode               = DMA_Mode_Normal,
                .DMA_Priority           = DMA_Priority_High,
                .DMA_FIFOMode           = DMA_FIFOMode_Disable,
                /* .DMA_FIFOThreshold */
                .DMA_MemoryBurst        = DMA_MemoryBurst_Single,
                .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single,
            },
        },
    },
    .sclk                                          = {
        .gpio = GPIOA,
        .init = {
            .GPIO_Pin   = GPIO_Pin_5,
            .GPIO_Speed = GPIO_Speed_100MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_UP
        },
    },
    .miso                                          = {
        .gpio = GPIOA,
        .init = {
            .GPIO_Pin   = GPIO_Pin_6,
            .GPIO_Speed = GPIO_Speed_50MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_UP
        },
    },
    .mosi                                          = {
        .gpio = GPIOA,
        .init = {
            .GPIO_Pin   = GPIO_Pin_7,
            .GPIO_Speed = GPIO_Speed_50MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_UP
        },
    },
    .slave_count                                   = 1,
    .ssel                                          = {
        {
            .gpio = GPIOA,
            .init = {
                .GPIO_Pin   = GPIO_Pin_4,
                .GPIO_Speed = GPIO_Speed_50MHz,
                .GPIO_Mode  = GPIO_Mode_OUT,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            }
        }
    }
};

static uint32_t pios_spi_gyro_id;
void PIOS_SPI_gyro_irq_handler(void)
{
    /* Call into the generic code to handle the IRQ for this specific device */
    PIOS_SPI_IRQ_Handler(pios_spi_gyro_id);
}


/*
 * SPI3 Interface
 * Used for Flash and the RFM22B
 */
void PIOS_SPI_telem_flash_irq_handler(void);
void DMA1_Stream0_IRQHandler(void) __attribute__((alias("PIOS_SPI_telem_flash_irq_handler")));
void DMA1_Stream5_IRQHandler(void) __attribute__((alias("PIOS_SPI_telem_flash_irq_handler")));
static const struct pios_spi_cfg pios_spi_telem_flash_cfg = {
    .regs  = SPI3,
    .remap = GPIO_AF_SPI3,
    .init  = {
        .SPI_Mode              = SPI_Mode_Master,
        .SPI_Direction         = SPI_Direction_2Lines_FullDuplex,
        .SPI_DataSize          = SPI_DataSize_8b,
        .SPI_NSS                                   = SPI_NSS_Soft,
        .SPI_FirstBit          = SPI_FirstBit_MSB,
        .SPI_CRCPolynomial     = 7,
        .SPI_CPOL              = SPI_CPOL_Low,
        .SPI_CPHA              = SPI_CPHA_1Edge,
        .SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8,
    },
    .use_crc = false,
    .dma     = {
        .irq                                       = {
            // Note this is the stream ID that triggers interrupts (in this case RX)
            .flags = (DMA_IT_TCIF0 | DMA_IT_TEIF0 | DMA_IT_HTIF0),
            .init  = {
                .NVIC_IRQChannel    = DMA1_Stream0_IRQn,
                .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
                .NVIC_IRQChannelSubPriority        = 0,
                .NVIC_IRQChannelCmd = ENABLE,
            },
        },

        .rx                                        = {
            .channel = DMA1_Stream0,
            .init    = {
                .DMA_Channel            = DMA_Channel_0,
                .DMA_PeripheralBaseAddr = (uint32_t)&(SPI3->DR),
                .DMA_DIR                = DMA_DIR_PeripheralToMemory,
                .DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
                .DMA_MemoryInc          = DMA_MemoryInc_Enable,
                .DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
                .DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
                .DMA_Mode               = DMA_Mode_Normal,
                .DMA_Priority           = DMA_Priority_Medium,
                // TODO: Enable FIFO
                .DMA_FIFOMode           = DMA_FIFOMode_Disable,
                .DMA_FIFOThreshold      = DMA_FIFOThreshold_Full,
                .DMA_MemoryBurst        = DMA_MemoryBurst_Single,
                .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single,
            },
        },
        .tx                                        = {
            .channel = DMA1_Stream5,
            .init    = {
                .DMA_Channel            = DMA_Channel_0,
                .DMA_PeripheralBaseAddr = (uint32_t)&(SPI3->DR),
                .DMA_DIR                = DMA_DIR_MemoryToPeripheral,
                .DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
                .DMA_MemoryInc          = DMA_MemoryInc_Enable,
                .DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
                .DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
                .DMA_Mode               = DMA_Mode_Normal,
                .DMA_Priority           = DMA_Priority_Medium,
                .DMA_FIFOMode           = DMA_FIFOMode_Disable,
                .DMA_FIFOThreshold      = DMA_FIFOThreshold_Full,
                .DMA_MemoryBurst        = DMA_MemoryBurst_Single,
                .DMA_PeripheralBurst    = DMA_PeripheralBurst_Single,
            },
        },
    },
    .sclk                                          = {
        .gpio = GPIOC,
        .init = {
            .GPIO_Pin   = GPIO_Pin_10,
            .GPIO_Speed = GPIO_Speed_100MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL
        },
    },
    .miso                                          = {
        .gpio = GPIOC,
        .init = {
            .GPIO_Pin   = GPIO_Pin_11,
            .GPIO_Speed = GPIO_Speed_50MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL
        },
    },
    .mosi                                          = {
        .gpio = GPIOC,
        .init = {
            .GPIO_Pin   = GPIO_Pin_12,
            .GPIO_Speed = GPIO_Speed_50MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL
        },
    },
    .slave_count                                   = 2,
    .ssel                                          = {
        { // RFM22b
            .gpio = GPIOA,
            .init = {
                .GPIO_Pin   = GPIO_Pin_15,
                .GPIO_Speed = GPIO_Speed_50MHz,
                .GPIO_Mode  = GPIO_Mode_OUT,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            }
        },
        { // Flash
            .gpio = GPIOB,
            .init = {
                .GPIO_Pin   = GPIO_Pin_3,
                .GPIO_Speed = GPIO_Speed_50MHz,
                .GPIO_Mode  = GPIO_Mode_OUT,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            }
        },
    },
};

uint32_t pios_spi_telem_flash_id;
void PIOS_SPI_telem_flash_irq_handler(void)
{
    /* Call into the generic code to handle the IRQ for this specific device */
    PIOS_SPI_IRQ_Handler(pios_spi_telem_flash_id);
}


#if defined(PIOS_INCLUDE_RFM22B)
#include <pios_rfm22b_priv.h>

static const struct pios_exti_cfg pios_exti_rfm22b_cfg __exti_config = {
    .vector = PIOS_RFM22_EXT_Int,
    .line   = EXTI_Line2,
    .pin    = {
        .gpio = GPIOD,
        .init = {
            .GPIO_Pin   = GPIO_Pin_2,
            .GPIO_Speed = GPIO_Speed_100MHz,
            .GPIO_Mode  = GPIO_Mode_IN,
            .GPIO_OType = GPIO_OType_OD,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL,
        },
    },
    .irq                                       = {
        .init                                  = {
            .NVIC_IRQChannel    = EXTI2_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_LOW,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
    .exti                                      = {
        .init                                  = {
            .EXTI_Line    = EXTI_Line2, // matches above GPIO pin
            .EXTI_Mode    = EXTI_Mode_Interrupt,
            .EXTI_Trigger = EXTI_Trigger_Falling,
            .EXTI_LineCmd = ENABLE,
        },
    },
};

const struct pios_rfm22b_cfg pios_rfm22b_rm1_cfg = {
    .spi_cfg   = &pios_spi_telem_flash_cfg,
    .exti_cfg  = &pios_exti_rfm22b_cfg,
    .RFXtalCap = 0x7f,
    .slave_num = 0,
    .gpio_direction = GPIO0_RX_GPIO1_TX,
};

const struct pios_rfm22b_cfg pios_rfm22b_rm2_cfg = {
    .spi_cfg   = &pios_spi_telem_flash_cfg,
    .exti_cfg  = &pios_exti_rfm22b_cfg,
    .RFXtalCap = 0x7f,
    .slave_num = 0,
    .gpio_direction = GPIO0_TX_GPIO1_RX,
};

const struct pios_rfm22b_cfg *PIOS_BOARD_HW_DEFS_GetRfm22Cfg(uint32_t board_revision)
{
    switch (board_revision) {
    case 2:
        return &pios_rfm22b_rm1_cfg;

        break;
    case 3:
        return &pios_rfm22b_rm2_cfg;

        break;
    default:
        PIOS_DEBUG_Assert(0);
    }
    return NULL;
}

#endif /* PIOS_INCLUDE_RFM22B */

#endif /* PIOS_INCLUDE_SPI */

#if defined(PIOS_INCLUDE_FLASH)
#include "pios_flashfs_logfs_priv.h"
#include "pios_flash_jedec_priv.h"
#include "pios_flash_internal_priv.h"


static const struct pios_flash_internal_cfg flash_internal_cfg = {};

static const struct flashfs_logfs_cfg flashfs_internal_cfg = {
    .fs_magic      = 0x99abcfef,
    .total_fs_size = EE_BANK_SIZE, /* 32K bytes (2x16KB sectors) */
    .arena_size    = 0x00004000, /* 64 * slot size = 16K bytes = 1 sector */
    .slot_size     = 0x00000100, /* 256 bytes */

    .start_offset  = EE_BANK_BASE, /* start after the bootloader */
    .sector_size   = 0x00004000, /* 16K bytes */
    .page_size     = 0x00004000, /* 16K bytes */
};

#endif /* PIOS_INCLUDE_FLASH */

#include <pios_usart_priv.h>

#ifdef PIOS_INCLUDE_COM_TELEM

/*
 * MAIN USART
 */
static const struct pios_usart_cfg pios_usart_main_cfg = {
    .regs  = USART1,
    .remap = GPIO_AF_USART1,
    .init  = {
        .USART_BaudRate   = 57600,
        .USART_WordLength = USART_WordLength_8b,
        .USART_Parity     = USART_Parity_No,
        .USART_StopBits   = USART_StopBits_1,
        .USART_HardwareFlowControl             = USART_HardwareFlowControl_None,
        .USART_Mode                            = USART_Mode_Rx | USART_Mode_Tx,
    },
    .irq                                       = {
        .init                                  = {
            .NVIC_IRQChannel    = USART1_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
    .rx                                        = {
        .gpio = GPIOA,
        .init = {
            .GPIO_Pin   = GPIO_Pin_10,
            .GPIO_Speed = GPIO_Speed_2MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_UP
        },
    },
    .tx                                        = {
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
#endif /* PIOS_INCLUDE_COM_TELEM */

#ifdef PIOS_INCLUDE_DSM

#include "pios_dsm_priv.h"
static const struct pios_usart_cfg pios_usart_dsm_main_cfg = {
    .regs  = USART1,
    .remap = GPIO_AF_USART1,
    .init  = {
        .USART_BaudRate   = 115200,
        .USART_WordLength = USART_WordLength_8b,
        .USART_Parity     = USART_Parity_No,
        .USART_StopBits   = USART_StopBits_1,
        .USART_HardwareFlowControl             = USART_HardwareFlowControl_None,
        .USART_Mode                            = USART_Mode_Rx,
    },
    .irq                                       = {
        .init                                  = {
            .NVIC_IRQChannel    = USART1_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
    .rx                                        = {
        .gpio = GPIOA,
        .init = {
            .GPIO_Pin   = GPIO_Pin_10,
            .GPIO_Speed = GPIO_Speed_2MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_UP
        },
    },
    .tx                                        = {
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

// Because of the inverter on the main port this will not
// work.  Notice the mode is set to IN to maintain API
// compatibility but protect the pins
static const struct pios_dsm_cfg pios_dsm_main_cfg = {
    .bind               = {
        .gpio = GPIOA,
        .init = {
            .GPIO_Pin   = GPIO_Pin_10,
            .GPIO_Speed = GPIO_Speed_2MHz,
            .GPIO_Mode  = GPIO_Mode_IN,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL
        },
    },
};

#endif /* PIOS_INCLUDE_DSM */

#include <pios_sbus_priv.h>
#if defined(PIOS_INCLUDE_SBUS)
/*
 * S.Bus USART
 */
#include <pios_sbus_priv.h>

static const struct pios_usart_cfg pios_usart_sbus_main_cfg = {
    .regs  = USART1,
    .remap = GPIO_AF_USART1,
    .init  = {
        .USART_BaudRate   = 100000,
        .USART_WordLength = USART_WordLength_8b,
        .USART_Parity     = USART_Parity_Even,
        .USART_StopBits   = USART_StopBits_2,
        .USART_HardwareFlowControl             = USART_HardwareFlowControl_None,
        .USART_Mode                            = USART_Mode_Rx,
    },
    .irq                                       = {
        .init                                  = {
            .NVIC_IRQChannel    = USART1_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
    .rx                                        = {
        .gpio = GPIOA,
        .init = {
            .GPIO_Pin   = GPIO_Pin_10,
            .GPIO_Speed = GPIO_Speed_2MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_UP
        },
    },
    .tx                                        = {
        .gpio = GPIOA,
        .init = {
            .GPIO_Pin   = GPIO_Pin_9,
            .GPIO_Speed = GPIO_Speed_2MHz,
            .GPIO_Mode  = GPIO_Mode_OUT,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL
        },
    },
};

#endif /* PIOS_INCLUDE_SBUS */

// Need this defined regardless to be able to turn it off
static const struct pios_sbus_cfg pios_sbus_cfg = {
    /* Inverter configuration */
    .inv                = {
        .gpio = GPIOC,
        .init = {
            .GPIO_Pin   = GPIO_Pin_0,
            .GPIO_Speed = GPIO_Speed_2MHz,
            .GPIO_Mode  = GPIO_Mode_OUT,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_UP
        },
    },
    .gpio_inv_enable  = Bit_SET,
    .gpio_inv_disable = Bit_RESET,
    .gpio_clk_func    = RCC_AHB1PeriphClockCmd,
    .gpio_clk_periph  = RCC_AHB1Periph_GPIOC,
};


#ifdef PIOS_INCLUDE_COM_FLEXI
/*
 * FLEXI PORT
 */
static const struct pios_usart_cfg pios_usart_flexi_cfg = {
    .regs  = USART3,
    .remap = GPIO_AF_USART3,
    .init  = {
        .USART_BaudRate   = 57600,
        .USART_WordLength = USART_WordLength_8b,
        .USART_Parity     = USART_Parity_No,
        .USART_StopBits   = USART_StopBits_1,
        .USART_HardwareFlowControl             =
            USART_HardwareFlowControl_None,
        .USART_Mode                            = USART_Mode_Rx | USART_Mode_Tx,
    },
    .irq                                       = {
        .init                                  = {
            .NVIC_IRQChannel    = USART3_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
    .rx                                        = {
        .gpio = GPIOB,
        .init = {
            .GPIO_Pin   = GPIO_Pin_11,
            .GPIO_Speed = GPIO_Speed_2MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_UP
        },
    },
    .tx                                        = {
        .gpio = GPIOB,
        .init = {
            .GPIO_Pin   = GPIO_Pin_10,
            .GPIO_Speed = GPIO_Speed_2MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_UP
        },
    },
};

#endif /* PIOS_INCLUDE_COM_FLEXI */

#ifdef PIOS_INCLUDE_DSM

#include "pios_dsm_priv.h"
static const struct pios_usart_cfg pios_usart_dsm_flexi_cfg = {
    .regs  = USART3,
    .remap = GPIO_AF_USART3,
    .init  = {
        .USART_BaudRate   = 115200,
        .USART_WordLength = USART_WordLength_8b,
        .USART_Parity     = USART_Parity_No,
        .USART_StopBits   = USART_StopBits_1,
        .USART_HardwareFlowControl             = USART_HardwareFlowControl_None,
        .USART_Mode                            = USART_Mode_Rx,
    },
    .irq                                       = {
        .init                                  = {
            .NVIC_IRQChannel    = USART3_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
    .rx                                        = {
        .gpio = GPIOB,
        .init = {
            .GPIO_Pin   = GPIO_Pin_11,
            .GPIO_Speed = GPIO_Speed_2MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_UP
        },
    },
    .tx                                        = {
        .gpio = GPIOB,
        .init = {
            .GPIO_Pin   = GPIO_Pin_10,
            .GPIO_Speed = GPIO_Speed_2MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_UP
        },
    },
};

static const struct pios_dsm_cfg pios_dsm_flexi_cfg = {
    .bind               = {
        .gpio = GPIOB,
        .init = {
            .GPIO_Pin   = GPIO_Pin_11,
            .GPIO_Speed = GPIO_Speed_2MHz,
            .GPIO_Mode  = GPIO_Mode_OUT,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL
        },
    },
};

#endif /* PIOS_INCLUDE_DSM */

/*
 * HK OSD
 */
static const struct pios_usart_cfg pios_usart_hkosd_main_cfg = {
    .regs  = USART1,
    .remap = GPIO_AF_USART1,
    .init  = {
        .USART_BaudRate   = 57600,
        .USART_WordLength = USART_WordLength_8b,
        .USART_Parity     = USART_Parity_No,
        .USART_StopBits   = USART_StopBits_1,
        .USART_HardwareFlowControl             = USART_HardwareFlowControl_None,
        .USART_Mode                            = USART_Mode_Rx | USART_Mode_Tx,
    },
    .irq                                       = {
        .init                                  = {
            .NVIC_IRQChannel    = USART1_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
    .rx                                        = {
        .gpio = GPIOA,
        .init = {
            .GPIO_Pin   = GPIO_Pin_10,
            .GPIO_Speed = GPIO_Speed_2MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_UP
        },
    },
    .tx                                        = {
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

static const struct pios_usart_cfg pios_usart_hkosd_flexi_cfg = {
    .regs  = USART3,
    .remap = GPIO_AF_USART3,
    .init  = {
        .USART_BaudRate   = 57600,
        .USART_WordLength = USART_WordLength_8b,
        .USART_Parity     = USART_Parity_No,
        .USART_StopBits   = USART_StopBits_1,
        .USART_HardwareFlowControl             = USART_HardwareFlowControl_None,
        .USART_Mode                            = USART_Mode_Rx | USART_Mode_Tx,
    },
    .irq                                       = {
        .init                                  = {
            .NVIC_IRQChannel    = USART3_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
    .rx                                        = {
        .gpio = GPIOB,
        .init = {
            .GPIO_Pin   = GPIO_Pin_11,
            .GPIO_Speed = GPIO_Speed_2MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_UP
        },
    },
    .tx                                        = {
        .gpio = GPIOB,
        .init = {
            .GPIO_Pin   = GPIO_Pin_10,
            .GPIO_Speed = GPIO_Speed_2MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_UP
        },
    },
};

static const struct pios_usart_cfg pios_usart_rcvrport_cfg = {
    .regs  = USART6,
    .remap = GPIO_AF_USART6,
    .init  = {
        .USART_BaudRate   = 57600,
        .USART_WordLength = USART_WordLength_8b,
        .USART_Parity     = USART_Parity_No,
        .USART_StopBits   = USART_StopBits_1,
        .USART_HardwareFlowControl             = USART_HardwareFlowControl_None,
        .USART_Mode                            = USART_Mode_Rx | USART_Mode_Tx,
    },
    .irq                                       = {
        .init                                  = {
            .NVIC_IRQChannel    = USART6_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },

    .tx                                        = {
        // *  7: PC6 = TIM8 CH1, USART6 TX
        .gpio = GPIOC,
        .init = {
            .GPIO_Pin   = GPIO_Pin_6,
            .GPIO_Speed = GPIO_Speed_2MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_UP
        },
        .pin_source                            = GPIO_PinSource6,
    },

    .rx                                        = {
        // *  8: PC7 = TIM8 CH2, USART6 RX
        .gpio = GPIOC,
        .init = {
            .GPIO_Pin   = GPIO_Pin_7,
            .GPIO_Speed = GPIO_Speed_2MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_PuPd  = GPIO_PuPd_UP
        },
        .pin_source                            = GPIO_PinSource7,
    }
};

#if defined(PIOS_INCLUDE_COM)

#include <pios_com_priv.h>

#endif /* PIOS_INCLUDE_COM */

#if defined(PIOS_INCLUDE_I2C)

#include <pios_i2c_priv.h>

/*
 * I2C Adapters
 */
void PIOS_I2C_mag_pressure_adapter_ev_irq_handler(void);
void PIOS_I2C_mag_pressureadapter_er_irq_handler(void);
void I2C1_EV_IRQHandler()
__attribute__((alias("PIOS_I2C_mag_pressure_adapter_ev_irq_handler")));
void I2C1_ER_IRQHandler()
__attribute__((alias("PIOS_I2C_mag_pressure_adapter_er_irq_handler")));

static const struct pios_i2c_adapter_cfg pios_i2c_mag_pressure_adapter_cfg = {
    .regs  = I2C1,
    .remap = GPIO_AF_I2C1,
    .init  = {
        .I2C_Mode = I2C_Mode_I2C,
        .I2C_OwnAddress1                       = 0,
        .I2C_Ack  = I2C_Ack_Enable,
        .I2C_AcknowledgedAddress               = I2C_AcknowledgedAddress_7bit,
        .I2C_DutyCycle                         = I2C_DutyCycle_2,
        .I2C_ClockSpeed                        = 400000,                      /* bits/s */
    },
    .transfer_timeout_ms                       = 50,
    .scl                                       = {
        .gpio = GPIOB,
        .init = {
            .GPIO_Pin   = GPIO_Pin_8,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_Speed = GPIO_Speed_50MHz,
            .GPIO_OType = GPIO_OType_OD,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL,
        },
    },
    .sda                                       = {
        .gpio = GPIOB,
        .init = {
            .GPIO_Pin   = GPIO_Pin_9,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_Speed = GPIO_Speed_50MHz,
            .GPIO_OType = GPIO_OType_OD,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL,
        },
    },
    .event                                     = {
        .flags = 0,     /* FIXME: check this */
        .init  = {
            .NVIC_IRQChannel    = I2C1_EV_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGHEST,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
    .error                                     = {
        .flags = 0,     /* FIXME: check this */
        .init  = {
            .NVIC_IRQChannel    = I2C1_ER_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGHEST,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

uint32_t pios_i2c_mag_pressure_adapter_id;
void PIOS_I2C_mag_pressure_adapter_ev_irq_handler(void)
{
    /* Call into the generic code to handle the IRQ for this specific device */
    PIOS_I2C_EV_IRQ_Handler(pios_i2c_mag_pressure_adapter_id);
}

void PIOS_I2C_mag_pressure_adapter_er_irq_handler(void)
{
    /* Call into the generic code to handle the IRQ for this specific device */
    PIOS_I2C_ER_IRQ_Handler(pios_i2c_mag_pressure_adapter_id);
}


void PIOS_I2C_flexiport_adapter_ev_irq_handler(void);
void PIOS_I2C_flexiport_adapter_er_irq_handler(void);
void I2C2_EV_IRQHandler() __attribute__((alias("PIOS_I2C_flexiport_adapter_ev_irq_handler")));
void I2C2_ER_IRQHandler() __attribute__((alias("PIOS_I2C_flexiport_adapter_er_irq_handler")));

static const struct pios_i2c_adapter_cfg pios_i2c_flexiport_adapter_cfg = {
    .regs  = I2C2,
    .remap = GPIO_AF_I2C2,
    .init  = {
        .I2C_Mode = I2C_Mode_I2C,
        .I2C_OwnAddress1                       = 0,
        .I2C_Ack  = I2C_Ack_Enable,
        .I2C_AcknowledgedAddress               = I2C_AcknowledgedAddress_7bit,
        .I2C_DutyCycle                         = I2C_DutyCycle_2,
        .I2C_ClockSpeed                        = 400000,                      /* bits/s */
    },
    .transfer_timeout_ms                       = 50,
    .scl                                       = {
        .gpio = GPIOB,
        .init = {
            .GPIO_Pin   = GPIO_Pin_10,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_Speed = GPIO_Speed_50MHz,
            .GPIO_OType = GPIO_OType_OD,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL,
        },
    },
    .sda                                       = {
        .gpio = GPIOB,
        .init = {
            .GPIO_Pin   = GPIO_Pin_11,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_Speed = GPIO_Speed_50MHz,
            .GPIO_OType = GPIO_OType_OD,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL,
        },
    },
    .event                                     = {
        .flags = 0,           /* FIXME: check this */
        .init  = {
            .NVIC_IRQChannel    = I2C2_EV_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGHEST,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
    .error                                     = {
        .flags = 0,           /* FIXME: check this */
        .init  = {
            .NVIC_IRQChannel    = I2C2_ER_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGHEST,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

uint32_t pios_i2c_flexiport_adapter_id;
void PIOS_I2C_flexiport_adapter_ev_irq_handler(void)
{
    /* Call into the generic code to handle the IRQ for this specific device */
    PIOS_I2C_EV_IRQ_Handler(pios_i2c_flexiport_adapter_id);
}

void PIOS_I2C_flexiport_adapter_er_irq_handler(void)
{
    /* Call into the generic code to handle the IRQ for this specific device */
    PIOS_I2C_ER_IRQ_Handler(pios_i2c_flexiport_adapter_id);
}


void PIOS_I2C_pressure_adapter_ev_irq_handler(void);
void PIOS_I2C_pressure_adapter_er_irq_handler(void);

#endif /* PIOS_INCLUDE_I2C */

#if defined(PIOS_INCLUDE_RTC)
/*
 * Realtime Clock (RTC)
 */
#include <pios_rtc_priv.h>

void PIOS_RTC_IRQ_Handler(void);
void RTC_WKUP_IRQHandler() __attribute__((alias("PIOS_RTC_IRQ_Handler")));
static const struct pios_rtc_cfg pios_rtc_main_cfg = {
    .clksrc    = RCC_RTCCLKSource_HSE_Div8, // Divide 8 Mhz crystal down to 1
    // For some reason it's acting like crystal is 16 Mhz.  This clock is then divided
    // by another 16 to give a nominal 62.5 khz clock
    .prescaler = 100, // Every 100 cycles gives 625 Hz
    .irq                                       = {
        .init                                  = {
            .NVIC_IRQChannel    = RTC_WKUP_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

void PIOS_RTC_IRQ_Handler(void)
{
    PIOS_RTC_irq_handler();
}

#endif /* if defined(PIOS_INCLUDE_RTC) */

#include "pios_tim_priv.h"

static const TIM_TimeBaseInitTypeDef tim_3_5_time_base = {
    .TIM_Prescaler         = (PIOS_PERIPHERAL_APB1_CLOCK / 1000000) - 1,
    .TIM_ClockDivision     = TIM_CKD_DIV1,
    .TIM_CounterMode       = TIM_CounterMode_Up,
    .TIM_Period            = ((1000000 / PIOS_SERVO_UPDATE_HZ) - 1),
    .TIM_RepetitionCounter = 0x0000,
};
static const TIM_TimeBaseInitTypeDef tim_9_10_11_time_base = {
    .TIM_Prescaler         = (PIOS_PERIPHERAL_APB2_CLOCK / 1000000) - 1,
    .TIM_ClockDivision     = TIM_CKD_DIV1,
    .TIM_CounterMode       = TIM_CounterMode_Up,
    .TIM_Period            = ((1000000 / PIOS_SERVO_UPDATE_HZ) - 1),
    .TIM_RepetitionCounter = 0x0000,
};

static const struct pios_tim_clock_cfg tim_3_cfg = {
    .timer = TIM3,
    .time_base_init                            = &tim_3_5_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM3_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

static const struct pios_tim_clock_cfg tim_5_cfg = {
    .timer = TIM5,
    .time_base_init                            = &tim_3_5_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM5_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

static const struct pios_tim_clock_cfg tim_9_cfg = {
    .timer = TIM9,
    .time_base_init                            = &tim_9_10_11_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM1_BRK_TIM9_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

static const struct pios_tim_clock_cfg tim_10_cfg = {
    .timer = TIM10,
    .time_base_init                            = &tim_9_10_11_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM1_UP_TIM10_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

static const struct pios_tim_clock_cfg tim_11_cfg = {
    .timer = TIM11,
    .time_base_init                            = &tim_9_10_11_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM1_TRG_COM_TIM11_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

// Set up timers that only have inputs on APB1
// TIM2,3,4,5,6,7,12,13,14
static const TIM_TimeBaseInitTypeDef tim_apb1_time_base = {
    .TIM_Prescaler         = (PIOS_PERIPHERAL_APB1_CLOCK / 1000000) - 1,
    .TIM_ClockDivision     = TIM_CKD_DIV1,
    .TIM_CounterMode       = TIM_CounterMode_Up,
    .TIM_Period            = 0xFFFF,
    .TIM_RepetitionCounter = 0x0000,
};


// Set up timers that only have inputs on APB2
// TIM1,8,9,10,11
static const TIM_TimeBaseInitTypeDef tim_apb2_time_base = {
    .TIM_Prescaler         = (PIOS_PERIPHERAL_APB2_CLOCK / 1000000) - 1,
    .TIM_ClockDivision     = TIM_CKD_DIV1,
    .TIM_CounterMode       = TIM_CounterMode_Up,
    .TIM_Period            = 0xFFFF,
    .TIM_RepetitionCounter = 0x0000,
};

static const struct pios_tim_clock_cfg tim_1_cfg = {
    .timer = TIM1,
    .time_base_init                            = &tim_apb2_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM1_CC_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

static const struct pios_tim_clock_cfg tim_4_cfg = {
    .timer = TIM4,
    .time_base_init                            = &tim_apb1_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM4_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};
static const struct pios_tim_clock_cfg tim_8_cfg = {
    .timer = TIM8,
    .time_base_init                            = &tim_apb2_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM8_CC_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

static const struct pios_tim_clock_cfg tim_12_cfg = {
    .timer = TIM12,
    .time_base_init                            = &tim_apb1_time_base,
    .irq   = {
        .init                                  = {
            .NVIC_IRQChannel    = TIM8_BRK_TIM12_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};


/**
 * Pios servo configuration structures
 * Using TIM3, TIM9, TIM2, TIM5
 */
#include <pios_servo_priv.h>
static const struct pios_tim_channel pios_tim_servoport_all_pins[] = {
    {
        .timer = TIM3,
        .timer_chan = TIM_Channel_3,
        .pin   = {
            .gpio = GPIOB,
            .init = {
                .GPIO_Pin   = GPIO_Pin_0,
                .GPIO_Speed = GPIO_Speed_2MHz,
                .GPIO_Mode  = GPIO_Mode_AF,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
            .pin_source     = GPIO_PinSource0,
        },
        .remap = GPIO_AF_TIM3,
    },
    {
        .timer = TIM3,
        .timer_chan = TIM_Channel_4,
        .pin   = {
            .gpio = GPIOB,
            .init = {
                .GPIO_Pin   = GPIO_Pin_1,
                .GPIO_Speed = GPIO_Speed_2MHz,
                .GPIO_Mode  = GPIO_Mode_AF,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
            .pin_source     = GPIO_PinSource1,
        },
        .remap = GPIO_AF_TIM3,
    },
    {
        .timer = TIM9,
        .timer_chan = TIM_Channel_2,
        .pin   = {
            .gpio = GPIOA,
            .init = {
                .GPIO_Pin   = GPIO_Pin_3,
                .GPIO_Speed = GPIO_Speed_2MHz,
                .GPIO_Mode  = GPIO_Mode_AF,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
            .pin_source     = GPIO_PinSource3,
        },
        .remap = GPIO_AF_TIM9,
    },
    {
        .timer = TIM2,
        .timer_chan = TIM_Channel_3,
        .pin   = {
            .gpio = GPIOA,
            .init = {
                .GPIO_Pin   = GPIO_Pin_2,
                .GPIO_Speed = GPIO_Speed_2MHz,
                .GPIO_Mode  = GPIO_Mode_AF,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
            .pin_source     = GPIO_PinSource2,
        },
        .remap = GPIO_AF_TIM2,
    },
    {
        .timer = TIM5,
        .timer_chan = TIM_Channel_2,
        .pin   = {
            .gpio = GPIOA,
            .init = {
                .GPIO_Pin   = GPIO_Pin_1,
                .GPIO_Speed = GPIO_Speed_2MHz,
                .GPIO_Mode  = GPIO_Mode_AF,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
            .pin_source     = GPIO_PinSource1,
        },
        .remap = GPIO_AF_TIM5,
    },
    {
        .timer = TIM5,
        .timer_chan = TIM_Channel_1,
        .pin   = {
            .gpio = GPIOA,
            .init = {
                .GPIO_Pin   = GPIO_Pin_0,
                .GPIO_Speed = GPIO_Speed_2MHz,
                .GPIO_Mode  = GPIO_Mode_AF,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
            .pin_source     = GPIO_PinSource0,
        },
        .remap = GPIO_AF_TIM5,
    },
    // PWM pins on FlexiIO(receiver) port
    {
        // * 6: PB15 = SPI2 MOSI, TIM12 CH2

        .timer = TIM12,
        .timer_chan = TIM_Channel_2,
        .pin   = {
            .gpio = GPIOB,
            .init = {
                .GPIO_Pin   = GPIO_Pin_15,
                .GPIO_Speed = GPIO_Speed_2MHz,
                .GPIO_Mode  = GPIO_Mode_AF,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
            .pin_source     = GPIO_PinSource15,
        },
        .remap = GPIO_AF_TIM12,
    },
    {
        // *  7: PC6 = TIM8 CH1, USART6 TX
        .timer = TIM8,
        .timer_chan = TIM_Channel_1,
        .pin   = {
            .gpio = GPIOC,
            .init = {
                .GPIO_Pin   = GPIO_Pin_6,
                .GPIO_Speed = GPIO_Speed_2MHz,
                .GPIO_Mode  = GPIO_Mode_AF,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
            .pin_source     = GPIO_PinSource6,
        },
        .remap = GPIO_AF_TIM8,
    },

    {
        // *  8: PC7 = TIM8 CH2, USART6 RX
        .timer = TIM8,
        .timer_chan = TIM_Channel_2,
        .pin   = {
            .gpio = GPIOC,
            .init = {
                .GPIO_Pin   = GPIO_Pin_7,
                .GPIO_Speed = GPIO_Speed_2MHz,
                .GPIO_Mode  = GPIO_Mode_AF,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
            .pin_source     = GPIO_PinSource7,
        },
        .remap = GPIO_AF_TIM8,
    },

    {
        // *  9: PC8 = TIM8 CH3
        .timer = TIM8,
        .timer_chan = TIM_Channel_3,
        .pin   = {
            .gpio = GPIOC,
            .init = {
                .GPIO_Pin   = GPIO_Pin_8,
                .GPIO_Speed = GPIO_Speed_2MHz,
                .GPIO_Mode  = GPIO_Mode_AF,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
            .pin_source     = GPIO_PinSource8,
        },
        .remap = GPIO_AF_TIM8,
    },

    {
        // * 10: PC9 = TIM8 CH4
        .timer = TIM8,
        .timer_chan = TIM_Channel_4,
        .pin   = {
            .gpio = GPIOC,
            .init = {
                .GPIO_Pin   = GPIO_Pin_9,
                .GPIO_Speed = GPIO_Speed_2MHz,
                .GPIO_Mode  = GPIO_Mode_AF,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
            .pin_source     = GPIO_PinSource9,
        },
        .remap = GPIO_AF_TIM8,
    },

    {
        // *  5: PB14 = SPI2 MISO, TIM12 CH1, USART3 RTS
        .timer = TIM12,
        .timer_chan = TIM_Channel_1,
        .pin   = {
            .gpio = GPIOB,
            .init = {
                .GPIO_Pin   = GPIO_Pin_14,
                .GPIO_Speed = GPIO_Speed_2MHz,
                .GPIO_Mode  = GPIO_Mode_AF,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
            .pin_source     = GPIO_PinSource14,
        },
        .remap = GPIO_AF_TIM12,
    },
};
#define PIOS_SERVOPORT_ALL_PINS_PWMOUT        6
#define PIOS_SERVOPORT_ALL_PINS_PWMOUT_IN_PPM 11
#define PIOS_SERVOPORT_ALL_PINS_PWMOUT_IN     12

const struct pios_servo_cfg pios_servo_cfg_out = {
    .tim_oc_init          = {
        .TIM_OCMode       = TIM_OCMode_PWM1,
        .TIM_OutputState  = TIM_OutputState_Enable,
        .TIM_OutputNState = TIM_OutputNState_Disable,
        .TIM_Pulse        = PIOS_SERVOS_INITIAL_POSITION,
        .TIM_OCPolarity   = TIM_OCPolarity_High,
        .TIM_OCNPolarity  = TIM_OCPolarity_High,
        .TIM_OCIdleState  = TIM_OCIdleState_Reset,
        .TIM_OCNIdleState = TIM_OCNIdleState_Reset,
    },
    .channels     = pios_tim_servoport_all_pins,
    .num_channels = PIOS_SERVOPORT_ALL_PINS_PWMOUT,
};
// All servo outputs, servo input ch1 ppm, ch2-6 outputs
const struct pios_servo_cfg pios_servo_cfg_out_in_ppm = {
    .tim_oc_init          = {
        .TIM_OCMode       = TIM_OCMode_PWM1,
        .TIM_OutputState  = TIM_OutputState_Enable,
        .TIM_OutputNState = TIM_OutputNState_Disable,
        .TIM_Pulse        = PIOS_SERVOS_INITIAL_POSITION,
        .TIM_OCPolarity   = TIM_OCPolarity_High,
        .TIM_OCNPolarity  = TIM_OCPolarity_High,
        .TIM_OCIdleState  = TIM_OCIdleState_Reset,
        .TIM_OCNIdleState = TIM_OCNIdleState_Reset,
    },
    .channels     = pios_tim_servoport_all_pins,
    .num_channels = PIOS_SERVOPORT_ALL_PINS_PWMOUT_IN_PPM,
};
// All servo outputs, servo inputs ch1-6 Outputs
const struct pios_servo_cfg pios_servo_cfg_out_in = {
    .tim_oc_init          = {
        .TIM_OCMode       = TIM_OCMode_PWM1,
        .TIM_OutputState  = TIM_OutputState_Enable,
        .TIM_OutputNState = TIM_OutputNState_Disable,
        .TIM_Pulse        = PIOS_SERVOS_INITIAL_POSITION,
        .TIM_OCPolarity   = TIM_OCPolarity_High,
        .TIM_OCNPolarity  = TIM_OCPolarity_High,
        .TIM_OCIdleState  = TIM_OCIdleState_Reset,
        .TIM_OCNIdleState = TIM_OCNIdleState_Reset,
    },
    .channels     = pios_tim_servoport_all_pins,
    .num_channels = PIOS_SERVOPORT_ALL_PINS_PWMOUT_IN,
};


/*
 * PWM Inputs
 * TIM1, TIM8, TIM12
 */
#if defined(PIOS_INCLUDE_PWM) || defined(PIOS_INCLUDE_PPM)
#include <pios_pwm_priv.h>
static const struct pios_tim_channel pios_tim_rcvrport_all_channels[] = {
    {
        .timer = TIM12,
        .timer_chan = TIM_Channel_1,
        .pin   = {
            .gpio = GPIOB,
            .init = {
                .GPIO_Pin   = GPIO_Pin_14,
                .GPIO_Speed = GPIO_Speed_2MHz,
                .GPIO_Mode  = GPIO_Mode_AF,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
            .pin_source     = GPIO_PinSource14,
        },
        .remap = GPIO_AF_TIM12,
    },
    {
        .timer = TIM12,
        .timer_chan = TIM_Channel_2,
        .pin   = {
            .gpio = GPIOB,
            .init = {
                .GPIO_Pin   = GPIO_Pin_15,
                .GPIO_Speed = GPIO_Speed_2MHz,
                .GPIO_Mode  = GPIO_Mode_AF,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
            .pin_source     = GPIO_PinSource15,
        },
        .remap = GPIO_AF_TIM12,
    },
    {
        .timer = TIM8,
        .timer_chan = TIM_Channel_1,
        .pin   = {
            .gpio = GPIOC,
            .init = {
                .GPIO_Pin   = GPIO_Pin_6,
                .GPIO_Speed = GPIO_Speed_2MHz,
                .GPIO_Mode  = GPIO_Mode_AF,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
            .pin_source     = GPIO_PinSource6,
        },
        .remap = GPIO_AF_TIM8,
    },
    {
        .timer = TIM8,
        .timer_chan = TIM_Channel_2,
        .pin   = {
            .gpio = GPIOC,
            .init = {
                .GPIO_Pin   = GPIO_Pin_7,
                .GPIO_Speed = GPIO_Speed_2MHz,
                .GPIO_Mode  = GPIO_Mode_AF,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
            .pin_source     = GPIO_PinSource7,
        },
        .remap = GPIO_AF_TIM8,
    },
    {
        .timer = TIM8,
        .timer_chan = TIM_Channel_3,
        .pin   = {
            .gpio = GPIOC,
            .init = {
                .GPIO_Pin   = GPIO_Pin_8,
                .GPIO_Speed = GPIO_Speed_2MHz,
                .GPIO_Mode  = GPIO_Mode_AF,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
            .pin_source     = GPIO_PinSource8,
        },
        .remap = GPIO_AF_TIM8,
    },
    {
        .timer = TIM8,
        .timer_chan = TIM_Channel_4,
        .pin   = {
            .gpio = GPIOC,
            .init = {
                .GPIO_Pin   = GPIO_Pin_9,
                .GPIO_Speed = GPIO_Speed_2MHz,
                .GPIO_Mode  = GPIO_Mode_AF,
                .GPIO_OType = GPIO_OType_PP,
                .GPIO_PuPd  = GPIO_PuPd_UP
            },
            .pin_source     = GPIO_PinSource9,
        },
        .remap = GPIO_AF_TIM8,
    },
};

const struct pios_pwm_cfg pios_pwm_cfg = {
    .tim_ic_init         = {
        .TIM_ICPolarity  = TIM_ICPolarity_Rising,
        .TIM_ICSelection = TIM_ICSelection_DirectTI,
        .TIM_ICPrescaler = TIM_ICPSC_DIV1,
        .TIM_ICFilter    = 0x0,
    },
    .channels     = pios_tim_rcvrport_all_channels,
    .num_channels = NELEMENTS(pios_tim_rcvrport_all_channels),
};
// this configures outputs 2-6 as pwm inputs
const struct pios_pwm_cfg pios_pwm_ppm_cfg = {
    .tim_ic_init         = {
        .TIM_ICPolarity  = TIM_ICPolarity_Rising,
        .TIM_ICSelection = TIM_ICSelection_DirectTI,
        .TIM_ICPrescaler = TIM_ICPSC_DIV1,
        .TIM_ICFilter    = 0x0,
    },
    .channels     = &pios_tim_rcvrport_all_channels[1],
    .num_channels = NELEMENTS(pios_tim_rcvrport_all_channels) - 1,
};

#endif /* if defined(PIOS_INCLUDE_PWM) || defined(PIOS_INCLUDE_PPM) */

/*
 * PPM Input
 */
#if defined(PIOS_INCLUDE_PPM)
#include <pios_ppm_priv.h>
static const struct pios_ppm_cfg pios_ppm_cfg = {
    .tim_ic_init         = {
        .TIM_ICPolarity  = TIM_ICPolarity_Rising,
        .TIM_ICSelection = TIM_ICSelection_DirectTI,
        .TIM_ICPrescaler = TIM_ICPSC_DIV1,
        .TIM_ICFilter    = 0x0,
        .TIM_Channel     = TIM_Channel_1,
    },
    /* Use only the first channel for ppm */
    .channels     = &pios_tim_rcvrport_all_channels[0],
    .num_channels = 1,
};

#endif // PPM

#if defined(PIOS_INCLUDE_GCSRCVR)
#include "pios_gcsrcvr_priv.h"
#endif /* PIOS_INCLUDE_GCSRCVR */

#if defined(PIOS_INCLUDE_RCVR)
#include "pios_rcvr_priv.h"
#endif /* PIOS_INCLUDE_RCVR */

#if defined(PIOS_INCLUDE_USB)
#include "pios_usb_priv.h"

static const struct pios_usb_cfg pios_usb_main_rm1_cfg = {
    .irq                                       = {
        .init                                  = {
            .NVIC_IRQChannel    = OTG_FS_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
            .NVIC_IRQChannelSubPriority        = 0,                 // PriorityGroup=4
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
    .vsense                                    = {
        .gpio = GPIOB,
        .init = {
            .GPIO_Pin   = GPIO_Pin_13,
            .GPIO_Speed = GPIO_Speed_25MHz,
            .GPIO_Mode  = GPIO_Mode_IN,
            .GPIO_OType = GPIO_OType_OD,
        },
    },
    .vsense_active_low                         = false
};

static const struct pios_usb_cfg pios_usb_main_rm2_cfg = {
    .irq                                       = {
        .init                                  = {
            .NVIC_IRQChannel    = OTG_FS_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
            .NVIC_IRQChannelSubPriority        = 0,                 // PriorityGroup=4
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
    .vsense                                    = {
        .gpio = GPIOC,
        .init = {
            .GPIO_Pin   = GPIO_Pin_5,
            .GPIO_Speed = GPIO_Speed_25MHz,
            .GPIO_Mode  = GPIO_Mode_IN,
            .GPIO_OType = GPIO_OType_OD,
        },
    },
    .vsense_active_low                         = false
};

const struct pios_usb_cfg *PIOS_BOARD_HW_DEFS_GetUsbCfg(uint32_t board_revision)
{
    switch (board_revision) {
    case 2:
        return &pios_usb_main_rm1_cfg;

        break;
    case 3:
        return &pios_usb_main_rm2_cfg;

        break;
    default:
        PIOS_DEBUG_Assert(0);
    }
    return NULL;
}

#include "pios_usb_board_data_priv.h"
#include "pios_usb_desc_hid_cdc_priv.h"
#include "pios_usb_desc_hid_only_priv.h"
#include "pios_usbhook.h"

#endif /* PIOS_INCLUDE_USB */

#if defined(PIOS_INCLUDE_COM_MSG)

#include <pios_com_msg_priv.h>

#endif /* PIOS_INCLUDE_COM_MSG */

#if defined(PIOS_INCLUDE_USB_HID) && !defined(PIOS_INCLUDE_USB_CDC)
#include <pios_usb_hid_priv.h>

const struct pios_usb_hid_cfg pios_usb_hid_cfg = {
    .data_if    = 0,
    .data_rx_ep = 1,
    .data_tx_ep = 1,
};
#endif /* PIOS_INCLUDE_USB_HID && !PIOS_INCLUDE_USB_CDC */

#if defined(PIOS_INCLUDE_USB_HID) && defined(PIOS_INCLUDE_USB_CDC)
#include <pios_usb_cdc_priv.h>

const struct pios_usb_cdc_cfg pios_usb_cdc_cfg = {
    .ctrl_if    = 0,
    .ctrl_tx_ep = 2,

    .data_if    = 1,
    .data_rx_ep = 3,
    .data_tx_ep = 3,
};

#include <pios_usb_hid_priv.h>

const struct pios_usb_hid_cfg pios_usb_hid_cfg = {
    .data_if    = 2,
    .data_rx_ep = 1,
    .data_tx_ep = 1,
};
#endif /* PIOS_INCLUDE_USB_HID && PIOS_INCLUDE_USB_CDC */

#ifdef PIOS_INCLUDE_WS2811
#include <pios_ws2811_cfg.h>
#include <hwsettings.h>
#define PIOS_WS2811_TIM_DIVIDER (PIOS_PERIPHERAL_APB2_CLOCK / (800000 * PIOS_WS2811_TIM_PERIOD))

void DMA2_Stream1_IRQHandler(void) __attribute__((alias("PIOS_WS2811_irq_handler")));
// list of pin configurable as ws281x outputs.
// this will not clash with PWM in or servo output as
// pins will be reconfigured as _OUT so the alternate function is disabled.
const struct pios_ws2811_pin_cfg pios_ws2811_pin_cfg[] = {
    [HWSETTINGS_WS2811LED_OUT_SERVOOUT1] =   {
        .gpio     = GPIOB,
        .gpioInit =                          {
            .GPIO_Pin   = GPIO_Pin_0,
            .GPIO_Speed = GPIO_Speed_25MHz,
            .GPIO_Mode  = GPIO_Mode_OUT,
            .GPIO_OType = GPIO_OType_PP,
        },
    },
    [HWSETTINGS_WS2811LED_OUT_SERVOOUT2] =   {
        .gpio     = GPIOB,
        .gpioInit =                          {
            .GPIO_Pin   = GPIO_Pin_1,
            .GPIO_Speed = GPIO_Speed_25MHz,
            .GPIO_Mode  = GPIO_Mode_OUT,
            .GPIO_OType = GPIO_OType_PP,
        },
    },
    [HWSETTINGS_WS2811LED_OUT_SERVOOUT3] =   {
        .gpio     = GPIOA,
        .gpioInit =                          {
            .GPIO_Pin   = GPIO_Pin_3,
            .GPIO_Speed = GPIO_Speed_25MHz,
            .GPIO_Mode  = GPIO_Mode_OUT,
            .GPIO_OType = GPIO_OType_PP,
        },
    },
    [HWSETTINGS_WS2811LED_OUT_SERVOOUT4] =   {
        .gpio     = GPIOA,
        .gpioInit =                          {
            .GPIO_Pin   = GPIO_Pin_2,
            .GPIO_Speed = GPIO_Speed_25MHz,
            .GPIO_Mode  = GPIO_Mode_OUT,
            .GPIO_OType = GPIO_OType_PP,
        },
    },
    [HWSETTINGS_WS2811LED_OUT_SERVOOUT5] =   {
        .gpio     = GPIOA,
        .gpioInit =                          {
            .GPIO_Pin   = GPIO_Pin_1,
            .GPIO_Speed = GPIO_Speed_25MHz,
            .GPIO_Mode  = GPIO_Mode_OUT,
            .GPIO_OType = GPIO_OType_PP,
        },
    },
    [HWSETTINGS_WS2811LED_OUT_SERVOOUT6] =   {
        .gpio     = GPIOA,
        .gpioInit =                          {
            .GPIO_Pin   = GPIO_Pin_0,
            .GPIO_Speed = GPIO_Speed_25MHz,
            .GPIO_Mode  = GPIO_Mode_OUT,
            .GPIO_OType = GPIO_OType_PP,
        },
    },
    [HWSETTINGS_WS2811LED_OUT_FLEXIIOPIN3] = {
        .gpio     = GPIOB,
        .gpioInit =                          {
            .GPIO_Pin   = GPIO_Pin_12,
            .GPIO_Speed = GPIO_Speed_25MHz,
            .GPIO_Mode  = GPIO_Mode_OUT,
            .GPIO_OType = GPIO_OType_PP,
        },
    },
    [HWSETTINGS_WS2811LED_OUT_FLEXIIOPIN4] = {
        .gpio     = GPIOB,
        .gpioInit =                          {
            .GPIO_Pin   = GPIO_Pin_13,
            .GPIO_Speed = GPIO_Speed_25MHz,
            .GPIO_Mode  = GPIO_Mode_OUT,
            .GPIO_OType = GPIO_OType_PP,
        },
    },
};

const struct pios_ws2811_cfg pios_ws2811_cfg = {
    .timer     = TIM1,
    .timerInit = {
        .TIM_Prescaler         = PIOS_WS2811_TIM_DIVIDER - 1,
        .TIM_ClockDivision     = TIM_CKD_DIV1,
        .TIM_CounterMode       = TIM_CounterMode_Up,
        // period (1.25 uS per period
        .TIM_Period                            = PIOS_WS2811_TIM_PERIOD,
        .TIM_RepetitionCounter = 0x0000,
    },

    .timerCh1     = 1,
    .streamCh1    = DMA2_Stream1,
    .timerCh2     = 3,
    .streamCh2    = DMA2_Stream6,
    .streamUpdate = DMA2_Stream5,

    // DMA streamCh1, triggered by timerCh1 pwm signal.
    // if FrameBuffer indicates, reset output value early to indicate "0" bit to ws2812
    .dmaInitCh1 = PIOS_WS2811_DMA_CH1_CONFIG(DMA_Channel_6),
    .dmaItCh1   = DMA_IT_TEIF1 | DMA_IT_TCIF1,

    // DMA streamCh2, triggered by timerCh2 pwm signal.
    // Reset output value late to indicate "1" bit to ws2812.
    .dmaInitCh2 = PIOS_WS2811_DMA_CH2_CONFIG(DMA_Channel_6),
    .dmaItCh2   = DMA_IT_TEIF6 | DMA_IT_TCIF6,

    // DMA streamUpdate Triggered by timer update event
    // Outputs a high logic level at beginning of a cycle
    .dmaInitUpdate = PIOS_WS2811_DMA_UPDATE_CONFIG(DMA_Channel_6),
    .dmaItUpdate   = DMA_IT_TEIF5 | DMA_IT_TCIF5,
    .dmaSource     = TIM_DMA_CC1 | TIM_DMA_CC3 | TIM_DMA_Update,

    // DMAInitCh1 interrupt vector, used to block timer at end of framebuffer transfer
    .irq                                       = {
        .flags = (DMA_IT_TCIF1),
        .init  = {
            .NVIC_IRQChannel    = DMA2_Stream1_IRQn,
            .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGHEST,
            .NVIC_IRQChannelSubPriority        = 0,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

void PIOS_WS2811_irq_handler(void)
{
    PIOS_WS2811_DMA_irq_handler();
}
#endif // PIOS_INCLUDE_WS2811
