/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2011 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 * This is an interface module for handling NOR in yaffs1 mode.
 */

/* This code is intended to be used with "regular" NOR that is bit-modifyable.
 *
 * Each "chunk" is a contguous area of 512 + 16 bytes.
 * This makes 248 such chunks with some space left over where a format markerr
 * is stored.
 */

// simposix yaffs simulation

#include "yaffs_nor_drv.h"

#include "yportenv.h"
#include "yaffs_trace.h"

#include "yaffs_flashif.h"
#include "yaffs_guts.h"

/* Tunable data */
#define DATA_BYTES_PER_CHUNK	512
#define SPARE_BYTES_PER_CHUNK	0
#define BLOCK_SIZE_IN_BYTES 	(128*1024)
#define BYTES_IN_DEVICE		(4*1024*1024)

#define BYTES_PER_CHUNK		(DATA_BYTES_PER_CHUNK + SPARE_BYTES_PER_CHUNK)
#define SPARE_AREA_OFFSET	DATA_BYTES_PER_CHUNK
#define CHUNKS_PER_BLOCK (BLOCK_SIZE_IN_BYTES/BYTES_PER_CHUNK)

#define BLOCKS_IN_DEVICE	(BYTES_IN_DEVICE/BLOCK_SIZE_IN_BYTES)

#define YNOR_PREMARKER          (0xF6)
#define YNOR_POSTMARKER         (0xF0)

#define FORMAT_OFFSET		BLOCK_SIZE_IN_BYTES
#define FORMAT_VALUE		0x1234

/* Compile this for a simulation */
#include "ynorsim.h"

static struct nor_sim *nor_sim;

#define nor_drv_FlashInit() do {nor_sim = ynorsim_initialise("emfile-nor", BLOCKS_IN_DEVICE, BLOCK_SIZE_IN_BYTES+16); } while(0)
#define nor_drv_FlashDeinit() ynorsim_shutdown(nor_sim)
#define nor_drv_FlashWrite32(addr,buf,nwords) ynorsim_wr32(nor_sim,addr,buf,nwords)
#define nor_drv_FlashRead32(addr,buf,nwords) ynorsim_rd32(nor_sim,addr,buf,nwords)
#define nor_drv_FlashEraseBlock(addr) ynorsim_erase(nor_sim,addr)
#define DEVICE_BASE     ynorsim_get_base(nor_sim)

static	int nor_drv_CheckBad(struct yaffs_dev *dev, int block_no)
{
	return YAFFS_OK;
}

static int nor_drv_MarkBad(struct yaffs_dev *dev, int block_no)
{
	return YAFFS_OK;
}


static u32 *Block2Addr(struct yaffs_dev *dev, int blockNumber)
{
	u8 *addr;

	dev=dev;

	addr = (u8*)DEVICE_BASE;
	addr += blockNumber * (BLOCK_SIZE_IN_BYTES + 16);

	return (u32 *) addr;
}

static u32 *Block2FormatAddr(struct yaffs_dev *dev, int blockNumber)
{
	u8 *addr;

	addr = (u8*) Block2Addr(dev,blockNumber);
	addr += FORMAT_OFFSET;

	return (u32 *)addr;
}

static u32 *Chunk2DataAddr(struct yaffs_dev *dev, int chunk_id)
{
	unsigned block;
	unsigned chunkInBlock;
	u8 *addr;

	block = chunk_id/dev->param.chunks_per_block;
	chunkInBlock = chunk_id % dev->param.chunks_per_block;

	addr = (u8*) Block2Addr(dev,block);
	addr += chunkInBlock * BYTES_PER_CHUNK;

	return (u32 *)addr;
}



static int nor_drv_WriteChunkToNAND(struct yaffs_dev *dev,int nand_chunk,
				    const u8 *data, int data_len,
				    const u8 *oob, int oob_len)
{
	u32 *dataAddr = Chunk2DataAddr(dev,nand_chunk);


	(void) oob_len;

	if(data) {
		/* Write the data */
		nor_drv_FlashWrite32(dataAddr,(u32 *)data, data_len/ sizeof(u32));
	}

	return YAFFS_OK;
}

static int nor_drv_ReadChunkFromNAND(struct yaffs_dev *dev,int nand_chunk,
					u8 *data, int data_len,
					u8 *oob, int oob_len,
					enum yaffs_ecc_result *ecc_result)
{

	u32 *dataAddr = Chunk2DataAddr(dev,nand_chunk);

