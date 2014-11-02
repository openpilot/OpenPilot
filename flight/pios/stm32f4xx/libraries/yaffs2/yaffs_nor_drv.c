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
 *
 * As per the YAFFS NAND model above, YAFFS needs pages with data and spare areas.
 * To get this working requires that the pages and spare area be emulated.
 *
 *  Revo's nor flash is organized as 32 sectors, each containing 256 pages.
 *  Each page is 256 bytes wide. Memory can be viewed either as 8,192 pages or
 *  as 2,097,152 bytes. The en- tire memory can be erased using the BULK ERASE
 *  command, or it can be erased one sector at a time using the
 *  SECTOR ERASE command.
 */

#include "yaffs_nor_drv.h"

#include "yportenv.h"
#include "yaffs_trace.h"

#include "yaffs_flashif.h"
#include "yaffs_guts.h"

#include "pios.h"
#include "pios_mem.h"


// For revo, we have NOR flash in sector erasable size of 65,536B erasable blocks. Here’s what we can do:
// Each chunk is going to be 512bytes of data area + 16 bytes of spare area = 528 bytes.
// Each block is arranged as (65536)/528 = 124 chunks

#define FORMAT_OFFSET		(CHUNKS_PER_BLOCK * BYTES_PER_CHUNK)
#define FORMAT_VALUE		0x1234



#define nor_drv_FlashInit()  do{} while(0)
#define nor_drv_FlashDeinit() do {} while(0)
#define nor_drv_FlashWrite32(addr,buf,nwords) Y_FlashWrite(addr,buf,nwords)
#define nor_drv_FlashRead32(addr,buf,nwords)  Y_FlashRead(addr,buf,nwords)
#define nor_drv_FlashEraseBlock(addr)         Y_FlashErase(addr,BLOCK_SIZE_IN_BYTES)
#define DEVICE_BASE     (32 * 1024 * 1024)



static u32 *Block2Addr(struct yaffs_dev *dev, int blockNumber)
{
	u8 *addr;

	struct pios_yaffs_driver_context *context =
	    (pios_yaffs_driver_context *)dev->driver_context;

	addr = (u8*)DEVICE_BASE;
	addr += blockNumber * (context->cfg->sector_size);  //BLOCK_SIZE_IN_BYTES;

	return (u32 *) addr;
}

static u32 *Block2FormatAddr(struct yaffs_dev *dev, int blockNumber)
{
	u8 *addr;

	addr = (u8*) Block2Addr(dev,blockNumber+1);
	addr -= dev->param->total_bytes_per_chunk;

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
	addr += chunkInBlock * dev->param->total_bytes_per_chunk;

	return (u32 *)addr;
}

static int nor_drv_WriteChunkToNAND(struct yaffs_dev *dev,
                                    int nand_chunk,
				    const u8 *data,
				    int data_len,
				    const u8 *oob,
				    int oob_len)
{
        // inband tags with yaffs2 means that oob and oob_len are
	// always null/zero as per yaffs_tagsmarshall.c
	u32 *dataAddr = Chunk2DataAddr(dev,nand_chunk);

	if(data) {
		/* Write the data */
		nor_drv_FlashWrite32(dataAddr,(u32 *)data, data_len/ sizeof(u32));
	}

	return YAFFS_OK;
}


static int nor_drv_ReadChunkFromNAND(struct yaffs_dev *dev,
                                     int nand_chunk,
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
		yaffs_trace(YAFFS_TRACE_ALWAYS,
			"Attempt to erase non-existant block %d\n",
			blockNumber);
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



struct yaffs_dev *yaffs_nor_install_drv(const char *name,
		const struct flashfs_logfs_cfg *cfg,
		const struct pios_flash_driver *driver,
		uintptr_t flash_id)
{

	struct yaffs_dev *dev = pios_malloc(sizeof(struct yaffs_dev));
	char *name_copy = strdup(name);
	struct yaffs_param *param;
	struct yaffs_driver *drv;

	struct pios_yaffs_driver_context *context =
		pios_malloc(sizeof(struct pios_yaffs_driver_context ));

	if(!dev || !name_copy || !context) {
		pios_free(name_copy);
		pios_free(dev);
		pios_free(context);
		return NULL;
	}

	context->cfg = cfg;
	context->driver = driver;
	context->flash_id = flash_id;


	param = &dev->param;
	drv = &dev->drv;

	memset(dev, 0, sizeof(*dev));

	param->name = name_copy;

	// We can choose the size of a chunk to be multiples of our
	// page size.  Give we have tags overhead, a 1:1 mapping between
	// page and chunks is a large loss of storage.
	param->total_bytes_per_chunk = 4*cfg->page_size;

	// Calculate the start and end block/sectors
	param->start_block = cfg->start_offset/cfg->sector_size;  // Can use block 0
	param->end_block = (cfg->total_fs_size - cfg->start_offset)/cfg->sector_size -1; // Last block

	// Chunks per block is based on sector size and the overall arbitrary chunk size
	// we reserve one chunk per block for flagging formatting status.
	param->chunks_per_block = (cfg->sector_size/param->total_bytes_per_chunk) -1;
	param->n_reserved_blocks = 2; // wow 2*64KB means we can't forward to split this up

	// using inband_tags means the spare area does not need to be
	// supported in the above methods
	param->inband_tags = TRUE;
	param->is_yaffs2 = TRUE;
	param->n_caches = 10;
	param->refresh_period = 1000;
	param->empty_lost_n_found = TRUE;
	param->n_caches = 10;
	param->disable_soft_del = 0;

	// the tagsmarshall takes care of the following yaffs2 methods
	// with support for inband tags
	// writeChunkWithTagsToNAND
	// readChunkWithTagsFromNAND
	// markNANDBlockBad
	// queryNANDBlock

	drv->drv_write_chunk_fn = nor_drv_WriteChunkToNAND;
	drv->drv_read_chunk_fn = nor_drv_ReadChunkFromNAND;
	drv->drv_erase_fn = nor_drv_EraseBlockInNAND;
	drv->drv_initialise_fn = nor_drv_InitialiseNAND;
	drv->drv_deinitialise_fn = nor_drv_Deinitialise_flash_fn;



	// if we need to, store ptr to config here
	dev->driver_context = (void *) context;

	yaffs_add_device(dev);

	return NULL;
}
