#include <pios_config.h>

#if defined(PIOS_INCLUDE_LED)

#include <pios_led_priv.h>
static const struct pios_led pios_leds[] = {
	[PIOS_LED_HEARTBEAT] = {
		.pin = {
			.gpio = GPIOA,
			.init = {
				.GPIO_Pin   = GPIO_Pin_3,
				.GPIO_Mode  = GPIO_Mode_Out_PP,
				.GPIO_Speed = GPIO_Speed_50MHz,
			},
		},
	},
	[PIOS_LED_ALARM] = {
		.pin = {
			.gpio = GPIOA,
			.init = {
				.GPIO_Pin   = GPIO_Pin_2,
				.GPIO_Mode  = GPIO_Mode_Out_PP,
				.GPIO_Speed = GPIO_Speed_50MHz,
			},
		},
	},
};

static const struct pios_led_cfg pios_led_cfg = {
	.leds     = pios_leds,
	.num_leds = NELEMENTS(pios_leds),
};

#endif	/* PIOS_INCLUDE_LED */

#if defined(PIOS_INCLUDE_SPI)

#include <pios_spi_priv.h>

/* SPI2 Interface
 *      - Used for mainboard communications and magnetometer
 *
 * NOTE: Leave this declared as const data so that it ends up in the 
 * .rodata section (ie. Flash) rather than in the .bss section (RAM).
 */
void PIOS_SPI_op_mag_irq_handler(void);
void DMA1_Channel5_IRQHandler() __attribute__ ((alias("PIOS_SPI_op_mag_irq_handler")));
void DMA1_Channel4_IRQHandler() __attribute__ ((alias("PIOS_SPI_op_mag_irq_handler")));
static const struct pios_spi_cfg pios_spi_op_mag_cfg = {
	.regs = SPI2,
	.init = {
		 .SPI_Mode = SPI_Mode_Slave,
		 .SPI_Direction = SPI_Direction_2Lines_FullDuplex,
		 .SPI_DataSize = SPI_DataSize_8b,
		 .SPI_NSS = SPI_NSS_Hard,
		 .SPI_FirstBit = SPI_FirstBit_MSB,
		 .SPI_CRCPolynomial = 7,
		 .SPI_CPOL = SPI_CPOL_High,
		 .SPI_CPHA = SPI_CPHA_2Edge,
		 },
	.use_crc = TRUE,
	.dma = {
		.ahb_clk = RCC_AHBPeriph_DMA1,

		.irq = {
			.flags =
			(DMA1_FLAG_TC4 | DMA1_FLAG_TE4 | DMA1_FLAG_HT4 |
			 DMA1_FLAG_GL4),
			.init = {
				 .NVIC_IRQChannel = DMA1_Channel4_IRQn,
				 .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
				 .NVIC_IRQChannelSubPriority = 0,
				 .NVIC_IRQChannelCmd = ENABLE,
				 },
			},

		.rx = {
		       .channel = DMA1_Channel4,
		       .init = {
				.DMA_PeripheralBaseAddr =
				(uint32_t) & (SPI2->DR),
				.DMA_DIR = DMA_DIR_PeripheralSRC,
				.DMA_PeripheralInc =
				DMA_PeripheralInc_Disable,
				.DMA_MemoryInc = DMA_MemoryInc_Enable,
				.DMA_PeripheralDataSize =
				DMA_PeripheralDataSize_Byte,
				.DMA_MemoryDataSize =
				DMA_MemoryDataSize_Byte,
				.DMA_Mode = DMA_Mode_Normal,
				.DMA_Priority = DMA_Priority_Medium,
				.DMA_M2M = DMA_M2M_Disable,
				},
		       },
		.tx = {
		       .channel = DMA1_Channel5,
		       .init = {
				.DMA_PeripheralBaseAddr =
				(uint32_t) & (SPI2->DR),
				.DMA_DIR = DMA_DIR_PeripheralDST,
				.DMA_PeripheralInc =
				DMA_PeripheralInc_Disable,
				.DMA_MemoryInc = DMA_MemoryInc_Enable,
				.DMA_PeripheralDataSize =
				DMA_PeripheralDataSize_Byte,
				.DMA_MemoryDataSize =
				DMA_MemoryDataSize_Byte,
				.DMA_Mode = DMA_Mode_Normal,
				.DMA_Priority = DMA_Priority_Medium,
				.DMA_M2M = DMA_M2M_Disable,
				},
		       },
		},
	.ssel = {
		 .gpio = GPIOB,
		 .init = {
			  .GPIO_Pin = GPIO_Pin_12,
			  .GPIO_Speed = GPIO_Speed_10MHz,
			  .GPIO_Mode = GPIO_Mode_IN_FLOATING,
			  },
		 },
	.sclk = {
		 .gpio = GPIOB,
		 .init = {
			  .GPIO_Pin = GPIO_Pin_13,
			  .GPIO_Speed = GPIO_Speed_10MHz,
			  .GPIO_Mode = GPIO_Mode_IN_FLOATING,
			  },
		 },
	.miso = {
		 .gpio = GPIOB,
		 .init = {
			  .GPIO_Pin = GPIO_Pin_14,
			  .GPIO_Speed = GPIO_Speed_10MHz,
			  .GPIO_Mode = GPIO_Mode_AF_PP,
			  },
		 },
	.mosi = {
		 .gpio = GPIOB,
		 .init = {
			  .GPIO_Pin = GPIO_Pin_15,
			  .GPIO_Speed = GPIO_Speed_10MHz,
			  .GPIO_Mode = GPIO_Mode_IN_FLOATING,
			  },
		 },
};

