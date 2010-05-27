/**
 ******************************************************************************
 *
 * @file       pios_spi.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	        Parts by Thorsten Klose (tk@midibox.org) (tk@midibox.org)
 * @brief      Hardware Abstraction Layer for SPI ports of STM32
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_SPI SPI Functions
 * @notes
 * J19 provides two RCLK (alias Chip Select) lines.<BR>
 * It's a software emulated SPI, therefore the selected spi_prescaler has no
 * effect! Bytes are transfered so fast as possible. The usage of
 * PIOS_SPI_PIN_DRIVER_STRONG is strongly recommended ;)<BR>
 * DMA transfers are not supported by the emulation, so that
 * PIOS_SPI_TransferBlock() will consume CPU time (but the callback handling does work).
 *
 * Note that additional chip select lines can be easily added by using
 * the remaining free GPIOs of the core module. Shared SPI ports should be
 * arbitrated with (FreeRTOS based) Mutexes to avoid collisions!
 * @{
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


/* Local variables */
static void (*spi_callback[PIOS_SPI_NUM])(void);
static uint8_t tx_dummy_byte;
static uint8_t rx_dummy_byte;


/**
* Initialises SPI pins
* \param[in] mode currently only mode 0 supported
* \return < 0 if initialisation failed
*/
int32_t PIOS_SPI_Init(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	DMA_StructInit(&DMA_InitStructure);
	NVIC_InitTypeDef NVIC_InitStructure;

#if (PIOS_SPI0_ENABLED)
	/* SPI0 */
	/* Disable callback function */
	spi_callback[0] = NULL;

	/* Set RC pin(s) to 1 */
	PIOS_SPI_RC_PinSet(0, 1); // spi, rc_pin, pin_value

	/* IO configuration */
	PIOS_SPI_IO_Init(0, PIOS_SPI_PIN_DRIVER_WEAK_OD);

	/* Enable SPI peripheral clock (APB2 == high speed) */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	/* Enable DMA1 clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	/* DMA Configuration for SPI Rx Event */
	DMA_Cmd(PIOS_SPI0_DMA_RX_PTR, DISABLE);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&PIOS_SPI0_PTR->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = 0; /* Will be configured later */
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 0; /* Will be configured later */
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(PIOS_SPI0_DMA_RX_PTR, &DMA_InitStructure);

	/* DMA Configuration for SPI Tx Event */
	/* (partly re-using previous DMA setup) */
	DMA_Cmd(PIOS_SPI0_DMA_TX_PTR, DISABLE);
	DMA_InitStructure.DMA_MemoryBaseAddr = 0; /* Will be configured later */
	DMA_InitStructure.DMA_BufferSize = 0; /* Will be configured later */
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_Init(PIOS_SPI0_DMA_TX_PTR, &DMA_InitStructure);

	/* Initial SPI peripheral configuration */
	PIOS_SPI_TransferModeInit(0, PIOS_SPI_MODE_CLK1_PHASE1, PIOS_SPI_PRESCALER_128);

	/* Enable SPI */
	SPI_Cmd(PIOS_SPI0_PTR, ENABLE);

	/* Enable SPI interrupts to DMA */
	SPI_I2S_DMACmd(PIOS_SPI0_PTR, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, ENABLE);

	/* Configure DMA interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = PIOS_SPI0_DMA_IRQ_CHANNEL;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_SPI_IRQ_DMA_PRIORITY;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

#if (PIOS_SPI1_ENABLED)
	/* SPI1 */
	/* Disable callback function */
	spi_callback[1] = NULL;

	/* Set RC pin(s) to 1 */
	PIOS_SPI_RC_PinSet(1, 1); /* spi, pin_value */

	/* IO configuration */
	PIOS_SPI_IO_Init(1, PIOS_SPI_PIN_DRIVER_WEAK_OD);

	/* Enable SPI peripheral clock (APB1 == slow speed) */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	/* Enable DMA1 clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	/* DMA Configuration for SPI Rx Event */
	DMA_Cmd(PIOS_SPI1_DMA_RX_PTR, DISABLE);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&PIOS_SPI1_PTR->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = 0; // will be configured later
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = 0; // will be configured later
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(PIOS_SPI1_DMA_RX_PTR, &DMA_InitStructure);

	/* DMA Configuration for SPI Tx Event */
	/* (partly re-using previous DMA setup) */
	DMA_Cmd(PIOS_SPI1_DMA_TX_PTR, DISABLE);
	DMA_InitStructure.DMA_MemoryBaseAddr = 0; // will be configured later
	DMA_InitStructure.DMA_BufferSize = 0; // will be configured later
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_Init(PIOS_SPI1_DMA_TX_PTR, &DMA_InitStructure);

	/* Initial SPI peripheral configuration */
	PIOS_SPI_TransferModeInit(1, PIOS_SPI_MODE_CLK1_PHASE1, PIOS_SPI_PRESCALER_128);

	/* Enable SPI */
	SPI_Cmd(PIOS_SPI1_PTR, ENABLE);

	/* Enable SPI interrupts to DMA */
	SPI_I2S_DMACmd(PIOS_SPI1_PTR, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, ENABLE);

	/* Configure DMA interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = PIOS_SPI1_DMA_IRQ_CHANNEL;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PIOS_SPI_IRQ_DMA_PRIORITY; /* defined in PIOS_irq.h */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif

	/* No error */
	return 0;
}


