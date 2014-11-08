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

// TODO Add in OP's copyright

/* This code is intended to be used with "regular" NOR that is bit-modifyable.
 *
 *  Revo's nor flash is organized as 32 sectors, each containing 256 pages.
 *  Each page is 256 bytes wide. Memory can be viewed either as 8,192 pages or
 *  as 2,097,152 bytes. The en- tire memory can be erased using the BULK ERASE
 *  command, or it can be erased one sector at a time using the
 *  SECTOR ERASE command.
 *
 *  yaffs2 will manage 32 blocks (i.e. sectors) with chunks of 4 pages per chunk
 *  Each chunk will have an inband tag managed by yaffs marshalling code.
 *  TODO add description of the format marker.
 */

#include "pios.h"
#include "pios_mem.h"
#include "pios_flashfs_logfs_priv.h"

#include "yaffs_nor_drv.h"

#include "yportenv.h"
#include "yaffs_trace.h"

#include "yaffs_flashif.h"
#include "yaffs_guts.h"

// #define USE_NORSIM

#if defined(USE_NORSIM)
#include "ynorsim.h"
#endif


/*
 * The bits within these enum values must progress ONLY
 * from 1 -> 0 so that we can write later ones on top
 * of earlier ones in NOR flash without an erase cycle.
 */
enum arena_state {
    /*
     * The flash subsystem is only capable of
     * writing words or halfwords. In this case we use halfwords.
     * In addition to that it is only capable to write to erased
     * cells (0xffff) or write a cell from anything to (0x0000).
     * To cope with this, the F3 needs carefully crafted enum values.
     * For this to work the underlying flash driver has to
     * check each halfword if it has changed before writing.
     */
    ARENA_STATE_ERASED    = 0xFFFFFFFF,
    ARENA_STATE_FORMATTED = 0xE6E6FFFF,
    ARENA_STATE_REFORMAT  = 0xE6E66666,
    ARENA_STATE_BAD       = 0x00000000,
};

struct arena_header {
    uint32_t magic;
    enum arena_state state;
} __attribute__((packed));


static u32 Block2Addr(struct yaffs_dev *dev, int blockNumber)
{
	u32 addr;

	struct pios_yaffs_driver_context *context =
	    (struct pios_yaffs_driver_context *)dev->driver_context;

	addr = context->cfg->start_offset;
	addr += blockNumber * (context->cfg->sector_size);

	return addr;
}

static u32 Block2FormatAddr(struct yaffs_dev *dev, int blockNumber)
{
	u32 addr;
	struct pios_yaffs_driver_context *context =
	    (struct pios_yaffs_driver_context *)dev->driver_context;

	addr = Block2Addr(dev,blockNumber);

	// the arena size is smaller than the sector size, providing
	// space to write a per block data structure
	addr += context->cfg->arena_size;

	return addr;
}


static uintptr_t Chunk2DataAddr(struct yaffs_dev *dev, int chunk_id)
{
	unsigned block;
	unsigned chunkInBlock;
	uintptr_t addr;

	block = chunk_id/dev->param.chunks_per_block;
	chunkInBlock = chunk_id % dev->param.chunks_per_block;

	addr = Block2Addr(dev,block);
	addr += chunkInBlock * dev->param.total_bytes_per_chunk;

	return addr;
}



static int nor_drv_FlashWrite32(struct yaffs_dev *dev, uintptr_t addr, const uint8_t* buf,u32 write_len)
{
	struct pios_yaffs_driver_context *context =
	    (struct pios_yaffs_driver_context *)dev->driver_context;
	int retval = YAFFS_OK;
	uint16_t write_size;

	if (context->driver->start_transaction(context->flash_id) == 0)
	{
	  uintptr_t write_offset = addr;
	  while (write_len > 0 && retval == YAFFS_OK)
	  {
	 	     /* Individual writes must fit entirely within a single page buffer. */
	 	     write_size = MIN(write_len, context->cfg->page_size);
	 	     if (context->driver->write_data(context->flash_id,
	 		                                      write_offset,
	 		                                      (uint8_t *)buf,
	 		                                      write_size) != 0)
	 	     {
	 		            /* Failed to read the object data to the slot */
	 		            retval = YAFFS_FAIL;
	 	     }

	 	     write_offset += write_size;
	 	     write_len    -= write_size;
	 	     buf += write_size;
	  }

	  context->driver->end_transaction(context->flash_id);
	}
	else
	{
	    retval = YAFFS_FAIL;
	}

	return retval;
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
	uintptr_t dataAddr = Chunk2DataAddr(dev,nand_chunk);
	int retval = YAFFS_FAIL;
	oob = oob;
	oob_len = oob_len;

	if(data) {
		/* Write the data */
		retval = nor_drv_FlashWrite32(dev, dataAddr,data, data_len);
	}

	return retval;
}



