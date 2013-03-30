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

#ifdef PIOS_INCLUDE_OVERO

/**
 * Configures the SPI device to use a double buffered DMA for transferring
 * data.  At the end of each transfer (NSS goes high) it makes sure to reset
 * the DMA counter to the beginning of each packet and swap to the next
 * buffer
 */

#ifdef PIOS_INCLUDE_SPI

#include <pios_overo_priv.h>

#define PACKET_SIZE 1024

/* Provide a COM driver */
static void PIOS_OVERO_RegisterRxCallback(uint32_t overo_id, pios_com_callback rx_in_cb, uint32_t context);
static void PIOS_OVERO_RegisterTxCallback(uint32_t overo_id, pios_com_callback tx_out_cb, uint32_t context);
static void PIOS_OVERO_TxStart(uint32_t overo_id, uint16_t tx_bytes_avail);
static void PIOS_OVERO_RxStart(uint32_t overo_id, uint16_t rx_bytes_avail);

const struct pios_com_driver pios_overo_com_driver = {
	.set_baud   = NULL,
	.tx_start   = PIOS_OVERO_TxStart,
	.rx_start   = PIOS_OVERO_RxStart,
	.bind_tx_cb = PIOS_OVERO_RegisterTxCallback,
	.bind_rx_cb = PIOS_OVERO_RegisterRxCallback,
};

//! Data types
enum pios_overo_dev_magic {
	PIOS_OVERO_DEV_MAGIC = 0x85A3834A,
};

struct pios_overo_dev {
	enum pios_overo_dev_magic magic;
	const struct pios_overo_cfg * cfg;
	
	int8_t writing_buffer;
	uint32_t writing_offset;
	
	uint32_t packets;
	
	uint8_t tx_buffer[2][PACKET_SIZE];
	uint8_t rx_buffer[2][PACKET_SIZE];

	pios_com_callback rx_in_cb;
	uint32_t rx_in_context;
	pios_com_callback tx_out_cb;
	uint32_t tx_out_context;
};

#if defined(PIOS_INCLUDE_FREERTOS)
//! Private methods
static void PIOS_OVERO_WriteData(struct pios_overo_dev *overo_dev);
static bool PIOS_OVERO_validate(struct pios_overo_dev * overo_dev);
static struct pios_overo_dev * PIOS_OVERO_alloc(void);

static bool PIOS_OVERO_validate(struct pios_overo_dev * overo_dev)
{
	return (overo_dev->magic == PIOS_OVERO_DEV_MAGIC);
}

static struct pios_overo_dev * PIOS_OVERO_alloc(void)
{
	struct pios_overo_dev * overo_dev;
	
	overo_dev = (struct pios_overo_dev *)pvPortMalloc(sizeof(*overo_dev));
	if (!overo_dev) return(NULL);
	
	overo_dev->rx_in_cb = 0;
	overo_dev->rx_in_context = 0;
	overo_dev->tx_out_cb = 0;
	overo_dev->tx_out_context = 0;
	overo_dev->packets = 0;
	overo_dev->magic = PIOS_OVERO_DEV_MAGIC;
	return(overo_dev);
}

/**
 * Take data from the PIOS_COM buffer and transfer it to the currently inactive DMA
 * circular buffer
 */
static void PIOS_OVERO_WriteData(struct pios_overo_dev *overo_dev)
{
	// TODO: How do we protect against the DMA buffer swapping midway through adding data
	// to this buffer.  If we were writing at the beginning it could cause a weird race.
	if (overo_dev->tx_out_cb) {

		int32_t max_bytes = PACKET_SIZE - overo_dev->writing_offset;

		if (max_bytes > 0) {
			uint16_t bytes_added;
			bool tx_need_yield = false;
			uint8_t *writing_pointer = &overo_dev->tx_buffer[overo_dev->writing_buffer][overo_dev->writing_offset];

			bytes_added = (overo_dev->tx_out_cb)(overo_dev->tx_out_context, writing_pointer, max_bytes, NULL, &tx_need_yield);

#if defined(OVERO_USES_BLOCKING_WRITE)
			if (tx_need_yield) {
				vPortYieldFromISR();
			}
#endif
			overo_dev->writing_offset += bytes_added;
		}
	}
}

/**
 * Called at the end of each DMA transaction.  Refresh the flag indicating which
 * DMA buffer to write new data from the PIOS_COM fifo into the buffer
 */