/**
* (Re-)initializes SPI IO Pins
* By default, all output pins are configured with weak open drain drivers for 2 MHz
* \param[in] spi SPI number (0, 1 or 2)
* \param[in] spi_pin_driver configures the driver strength:
* <UL>
*   <LI>PIOS_SPI_PIN_DRIVER_STRONG: Configures outputs for up to 50 MHz
*   <LI>PIOS_SPI_PIN_DRIVER_STRONG_OD: Configures outputs as open drain
*       for up to 50 MHz (allows voltage shifting via pull-resistors)
*   <LI>PIOS_SPI_PIN_DRIVER_WEAK: Configures outputs for up to 2 MHz (better EMC)
*   <LI>PIOS_SPI_PIN_DRIVER_WEAK_OD: Configures outputs as open drain for
*       up to 2 MHz (allows voltage shifting via pull-resistors)
* </UL>
* \return 0 if no error
* \return -1 if disabled SPI port selected
* \return -2 if unsupported SPI port selected
* \return -3 if unsupported pin driver mode
*/
int32_t PIOS_SPI_IO_Init(uint8_t spi, SPIPinDriverTypeDef spi_pin_driver)
{
	/* Init GPIO structure */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);

	/* Select pin driver and output mode */
	uint32_t af_mode;
	uint32_t gp_mode;

	switch(spi_pin_driver) {
		case PIOS_SPI_PIN_DRIVER_STRONG:
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			af_mode = GPIO_Mode_AF_PP;
			gp_mode = GPIO_Mode_Out_PP;
			break;

		case PIOS_SPI_PIN_DRIVER_STRONG_OD:
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			af_mode = GPIO_Mode_AF_OD;
			gp_mode = GPIO_Mode_Out_OD;
			break;

		case PIOS_SPI_PIN_DRIVER_WEAK:
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
			af_mode = GPIO_Mode_AF_PP;
			gp_mode = GPIO_Mode_Out_PP;
			break;

		case PIOS_SPI_PIN_DRIVER_WEAK_OD:
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
			af_mode = GPIO_Mode_AF_OD;
			gp_mode = GPIO_Mode_Out_OD;
			break;

		default:
			 /* Unsupported pin driver mode */
			return -3;
	}

	switch(spi) {
		case 0:
			/* SCLK and DOUT are outputs assigned to alternate functions */
			GPIO_InitStructure.GPIO_Mode = af_mode;
			GPIO_InitStructure.GPIO_Pin  = PIOS_SPI0_SCLK_PIN;
			GPIO_Init(PIOS_SPI0_SCLK_PORT, &GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Pin  = PIOS_SPI0_MOSI_PIN;
			GPIO_Init(PIOS_SPI0_MOSI_PORT, &GPIO_InitStructure);

			/* RCLK outputs assigned to GPIO */
			GPIO_InitStructure.GPIO_Mode = gp_mode;
			GPIO_InitStructure.GPIO_Pin  = PIOS_SPI0_RCLK1_PIN;
			GPIO_Init(PIOS_SPI0_RCLK1_PORT, &GPIO_InitStructure);

			/* DIN is input with pull-up */
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
			GPIO_InitStructure.GPIO_Pin  = PIOS_SPI0_MISO_PIN;
			GPIO_Init(PIOS_SPI0_MISO_PORT, &GPIO_InitStructure);
			break;

		case 1:
			/* SCLK and DOUT are outputs assigned to alternate functions */
			GPIO_InitStructure.GPIO_Mode = af_mode;
			GPIO_InitStructure.GPIO_Pin  = PIOS_SPI1_SCLK_PIN;
			GPIO_Init(PIOS_SPI1_SCLK_PORT, &GPIO_InitStructure);
			GPIO_InitStructure.GPIO_Pin  = PIOS_SPI1_MOSI_PIN;
			GPIO_Init(PIOS_SPI1_MOSI_PORT, &GPIO_InitStructure);

			/* RCLK outputs assigned to GPIO */
			GPIO_InitStructure.GPIO_Mode = gp_mode;
			GPIO_InitStructure.GPIO_Pin  = PIOS_SPI1_RCLK1_PIN;
			GPIO_Init(PIOS_SPI1_RCLK1_PORT, &GPIO_InitStructure);

			/* DIN is input with pull-up */
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
			GPIO_InitStructure.GPIO_Pin  = PIOS_SPI1_MISO_PIN;
			GPIO_Init(PIOS_SPI1_MISO_PORT, &GPIO_InitStructure);
			break;

		default:
			/* Unsupported SPI port */
			return -2;
	}

	/* No error */
	return 0;
}