static int nor_drv_FlashRead32(struct yaffs_dev *dev, uintptr_t addr, uint8_t* buf, u32 read_len)
{
	struct pios_yaffs_driver_context *context =
	    (struct pios_yaffs_driver_context *)dev->driver_context;
	int retval = YAFFS_OK;
	uint16_t read_size;

	if (context->driver->start_transaction(context->flash_id) == 0)
	{
	     uintptr_t read_offset = addr;
	     while (read_len > 0 && retval == YAFFS_OK)
	     {
	    	 /* Individual reads must fit entirely within a single page buffer. */
	    	 read_size     = MIN(read_len, context->cfg->page_size);
	    	 if (context->driver->read_data(context->flash_id,
	    		 	 	 	read_offset,
		                                buf,
		                                read_size) != 0)
	    	 {
		            /* Failed to read the object data to the slot */
		            retval = YAFFS_FAIL;
	    	 }

	    	 read_offset += read_size;
	    	 read_len    -= read_size;
	    	 buf         += read_size;
	     }

	     context->driver->end_transaction(context->flash_id);
	}
	else
	{
	    retval = YAFFS_FAIL;
	}

	return retval;
}


static int nor_drv_ReadChunkFromNAND(struct yaffs_dev *dev,
                                     int nand_chunk,
					u8 *data,
					int data_len,
					u8 *oob,
					int oob_len,
					enum yaffs_ecc_result *ecc_result)
{


	uintptr_t dataAddr = Chunk2DataAddr(dev,nand_chunk);
	int retval = YAFFS_FAIL;
	oob = oob;
	oob_len = oob_len;

	if (data) {
	    PIOS_Assert((u32)data_len == dev->param.total_bytes_per_chunk);
            retval = nor_drv_FlashRead32(dev, dataAddr,data,dev->param.total_bytes_per_chunk);
	}

	//TODO How to implement ECC
	if(ecc_result)
		*ecc_result = YAFFS_ECC_RESULT_NO_ERROR;

	return retval;

}



static int nor_drv_FlashEraseBlock(struct yaffs_dev *dev,  uintptr_t addr)
{
	struct pios_yaffs_driver_context *context =
	    (struct pios_yaffs_driver_context *)dev->driver_context;

	int retval = YAFFS_OK;

	if (context->driver->start_transaction(context->flash_id) == 0)
	{
	    if (context->driver->erase_sector(context->flash_id, (uintptr_t)addr) != 0)
	    {
	    	retval = YAFFS_FAIL;
	    }

	    context->driver->end_transaction(context->flash_id);
	}
	else
	{
		retval = YAFFS_FAIL;
	}

	return retval;
}

static int nor_drv_FormatBlock(struct yaffs_dev *dev, int blockNumber)
{
	uintptr_t blockAddr = Block2Addr(dev,blockNumber);
	uintptr_t formatAddr = Block2FormatAddr(dev,blockNumber);

	nor_drv_FlashEraseBlock(dev, blockAddr);

	struct pios_yaffs_driver_context *context =
		    (struct pios_yaffs_driver_context *)dev->driver_context;

	struct arena_header arena_hdr = {
	        .magic = context->cfg->fs_magic,
	        .state = ARENA_STATE_FORMATTED,
	};

	return nor_drv_FlashWrite32(dev, formatAddr, (u8*)&arena_hdr, sizeof(arena_hdr));

}


static int nor_drv_UnformatBlock(struct yaffs_dev *dev, int blockNumber)
{
	uintptr_t formatAddr = Block2FormatAddr(dev,blockNumber);

	struct pios_yaffs_driver_context *context =
		    (struct pios_yaffs_driver_context *)dev->driver_context;

	struct arena_header arena_hdr = {
	        .magic = context->cfg->fs_magic,
	        .state = ARENA_STATE_REFORMAT,
	};

	return nor_drv_FlashWrite32(dev, formatAddr, (u8*)&arena_hdr, sizeof(arena_hdr));

}

