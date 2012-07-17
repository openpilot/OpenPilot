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

#define PACKET_SIZE 1024

static void PIOS_OVERO_NSS_IRQHandler();

static const struct pios_exti_cfg pios_exti_overo_cfg __exti_config = {
	.vector = PIOS_OVERO_NSS_IRQHandler,
	.line = EXTI_Line15,
	.pin = {
		.gpio = GPIOA,
		.init = {
			.GPIO_Pin = GPIO_Pin_15,
			.GPIO_Speed = GPIO_Speed_100MHz,
			.GPIO_Mode = GPIO_Mode_IN,
			.GPIO_OType = GPIO_OType_OD,
			.GPIO_PuPd = GPIO_PuPd_NOPULL,
		},
	},
	.irq = {
		.init = {
			.NVIC_IRQChannel = EXTI15_10_IRQn,
			.NVIC_IRQChannelPreemptionPriority = PIOS_IRQ_PRIO_MID,
			.NVIC_IRQChannelSubPriority = 0,
			.NVIC_IRQChannelCmd = ENABLE,
		},
	},
	.exti = {
		.init = {
			.EXTI_Line = EXTI_Line15, // matches above GPIO pin
			.EXTI_Mode = EXTI_Mode_Interrupt,
			.EXTI_Trigger = EXTI_Trigger_Rising,
			.EXTI_LineCmd = ENABLE,
		},
	},
};


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

//! Global variable
struct pios_overo_dev * overo_dev;

/**
* Initialises Overo pins
* \param[in] mode currently only mode 0 supported
* \return < 0 if initialisation failed
*/
int32_t PIOS_Overo_Init(const struct pios_overo_cfg * cfg)
{
	PIOS_Assert(cfg);

	overo_dev = (struct pios_overo_dev *) PIOS_OVERO_alloc();
	if (!overo_dev) goto out_fail;

	/* Bind the configuration to the device instance */
	overo_dev->cfg = cfg;

	/* Disable callback function */
	overo_dev->callback = NULL;

	/* Set a null buffer initially */
	overo_dev->new_tx_buffer = 0;
	overo_dev->new_rx_buffer = 0;

	/* only legal for single-slave config */
	PIOS_Assert(overo_dev->cfg->slave_count == 1);
	SPI_SSOutputCmd(overo_dev->cfg->regs, (overo_dev->cfg->init.SPI_Mode == SPI_Mode_Master) ? ENABLE : DISABLE);

	/* Initialize the GPIO pins */
	/* note __builtin_ctz() due to the difference between GPIO_PinX and GPIO_PinSourceX */
	GPIO_PinAFConfig(overo_dev->cfg->sclk.gpio,
					 __builtin_ctz(overo_dev->cfg->sclk.init.GPIO_Pin),
					 overo_dev->cfg->remap);
	GPIO_PinAFConfig(overo_dev->cfg->mosi.gpio,
					 __builtin_ctz(overo_dev->cfg->mosi.init.GPIO_Pin),
					 overo_dev->cfg->remap);
	GPIO_PinAFConfig(overo_dev->cfg->miso.gpio,
					 __builtin_ctz(overo_dev->cfg->miso.init.GPIO_Pin),
					 overo_dev->cfg->remap);
	GPIO_PinAFConfig(overo_dev->cfg->ssel[0].gpio,
					 __builtin_ctz(overo_dev->cfg->ssel[0].init.GPIO_Pin),
					 overo_dev->cfg->remap);

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

	/* Configure DMA interrupt */
	NVIC_Init((NVIC_InitTypeDef*)&(overo_dev->cfg->dma.irq.init));

	/* Configure the interrupt for rising edge of NSS */
	PIOS_EXTI_Init(&pios_exti_overo_cfg);

	return(0);

out_fail:
	return(-1);
}

/**
 * Transfers a block of bytes via DMA.
 * \param[in] overo_id SPI device handle
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
int32_t PIOS_Overo_SetNewBuffer(const uint8_t *send_buffer, uint8_t *receive_buffer, uint16_t len)
{
	bool valid = PIOS_OVERO_validate(overo_dev);
	PIOS_Assert(valid)

	bool overrun = overo_dev->new_tx_buffer || overo_dev->new_rx_buffer;
	/* Cache next buffer */
	overo_dev->new_tx_buffer = (uint32_t) send_buffer;
	overo_dev->new_rx_buffer = (uint32_t) receive_buffer;

	/* No error */
	return overrun ? -1 : 0;
}