/**
* (Re-)initialises SPI peripheral transfer mode
* By default, all SPI peripherals are configured with
* PIOS_SPI_MODE_CLK1_PHASE1 and PIOS_SPI_PRESCALER_128
*
* \param[in] spi SPI number (0 or 1)
* \param[in] spi_mode configures clock and capture phase:
* <UL>
*   <LI>PIOS_SPI_MODE_CLK0_PHASE0: Idle level of clock is 0, data captured at rising edge
*   <LI>PIOS_SPI_MODE_CLK0_PHASE1: Idle level of clock is 0, data captured at falling edge
*   <LI>PIOS_SPI_MODE_CLK1_PHASE0: Idle level of clock is 1, data captured at falling edge
*   <LI>PIOS_SPI_MODE_CLK1_PHASE1: Idle level of clock is 1, data captured at rising edge
* </UL>
* \param[in] spi_prescaler configures the SPI speed:
* <UL>
*   <LI>PIOS_SPI_PRESCALER_2: sets clock rate 27.7~ nS @ 72 MHz (36 MBit/s) (only supported for spi==0, spi1 uses 4 instead)
*   <LI>PIOS_SPI_PRESCALER_4: sets clock rate 55.5~ nS @ 72 MHz (18 MBit/s)
*   <LI>PIOS_SPI_PRESCALER_8: sets clock rate 111.1~ nS @ 72 MHz (9 MBit/s)
*   <LI>PIOS_SPI_PRESCALER_16: sets clock rate 222.2~ nS @ 72 MHz (4.5 MBit/s)
*   <LI>PIOS_SPI_PRESCALER_32: sets clock rate 444.4~ nS @ 72 MHz (2.25 MBit/s)
*   <LI>PIOS_SPI_PRESCALER_64: sets clock rate 888.8~ nS @ 72 MHz (1.125 MBit/s)
*   <LI>PIOS_SPI_PRESCALER_128: sets clock rate 1.7~ nS @ 72 MHz (0.562 MBit/s)
*   <LI>PIOS_SPI_PRESCALER_256: sets clock rate 3.5~ nS @ 72 MHz (0.281 MBit/s)
* </UL>
* \return 0 if no error
* \return -1 if disabled SPI port selected
* \return -2 if unsupported SPI port selected
* \return -3 if invalid spi_prescaler selected
* \return -4 if invalid spi_mode selected
*/
int32_t PIOS_SPI_TransferModeInit(uint8_t spi, SPIModeTypeDef spi_mode, SPIPrescalerTypeDef spi_prescaler)
{
	/* SPI configuration */
	SPI_InitTypeDef SPI_InitStructure;
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	switch(spi_mode) {
		case PIOS_SPI_MODE_CLK0_PHASE0:
			SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
			SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
			break;
		case PIOS_SPI_MODE_CLK0_PHASE1:
			SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
			SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
			break;
		case PIOS_SPI_MODE_CLK1_PHASE0:
			SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
			SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
			break;
		case PIOS_SPI_MODE_CLK1_PHASE1:
			SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
			SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
			break;
		default:
			/* Invalid SPI clock/phase mode */
			return -4;
	}

	if(spi_prescaler >= 8) {
		/* Invalid prescaler selected */
		return -3;
	}

	switch(spi) {
		case 0: {
			uint16_t prev_cr1 = PIOS_SPI0_PTR->CR1;
			/* SPI1 perpipheral is located in APB2 domain and clocked at full speed */
			/* therefore we have to add +1 to the prescaler */
			SPI_InitStructure.SPI_BaudRatePrescaler = ((uint16_t)spi_prescaler&7) << 3;
			SPI_Init(PIOS_SPI0_PTR, &SPI_InitStructure);

			if((prev_cr1 ^ PIOS_SPI0_PTR->CR1) & 3) {
				/* CPOL and CPHA located at bit #1 and #0 */
				/* clock configuration has been changed - we should send a dummy byte */
				/* before the application activates chip select. */
				/* this solves a dependency between SDCard and ENC28J60 driver */
				PIOS_SPI_TransferByte(spi, 0xff);
			}

			} break;

		case 1: {
			uint16_t prev_cr1 = PIOS_SPI1_PTR->CR1;

			/* SPI2 perpipheral is located in APB1 domain and clocked at half speed */
			if(spi_prescaler == 0) {
				SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
			} else {
				SPI_InitStructure.SPI_BaudRatePrescaler = (((uint16_t)spi_prescaler&7)-1) << 3;
			}
			SPI_Init(PIOS_SPI1_PTR, &SPI_InitStructure);

			if((prev_cr1 ^ PIOS_SPI1_PTR->CR1) & 3) {
				/* CPOL and CPHA located at bit #1 and #0 */
				/* clock configuration has been changed - we should send a dummy byte */
				/* before the application activates chip select. */
				/* this solves a dependency between SDCard and ENC28J60 driver */
				PIOS_SPI_TransferByte(spi, 0xff);
			}
			} break;

		default:
			/* Unsupported SPI port */
			return -2;
	}

	/* No error */
	return 0;
}


