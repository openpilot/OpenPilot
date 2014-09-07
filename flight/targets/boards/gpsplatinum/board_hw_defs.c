/**
 ******************************************************************************
 * @file       board_hw_defs.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @addtogroup OpenPilotSystem OpenPilot System
 * @{
 * @addtogroup OpenPilotCore OpenPilot Core
 * @{
 * @brief Defines board specific static initializers for hardware for the GPS board.
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

#define BOARD_REVISION_GPSP 1
/*
 * GPS Platinum board.
 * pins allocation:
 * port         |  Pins
 * -------------|-------------
 * GPS I2C      | PB7 SDA
 *              | PB6 SCL
 * ---------------------------
 * Led HB       | PB4
 * ---------------------------
 * Mag/Flash SPI| PA4 MAG SS
 *              | PA5 SCK
 *              | PA6 MISO
 *              | PA7 MOSI
 *              | PB1 FLASH SS
 *              | PB0 Mag Int
 *----------------------------
 * Main Port    | PA9  TX
 *              | PA10 RX
 *----------------------------
 */
#if defined(PIOS_INCLUDE_LED)

#include <pios_led_priv.h>

static const struct pios_gpio pios_leds_gpsp[] = {
    // PB4
    [PIOS_LED_HEARTBEAT] = {
        .pin                =             {
            .gpio = GPIOB,
            .init =             {
                .GPIO_Pin   = GPIO_Pin_4,
                .GPIO_Mode  = GPIO_Mode_OUT,
                .GPIO_OType = GPIO_OType_OD,
                .GPIO_Speed = GPIO_Speed_Level_1,
            },
        },
        .active_low         = false
    },
};

static const struct pios_gpio_cfg pios_led_cfg_gpsp = {
    .gpios     = pios_leds_gpsp,
    .num_gpios = NELEMENTS(pios_leds_gpsp),
};

const struct pios_gpio_cfg *PIOS_BOARD_HW_DEFS_GetLedCfg(__attribute__((unused)) uint32_t board_revision)
{
    return &pios_led_cfg_gpsp;
}

#endif /* PIOS_INCLUDE_LED */

#if defined(PIOS_INCLUDE_SPI)

#include <pios_spi_priv.h>

void PIOS_SPI_mag_flash_irq_handler(void);
void DMA1_Channel2_3_IRQHandler() __attribute__((alias("PIOS_SPI_mag_flash_irq_handler")));


static uint32_t pios_spi_mag_flash_id;
static const struct pios_spi_cfg pios_spi_mag_flash_cfg = {
    .remap = GPIO_AF_0,
    .regs  = SPI1,
    .init  = {
        .SPI_Mode              = SPI_Mode_Master,
        .SPI_Direction         = SPI_Direction_2Lines_FullDuplex,
        .SPI_DataSize          = SPI_DataSize_8b,
        .SPI_NSS                         = SPI_NSS_Soft,
        .SPI_FirstBit          = SPI_FirstBit_MSB,
        .SPI_CRCPolynomial     = 7,
        .SPI_CPOL              = SPI_CPOL_High,
        .SPI_CPHA              = SPI_CPHA_2Edge,
        .SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8,
    },
    .dma                                 = {
        .ahb_clk = RCC_AHBPeriph_DMA1,

        .irq     = {
            .flags = (DMA1_FLAG_TC2 | DMA1_FLAG_TE2 | DMA1_FLAG_HT2 | DMA1_FLAG_GL2),
            .init  = {
                .NVIC_IRQChannel    = DMA1_Channel2_3_IRQn,
                .NVIC_IRQChannelPriority = PIOS_IRQ_PRIO_MID,
                .NVIC_IRQChannelCmd = ENABLE,
            },
        },

        .rx                              = {
            .channel = DMA1_Channel2,
            .init    = {
                .DMA_PeripheralBaseAddr  = (uint32_t)&(SPI1->DR),
                .DMA_DIR                 = DMA_DIR_PeripheralSRC,
                .DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
                .DMA_MemoryInc          = DMA_MemoryInc_Enable,
                .DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
                .DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
                .DMA_Mode     = DMA_Mode_Normal,
                .DMA_Priority = DMA_Priority_Medium,
                .DMA_M2M                 = DMA_M2M_Disable,
            },
        },
        .tx                              = {
            .channel = DMA1_Channel3,
            .init    = {
                .DMA_PeripheralBaseAddr  = (uint32_t)&(SPI1->DR),
                .DMA_DIR                 = DMA_DIR_PeripheralDST,
                .DMA_PeripheralInc      = DMA_PeripheralInc_Disable,
                .DMA_MemoryInc          = DMA_MemoryInc_Enable,
                .DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte,
                .DMA_MemoryDataSize     = DMA_MemoryDataSize_Byte,
                .DMA_Mode     = DMA_Mode_Normal,
                .DMA_Priority = DMA_Priority_Medium,
                .DMA_M2M                 = DMA_M2M_Disable,
            },
        },
    },
    .use_crc = false,
    .sclk    = {
        .gpio = GPIOA,
        .init = {
            .GPIO_Pin   = GPIO_Pin_5,
            .GPIO_Speed = GPIO_Speed_10MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_PP,
        },
    },
    .miso                                = {
        .gpio = GPIOA,
        .init = {
            .GPIO_Pin   = GPIO_Pin_6,
            .GPIO_Speed = GPIO_Speed_10MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_OD,
        },
    },
    .mosi                                = {
        .gpio = GPIOA,
        .init = {
            .GPIO_Pin   = GPIO_Pin_7,
            .GPIO_Speed = GPIO_Speed_10MHz,
            .GPIO_Mode  = GPIO_Mode_AF,
            .GPIO_OType = GPIO_OType_PP,
        },
    },
    .slave_count                         = 2,
    .ssel                                = {
        {
            .gpio = GPIOA,
            .init = {
                .GPIO_Pin   = GPIO_Pin_4,
                .GPIO_Speed = GPIO_Speed_10MHz,
                .GPIO_Mode  = GPIO_Mode_OUT,
                .GPIO_OType = GPIO_OType_PP,
            }
        },
        {
            .gpio = GPIOB,
            .init = {
                .GPIO_Pin   = GPIO_Pin_1,
                .GPIO_Speed = GPIO_Speed_10MHz,
                .GPIO_Mode  = GPIO_Mode_OUT,
                .GPIO_OType = GPIO_OType_PP,
            }
        }
    },
};