void PIOS_OVERO_DMA_irq_handler(uint32_t overo_id)
{
	struct pios_overo_dev * overo_dev = (struct pios_overo_dev *) overo_id;
	if(!PIOS_OVERO_validate(overo_dev))
		return;
	
	DMA_ClearFlag(overo_dev->cfg->dma.tx.channel, overo_dev->cfg->dma.irq.flags);

	overo_dev->writing_buffer = 1 - DMA_GetCurrentMemoryTarget(overo_dev->cfg->dma.tx.channel);
	overo_dev->writing_offset = 0;

/*	bool rx_need_yield;
	// Get data from the Rx buffer and add to the fifo
	(void) (overo_dev->rx_in_cb)(overo_dev->rx_in_context, 
								 &overo_dev->rx_buffer[overo_dev->writing_buffer][0], 
								PACKET_SIZE, NULL, &rx_need_yield);

	if(rx_need_yield) {
		vPortYieldFromISR();
	}

	// Fill the buffer with known value to prevent rereading these bytes
	memset(&overo_dev->rx_buffer[overo_dev->writing_buffer][0], 0xFF, PACKET_SIZE);
*/
	// Fill the buffer with known value to prevent resending any bytes
	memset(&overo_dev->tx_buffer[overo_dev->writing_buffer][0], 0xFF, PACKET_SIZE);
	
	// Load any pending bytes from TX fifo
	PIOS_OVERO_WriteData(overo_dev);

	overo_dev->packets++;
}

/**
 * Debugging information to check how it is runnign
 */
int32_t PIOS_OVERO_GetPacketCount(uint32_t overo_id)
{
	struct pios_overo_dev * overo_dev = (struct pios_overo_dev *) overo_id;
	PIOS_Assert(PIOS_OVERO_validate(overo_dev));

	return overo_dev->packets;
}

/**
 * Debugging information to check how it is runnign
 */
int32_t PIOS_OVERO_GetWrittenBytes(uint32_t overo_id)
{
	struct pios_overo_dev * overo_dev = (struct pios_overo_dev *) overo_id;
	PIOS_Assert(PIOS_OVERO_validate(overo_dev));
	
	return overo_dev->writing_offset;
}

/**
 * Initialise a single Overo device
 */
int32_t PIOS_OVERO_Init(uint32_t * overo_id, const struct pios_overo_cfg * cfg)
{
	PIOS_DEBUG_Assert(overo_id);
	PIOS_DEBUG_Assert(cfg);
	
	struct pios_overo_dev *overo_dev;
	
	overo_dev = (struct pios_overo_dev *) PIOS_OVERO_alloc();
	if (!overo_dev) goto out_fail;
	
	/* Bind the configuration to the device instance */
	overo_dev->cfg = cfg;
	overo_dev->writing_buffer = 1; // First writes to second buffer

	/* Put buffers to a known state */
	memset(&overo_dev->tx_buffer[0][0], 0xFF, PACKET_SIZE);
	memset(&overo_dev->tx_buffer[1][0], 0xFF, PACKET_SIZE);
	memset(&overo_dev->rx_buffer[0][0], 0xFF, PACKET_SIZE);
	memset(&overo_dev->rx_buffer[1][0], 0xFF, PACKET_SIZE);

	/*
	 * Enable the SPI device
	 *
	 * 1. Enable the SPI port 
	 * 2. Enable DMA with circular buffered DMA (validate config)
	 * 3. Enable the DMA Tx IRQ
	 */
	
	//PIOS_Assert(overo_dev->cfg->dma.tx-> == CIRCULAR);
	//PIOS_Assert(overo_dev->cfg->dma.rx-> == CIRCULAR);
	
	/* only legal for single-slave config */
	PIOS_Assert(overo_dev->cfg->slave_count == 1);
	SPI_SSOutputCmd(overo_dev->cfg->regs, DISABLE);
	
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
	
	/* Configure circular buffer targets. Configure 0 to be initially active */
	DMA_InitTypeDef dma_init;

	DMA_DeInit(overo_dev->cfg->dma.rx.channel);
	dma_init = overo_dev->cfg->dma.rx.init;
	dma_init.DMA_Memory0BaseAddr =  (uint32_t) overo_dev->rx_buffer[0];
	dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma_init.DMA_BufferSize = PACKET_SIZE;
	DMA_Init(overo_dev->cfg->dma.rx.channel, &dma_init);
	DMA_DoubleBufferModeConfig(overo_dev->cfg->dma.rx.channel, (uint32_t) overo_dev->rx_buffer[1], DMA_Memory_0);
	DMA_DoubleBufferModeCmd(overo_dev->cfg->dma.rx.channel, ENABLE);
	
	DMA_DeInit(overo_dev->cfg->dma.tx.channel);
	dma_init = overo_dev->cfg->dma.tx.init;
	dma_init.DMA_Memory0BaseAddr =  (uint32_t) overo_dev->tx_buffer[0];
	dma_init.DMA_MemoryInc = DMA_MemoryInc_Enable;
	dma_init.DMA_BufferSize = PACKET_SIZE;
	DMA_Init(overo_dev->cfg->dma.tx.channel, &dma_init);
	DMA_DoubleBufferModeConfig(overo_dev->cfg->dma.tx.channel, (uint32_t) overo_dev->tx_buffer[1], DMA_Memory_0);
	DMA_DoubleBufferModeCmd(overo_dev->cfg->dma.tx.channel, ENABLE);

	/* Set the packet size */
	DMA_SetCurrDataCounter(overo_dev->cfg->dma.rx.channel, PACKET_SIZE);
	DMA_SetCurrDataCounter(overo_dev->cfg->dma.tx.channel, PACKET_SIZE);

	/* Initialize the SPI block */
	SPI_DeInit(overo_dev->cfg->regs);
	SPI_Init(overo_dev->cfg->regs, (SPI_InitTypeDef*)&(overo_dev->cfg->init));
	
	SPI_CalculateCRC(overo_dev->cfg->regs, DISABLE);
	
	/* Enable SPI */
	SPI_Cmd(overo_dev->cfg->regs, ENABLE);
	
	/* Enable SPI interrupts to DMA */
	SPI_I2S_DMACmd(overo_dev->cfg->regs, SPI_I2S_DMAReq_Tx | SPI_I2S_DMAReq_Rx, ENABLE);

	/* Configure DMA interrupt */
	NVIC_Init((NVIC_InitTypeDef*)&(overo_dev->cfg->dma.irq.init));
	DMA_ITConfig(overo_dev->cfg->dma.tx.channel, DMA_IT_TC, ENABLE);

	/* Enable the DMA channels */
	DMA_Cmd(overo_dev->cfg->dma.tx.channel, ENABLE);
	DMA_Cmd(overo_dev->cfg->dma.rx.channel, ENABLE);
	
	*overo_id = (uint32_t) overo_dev;

	return(0);
	
out_fail:
	return(-1);
}