/**
 * Set the callback function
 */
int32_t PIOS_Overo_SetCallback(void *callback)
{
	overo_dev->callback = callback;
	return 0;
}

/**
 * On the rising edge of NSS schedule a new transaction.  This cannot be
 * done by the DMA complete because there is 150 us between that and the
 * Overo deasserting the CS line.  We don't want to spin that long in an
 * isr.
 *
 * 1. Disable the DMA channel
 * 2. Check that the DMA counter is at the end of the buffer (increase an
 *    error counter if not)
 * 3. Reset the DMA counter to the end of the beginning of the buffer
 * 4. Swap the buffer
 * 5. Enable the DMA channel
 */
void PIOS_OVERO_NSS_IRQHandler()
{
	static uint32_t error_counter = 0;

	bool valid = PIOS_OVERO_validate(overo_dev);
	PIOS_Assert(valid)

	/* Disable the SPI peripheral */
	SPI_Cmd(overo_dev->cfg->regs, DISABLE);

	/* Disable the DMA commands */
	DMA_Cmd(overo_dev->cfg->dma.tx.channel, DISABLE);
	DMA_Cmd(overo_dev->cfg->dma.rx.channel, DISABLE);

	/* Check that the previous DMA transfer completed */
	if(DMA_GetCurrDataCounter(overo_dev->cfg->dma.tx.channel) || 
	   DMA_GetCurrDataCounter(overo_dev->cfg->dma.rx.channel))
		error_counter++;

	/* Disable and initialize the SPI peripheral */
	SPI_DeInit(overo_dev->cfg->regs);
	SPI_Init(overo_dev->cfg->regs, (SPI_InitTypeDef*)&(overo_dev->cfg->init));
	SPI_Cmd(overo_dev->cfg->regs, DISABLE);

	/* Enable SPI interrupts to DMA */
	SPI_I2S_DMACmd(overo_dev->cfg->regs, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, ENABLE);

	/* Reinit the DMA channels */
	DMA_InitTypeDef dma_init;

	DMA_DeInit(overo_dev->cfg->dma.rx.channel);
	dma_init = overo_dev->cfg->dma.rx.init;
	if (overo_dev->new_rx_buffer) {
		/* Enable memory addr. increment - bytes written into receive buffer */
		dma_init.DMA_Memory0BaseAddr = (uint32_t) overo_dev->new_rx_buffer;
		dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
		dma_init.DMA_BufferSize = PACKET_SIZE;
	}
	DMA_Init(overo_dev->cfg->dma.rx.channel, &(dma_init));

	DMA_DeInit(overo_dev->cfg->dma.tx.channel);
	dma_init = overo_dev->cfg->dma.tx.init;
	if (overo_dev->new_tx_buffer) {
		/* Enable memory addr. increment - bytes written into receive buffer */
		dma_init.DMA_Memory0BaseAddr = (uint32_t) overo_dev->new_tx_buffer;
		dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
		dma_init.DMA_BufferSize = PACKET_SIZE;
	}
	DMA_Init(overo_dev->cfg->dma.tx.channel, &(dma_init));
	
	/* Make sure to flush out the receive buffer */
	(void)SPI_I2S_ReceiveData(overo_dev->cfg->regs);
	
	/* Enable the DMA endpoints for valid buffers */
	if(overo_dev->new_rx_buffer)
		DMA_Cmd(overo_dev->cfg->dma.rx.channel, ENABLE);
	if(overo_dev->new_tx_buffer)
		DMA_Cmd(overo_dev->cfg->dma.tx.channel, ENABLE);

	/* Reenable the SPI peripheral */
	SPI_Cmd(overo_dev->cfg->regs, ENABLE);
	
	/* Indicate these buffers have been used */
	overo_dev->new_tx_buffer = 0;
	overo_dev->new_rx_buffer = 0;

	if (overo_dev->callback != NULL)
		overo_dev->callback(error_counter);
}

#endif

/**
  * @}
  * @}
  */