/**
* Controls the RC (Register Clock alias Chip Select) pin of a SPI port
* \param[in] spi SPI number (0 or 1)
* \param[in] pin_value 0 or 1
* \return 0 if no error
* \return -1 if disabled SPI port selected
* \return -2 if unsupported SPI port selected
*/
int32_t PIOS_SPI_RC_PinSet(uint8_t spi, uint8_t pin_value)
{
	switch(spi) {
		case 0:
			if(pin_value) {
				PIOS_SPI0_RCLK1_PORT->BSRR = PIOS_SPI0_RCLK1_PIN;
			} else {
				PIOS_SPI0_RCLK1_PORT->BRR = PIOS_SPI0_RCLK1_PIN;
			}
			break;

		case 1:
			if(pin_value) {
				PIOS_SPI1_RCLK1_PORT->BSRR = PIOS_SPI1_RCLK1_PIN;
			} else {
				PIOS_SPI1_RCLK1_PORT->BRR  = PIOS_SPI1_RCLK1_PIN;
			}

			break;

		default:
			/* Unsupported SPI port */
			return -2;
	}

	/* No error */
	return 0;
}


/**
* Transfers a byte to SPI output and reads back the return value from SPI input
* \param[in] spi SPI number (0 or 1)
* \param[in] b the byte which should be transfered
* \return >= 0: the read byte
* \return -1 if disabled SPI port selected
* \return -2 if unsupported SPI port selected
* \return -3 if unsupported SPI mode configured via PIOS_SPI_TransferModeInit()
*/
int32_t PIOS_SPI_TransferByte(uint8_t spi, uint8_t b)
{
	SPI_TypeDef *spi_ptr;

	switch(spi) {
		case 0:
			spi_ptr = PIOS_SPI0_PTR;
			break;

		case 1:
			spi_ptr = PIOS_SPI1_PTR;
			break;

		default:
			/* Unsupported SPI port */
			return -2;
	}

	/* Send byte */
	spi_ptr->DR = b;

	if(spi_ptr->SR); /* Dummy read due to undocumented pipelining issue :-/ */
	/* TK: without this read (which can be done to any bus location) we could sporadically */
	/* get the status byte at the moment where DR is written. Accordingly, the busy bit */
	/* will be 0. */
	/* you won't see this dummy read in STM drivers, as they never have a DR write */
	/* followed by SR read, or as they are using SPI1/SPI2 pointers, which results into */
	/* some additional CPU instructions between strh/ldrh accesses. */
	/* We use a bus access instead of NOPs to avoid any risk for back-to-back transactions */
	/* over AHB (if SPI1/SPI2 pointers are used, there is still a risk for such a scenario, */
	/* e.g. if DMA loads the bus!) */

	/* Wait until SPI transfer finished */
	while(spi_ptr->SR & SPI_I2S_FLAG_BSY);

	/* Return received byte */
	return spi_ptr->DR;
}