void PIOS_SPI_mag_flash_irq_handler(void)
{
    /* Call into the generic code to handle the IRQ for this specific device */
    PIOS_SPI_IRQ_Handler(pios_spi_mag_flash_id);
}

#ifdef PIOS_INCLUDE_FLASH
#include "pios_flash_jedec_priv.h"
#include "pios_flash.h"
#endif /* PIOS_INCLUDE_FLASH */

#if defined(PIOS_INCLUDE_HMC5X83)
pios_hmc5x83_dev_t onboard_mag;
#include "pios_hmc5x83.h"
#ifdef PIOS_HMC5X83_HAS_GPIOS
bool pios_board_mag_handler()
{
    return PIOS_HMC5x83_IRQHandler(onboard_mag);
}
static const struct pios_exti_cfg pios_exti_mag_cfg __exti_config = {
    .vector = pios_board_mag_handler,
    .line   = EXTI_Line7,
    .pin    = {
        .gpio = GPIOB,
        .init = {
            .GPIO_Pin   = GPIO_Pin_7,
            .GPIO_Speed = GPIO_Speed_Level_3,
            .GPIO_Mode  = GPIO_Mode_IN,
            .GPIO_OType = GPIO_OType_OD,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL,
        },
    },
    .irq                             = {
        .init                        = {
            .NVIC_IRQChannel    = EXTI4_15_IRQn,
            .NVIC_IRQChannelPriority = PIOS_IRQ_PRIO_LOW,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
    .exti                            = {
        .init                        = {
            .EXTI_Line    = EXTI_Line7, // matches above GPIO pin
            .EXTI_Mode    = EXTI_Mode_Interrupt,
            .EXTI_Trigger = EXTI_Trigger_Rising,
            .EXTI_LineCmd = ENABLE,
        },
    },
};
#endif /* ifdef PIOS_HMC5X83_HAS_GPIOS */

static const struct pios_hmc5x83_cfg pios_mag_cfg = {
#ifdef PIOS_HMC5X83_HAS_GPIOS
    .exti_cfg  = &pios_exti_mag_cfg,
#endif
    .M_ODR     = PIOS_HMC5x83_ODR_30,
    .Meas_Conf = PIOS_HMC5x83_MEASCONF_NORMAL,
    .Gain             = PIOS_HMC5x83_GAIN_1_3,
    .Mode             = PIOS_HMC5x83_MODE_CONTINUOUS,
    .Driver    = &PIOS_HMC5x83_SPI_DRIVER,
    .TempCompensation = true,
};
#endif /* PIOS_INCLUDE_HMC5883 */


#endif /* PIOS_INCLUDE_SPI */

#if defined(PIOS_INCLUDE_USART)

#include "pios_usart_priv.h"

static const struct pios_usart_cfg pios_usart_generic_main_cfg = {
    .regs  = USART1,
    .remap = GPIO_AF_1,
    .init  = {
        .USART_BaudRate   = 57600,
        .USART_WordLength = USART_WordLength_8b,
        .USART_Parity     = USART_Parity_No,
        .USART_StopBits   = USART_StopBits_1,
        .USART_HardwareFlowControl   = USART_HardwareFlowControl_None,
        .USART_Mode                  = USART_Mode_Rx | USART_Mode_Tx,
    },
    .irq                             = {
        .init                        = {
            .NVIC_IRQChannel    = USART1_IRQn,
            .NVIC_IRQChannelPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
    .rx                              = {
        .gpio = GPIOA,
        .init = {
            .GPIO_Pin   = GPIO_Pin_10,
            .GPIO_Speed = GPIO_Speed_2MHz,
            .GPIO_OType = GPIO_OType_OD,
            .GPIO_Mode  = GPIO_Mode_AF,
        },
    },
    .tx                              = {
        .gpio = GPIOA,
        .init = {
            .GPIO_Pin   = GPIO_Pin_9,
            .GPIO_Speed = GPIO_Speed_2MHz,
            .GPIO_OType = GPIO_OType_PP,
            .GPIO_Mode  = GPIO_Mode_AF,
        },
    },
};

#endif /* PIOS_INCLUDE_USART */

#if defined(PIOS_INCLUDE_COM)

#include "pios_com_priv.h"

#endif /* PIOS_INCLUDE_COM */

#if defined(PIOS_INCLUDE_RTC)
/*
 * Realtime Clock (RTC)
 */
#include <pios_rtc_priv.h>

void PIOS_RTC_IRQ_Handler(void);
void RTC_IRQHandler() __attribute__((alias("PIOS_RTC_IRQ_Handler")));
static const struct pios_rtc_cfg pios_rtc_main_cfg = {
    .clksrc    = RCC_RTCCLKSource_LSI,
    .prescaler = 100,
    .irq                             = {
        .init                        = {
            .NVIC_IRQChannel    = RTC_IRQn,
            .NVIC_IRQChannelPriority = PIOS_IRQ_PRIO_MID,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

void PIOS_RTC_IRQ_Handler(void)
{
    PIOS_RTC_irq_handler();
}

#endif /* if defined(PIOS_INCLUDE_RTC) */

#if defined(PIOS_INCLUDE_I2C)

#include <pios_i2c_priv.h>

/*
 * I2C Adapters
 */

void PIOS_I2C_gps_irq_handler(void);
void I2C1_IRQHandler() __attribute__((alias("PIOS_I2C_gps_irq_handler")));

static const struct pios_i2c_adapter_cfg pios_i2c_gps_cfg = {
    .remap = GPIO_AF_1,
    .regs  = I2C1,
    .init  = {
        .I2C_Mode                    = I2C_Mode_I2C,
        .I2C_AnalogFilter  = I2C_AnalogFilter_Enable,
        .I2C_DigitalFilter = 0x00,
        .I2C_OwnAddress1   = 0x00,
        .I2C_Ack    = I2C_Ack_Enable,
        .I2C_AcknowledgedAddress     = I2C_AcknowledgedAddress_7bit,
        .I2C_Timing = (uint32_t)0x00210507,
    },
    .transfer_timeout_ms             = 50,
    .scl                             = {
        .gpio = GPIOB,
        .init = {
            .GPIO_Pin   = GPIO_Pin_6,
            .GPIO_Speed = GPIO_Speed_2MHz,
            .GPIO_OType = GPIO_OType_OD,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL,
            .GPIO_Mode  = GPIO_Mode_AF,
        },
        .pin_source                  = GPIO_PinSource6,
    },
    .sda                             = {
        .gpio = GPIOB,
        .init = {
            .GPIO_Pin   = GPIO_Pin_7,
            .GPIO_Speed = GPIO_Speed_2MHz,
            .GPIO_OType = GPIO_OType_OD,
            .GPIO_PuPd  = GPIO_PuPd_NOPULL,
            .GPIO_Mode  = GPIO_Mode_AF,
        },
        .pin_source                  = GPIO_PinSource7,
    },
    .event                           = {
        .flags = 0,
        .init  = {
            .NVIC_IRQChannel    = I2C1_IRQn,
            .NVIC_IRQChannelPriority = PIOS_IRQ_PRIO_HIGH,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
    .error                           = {
        .flags = 0,
        .init  = {
            .NVIC_IRQChannel    = I2C1_IRQn,
            .NVIC_IRQChannelPriority = PIOS_IRQ_PRIO_HIGH,
            .NVIC_IRQChannelCmd = ENABLE,
        },
    },
};

uint32_t pios_i2c_gps_id;
void PIOS_I2C_gps_irq_handler(void)
{
    /* Call into the generic code to handle the IRQ for this specific device */
    PIOS_I2C_IRQ_Handler(pios_i2c_gps_id);
}


#endif /* PIOS_INCLUDE_I2C */

#if defined(PIOS_INCLUDE_COM_MSG)

#include <pios_com_msg_priv.h>

#endif /* PIOS_INCLUDE_COM_MSG */