uint32_t pios_spi_op_mag_id;
void PIOS_SPI_op_mag_irq_handler(void)
{
	/* Call into the generic code to handle the IRQ for this specific device */
	PIOS_SPI_IRQ_Handler(pios_spi_op_mag_id);
}

/* SPI1 Interface
 *    - Used for BMA180 accelerometer
 */
void PIOS_SPI_accel_irq_handler(void);
void DMA1_Channel2_IRQHandler() __attribute__ ((alias("PIOS_SPI_accel_irq_handler")));
void DMA1_Channel3_IRQHandler() __attribute__ ((alias("PIOS_SPI_accel_irq_handler")));
static const struct pios_spi_cfg pios_spi_accel_cfg = {
	.regs = SPI1,
	.init   = {
			.SPI_Mode              = SPI_Mode_Master,
			.SPI_Direction         = SPI_Direction_2Lines_FullDuplex,
			.SPI_DataSize          = SPI_DataSize_8b,
			.SPI_NSS               = SPI_NSS_Soft,
			.SPI_FirstBit          = SPI_FirstBit_MSB,
			.SPI_CRCPolynomial     = 7,
			.SPI_CPOL              = SPI_CPOL_High,
			.SPI_CPHA              = SPI_CPHA_1Edge,
			.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2,
	},
	.use_crc = FALSE,
	.dma = {
			.ahb_clk  = RCC_AHBPeriph_DMA1,

			.irq = {
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
							.DMA_Priority           = DMA_Priority_High,
							.DMA_M2M                = DMA_M2M_Disable,
					},
			},
	},
	.ssel = {
			.gpio = GPIOA,
			.init = {
					.GPIO_Pin   = GPIO_Pin_4,
					.GPIO_Speed = GPIO_Speed_10MHz,
					.GPIO_Mode  = GPIO_Mode_Out_PP,
			},
	},
	.sclk = {
			.gpio = GPIOA,
			.init = {
					.GPIO_Pin   = GPIO_Pin_5,
					.GPIO_Speed = GPIO_Speed_10MHz,
					.GPIO_Mode  = GPIO_Mode_AF_PP,
			},
	},
	.miso = {
			.gpio = GPIOA,
			.init = {
					.GPIO_Pin   = GPIO_Pin_6,
					.GPIO_Speed = GPIO_Speed_10MHz,
					.GPIO_Mode  = GPIO_Mode_IN_FLOATING,
			},
	},
	.mosi = {
			.gpio = GPIOA,
			.init = {
					.GPIO_Pin   = GPIO_Pin_7,
					.GPIO_Speed = GPIO_Speed_10MHz,
					.GPIO_Mode  = GPIO_Mode_AF_PP,
			},
	},
};

static uint32_t pios_spi_accel_id;
void PIOS_SPI_accel_irq_handler(void)
{
  /* Call into the generic code to handle the IRQ for this specific device */
	PIOS_SPI_IRQ_Handler(pios_spi_accel_id);
}

#endif /* PIOS_INCLUDE_SPI */

#if defined(PIOS_INCLUDE_GPS)
#include <pios_usart_priv.h>

/*
 * GPS USART
 */
static const struct pios_usart_cfg pios_usart_gps_cfg = {
	.regs = USART1,
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
			 .NVIC_IRQChannel = USART1_IRQn,
			 .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			 .NVIC_IRQChannelSubPriority = 0,
			 .NVIC_IRQChannelCmd = ENABLE,
			 },
		},
	.rx = {
	       .gpio = GPIOA,
	       .init = {
			.GPIO_Pin = GPIO_Pin_10,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode = GPIO_Mode_IPU,
			},
	       },
	.tx = {
	       .gpio = GPIOA,
	       .init = {
			.GPIO_Pin = GPIO_Pin_9,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode = GPIO_Mode_AF_PP,
			},
	       },
};

#endif /* PIOS_INCLUDE_GPS */

#ifdef PIOS_COM_AUX
/*
 * AUX USART
 */