static void PIOS_OVERO_RxStart(uint32_t overo_id, uint16_t rx_bytes_avail)
{
	struct pios_overo_dev * overo_dev = (struct pios_overo_dev *)overo_id;
	
	bool valid = PIOS_OVERO_validate(overo_dev);
	PIOS_Assert(valid);
	
	// DMA RX enable (enable IRQ) ?
}

static void PIOS_OVERO_TxStart(uint32_t overo_id, uint16_t tx_bytes_avail)
{
	struct pios_overo_dev * overo_dev = (struct pios_overo_dev *)overo_id;
	
	bool valid = PIOS_OVERO_validate(overo_dev);
	PIOS_Assert(valid);
	
	// DMA TX enable (enable IRQ) ?

	// Load any pending bytes from TX fifo
	//PIOS_OVERO_WriteData(overo_dev);
}

static void PIOS_OVERO_RegisterRxCallback(uint32_t overo_id, pios_com_callback rx_in_cb, uint32_t context)
{
	struct pios_overo_dev * overo_dev = (struct pios_overo_dev *)overo_id;
	
	bool valid = PIOS_OVERO_validate(overo_dev);
	PIOS_Assert(valid);
	
	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	overo_dev->rx_in_context = context;
	overo_dev->rx_in_cb = rx_in_cb;
}

static void PIOS_OVERO_RegisterTxCallback(uint32_t overo_id, pios_com_callback tx_out_cb, uint32_t context)
{
	struct pios_overo_dev * overo_dev = (struct pios_overo_dev *)overo_id;
	
	bool valid = PIOS_OVERO_validate(overo_dev);
	PIOS_Assert(valid);
	
	/* 
	 * Order is important in these assignments since ISR uses _cb
	 * field to determine if it's ok to dereference _cb and _context
	 */
	overo_dev->tx_out_context = context;
	overo_dev->tx_out_cb = tx_out_cb;
}

#else

static void PIOS_OVERO_RegisterTxCallback(uint32_t overo_id, pios_com_callback tx_out_cb, uint32_t context) {};
static void PIOS_OVERO_RegisterRxCallback(uint32_t overo_id, pios_com_callback rx_in_cb, uint32_t context) {};
static void PIOS_OVERO_TxStart(uint32_t overo_id, uint16_t tx_bytes_avail) {};
static void PIOS_OVERO_RxStart(uint32_t overo_id, uint16_t rx_bytes_avail) {};

#endif /* PIOS_INCLUDE_FREERTOS */

#endif /* PIOS_INCLUDE_SPI */

#endif /* PIOS_INCLUDE_OVERO */

/**
  * @}
  * @}
  */