/**
* Transfers a block of bytes via DMA.
* \param[in] spi SPI number (0 or 1)
* \param[in] send_buffer pointer to buffer which should be sent.<BR>
* If NULL, 0xff (all-one) will be sent.
* \param[in] receive_buffer pointer to buffer which should get the received values.<BR>
* If NULL, received bytes will be discarded.
* \param[in] len number of bytes which should be transfered
* \param[in] callback pointer to callback function which will be executed
* from DMA channel interrupt once the transfer is finished.
* If NULL, no callback function will be used, and PIOS_SPI_TransferBlock() will
* block until the transfer is finished.
* \return >= 0 if no error during transfer
* \return -1 if disabled SPI port selected
* \return -2 if unsupported SPI port selected
* \return -3 if function has been called during an ongoing DMA transfer
*/
int32_t PIOS_SPI_TransferBlock(uint8_t spi, uint8_t *send_buffer, uint8_t *receive_buffer, uint16_t len, void *callback)
{
	DMA_Channel_TypeDef *dma_tx_ptr, *dma_rx_ptr;

	switch(spi) {
		case 0:
			dma_tx_ptr = PIOS_SPI0_DMA_TX_PTR;
			dma_rx_ptr = PIOS_SPI0_DMA_RX_PTR;
			break;

		case 1:
			dma_tx_ptr = PIOS_SPI1_DMA_TX_PTR;
			dma_rx_ptr = PIOS_SPI1_DMA_RX_PTR;
			break;

		default:
			/* Unsupported SPI port */
			return -2;
	}

	/* Exit if ongoing transfer */
	if(dma_rx_ptr->CNDTR) {
		return -3;
	}

	/* Set callback function */
	spi_callback[spi] = callback;

	/* Configure Rx channel */
	DMA_Cmd(dma_rx_ptr, DISABLE);
	if(receive_buffer != NULL) {
		/* Enable memory addr. increment - bytes written into receive buffer */
		dma_rx_ptr->CMAR = (uint32_t)receive_buffer;
		dma_rx_ptr->CCR |= DMA_MemoryInc_Enable;
	} else {
		/* Disable memory addr. increment - bytes written into dummy buffer */
		rx_dummy_byte = 0xff;
		dma_rx_ptr->CMAR = (uint32_t)&rx_dummy_byte;
		dma_rx_ptr->CCR &= ~DMA_MemoryInc_Enable;
	}
	dma_rx_ptr->CNDTR = len;
	DMA_Cmd(dma_rx_ptr, ENABLE);

	/* Configure Tx channel */
	DMA_Cmd(dma_tx_ptr, DISABLE);
	if(send_buffer != NULL) {
		/* Enable memory addr. increment - bytes read from send buffer */
		dma_tx_ptr->CMAR = (uint32_t)send_buffer;
		dma_tx_ptr->CCR |= DMA_MemoryInc_Enable;
	} else {
		/* Disable memory addr. increment - bytes read from dummy buffer */
		tx_dummy_byte = 0xff;
		dma_tx_ptr->CMAR = (uint32_t)&tx_dummy_byte;
		dma_tx_ptr->CCR &= ~DMA_MemoryInc_Enable;
	}
	dma_tx_ptr->CNDTR = len;

	/* Enable DMA interrupt if callback function active */
	DMA_ITConfig(dma_rx_ptr, DMA_IT_TC, (callback != NULL) ? ENABLE : DISABLE);

	/* Start DMA transfer */
	DMA_Cmd(dma_tx_ptr, ENABLE);

	/* Wait until all bytes have been transmitted/received */
	while(dma_rx_ptr->CNDTR);

	/* No error */
	return 0;
}


/**
* Called when callback function has been defined and SPI transfer has finished
*/
PIOS_SPI0_DMA_IRQHANDLER_FUNC
{
	DMA_ClearFlag(PIOS_SPI0_DMA_RX_IRQ_FLAGS);

	if(spi_callback[0] != NULL) {
		spi_callback[0]();
	}
}

PIOS_SPI1_DMA_IRQHANDLER_FUNC
{
	DMA_ClearFlag(PIOS_SPI1_DMA_RX_IRQ_FLAGS);

	if(spi_callback[1] != NULL) {
		spi_callback[1]();
	}
}

#endif
