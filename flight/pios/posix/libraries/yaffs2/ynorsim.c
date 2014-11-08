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

#define YNORSIM_FNAME          "emfile-nor"

/* Set YNORSIM_BIT_CHANGES to a a value from 1..30 to
 * simulate bit flipping as the programming happens.
 * A low value results in faster simulation with less chance of encountering a partially programmed
 * word.
 */

// #define YNORSIM_BIT_CHANGES 15
#define YNORSIM_BIT_CHANGES    2

#if 0
/* Simulate 32MB of flash in 256k byte blocks.
 * This stuff is x32.
 */

#define YNORSIM_BLOCK_SIZE_U32 (256 * 1024 / 4)
#define YNORSIM_DEV_SIZE_U32   (32 * 1024 * 1024 / 4)
#else
/* Simulate 8MB of flash in 256k byte blocks.
 * This stuff is x32.
 */

#define YNORSIM_BLOCK_SIZE_U32 (256 * 1024 / 4)
#define YNORSIM_DEV_SIZE_U32   (8 * 1024 * 1024 / 4)
#endif

struct nor_sim {
    int  n_blocks;
    int  block_size_bytes;
    int  file_size;
    u32  *word;
    int  initialised;
    char *fname;
    int  remaining_ops;
    int  nops_so_far;
};

int ops_multiplier = 500;
extern int random_seed;
extern int simulate_power_failure;

static void NorError(struct nor_sim *sim)
{
    printf("Nor error on device %s\n", sim->fname);
    while (1) {}
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
    if (sim->initialised) {
        return;
    }
    srand(random_seed);
    sim->remaining_ops = 1000000000;
    sim->remaining_ops =
        (rand() % 10000) * ops_multiplier * YNORSIM_BIT_CHANGES;
    ynorsim_restore_image(sim);
    sim->initialised   = 1;
}

/* Public functions. */

void ynorsim_rd32(struct nor_sim *sim, u32 *addr, u32 *buf, int nwords)
{
    sim = sim;
    while (nwords > 0) {
        *buf = *addr;
        buf++;
        addr++;
        nwords--;
    }
}

void ynorsim_wr_one_word32(struct nor_sim *sim, u32 *addr, u32 val)
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
            tmp  &= ~m;
            *addr = tmp;
            ynorsim_maybe_power_fail(sim);
        }
    }

    *addr = tmp & val;
    ynorsim_maybe_power_fail(sim);
}

void ynorsim_wr32(struct nor_sim *sim, u32 *addr, u32 *buf, int nwords)
{
    while (nwords > 0) {
        ynorsim_wr_one_word32(sim, addr, *buf);
        addr++;
        buf++;
        nwords--;
    }
}

void ynorsim_erase(struct nor_sim *sim, u32 *addr)
{
    /* Todo... bit flipping */
    pios_trace(PIOS_TRACE_TEST, "ynorsim_erase");
    memset(addr, 0xFF, sim->block_size_bytes);
}

struct nor_sim *ynorsim_initialise(char *name, int n_blocks,
                                   int block_size_bytes)
{
    struct nor_sim *sim;

    sim = malloc(sizeof(*sim));
    if (!sim) {
        return NULL;
    }

    memset(sim, 0, sizeof(*sim));
    sim->n_blocks  = n_blocks;
    sim->block_size_bytes = block_size_bytes;
    sim->file_size = n_blocks * block_size_bytes;
    sim->word = malloc(sim->file_size);
    sim->fname     = strdup(name);

    if (!sim->word) {
        return NULL;
    }

    ynorsim_ready(sim);
    return sim;
}

void ynorsim_shutdown(struct nor_sim *sim)
{
    ynorsim_save_image(sim);
    sim->initialised = 0;
}

u32 *ynorsim_get_base(struct nor_sim *sim)
{
    return sim->word;
}