static const struct pios_usart_cfg pios_usart_aux_cfg = {
	.regs = USART4,
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
			 .NVIC_IRQChannel = USART4_IRQn,
			 .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGH,
			 .NVIC_IRQChannelSubPriority = 0,
			 .NVIC_IRQChannelCmd = ENABLE,
			 },
		},
	.rx = {
	       .gpio = GPIOB,
	       .init = {
			.GPIO_Pin = GPIO_Pin_11,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode = GPIO_Mode_IPU,
			},
	       },
	.tx = {
	       .gpio = GPIOB,
	       .init = {
			.GPIO_Pin = GPIO_Pin_10,
			.GPIO_Speed = GPIO_Speed_2MHz,
			.GPIO_Mode = GPIO_Mode_AF_PP,
			},
	       },
};

#endif /* PIOS_COM_AUX */


#if defined(PIOS_INCLUDE_COM)

#include <pios_com_priv.h>

#endif /* PIOS_INCLUDE_COM */

#if defined(PIOS_INCLUDE_I2C)

#include <pios_i2c_priv.h>

/*
 * I2C Adapters
 */

void PIOS_I2C_pres_mag_adapter_ev_irq_handler(void);
void PIOS_I2C_pres_mag_adapter_er_irq_handler(void);
void I2C1_EV_IRQHandler()
    __attribute__ ((alias("PIOS_I2C_pres_mag_adapter_ev_irq_handler")));
void I2C1_ER_IRQHandler()
    __attribute__ ((alias("PIOS_I2C_pres_mag_adapter_er_irq_handler")));

static const struct pios_i2c_adapter_cfg pios_i2c_pres_mag_adapter_cfg = {
	.regs = I2C1,
	.init = {
		 .I2C_Mode = I2C_Mode_I2C,
		 .I2C_OwnAddress1 = 0,
		 .I2C_Ack = I2C_Ack_Enable,
		 .I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit,
		 .I2C_DutyCycle = I2C_DutyCycle_2,
		 .I2C_ClockSpeed = 200000,	/* bits/s */
		 },
	.transfer_timeout_ms = 50,
	.scl = {
		.gpio = GPIOB,
		.init = {
			 .GPIO_Pin = GPIO_Pin_6,
			 .GPIO_Speed = GPIO_Speed_10MHz,
			 .GPIO_Mode = GPIO_Mode_AF_OD,
			 },
		},
	.sda = {
		.gpio = GPIOB,
		.init = {
			 .GPIO_Pin = GPIO_Pin_7,
			 .GPIO_Speed = GPIO_Speed_10MHz,
			 .GPIO_Mode = GPIO_Mode_AF_OD,
			 },
		},
	.event = {
		  .flags = 0,	/* FIXME: check this */
		  .init = {
			   .NVIC_IRQChannel = I2C1_EV_IRQn,
			   .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGHEST,
			   .NVIC_IRQChannelSubPriority = 0,
			   .NVIC_IRQChannelCmd = ENABLE,
			   },
		  },
	.error = {
		  .flags = 0,	/* FIXME: check this */
		  .init = {
			   .NVIC_IRQChannel = I2C1_ER_IRQn,
			   .NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_HIGHEST,
			   .NVIC_IRQChannelSubPriority = 0,
			   .NVIC_IRQChannelCmd = ENABLE,
			   },
		  },
};

uint32_t pios_i2c_pres_mag_adapter_id;
void PIOS_I2C_pres_mag_adapter_ev_irq_handler(void)
{
	/* Call into the generic code to handle the IRQ for this specific device */
	PIOS_I2C_EV_IRQ_Handler(pios_i2c_pres_mag_adapter_id);
}

void PIOS_I2C_pres_mag_adapter_er_irq_handler(void)
{
	/* Call into the generic code to handle the IRQ for this specific device */
	PIOS_I2C_ER_IRQ_Handler(pios_i2c_pres_mag_adapter_id);
}


void PIOS_I2C_gyro_adapter_ev_irq_handler(void);
void PIOS_I2C_gyro_adapter_er_irq_handler(void);
void I2C2_EV_IRQHandler() __attribute__ ((alias ("PIOS_I2C_gyro_adapter_ev_irq_handler")));
void I2C2_ER_IRQHandler() __attribute__ ((alias ("PIOS_I2C_gyro_adapter_er_irq_handler")));

static const struct pios_i2c_adapter_cfg pios_i2c_gyro_adapter_cfg = {
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

uint32_t pios_i2c_gyro_adapter_id;
void PIOS_I2C_gyro_adapter_ev_irq_handler(void)
{
	/* Call into the generic code to handle the IRQ for this specific device */
	PIOS_I2C_EV_IRQ_Handler(pios_i2c_gyro_adapter_id);
}

void PIOS_I2C_gyro_adapter_er_irq_handler(void)
{
  /* Call into the generic code to handle the IRQ for this specific device */
  PIOS_I2C_ER_IRQ_Handler(pios_i2c_gyro_adapter_id);
}

#endif /* PIOS_INCLUDE_I2C */
