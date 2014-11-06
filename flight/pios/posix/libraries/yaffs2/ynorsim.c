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

#include "ynorsim.h"

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "pios_trace.h"

/* Set YNORSIM_BIT_CHANGES to a a value from 1..30 to
 *simulate bit flipping as the programming happens.
 * A low value results in faster simulation with less chance of encountering a partially programmed
 * word.
 */

//#define YNORSIM_BIT_CHANGES 15
#define YNORSIM_BIT_CHANGES 2


struct nor_sim {
	int n_blocks;
	int block_size_bytes;
	int file_size;
	u32 *word;
	int initialised;
	char *fname;
	int remaining_ops;
	int nops_so_far;
};

#define NORSIM_MAX_ID 4
static struct nor_sim _norSimData[NORSIM_MAX_ID];

int ops_multiplier = 500;
extern int random_seed;
extern int simulate_power_failure;

static void NorError(struct nor_sim *sim)
{
	pios_trace(PIOS_TRACE_ERROR, "NorError on device %s", sim->fname);
	exit(1);
}

static void ynorsim_save_image(struct nor_sim *sim)
{
	int h;
	pios_trace(PIOS_TRACE_TEST, "ynorsim_save_image");
	h = open(sim->fname, O_RDWR | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
	write(h, sim->word, sim->file_size);
	close(h);
}

static void ynorsim_restore_image(struct nor_sim *sim)
{
	int h;
	pios_trace(PIOS_TRACE_TEST, "ynorsim_restore_image");
	h = open(sim->fname, O_RDONLY, S_IREAD | S_IWRITE);
	memset(sim->word, 0xFF, sim->file_size);
	read(h, sim->word, sim->file_size);
	close(h);
}

static void ynorsim_power_fail(struct nor_sim *sim)
{
	ynorsim_save_image(sim);
	exit(1);
}

static void ynorsim_maybe_power_fail(struct nor_sim *sim)
{
	sim->nops_so_far++;
	sim->remaining_ops--;
	if (simulate_power_failure && sim->remaining_ops < 1) {
		printf("Simulated power failure after %d operations\n",
		       sim->nops_so_far);
		ynorsim_power_fail(sim);
	}
}

static void ynorsim_ready(struct nor_sim *sim)
{
	if (sim->initialised)
		return;
	srand(random_seed);
	sim->remaining_ops = 1000000000;
	sim->remaining_ops =
	    (rand() % 10000) * ops_multiplier * YNORSIM_BIT_CHANGES;
	ynorsim_restore_image(sim);
	sim->initialised = 1;
}

/* Public functions. */
int32_t PIOS_Flash_Norsim_ReadData(uintptr_t flash_id, uint32_t addr_val, uint8_t *data, uint16_t len)
{
	struct nor_sim *sim;
	sim = &_norSimData[flash_id];
	pios_trace(PIOS_TRACE_TEST, "PIOS_Flash_Norsim_ReadData");
	uint16_t nwords = len / 4;
	u32 *buf = (u32 *)data;
	u32 *addr = sim->word + (u32 *)addr_val;
	while (nwords > 0) {
			*buf = *addr;
			buf++;
			addr++;
			nwords--;
	}
}


static void ynorsim_wr_one_word32(struct nor_sim *sim, u32 * addr, u32 val)
{
	u32 tmp;
	u32 m;
	int i;

	tmp = *addr;
	if (val & ~tmp) {
		/* Fail due to trying to change a zero into a 1 */
		printf("attempt to set a zero to one (%x)->(%x)\n", tmp, val);
		NorError(sim);
	}

	for (i = 0; i < YNORSIM_BIT_CHANGES; i++) {
		m = 1 << (rand() & 31);
		if (!(m & val)) {
			tmp &= ~m;
			*addr = tmp;
			ynorsim_maybe_power_fail(sim);
		}

	}

	*addr = tmp & val;
	ynorsim_maybe_power_fail(sim);
}


int32_t PIOS_Flash_Norsim_WriteChunks(uintptr_t flash_id, uint32_t addr, struct pios_flash_chunk chunks[], uint32_t num_chunks)
{
  return -1;
}

int32_t PIOS_Flash_Norsim_WriteData(uintptr_t flash_id, uint32_t addr_val, uint8_t *data, uint16_t len)
{
	struct nor_sim *sim;
	sim = &_norSimData[flash_id];
	pios_trace(PIOS_TRACE_TEST, "PIOS_Flash_Norsim_WriteData");
	uint16_t nwords = len / 4;
	u32 *buf = (u32 *)data;
	u32 *addr = sim->word + (u32 *)addr_val;
	while (nwords > 0) {
		ynorsim_wr_one_word32(sim, addr, *buf);
		addr++;
		buf++;
		nwords--;
	}
}

int32_t PIOS_Flash_Norsim_EraseSector(uintptr_t flash_id, uint32_t addr)
{
	struct nor_sim *sim;
	sim = &_norSimData[flash_id];
	pios_trace(PIOS_TRACE_TEST, "PIOS_Flash_Norsim_EraseSector");
	memset(sim->word + addr, 0xFF, sim->block_size_bytes);
}

int32_t PIOS_Flash_Norsim_EraseChip(uintptr_t flash_id)
{
	struct nor_sim *sim;
  	sim = &_norSimData[flash_id];
	pios_trace(PIOS_TRACE_TEST, "PIOS_Flash_Norsim_EraseChip");
	memset(sim->word, 0xFF, sim->file_size);
	return 0;
}

void ynorsim_initialise(char *name,
                                   uintptr_t flash_id,
                                   int n_blocks,
				   int block_size_bytes)
{
	struct nor_sim *sim;
	sim = &_norSimData[flash_id];

	memset(sim, 0, sizeof(*sim));
	sim->n_blocks = n_blocks;
	sim->block_size_bytes = block_size_bytes;
	sim->file_size = n_blocks * block_size_bytes;
	sim->word = malloc(sim->file_size);
	sim->fname = strdup(name);

	if(!sim->word)
		return NULL;

	ynorsim_ready(sim);
	return;
}

void ynorsim_shutdown(uintptr_t flash_id)
{
	struct nor_sim *sim;
  	sim = &_norSimData[flash_id];
	ynorsim_save_image(sim);
	sim->initialised = 0;
}

int32_t PIOS_Flash_Norsim_StartTransaction(uintptr_t flash_id)
{
  return 0;
}

int32_t PIOS_Flash_Norsim_EndTransaction(uintptr_t flash_id)
{
  return 0;
}


/* Provide a flash driver to external drivers */
const struct pios_flash_driver pios_norsim_flash_driver = {
    .start_transaction = PIOS_Flash_Norsim_StartTransaction,
    .end_transaction   = PIOS_Flash_Norsim_EndTransaction,
    .erase_chip   = PIOS_Flash_Norsim_EraseChip,
    .erase_sector = PIOS_Flash_Norsim_EraseSector,
    .write_chunks = PIOS_Flash_Norsim_WriteChunks,
    .write_data   = PIOS_Flash_Norsim_WriteData,
    .read_data    = PIOS_Flash_Norsim_ReadData,
};

