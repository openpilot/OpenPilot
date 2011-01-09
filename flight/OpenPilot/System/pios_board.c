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
#include <pios_spi_priv.h>
#include <pios_usart_priv.h>
#include <pios_com_priv.h>
#include <pios_i2c_priv.h>
#include <openpilot.h>
#include <uavobjectsinit.h>

/**
 * PIOS_Board_Init()
 * initializes all the core subsystems on this specific hardware
 * called from System/openpilot.c
 */
void PIOS_Board_Init(void) {

	/* Delay system */
	PIOS_DELAY_Init();	
	
	/* SPI Init */
	PIOS_SPI_Init();

	/* Enable and mount the SDCard */
	PIOS_SDCARD_Init();
	PIOS_SDCARD_MountFS(0);
#if defined(PIOS_INCLUDE_SPEKTRUM)
	/* SPEKTRUM init must come before comms */
	PIOS_SPEKTRUM_Init();
#endif
	/* Initialize UAVObject libraries */
	EventDispatcherInitialize();
	UAVObjInitialize();
	UAVObjectsInitializeAll();

	/* Initialize the alarms library */
	AlarmsInitialize();

	/* Initialize the task monitor library */
	TaskMonitorInitialize();

	/* Initialize the PiOS library */
	PIOS_COM_Init();
	PIOS_Servo_Init();
	PIOS_ADC_Init();
	PIOS_GPIO_Init();

#if defined(PIOS_INCLUDE_PWM)
	PIOS_PWM_Init();
#endif
#if defined(PIOS_INCLUDE_PPM)
	PIOS_PPM_Init();
#endif
#if defined(PIOS_INCLUDE_USB_HID)
	PIOS_USB_HID_Init(0);
#endif
	PIOS_I2C_Init();
	PIOS_IAP_Init();
}

/* MicroSD Interface
 * 
 * NOTE: Leave this declared as const data so that it ends up in the 
 * .rodata section (ie. Flash) rather than in the .bss section (RAM).
 */