	if (data) {
		nor_drv_FlashRead32(dataAddr,(u32 *)data,dev->param.total_bytes_per_chunk / sizeof(u32));
	}


	if(ecc_result)
		*ecc_result = YAFFS_ECC_RESULT_NO_ERROR;

	return YAFFS_OK;

}


static int nor_drv_FormatBlock(struct yaffs_dev *dev, int blockNumber)
{
	u32 *blockAddr = Block2Addr(dev,blockNumber);
	u32 *formatAddr = Block2FormatAddr(dev,blockNumber);
	u32 formatValue = FORMAT_VALUE;

	nor_drv_FlashEraseBlock(blockAddr);
	nor_drv_FlashWrite32(formatAddr,&formatValue,1);

	return YAFFS_OK;
}

static int nor_drv_UnformatBlock(struct yaffs_dev *dev, int blockNumber)
{
	u32 *formatAddr = Block2FormatAddr(dev,blockNumber);
	u32 formatValue = 0;

	nor_drv_FlashWrite32(formatAddr,&formatValue,1);

	return YAFFS_OK;
}

static int nor_drv_IsBlockFormatted(struct yaffs_dev *dev, int blockNumber)
{
	u32 *formatAddr = Block2FormatAddr(dev,blockNumber);
	u32 formatValue;


	nor_drv_FlashRead32(formatAddr,&formatValue,1);

	return (formatValue == FORMAT_VALUE);
}

static int nor_drv_EraseBlockInNAND(struct yaffs_dev *dev, int blockNumber)
{

	if(blockNumber < 0 || blockNumber >= BLOCKS_IN_DEVICE)
	{
		return YAFFS_FAIL;
	}
	else
	{
		nor_drv_UnformatBlock(dev,blockNumber);
		nor_drv_FormatBlock(dev,blockNumber);
		return YAFFS_OK;
	}

}

static int nor_drv_InitialiseNAND(struct yaffs_dev *dev)
{
	int i;

	nor_drv_FlashInit();
	/* Go through the blocks formatting them if they are not formatted */
	for(i = dev->param.start_block; i <= dev->param.end_block; i++){
		if(!nor_drv_IsBlockFormatted(dev,i)){
			nor_drv_FormatBlock(dev,i);
		}
	}
	return YAFFS_OK;
}

static int nor_drv_Deinitialise_flash_fn(struct yaffs_dev *dev)
{
	dev=dev;

	nor_drv_FlashDeinit();

	return YAFFS_OK;
}


struct yaffs_dev *yaffs_nor_install_drv(const char *name)
{

	struct yaffs_dev *dev = malloc(sizeof(struct yaffs_dev));
	char *name_copy = strdup(name);
	struct yaffs_param *param;
	struct yaffs_driver *drv;


	if(!dev || !name_copy) {
		free(name_copy);
		free(dev);
		return NULL;
	}

	param = &dev->param;
	drv = &dev->drv;

	memset(dev, 0, sizeof(*dev));

	param->name = name_copy;

	param->total_bytes_per_chunk = DATA_BYTES_PER_CHUNK;
	param->chunks_per_block = CHUNKS_PER_BLOCK;
	param->n_reserved_blocks = 2;
	param->start_block = 0; // Can use block 0
	param->end_block = BLOCKS_IN_DEVICE - 1; // Last block
	param->use_nand_ecc = 0; // use YAFFS's ECC
	param->disable_soft_del = 1;

	// using inband_tags means the spare area does not need to be
	// supported in the above methods
	param->inband_tags = 1;
	param->is_yaffs2 = 1;
	param->n_caches = 10;
	param->refresh_period = 1000;
	param->empty_lost_n_found = 1;
	param->n_caches = 10;
	param->disable_soft_del = 0;

	drv->drv_write_chunk_fn = nor_drv_WriteChunkToNAND;
	drv->drv_read_chunk_fn = nor_drv_ReadChunkFromNAND;
	drv->drv_erase_fn = nor_drv_EraseBlockInNAND;
	drv->drv_initialise_fn = nor_drv_InitialiseNAND;
	drv->drv_deinitialise_fn = nor_drv_Deinitialise_flash_fn;
	drv->drv_check_bad_fn = nor_drv_CheckBad;
	drv->drv_mark_bad_fn = nor_drv_MarkBad;

	dev->driver_context = (void *) nor_sim;

	yaffs_add_device(dev);

	return NULL;
}