static int nor_drv_IsBlockFormatted(struct yaffs_dev *dev, int blockNumber)
{
	uintptr_t formatAddr = Block2FormatAddr(dev,blockNumber);

	struct arena_header arena_hdr = {0};

	nor_drv_FlashRead32(dev, formatAddr, (uint8_t *)&arena_hdr,sizeof(arena_hdr));

	struct pios_yaffs_driver_context *context =
			    (struct pios_yaffs_driver_context *)dev->driver_context;

	return (arena_hdr.magic == context->cfg->fs_magic && arena_hdr.state == ARENA_STATE_FORMATTED);
}

static int nor_drv_EraseBlockInNAND(struct yaffs_dev *dev, int blockNumber)
{
	if(blockNumber < 0 || blockNumber >= dev->param.end_block)
	{
		yaffs_trace(YAFFS_TRACE_ERROR,
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

  dev = dev;

#if defined(USE_NORSIM)

	struct pios_yaffs_driver_context *context =
				    (struct pios_yaffs_driver_context *)dev->driver_context;

	ynorsim_shutdown(context->flash_id);
#endif

	return YAFFS_OK;
}

static	int nor_drv_CheckBad(struct yaffs_dev *dev, int block_no)
{
	uintptr_t formatAddr = Block2FormatAddr(dev,block_no);

	struct arena_header arena_hdr = {0};

	nor_drv_FlashRead32(dev, formatAddr, (uint8_t *)&arena_hdr,sizeof(arena_hdr));

	struct pios_yaffs_driver_context *context =
			    (struct pios_yaffs_driver_context *)dev->driver_context;

	if ( arena_hdr.magic == context->cfg->fs_magic && arena_hdr.state == ARENA_STATE_BAD)
	{
		return YAFFS_FAIL;
	}
	else
	{
		return YAFFS_OK;
	}
}

static int nor_drv_MarkBad(struct yaffs_dev *dev, int block_no)
{
	uintptr_t formatAddr = Block2FormatAddr(dev,block_no);

	struct pios_yaffs_driver_context *context =
		    (struct pios_yaffs_driver_context *)dev->driver_context;

	struct arena_header arena_hdr = {
	        .magic = context->cfg->fs_magic,
	        .state = ARENA_STATE_BAD,
	};

	return nor_drv_FlashWrite32(dev, formatAddr, (u8*)&arena_hdr, sizeof(arena_hdr));
}


void yaffs_nor_install_drv(const char *name,
                           uint16_t max_name_len,
		const struct flashfs_logfs_cfg *cfg,
		const struct pios_flash_driver *driver,
		uintptr_t flash_id)
{

	struct yaffs_dev *dev = pios_malloc(sizeof(struct yaffs_dev));
	char *name_copy = pios_strndup(name, max_name_len);
	struct yaffs_param *param;
	struct yaffs_driver *drv;

	struct pios_yaffs_driver_context *context =
		pios_malloc(sizeof(struct pios_yaffs_driver_context ));

	if(!dev || !name_copy || !context) {
		pios_free(name_copy);
		pios_free(dev);
		pios_free(context);
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
	param->total_bytes_per_chunk = cfg->slot_size;

	// Calculate the start and end block/sectors
	param->start_block = cfg->start_offset/cfg->sector_size;  // Can use block 0
	param->end_block = (cfg->total_fs_size - cfg->start_offset)/cfg->sector_size -1; // Last block

	PIOS_Assert(cfg->arena_size < cfg->sector_size);

	// Chunks per block is based on sector size and the overall arbitrary chunk size
	// we reserve one chunk per block for flagging formatting status.
	param->chunks_per_block = (cfg->arena_size/param->total_bytes_per_chunk);
	param->n_reserved_blocks = 2; // wow 2*64KB means we can't forward to split this up

	// using inband_tags means the spare area does not need to be
	// supported in the above methods
	param->inband_tags = 1;
	param->is_yaffs2 = 1;
	param->n_caches = 10;
	param->refresh_period = 1000;
	param->empty_lost_n_found = 1;
	param->n_caches = 10;
	param->disable_soft_del = 0;

	//How to enable yaffs ecc???

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
	drv->drv_check_bad_fn = nor_drv_CheckBad;
	drv->drv_mark_bad_fn = nor_drv_MarkBad;


	dev->driver_context = (void *) context;


#if defined(USE_NORSIM)
	int blocks_in_device = param->end_block - param->start_block +1;
	ynorsim_initialise("emfile-nor", flash_id, blocks_in_device, cfg->sector_size);
#endif


	yaffs_add_device(dev);
}