void PIOS_SPI_sdcard_irq_handler(void);
void DMA1_Channel2_IRQHandler() __attribute__ ((alias ("PIOS_SPI_sdcard_irq_handler")));
void DMA1_Channel3_IRQHandler() __attribute__ ((alias ("PIOS_SPI_sdcard_irq_handler")));
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
    .SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256, /* Maximum divider (ie. slowest clock rate) */
  },
  .dma = {
    .ahb_clk  = RCC_AHBPeriph_DMA1,
    
    .irq = {
      .handler = PIOS_SPI_sdcard_irq_handler,
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
    .SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16, 
  },
  .use_crc = TRUE,
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
      .GPIO_Mode  = GPIO_Mode_IN_FLOATING,
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

/*
 * Telemetry USART
 */
void PIOS_USART_telem_irq_handler(void);
void USART2_IRQHandler() __attribute__ ((alias ("PIOS_USART_telem_irq_handler")));
const struct pios_usart_cfg pios_usart_telem_cfg = {
  .regs  = USART2,
  .init = {
    #if defined (PIOS_COM_TELEM_BAUDRATE)
        .USART_BaudRate        = PIOS_COM_TELEM_BAUDRATE,
    #else
        .USART_BaudRate        = 57600,
    #endif
    .USART_WordLength          = USART_WordLength_8b,
    .USART_Parity              = USART_Parity_No,
    .USART_StopBits            = USART_StopBits_1,
    .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
    .USART_Mode                = USART_Mode_Rx | USART_Mode_Tx,
  },
  .irq = {
    .handler = PIOS_USART_telem_irq_handler,
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
void PIOS_USART_gps_irq_handler(void);
void USART3_IRQHandler() __attribute__ ((alias ("PIOS_USART_gps_irq_handler")));
const struct pios_usart_cfg pios_usart_gps_cfg = {
  .regs = USART3,
  .remap = GPIO_PartialRemap_USART3,
  .init = {
    #if defined (PIOS_COM_GPS_BAUDRATE)
        .USART_BaudRate        = PIOS_COM_GPS_BAUDRATE,
    #else
        .USART_BaudRate        = 57600,
    #endif
    .USART_WordLength          = USART_WordLength_8b,
    .USART_Parity              = USART_Parity_No,
    .USART_StopBits            = USART_StopBits_1,
    .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
    .USART_Mode                = USART_Mode_Rx | USART_Mode_Tx,
  },
  .irq = {
    .handler = PIOS_USART_gps_irq_handler,
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
void PIOS_USART_aux_irq_handler(void);
void USART1_IRQHandler() __attribute__ ((alias ("PIOS_USART_aux_irq_handler")));
const struct pios_usart_cfg pios_usart_aux_cfg = {
  .regs = USART1,
  .init = {
    #if defined (PIOS_COM_AUX_BAUDRATE)
        .USART_BaudRate        = PIOS_COM_AUX_BAUDRATE,
    #else
        .USART_BaudRate        = 57600,
    #endif
    .USART_BaudRate            = 57600,
    .USART_WordLength          = USART_WordLength_8b,
    .USART_Parity              = USART_Parity_No,
    .USART_StopBits            = USART_StopBits_1,
    .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
    .USART_Mode                = USART_Mode_Rx | USART_Mode_Tx,
  },
  .irq = {
    .handler = PIOS_USART_aux_irq_handler,
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

#ifdef PIOS_COM_SPEKTRUM
/*
 * SPEKTRUM USART
 */
void PIOS_USART_spektrum_irq_handler(void);
void USART1_IRQHandler() __attribute__ ((alias ("PIOS_USART_spektrum_irq_handler")));
const struct pios_usart_cfg pios_usart_spektrum_cfg = {
  .regs = USART1,
  .init = {
    #if defined (PIOS_COM_SPEKTRUM_BAUDRATE)
        .USART_BaudRate        = PIOS_COM_SPEKTRUM_BAUDRATE,
    #else
        .USART_BaudRate        = 115200,
    #endif
    .USART_WordLength          = USART_WordLength_8b,
    .USART_Parity              = USART_Parity_No,
    .USART_StopBits            = USART_StopBits_1,
    .USART_HardwareFlowControl = USART_HardwareFlowControl_None,
    .USART_Mode                = USART_Mode_Rx,
  },
  .irq = {
    .handler = PIOS_USART_spektrum_irq_handler,
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
#endif

/*
 * Board specific number of devices.
 */
struct pios_usart_dev pios_usart_devs[] = {
#define PIOS_USART_TELEM  0
  {
    .cfg = &pios_usart_telem_cfg,
  },
#define PIOS_USART_GPS    1
  {
    .cfg = &pios_usart_gps_cfg,
  },
#ifdef PIOS_COM_AUX
#define PIOS_USART_AUX    2
  {
    .cfg = &pios_usart_aux_cfg,
  },
#endif
#ifdef PIOS_COM_SPEKTRUM
#define PIOS_USART_AUX    2
  {
    .cfg = &pios_usart_spektrum_cfg,
  },
#endif
};

uint8_t pios_usart_num_devices = NELEMENTS(pios_usart_devs);

void PIOS_USART_telem_irq_handler(void)
{
  PIOS_USART_IRQ_Handler(PIOS_USART_TELEM);
}

void PIOS_USART_gps_irq_handler(void)
{
  PIOS_USART_IRQ_Handler(PIOS_USART_GPS);
}

#ifdef PIOS_COM_AUX
void PIOS_USART_aux_irq_handler(void)
{
  PIOS_USART_IRQ_Handler(PIOS_USART_AUX);
}
#endif

#ifdef PIOS_COM_SPEKTRUM
void PIOS_USART_spektrum_irq_handler(void)
{
	SPEKTRUM_IRQHandler();
}
#endif

/*
 * COM devices
 */

/*
 * Board specific number of devices.
 */
extern const struct pios_com_driver pios_usart_com_driver;
extern const struct pios_com_driver pios_usb_com_driver;

struct pios_com_dev pios_com_devs[] = {
  {
    .id     = PIOS_USART_TELEM,
    .driver = &pios_usart_com_driver,
  },
  {
    .id     = PIOS_USART_GPS,
    .driver = &pios_usart_com_driver,
  },
#if defined(PIOS_INCLUDE_USB_HID)
  {
    .id     = 0,
    .driver = &pios_usb_com_driver,
  },
#endif
#ifdef PIOS_COM_AUX
  {
    .id     = PIOS_USART_AUX,
    .driver = &pios_usart_com_driver,
  },
#endif
#ifdef PIOS_COM_SPEKTRUM
  {
    .id     = PIOS_USART_AUX,
    .driver = &pios_usart_com_driver,
  },
#endif
};

const uint8_t pios_com_num_devices = NELEMENTS(pios_com_devs);

/*
 * I2C Adapters
 */

void PIOS_I2C_main_adapter_ev_irq_handler(void);
void PIOS_I2C_main_adapter_er_irq_handler(void);
void I2C2_EV_IRQHandler() __attribute__ ((alias ("PIOS_I2C_main_adapter_ev_irq_handler")));
void I2C2_ER_IRQHandler() __attribute__ ((alias ("PIOS_I2C_main_adapter_er_irq_handler")));

const struct pios_i2c_adapter_cfg pios_i2c_main_adapter_cfg = {
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
    .handler = PIOS_I2C_main_adapter_ev_irq_handler,
    .flags   = 0,		/* FIXME: check this */
    .init = {
      .NVIC_IRQChannel                   = I2C2_EV_IRQn,
      .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGHEST,
      .NVIC_IRQChannelSubPriority        = 0,
      .NVIC_IRQChannelCmd                = ENABLE,
    },
  },
  .error = {
    .handler = PIOS_I2C_main_adapter_er_irq_handler,
    .flags   = 0,		/* FIXME: check this */
    .init = {
      .NVIC_IRQChannel                   = I2C2_ER_IRQn,
      .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGHEST,
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

/**
 * @}
 */
