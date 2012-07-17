/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_OVERO OVERO Functions
 * @brief PIOS interface to read and write to overo 
 * @{
 *
 * @file       pios_overo.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Hardware Abstraction Layer for Overo communications
 * @see        The GNU Public License (GPL) Version 3
 * @notes
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
 * Configures the SPI device to use a double buffered DMA for transferring
 * data.  At the end of each transfer (NSS goes high) it makes sure to reset
 * the DMA counter to the beginning of each packet and swap to the next
 * buffer
 */

#if defined(PIOS_INCLUDE_SPI)

#include <pios_overo.h>

static bool PIOS_OVERO_validate(struct pios_overo_dev * com_dev)
{
	/* Should check device magic here */
	return(true);
}

#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_overo_dev * PIOS_OVERO_alloc(void)
{
	return (malloc(sizeof(struct pios_overo_dev)));
}
#else
#error Unsupported
#endif

/**
* Initialises Overo pins
* \param[in] mode currently only mode 0 supported
* \return < 0 if initialisation failed
*/
int32_t PIOS_Overo_Init(uint32_t * overo_id, const struct pios_overo_cfg * cfg)
{
	uint32_t	init_ssel = 0;

	PIOS_Assert(overo_id);
	PIOS_Assert(cfg);

	struct pios_overo_dev * overo_dev;

	overo_dev = (struct pios_overo_dev *) PIOS_OVERO_alloc();
	if (!overo_dev) goto out_fail;

	/* Bind the configuration to the device instance */
	overo_dev->cfg = cfg;

#if defined(PIOS_INCLUDE_FREERTOS)
	vSemaphoreCreateBinary(overo_dev->busy);
	xSemaphoreGive(overo_dev->busy);
#endif

	/* Disable callback function */
	overo_dev->callback = NULL;

	/* Set rx/tx dummy bytes to a known value */
	overo_dev->rx_dummy_byte = 0xFF;
	overo_dev->tx_dummy_byte = 0xFF;

	/* only legal for single-slave config */
	PIOS_Assert(overo_dev->cfg->slave_count == 1);
	init_ssel = 1;
	SPI_SSOutputCmd(overo_dev->cfg->regs, (overo_dev->cfg->init.SPI_Mode == SPI_Mode_Master) ? ENABLE : DISABLE);

	/* Initialize the GPIO pins */
	/* note __builtin_ctz() due to the difference between GPIO_PinX and GPIO_PinSourceX */
	if (overo_dev->cfg->remap) {
		GPIO_PinAFConfig(overo_dev->cfg->sclk.gpio,
				__builtin_ctz(overo_dev->cfg->sclk.init.GPIO_Pin),
				overo_dev->cfg->remap);
		GPIO_PinAFConfig(overo_dev->cfg->mosi.gpio,
				__builtin_ctz(overo_dev->cfg->mosi.init.GPIO_Pin),
				overo_dev->cfg->remap);
		GPIO_PinAFConfig(overo_dev->cfg->miso.gpio,
				__builtin_ctz(overo_dev->cfg->miso.init.GPIO_Pin),
				overo_dev->cfg->remap);
		for (uint32_t i = 0; i < init_ssel; i++) {
			GPIO_PinAFConfig(overo_dev->cfg->ssel[i].gpio,
					__builtin_ctz(overo_dev->cfg->ssel[i].init.GPIO_Pin),
					overo_dev->cfg->remap);
		}
	}
	GPIO_Init(overo_dev->cfg->sclk.gpio, (GPIO_InitTypeDef*)&(overo_dev->cfg->sclk.init));
	GPIO_Init(overo_dev->cfg->mosi.gpio, (GPIO_InitTypeDef*)&(overo_dev->cfg->mosi.init));
	GPIO_Init(overo_dev->cfg->miso.gpio, (GPIO_InitTypeDef*)&(overo_dev->cfg->miso.init));

	/* Configure DMA for SPI Rx */
	DMA_DeInit(overo_dev->cfg->dma.rx.channel);
	DMA_Cmd(overo_dev->cfg->dma.rx.channel, DISABLE);
	DMA_Init(overo_dev->cfg->dma.rx.channel, (DMA_InitTypeDef*)&(overo_dev->cfg->dma.rx.init));

	/* Configure DMA for SPI Tx */
	DMA_DeInit(overo_dev->cfg->dma.tx.channel);
	DMA_Cmd(overo_dev->cfg->dma.tx.channel, DISABLE);
	DMA_Init(overo_dev->cfg->dma.tx.channel, (DMA_InitTypeDef*)&(overo_dev->cfg->dma.tx.init));

	/* Initialize the SPI block */
	SPI_DeInit(overo_dev->cfg->regs);
	SPI_Init(overo_dev->cfg->regs, (SPI_InitTypeDef*)&(overo_dev->cfg->init));

	/* Configure CRC calculation */
	if (overo_dev->cfg->use_crc) {
		SPI_CalculateCRC(overo_dev->cfg->regs, ENABLE);
	} else {
		SPI_CalculateCRC(overo_dev->cfg->regs, DISABLE);
	}

	/* Enable SPI */
	SPI_Cmd(overo_dev->cfg->regs, ENABLE);

	/* Enable SPI interrupts to DMA */
	SPI_I2S_DMACmd(overo_dev->cfg->regs, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, ENABLE);

	/* Must store this before enabling interrupt */
	*spi_id = (uint32_t)overo_dev;

	/* Configure DMA interrupt */
	NVIC_Init((NVIC_InitTypeDef*)&(overo_dev->cfg->dma.irq.init));

	return(0);

out_fail:
	return(-1);
}

/**
 * Claim the SPI bus semaphore.  Calling the SPI functions does not require this
 * \param[in] spi SPI number (0 or 1)
 * \return 0 if no error
 * \return -1 if timeout before claiming semaphore
 */
int32_t PIOS_OVERO_ClaimBus(uint32_t spi_id)
{
#if defined(PIOS_INCLUDE_FREERTOS)
	struct pios_overo_dev * overo_dev = (struct pios_overo_dev *)overo_id;

	bool valid = PIOS_OVERO_validate(overo_dev);
	PIOS_Assert(valid)

	if (xSemaphoreTake(overo_dev->busy, 0xffff) != pdTRUE)
		return -1;
#endif
	return 0;
}

/**
 * Claim the SPI bus semaphore from an ISR.  Has no timeout.
 * \param[in] spi SPI number (0 or 1)
 * \return 0 if no error
 * \return -1 if timeout before claiming semaphore
 */
int32_t PIOS_OVERO_ClaimBusISR(uint32_t overo_io)
{
#if defined(PIOS_INCLUDE_FREERTOS)
	struct pios_overo_dev * overo_dev = (struct pios_spi_dev *)spi_id;
	
	bool valid = PIOS_OVERO_validate(overo_dev);
	PIOS_Assert(valid)
	
	if (xQueueGenericReceive(( xQueueHandle ) overo_dev->busy, NULL, 0x0000 , pdFALSE ) != pdTRUE)
		return -1;
#endif
	return 0;
}


/**
 * Release the SPI bus semaphore.  Calling the SPI functions does not require this
 * \param[in] spi SPI number (0 or 1)
 * \return 0 if no error
 */
int32_t PIOS_OVERO_ReleaseBus(uint32_t overo_id)
{
#if defined(PIOS_INCLUDE_FREERTOS)
	struct pios_overo_dev * overo_dev = (struct pios_overo_dev *)overo_id;

	bool valid = PIOS_OVERO_validate(overo_dev);
	PIOS_Assert(valid)

	xSemaphoreGive(overo_dev->busy);
#endif
	return 0;
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
* \return -3 if function has been called during an ongoing DMA transfer
*/
static int32_t SPI_DMA_TransferBlock(uint32_t spi_id, const uint8_t *send_buffer, uint8_t *receive_buffer, uint16_t len, void *callback)
{
	struct pios_overo_dev * overo_dev = (struct pios_spi_dev *)spi_id;

	bool valid = PIOS_SPI_validate(overo_dev);
	PIOS_Assert(valid)

	DMA_InitTypeDef       dma_init;

	/* Exit if ongoing transfer */
	if (DMA_GetCurrDataCounter(overo_dev->cfg->dma.rx.channel)) {
		return -3;
	}

	/* Disable the DMA channels */
	DMA_Cmd(overo_dev->cfg->dma.rx.channel, DISABLE);
	DMA_Cmd(overo_dev->cfg->dma.tx.channel, DISABLE);

	while(DMA_GetCmdStatus(overo_dev->cfg->dma.rx.channel) == ENABLE);
	while(DMA_GetCmdStatus(overo_dev->cfg->dma.tx.channel) == ENABLE);

	/* Disable the SPI peripheral */
	/* Initialize the SPI block */
	SPI_DeInit(overo_dev->cfg->regs);
	SPI_Init(overo_dev->cfg->regs, (SPI_InitTypeDef*)&(overo_dev->cfg->init));
	SPI_Cmd(overo_dev->cfg->regs, DISABLE);
	/* Configure CRC calculation */
	if (overo_dev->cfg->use_crc) {
		SPI_CalculateCRC(overo_dev->cfg->regs, ENABLE);
	} else {
		SPI_CalculateCRC(overo_dev->cfg->regs, DISABLE);
	}
	
	/* Enable SPI interrupts to DMA */
	SPI_I2S_DMACmd(overo_dev->cfg->regs, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, ENABLE);

	/* Set callback function */
	overo_dev->callback = callback;

	/*
	 * Configure Rx channel
	 */

	/* Start with the default configuration for this peripheral */
	dma_init = overo_dev->cfg->dma.rx.init;
	DMA_DeInit(overo_dev->cfg->dma.rx.channel);
	if (receive_buffer != NULL) {
		/* Enable memory addr. increment - bytes written into receive buffer */
		dma_init.DMA_Memory0BaseAddr = (uint32_t) receive_buffer;
		dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
	} else {
		/* Disable memory addr. increment - bytes written into dummy buffer */
		overo_dev->rx_dummy_byte = 0xFF;
		dma_init.DMA_Memory0BaseAddr = (uint32_t) &overo_dev->rx_dummy_byte;
		dma_init.DMA_MemoryInc = DMA_MemoryInc_Disable;
	}
	if (overo_dev->cfg->use_crc) {
		/* Make sure the CRC error flag is cleared before we start */
		SPI_I2S_ClearFlag(overo_dev->cfg->regs, SPI_FLAG_CRCERR);
	}

	dma_init.DMA_BufferSize = len;
	DMA_Init(overo_dev->cfg->dma.rx.channel, &(dma_init));

	/*
	 * Configure Tx channel
	 */

	/* Start with the default configuration for this peripheral */
	dma_init = overo_dev->cfg->dma.tx.init;
	DMA_DeInit(overo_dev->cfg->dma.tx.channel);
	if (send_buffer != NULL) {
		/* Enable memory addr. increment - bytes written into receive buffer */
		dma_init.DMA_Memory0BaseAddr = (uint32_t) send_buffer;
		dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
	} else {
		/* Disable memory addr. increment - bytes written into dummy buffer */
		overo_dev->tx_dummy_byte = 0xFF;
		dma_init.DMA_Memory0BaseAddr = (uint32_t) &overo_dev->tx_dummy_byte;
		dma_init.DMA_MemoryInc = DMA_MemoryInc_Disable;
	}

	if (overo_dev->cfg->use_crc) {
		/* The last byte of the payload will be replaced with the CRC8 */
		dma_init.DMA_BufferSize = len - 1;
	} else {
		dma_init.DMA_BufferSize = len;
	}

	DMA_Init(overo_dev->cfg->dma.tx.channel, &(dma_init));

	/* Enable DMA interrupt if callback function active */
	DMA_ITConfig(overo_dev->cfg->dma.rx.channel, DMA_IT_TC, (callback != NULL) ? ENABLE : DISABLE);

	/* Flush out the CRC registers */
	SPI_CalculateCRC(overo_dev->cfg->regs, DISABLE);
	(void)SPI_GetCRC(overo_dev->cfg->regs, SPI_CRC_Rx);
	SPI_I2S_ClearFlag(overo_dev->cfg->regs, SPI_FLAG_CRCERR);

	/* Make sure to flush out the receive buffer */
	(void)SPI_I2S_ReceiveData(overo_dev->cfg->regs);

	if (overo_dev->cfg->use_crc) {
		/* Need a 0->1 transition to reset the CRC logic */
		SPI_CalculateCRC(overo_dev->cfg->regs, ENABLE);
	}

	/* Start DMA transfers */
	DMA_Cmd(overo_dev->cfg->dma.rx.channel, ENABLE);
	DMA_Cmd(overo_dev->cfg->dma.tx.channel, ENABLE);

	/* Reenable the SPI device */
	SPI_Cmd(overo_dev->cfg->regs, ENABLE);

	if (callback) {
		/* User has requested a callback, don't wait for the transfer to complete. */
		return 0;
	}

	/* Wait until all bytes have been transmitted/received */
	while (DMA_GetCurrDataCounter(overo_dev->cfg->dma.rx.channel));

	/* Wait for the final bytes of the transfer to complete, including CRC byte(s). */
	while (!(SPI_I2S_GetFlagStatus(overo_dev->cfg->regs, SPI_I2S_FLAG_TXE)));

	/* Wait for the final bytes of the transfer to complete, including CRC byte(s). */
	while (SPI_I2S_GetFlagStatus(overo_dev->cfg->regs, SPI_I2S_FLAG_BSY));

	/* Check the CRC on the transfer if enabled. */
	if (overo_dev->cfg->use_crc) {
		/* Check the SPI CRC error flag */
		if (SPI_I2S_GetFlagStatus(overo_dev->cfg->regs, SPI_FLAG_CRCERR)) {
			return -4;
		}
	}

	/* No error */
	return 0;
}


/**
* Transfers a block of bytes via PIO or DMA.
* \param[in] spi_id SPI device handle
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
* \return -3 if function has been called during an ongoing DMA transfer
*/
int32_t PIOS_SPI_TransferBlock(uint32_t spi_id, const uint8_t *send_buffer, uint8_t *receive_buffer, uint16_t len, void *callback)
{
	return SPI_PIO_TransferBlock(spi_id, send_buffer, receive_buffer, len);
}

/**
* Check if a transfer is in progress
* \param[in] spi SPI number (0 or 1)
* \return >= 0 if no transfer is in progress
* \return -1 if disabled SPI port selected
* \return -2 if unsupported SPI port selected
* \return -3 if function has been called during an ongoing DMA transfer
*/
int32_t PIOS_OVERO_Busy(uint32_t overo_id)
{
	struct pios_overo_dev * overo_dev = (struct pios_overo_dev *)overo_id;

	bool valid = PIOS_OVERO_validate(overo_dev);
	PIOS_Assert(valid)

	/* DMA buffer has data or SPI transmit register not empty or SPI is busy*/
	if (DMA_GetCurrDataCounter(overo_dev->cfg->dma.rx.channel) ||
		!SPI_I2S_GetFlagStatus(overo_dev->cfg->regs, SPI_I2S_FLAG_TXE) ||
		SPI_I2S_GetFlagStatus(overo_dev->cfg->regs, SPI_I2S_FLAG_BSY))
	{
		return -3;
	}

	return(0);
}

void PIOS_OVERO_IRQ_Handler(uint32_t spi_id)
{
	struct pios_overo_dev * overo_dev = (struct pios_overo_dev *)overo_id;

	bool valid = PIOS_OVERO_validate(overo_dev);
	PIOS_Assert(valid)
	
	// FIXME XXX Only RX channel or better clear flags for both channels?
	DMA_ClearFlag(overo_dev->cfg->dma.rx.channel, overo_dev->cfg->dma.irq.flags);
	
	if(overo_dev->cfg->init.SPI_Mode == SPI_Mode_Master) {
		/* Wait for the final bytes of the transfer to complete, including CRC byte(s). */
		while (!(SPI_I2S_GetFlagStatus(overo_dev->cfg->regs, SPI_I2S_FLAG_TXE))) ;

		/* Wait for the final bytes of the transfer to complete, including CRC byte(s). */
		while (SPI_I2S_GetFlagStatus(overo_dev->cfg->regs, SPI_I2S_FLAG_BSY)) ;
	}

	if (overo_dev->callback != NULL) {
		bool crc_ok = true;
		uint8_t crc_val;

		if (SPI_I2S_GetFlagStatus(overo_dev->cfg->regs, SPI_FLAG_CRCERR)) {
			crc_ok = false;
			SPI_I2S_ClearFlag(overo_dev->cfg->regs, SPI_FLAG_CRCERR);
		}
		crc_val = SPI_GetCRC(overo_dev->cfg->regs, SPI_CRC_Rx);
		overo_dev->callback(crc_ok, crc_val);
	}
}

#endif

/**
  * @}
  * @}
  */
