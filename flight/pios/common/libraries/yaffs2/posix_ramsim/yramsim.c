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
 * NAND Simulator for testing YAFFS
 */

#include <string.h>

#include "yramsim.h"

#include "yaffs_nandif.h"


#define DATA_SIZE	2048
#define SPARE_SIZE	64
#define PAGE_SIZE	(DATA_SIZE + SPARE_SIZE)
#define PAGES_PER_BLOCK	64


typedef struct {
	unsigned char page[PAGES_PER_BLOCK][PAGE_SIZE];
	unsigned blockOk;
} Block;

typedef struct {
	Block **blockList;
	int nBlocks;
} SimData;


SimData *simDevs[N_RAM_SIM_DEVS];

static SimData *DevToSim(struct yaffs_dev *dev)
{
	struct ynandif_Geometry *geom = 
		(struct ynandif_Geometry *)(dev->driver_context);
	SimData * sim = (SimData*)(geom->privateData);
	return sim;
}


static void CheckInitialised(void)
{

}

static int yramsim_erase_internal(SimData *sim, unsigned blockId,int force)
{
	if(blockId < 0 || blockId >= sim->nBlocks){
		return 0;
	}

	if(!sim->blockList[blockId]){
		return 0;
	}

	if(!force && !sim->blockList[blockId]->blockOk){
		return 0;
	}

	memset(sim->blockList[blockId],0xff,sizeof(Block));
	sim->blockList[blockId]->blockOk = 1;

	return 1;
}




static int yramsim_initialise(struct yaffs_dev *dev)
{
	SimData *sim = DevToSim(dev);
	Block **blockList = sim->blockList;
	return blockList != NULL;
}


static int yramsim_deinitialise(struct yaffs_dev *dev)
{
	return 1;
}

static int yramsim_rd_chunk (struct yaffs_dev *dev, unsigned pageId,
					  unsigned char *data, unsigned dataLength,
					  unsigned char *spare, unsigned spareLength,
					  int *eccStatus)
{
	SimData *sim = DevToSim(dev);
	Block **blockList = sim->blockList;

	unsigned blockId = pageId / PAGES_PER_BLOCK;
	unsigned pageOffset = pageId % PAGES_PER_BLOCK;
	unsigned char * d;
	unsigned char *s;
	if(blockId >= sim->nBlocks ||
	   pageOffset >= PAGES_PER_BLOCK ||
	   dataLength >DATA_SIZE ||
	   spareLength > SPARE_SIZE ||
	   !eccStatus ||
	   !blockList[blockId]->blockOk){
		   return 0;
	}

	d = blockList[blockId]->page[pageOffset];
	s = d + DATA_SIZE;

	if(data)
		memcpy(data,d,dataLength);

	if(spare)
		memcpy(spare,s,spareLength);

	*eccStatus = 0; // 0 = no error, -1 = unfixable error, 1 = fixable

	return 1;
}

static int yramsim_wr_chunk (struct yaffs_dev *dev,unsigned pageId,
					   const unsigned char *data, unsigned dataLength,
					   const unsigned char *spare, unsigned spareLength)
{
	SimData *sim = DevToSim(dev);
	Block **blockList = sim->blockList;

	unsigned blockId = pageId / PAGES_PER_BLOCK;
	unsigned pageOffset = pageId % PAGES_PER_BLOCK;
	unsigned char * d;
	unsigned char *s;
	if(blockId >= sim->nBlocks ||
	   pageOffset >= PAGES_PER_BLOCK ||
	   dataLength >DATA_SIZE ||
	   spareLength > SPARE_SIZE ||
	   !blockList[blockId]->blockOk){
		   return 0;
	}

	d = blockList[blockId]->page[pageOffset];
	s = d + DATA_SIZE;

	if(data)
		memcpy(d,data,dataLength);

	if(spare)
		memcpy(s,spare,spareLength);

	return 1;
}


static int yramsim_erase(struct yaffs_dev *dev,unsigned blockId)
{
	SimData *sim = DevToSim(dev);

	CheckInitialised();
	return yramsim_erase_internal(sim,blockId,0);
}

static int yramsim_check_block_ok(struct yaffs_dev *dev,unsigned blockId)
{
	SimData *sim = DevToSim(dev);
	Block **blockList = sim->blockList;
	if(blockId >= sim->nBlocks){
		return 0;
	}

	return blockList[blockId]->blockOk ? 1 : 0;
}

static int yramsim_mark_block_bad(struct yaffs_dev *dev,unsigned blockId)
{
	SimData *sim = DevToSim(dev);
	Block **blockList = sim->blockList;
	if(blockId >= sim->nBlocks){
		return 0;
	}

	blockList[blockId]->blockOk = 0;

	return 1;
}


static SimData *yramsim_alloc_sim_data(u32 devId, u32 nBlocks)
{
	int ok = 1;

	Block **blockList;
	SimData *sim;
	Block *b;
	u32 i;

	if(devId >= N_RAM_SIM_DEVS)
		return NULL;

	sim = simDevs[devId];

	if(sim)
		return sim;

	sim = malloc(sizeof (SimData));
	if(!sim)
		return NULL;

	simDevs[devId] = sim;

	blockList = malloc(nBlocks * sizeof(Block *));

	sim->blockList = blockList;
	sim->nBlocks = nBlocks;
	if(!blockList){
		free(sim);
		return NULL;
	}

	for(i = 0; i < nBlocks; i++)
		blockList[i] = NULL;

	for(i = 0; i < nBlocks && ok; i++){
		b=  malloc(sizeof(Block));
		if(b){
			blockList[i] = b;
			yramsim_erase_internal(sim,i,1);
		}
		else
			ok = 0;
	}

	if(!ok){
		for(i = 0; i < nBlocks; i++)
			if(blockList[i]){
				free(blockList[i]);
				blockList[i] = NULL;
			}
		free(blockList);
		blockList = NULL;
		free(sim);
		sim = NULL;
	}

	return sim;
}


struct yaffs_dev *yramsim_CreateRamSim(const YCHAR *name,
				u32 devId, u32 nBlocks,
				u32 start_block, u32 end_block)
{
	SimData *sim;
	struct ynandif_Geometry *g;

	sim = yramsim_alloc_sim_data(devId, nBlocks);

	g = malloc(sizeof(*g));

	if(!sim || !g){
		if(g)
			free(g);
		return NULL;
	}

	if(start_block >= sim->nBlocks)
		start_block = 0;
	if(end_block == 0 || end_block >= sim->nBlocks)
		end_block = sim->nBlocks - 1;

	memset(g,0,sizeof(*g));
	g->start_block = start_block;
	g->end_block = end_block;
	g->dataSize = DATA_SIZE;
	g->spareSize= SPARE_SIZE;
	g->pagesPerBlock = PAGES_PER_BLOCK;
	g->hasECC = 1;
	g->inband_tags = 0;
	g->useYaffs2 = 1;
	g->initialise = yramsim_initialise;
	g->deinitialise = yramsim_deinitialise;
	g->readChunk = yramsim_rd_chunk,
	g->writeChunk = yramsim_wr_chunk,
	g->eraseBlock = yramsim_erase,
	g->checkBlockOk = yramsim_check_block_ok,
	g->markBlockBad = yramsim_mark_block_bad,
	g->privateData = (void *)sim;

	return yaffs_add_dev_from_geometry(name,g);
}
