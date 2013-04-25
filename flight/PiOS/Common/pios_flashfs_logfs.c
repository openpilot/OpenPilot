/**
 ******************************************************************************
 * @file       pios_flashfs_logfs.c
 * @author     PhoenixPilot, http://github.com/PhoenixPilot, Copyright (C) 2012
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_FLASHFS Flash Filesystem Function
 * @{
 * @brief Log Structured Filesystem for internal or external NOR Flash
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


#include "pios.h"

#ifdef PIOS_INCLUDE_FLASH

#include "openpilot.h"

#include "pios_flashfs_logfs_priv.h"

#include <stdbool.h>

#define MIN(x,y) ((x) < (y) ? (x) : (y))

/*
 * Filesystem state data tracked in RAM
 */

struct logfs_state {
	const struct flashfs_logfs_cfg * cfg;
	bool mounted;
	uint8_t active_arena_id;

	/* NOTE: num_active_slots + num_free_slots will not typically add
	 *       up to the number of slots in the arena since some of the
	 *       slots will be obsolete or otherwise invalidated
	 */
	uint16_t num_free_slots;   /* slots in free state */
	uint16_t num_active_slots; /* slots in active state */

	/* Underlying flash driver glue */
	const struct pios_flash_driver * driver;
	uintptr_t flash_id;
};

static struct logfs_state logfs;

/*
 * Internal Utility functions
 */

/**
 * @brief Return the offset in flash of a particular slot within an arena
 * @return address of the requested slot
 */
static uintptr_t logfs_get_addr(uint8_t arena_id, uint16_t slot_id)
{
	PIOS_Assert(arena_id < (logfs.cfg->total_fs_size / logfs.cfg->arena_size));
	PIOS_Assert(slot_id < (logfs.cfg->arena_size / logfs.cfg->slot_size));

	return (logfs.cfg->start_offset +
		(arena_id * logfs.cfg->arena_size) +
		(slot_id  * logfs.cfg->slot_size));
}

/*
 * The bits within these enum values must progress ONLY
 * from 1 -> 0 so that we can write later ones on top
 * of earlier ones in NOR flash without an erase cycle.
 */
enum arena_state {
	/*
	 * The STM32F30X flash subsystem is only capable of
	 * writing words or halfwords. In this case we use halfwords.
	 * In addition to that it is only capable to write to erased
	 * cells (0xffff) or write a cell from anything to (0x0000).
	 * To cope with this, the F3 needs carefully crafted enum values.
	 * For this to work the underlying flash driver has to
	 * check each halfword if it has changed before writing.
	 */
	ARENA_STATE_ERASED   = 0xFFFFFFFF,
	ARENA_STATE_RESERVED = 0xE6E6FFFF,
	ARENA_STATE_ACTIVE   = 0xE6E66666,
	ARENA_STATE_OBSOLETE = 0x00000000,
};

struct arena_header {
	uint32_t magic;
	enum arena_state state;
} __attribute__((packed));


/****************************************
 * Arena life-cycle transition functions
 ****************************************/

/**
 * @brief Erases all sectors within the given arena and sets arena to erased state.
 * @return 0 if success, < 0 on failure
 * @note Must be called while holding the flash transaction lock
 */
static int32_t logfs_erase_arena(uint8_t arena_id)
{
	uintptr_t arena_addr = logfs_get_addr (arena_id, 0);

	/* Erase all of the sectors in the arena */
	for (uint8_t sector_id = 0;
	     sector_id < (logfs.cfg->arena_size / logfs.cfg->sector_size);
	     sector_id++) {
		if (logfs.driver->erase_sector(logfs.flash_id,
						arena_addr + (sector_id * logfs.cfg->sector_size))) {
			return -1;
		}
	}

	/* Mark this arena as fully erased */
	struct arena_header arena_hdr = {
		.magic = logfs.cfg->fs_magic,
		.state = ARENA_STATE_ERASED,
	};

	if (logfs.driver->write_data(logfs.flash_id,
					arena_addr,
					(uint8_t *)&arena_hdr,
					sizeof(arena_hdr)) != 0) {
		return -2;
	}

	/* Arena is ready to be activated */
	return 0;
}

/**
 * @brief Marks the given arena as reserved so it can be filled.
 * @return 0 if success, < 0 on failure
 * @note Arena must have been previously erased before calling this
 * @note Must be called while holding the flash transaction lock
 */
static int32_t logfs_reserve_arena (uint8_t arena_id)
{
	uintptr_t arena_addr = logfs_get_addr (arena_id, 0);

	/* Read in the current arena header */
	struct arena_header arena_hdr;
	if (logfs.driver->read_data(logfs.flash_id,
					arena_addr,
					(uint8_t *)&arena_hdr,
					sizeof(arena_hdr)) != 0) {
		return -1;
	}
	if (arena_hdr.state != ARENA_STATE_ERASED) {
		/* Arena was not erased, can't reserve it */
		return -2;
	}

	/* Set the arena state to reserved */
	arena_hdr.state = ARENA_STATE_RESERVED;

	/* Write the arena header back to flash */
	if (logfs.driver->write_data(logfs.flash_id,
					arena_addr,
					(uint8_t *)&arena_hdr,
					sizeof(arena_hdr)) != 0) {
		return -3;
	}

	/* Arena is ready to be filled */
	return 0;
}

/**
 * @brief Erases all arenas available to this filesystem instance
 * @return 0 if success, < 0 on failure
 * @note Must be called while holding the flash transaction lock
 */
static int32_t logfs_erase_all_arenas()
{
	uint16_t num_arenas = logfs.cfg->total_fs_size / logfs.cfg->arena_size;

	for (uint16_t arena = 0; arena < num_arenas; arena++) {
	    PIOS_LED_Toggle(PIOS_LED_HEARTBEAT);
		if (logfs_erase_arena(arena) != 0)
			return -1;
	}

	return 0;
}

/**
 * @brief Marks the given arena as active so it can be mounted.
 * @return 0 if success, < 0 on failure
 * @note Arena must have been previously erased or reserved before calling this
 * @note Must be called while holding the flash transaction lock
 */
static int32_t logfs_activate_arena(uint8_t arena_id)
{
	uintptr_t arena_addr = logfs_get_addr(arena_id, 0);

	/* Make sure this arena has been previously erased */
	struct arena_header arena_hdr;
	if (logfs.driver->read_data(logfs.flash_id,
					arena_addr,
					(uint8_t *)&arena_hdr,
					sizeof (arena_hdr)) != 0) {
		/* Failed to read arena header */
		return -1;
	}
	if ((arena_hdr.state != ARENA_STATE_RESERVED) &&
		(arena_hdr.state != ARENA_STATE_ERASED)) {
		/* Arena was not erased or reserved, can't activate it */
		return -2;
	}

	/* Mark this arena as active */
	arena_hdr.state = ARENA_STATE_ACTIVE;
	if (logfs.driver->write_data(logfs.flash_id,
					arena_addr,
					(uint8_t *)&arena_hdr,
					sizeof(arena_hdr)) != 0) {
		return -3;
	}

	/* The arena is now activated and the log may be mounted */
	return 0;
}

/**
 * @brief Marks the given arena as obsolete.
 * @return 0 if success, < 0 on failure
 * @note Arena must have been previously active before calling this
 * @note Must be called while holding the flash transaction lock
 */
static int32_t logfs_obsolete_arena(uint8_t arena_id)
{
	uintptr_t arena_addr = logfs_get_addr (arena_id, 0);

	/* We shouldn't be retiring the currently active arena */
	PIOS_Assert(!logfs.mounted);

	/* Make sure this arena was previously active */
	struct arena_header arena_hdr;
	if (logfs.driver->read_data(logfs.flash_id,
					arena_addr,
					(uint8_t *)&arena_hdr,
					sizeof (arena_hdr)) != 0) {
		/* Failed to read arena header */
		return -1;
	}

	if (arena_hdr.state != ARENA_STATE_ACTIVE) {
		/* Arena was not previously active, can't obsolete it */
		return -2;
	}

	/* Mark this arena as obsolete */
	arena_hdr.state = ARENA_STATE_OBSOLETE;
	if (logfs.driver->write_data(logfs.flash_id,
					arena_addr,
					(uint8_t *)&arena_hdr,
					sizeof(arena_hdr)) != 0) {
		return -3;
	}

	/* Arena is now obsoleted */
	return 0;
}

/**
 * @brief Find the first active arena in flash
 * @return arena_id (>=0) of first active arena
 * @return -1 if no active arena is found
 * @return -2 if failed to read arena header
 * @note Must be called while holding the flash transaction lock
 */
static int32_t logfs_find_active_arena()
{
	/* Search for the lowest numbered active arena */
	for (uint8_t arena_id = 0;
	     arena_id < logfs.cfg->total_fs_size / logfs.cfg->arena_size;
	     arena_id++) {
		uintptr_t arena_addr = logfs_get_addr (arena_id, 0);
		/* Load the arena header */
		struct arena_header arena_hdr;
		if (logfs.driver->read_data(logfs.flash_id,
						arena_addr,
						(uint8_t *)&arena_hdr,
						sizeof (arena_hdr)) != 0) {
			return -2;
		}
		if ((arena_hdr.state == ARENA_STATE_ACTIVE) &&
			(arena_hdr.magic == logfs.cfg->fs_magic)) {
			/* This is the first active arena */
			return arena_id;
		}
	}

	/* Didn't find an active arena */
	return -1;
}

/*
 * The bits within these enum values must progress ONLY
 * from 1 -> 0 so that we can write later ones on top
 * of earlier ones in NOR flash without an erase cycle.
 */
enum slot_state {
	/*
	 * The STM32F30X flash subsystem is only capable of
	 * writing words or halfwords. In this case we use halfwords.
	 * In addition to that it is only capable to write to erased
	 * cells (0xffff) or write a cell from anything to (0x0000).
	 * To cope with this, the F3 needs carfully crafted enum values.
	 * For this to work the underlying flash driver has to
	 * check each halfword if it has changed before writing.
	 */
	SLOT_STATE_EMPTY    = 0xFFFFFFFF,
	SLOT_STATE_RESERVED = 0xFAFAFFFF,
	SLOT_STATE_ACTIVE   = 0xFAFAAAAA,
	SLOT_STATE_OBSOLETE = 0x00000000,
};

struct slot_header {
	enum slot_state state;
	uint32_t obj_id;
	uint16_t obj_inst_id;
	uint16_t obj_size;
} __attribute__((packed));

/* NOTE: Must be called while holding the flash transaction lock */
static int32_t logfs_raw_copy_bytes (uintptr_t src_addr, uint16_t src_size, uintptr_t dst_addr)
{
#define RAW_COPY_BLOCK_SIZE 16
	uint8_t data_block[RAW_COPY_BLOCK_SIZE];

	while (src_size) {
		uint16_t blk_size;
		if (src_size >= RAW_COPY_BLOCK_SIZE) {
			/* Copy a full block */
			blk_size = RAW_COPY_BLOCK_SIZE;
		} else {
			/* Copy the remainder */
			blk_size = src_size;
		}

		/* Read a block of data from source */
		if (logfs.driver->read_data(logfs.flash_id,
						src_addr,
						data_block,
						blk_size) != 0) {
			/* Failed to read next chunk from source */
			return -1;
		}

		/* Write a block of data to destination */
		if (logfs.driver->write_data(logfs.flash_id,
						dst_addr,
						data_block,
						blk_size) != 0) {
			/* Failed to write chunk to destination */
			return -2;
		}

		/* Update the src/dst pointers */
		src_size -= blk_size;
		src_addr += blk_size;
		dst_addr += blk_size;
	}

	return 0;
}

/*
 * Is the entire filesystem full?
 * true = all slots in the arena are in the ACTIVE state (ie. garbage collection won't free anything)
 * false = some slots in the arena are either currently free or could be free'd by garbage collection
 */
static bool logfs_fs_is_full(void)
{
	return (logfs.num_active_slots == (logfs.cfg->arena_size / logfs.cfg->slot_size) - 1);
}

/*
 * Is the log full?
 * true = there are no unwritten slots left in the log (garbage collection may or may not help)
 * false = there are still some entirely unused slots left in the log
 */
static bool logfs_log_is_full(void)
{
	return (logfs.num_free_slots == 0);
}

static int32_t logfs_unmount_log(void)
{
	PIOS_Assert (logfs.mounted);

	logfs.num_active_slots = 0;
	logfs.num_free_slots   = 0;
	logfs.mounted          = false;

	return 0;
}

static int32_t logfs_mount_log(uint8_t arena_id)
{
	PIOS_Assert (!logfs.mounted);

	logfs.num_active_slots = 0;
	logfs.num_free_slots   = 0;
	logfs.active_arena_id  = arena_id;

	/* Scan the log to find out how full it is */
	for (uint16_t slot_id = 1;
	     slot_id < (logfs.cfg->arena_size / logfs.cfg->slot_size);
	     slot_id++) {
		struct slot_header slot_hdr;
		uintptr_t slot_addr = logfs_get_addr (logfs.active_arena_id, slot_id);
		if (logfs.driver->read_data(logfs.flash_id,
						slot_addr,
						(uint8_t *)&slot_hdr,
						sizeof (slot_hdr)) != 0) {
			return -1;
		}

		/*
		 * Empty slots must be in a continguous block at the
		 * end of the arena.
		 */
		PIOS_Assert (slot_hdr.state == SLOT_STATE_EMPTY ||
			logfs.num_free_slots == 0);

		switch (slot_hdr.state) {
		case SLOT_STATE_EMPTY:
			logfs.num_free_slots++;
			break;
		case SLOT_STATE_ACTIVE:
			logfs.num_active_slots++;
			break;
		case SLOT_STATE_RESERVED:
		case SLOT_STATE_OBSOLETE:
			break;
		}
	}

	/* Scan is complete, mark the arena mounted */
	logfs.active_arena_id = arena_id;
	logfs.mounted = true;

	return 0;
}

/**
 * @brief Initialize the flash object setting FS
 * @return 0 if success, -1 if failure
 */
int32_t PIOS_FLASHFS_Logfs_Init(uintptr_t * fs_id, const struct flashfs_logfs_cfg * cfg, const struct pios_flash_driver * driver, uintptr_t flash_id)
{
	PIOS_Assert(cfg);
	PIOS_Assert(fs_id);
	PIOS_Assert(driver);

	/* We must have at least 2 arenas for garbage collection to work */
	PIOS_Assert((cfg->total_fs_size / cfg->arena_size > 1));

	/* Make sure the underlying flash driver provides the minimal set of required methods */
	PIOS_Assert(driver->start_transaction);
	PIOS_Assert(driver->end_transaction);
	PIOS_Assert(driver->erase_sector);
	PIOS_Assert(driver->write_data);
	PIOS_Assert(driver->read_data);

	/* Bind configuration parameters to this filesystem instance */
	logfs.cfg      = cfg;	/* filesystem configuration */
	logfs.driver   = driver; /* lower-level flash driver */
	logfs.flash_id = flash_id; /* lower-level flash device id */
	logfs.mounted  = false;

	int8_t rc;

	if (logfs.driver->start_transaction(logfs.flash_id) != 0) {
		rc = -1;
		goto out_exit;
	}

	bool found = false;
	int32_t arena_id;
	for (uint8_t try = 0; !found && try < 2; try++) {
		/* Find the active arena */
		arena_id = logfs_find_active_arena();
		if (arena_id >= 0) {
			/* Found the active arena */
			found = true;
			break;
		} else {
			/* No active arena found, erase and activate arena 0 */
			if (logfs_erase_arena(0) != 0)
				break;
			if (logfs_activate_arena(0) != 0)
				break;
		}
	}

	if (!found) {
		/* Still no active arena, something is broken */
		rc = -2;
		goto out_end_trans;
	}

	/* We've found an active arena, mount it */
	if (logfs_mount_log(arena_id) != 0) {
		/* Failed to mount the log, something is broken */
		rc = -3;
		goto out_end_trans;
	}

	/* Log has been mounted */
	rc = 0;

	*fs_id = (uintptr_t) &logfs;

out_end_trans:
	logfs.driver->end_transaction(logfs.flash_id);

out_exit:
	return rc;
}

/* NOTE: Must be called while holding the flash transaction lock */
static int32_t logfs_garbage_collect (void) {
	PIOS_Assert (logfs.mounted);

	/* Source arena is the active arena */
	uint8_t src_arena_id = logfs.active_arena_id;

	/* Compute destination arena */
	uint8_t dst_arena_id = (logfs.active_arena_id + 1) % (logfs.cfg->total_fs_size / logfs.cfg->arena_size);

	/* Erase destination arena */
	if (logfs_erase_arena (dst_arena_id) != 0) {
		return -1;
	}

	/* Reserve the destination arena so we can start filling it */
	if (logfs_reserve_arena (dst_arena_id) != 0) {
		/* Unable to reserve the arena */
		return -2;
	}

	/* Copy active slots from active arena to destination arena */
	uint16_t dst_slot_id = 1;
	for (uint16_t src_slot_id = 1;
	     src_slot_id < (logfs.cfg->arena_size / logfs.cfg->slot_size);
	     src_slot_id++) {
		struct slot_header slot_hdr;
		uintptr_t src_addr = logfs_get_addr (src_arena_id, src_slot_id);
		if (logfs.driver->read_data(logfs.flash_id,
						src_addr,
						(uint8_t *)&slot_hdr,
						sizeof (slot_hdr)) != 0) {
			return -3;
		}

		if (slot_hdr.state == SLOT_STATE_ACTIVE) {
			uintptr_t dst_addr = logfs_get_addr (dst_arena_id, dst_slot_id);
			if (logfs_raw_copy_bytes(src_addr,
							sizeof(slot_hdr) + slot_hdr.obj_size,
							dst_addr) != 0) {
				/* Failed to copy all bytes */
				return -4;
			}
			dst_slot_id++;
		}
	}

	/* Activate the destination arena */
	if (logfs_activate_arena (dst_arena_id) != 0) {
		return -5;
	}

	/* Unmount the source arena */
	if (logfs_unmount_log () != 0) {
		return -6;
	}

	/* Obsolete the source arena */
	if (logfs_obsolete_arena (src_arena_id) != 0) {
		return -7;
	}

	/* Mount the new arena */
	if (logfs_mount_log (dst_arena_id) != 0) {
		return -8;
	}

	return 0;
}

/* NOTE: Must be called while holding the flash transaction lock */
static int16_t logfs_object_find_next (struct slot_header * slot_hdr, uint16_t * curr_slot, uint32_t obj_id, uint16_t obj_inst_id)
{
	PIOS_Assert(slot_hdr);
	PIOS_Assert(curr_slot);

	/* First slot in the arena is reserved for arena header, skip it. */
	if (*curr_slot == 0) *curr_slot = 1;

	for (uint16_t slot_id = *curr_slot;
	     slot_id < (logfs.cfg->arena_size / logfs.cfg->slot_size);
	     slot_id++) {
		uintptr_t slot_addr = logfs_get_addr (logfs.active_arena_id, slot_id);

		if (logfs.driver->read_data(logfs.flash_id,
						slot_addr,
						(uint8_t *)slot_hdr,
						sizeof (*slot_hdr)) != 0) {
			return -2;
		}
		if (slot_hdr->state == SLOT_STATE_EMPTY) {
			/* We hit the end of the log */
			break;
		}
		if (slot_hdr->state == SLOT_STATE_ACTIVE &&
			slot_hdr->obj_id      == obj_id &&
			slot_hdr->obj_inst_id == obj_inst_id) {
			/* Found what we were looking for */
			*curr_slot = slot_id;
			return 0;
		}
	}

	/* No matching entry was found */
	return -1;
}

/* NOTE: Must be called while holding the flash transaction lock */
/* OPTIMIZE: could trust that there is at most one active version of every object and terminate the search when we find one */
static int8_t logfs_delete_object (uint32_t obj_id, uint16_t obj_inst_id)
{
	int8_t rc;

	bool more = true;
	uint16_t curr_slot_id = 0;
	do {
		struct slot_header slot_hdr;
		switch (logfs_object_find_next (&slot_hdr, &curr_slot_id, obj_id, obj_inst_id)) {
		case 0:
			/* Found a matching slot.  Obsolete it. */
			slot_hdr.state = SLOT_STATE_OBSOLETE;
			uintptr_t slot_addr = logfs_get_addr (logfs.active_arena_id, curr_slot_id);

			if (logfs.driver->write_data(logfs.flash_id,
							slot_addr,
							(uint8_t *)&slot_hdr,
							sizeof(slot_hdr)) != 0) {
				rc = -2;
				goto out_exit;
			}
			/* Object has been successfully obsoleted and is no longer active */
			logfs.num_active_slots--;
			break;
		case -1:
			/* Search completed, object not found */
			more = false;
			rc = 0;
			break;
		default:
			/* Error occurred during search */
			rc = -1;
			goto out_exit;
		}
	} while (more);

out_exit:
	return rc;
}

/* NOTE: Must be called while holding the flash transaction lock */
static int8_t logfs_reserve_free_slot (uint16_t * slot_id, struct slot_header * slot_hdr, uint32_t obj_id, uint16_t obj_inst_id, uint16_t obj_size)
{
	PIOS_Assert(slot_id);
	PIOS_Assert(slot_hdr);

	if (logfs.num_free_slots < 1) {
		/* No free slots to allocate */
		return -1;
	}

	if (obj_size > (logfs.cfg->slot_size - sizeof (slot_hdr))) {
		/* This object is too big for the slot */
		return -2;
	}

	uint16_t candidate_slot_id = (logfs.cfg->arena_size / logfs.cfg->slot_size) - logfs.num_free_slots;
	PIOS_Assert(candidate_slot_id > 0);

	uintptr_t slot_addr = logfs_get_addr (logfs.active_arena_id, candidate_slot_id);

	if (logfs.driver->read_data(logfs.flash_id,
					slot_addr,
					(uint8_t *)slot_hdr,
					sizeof (*slot_hdr)) != 0) {
		/* Failed to read slot header for candidate slot */
		return -3;
	}

	if (slot_hdr->state != SLOT_STATE_EMPTY) {
		/* Candidate slot isn't empty!  Something is broken. */
		PIOS_DEBUG_Assert(0);
		return -4;
	}

	/* Mark this slot as RESERVED */
	slot_hdr->state       = SLOT_STATE_RESERVED;
	slot_hdr->obj_id      = obj_id;
	slot_hdr->obj_inst_id = obj_inst_id;
	slot_hdr->obj_size    = obj_size;

	if (logfs.driver->write_data(logfs.flash_id,
					slot_addr,
					(uint8_t *)slot_hdr,
					sizeof(*slot_hdr)) != 0) {
		/* Failed to write the slot header */
		return -5;
	}

	/* FIXME: If the header write (above) failed, may have partially written data, thus corrupting that slot but we would have missed decrementing this counter */
	logfs.num_free_slots--;

	*slot_id = candidate_slot_id;
	return 0;
}

/* NOTE: Must be called while holding the flash transaction lock */
static int8_t logfs_append_to_log (uint32_t obj_id, uint16_t obj_inst_id, uint8_t * obj_data, uint16_t obj_size)
{
	/* Reserve a free slot for our new object */
	uint16_t free_slot_id;
	struct slot_header slot_hdr;
	if (logfs_reserve_free_slot (&free_slot_id, &slot_hdr, obj_id, obj_inst_id, obj_size) != 0) {
		/* Failed to reserve a free slot */
		return -1;
	}

	/* Compute slot address */
	uintptr_t slot_addr = logfs_get_addr (logfs.active_arena_id, free_slot_id);

	/* Write the data into the reserved slot, starting after the slot header */
	uintptr_t slot_offset = sizeof(slot_hdr);
	while (obj_size > 0) {
		/* Individual writes must fit entirely within a single page buffer. */
		uint16_t page_remaining = logfs.cfg->page_size - (slot_offset % logfs.cfg->page_size);
		uint16_t write_size = MIN(obj_size, page_remaining);
		if (logfs.driver->write_data (logfs.flash_id,
						slot_addr + slot_offset,
						obj_data,
						write_size) != 0) {
			/* Failed to write the object data to the slot */
			return -2;
		}

		/* Update our accounting */
		obj_data    += write_size;
		slot_offset += write_size;
		obj_size    -= write_size;
	}

	/* Mark this slot active in one atomic step */
	slot_hdr.state = SLOT_STATE_ACTIVE;
	if (logfs.driver->write_data (logfs.flash_id,
					slot_addr,
					(uint8_t *)&slot_hdr,
					sizeof(slot_hdr)) != 0) {
		/* Failed to mark the slot active */
		return -4;
	}

	/* Object has been successfully written to the slot */
	logfs.num_active_slots++;
	return 0;
}


/**********************************
 *
 * Provide a PIOS_FLASHFS_* driver
 *
 *********************************/
#include "pios_flashfs.h"	/* API for flash filesystem */

/**
 * @brief Saves one object instance to the filesystem
 * @param[in] fs_id The filesystem to use for this action
 * @param[in] obj UAVObject ID of the object to save
 * @param[in] obj_inst_id The instance number of the object being saved
 * @param[in] obj_data Contents of the object being saved
 * @param[in] obj_size Size of the object being saved
 * @return 0 if success or error code
 * @retval -1 if failed to start transaction
 * @retval -2 if failure to delete any previous versions of the object
 * @retval -3 if filesystem is entirely full and garbage collection won't help
 * @retval -4 if garbage collection failed
 * @retval -5 if filesystem is full even after garbage collection should have freed space
 * @retval -6 if writing the new object to the filesystem failed
 */
int32_t PIOS_FLASHFS_ObjSave(uint32_t fs_id, uint32_t obj_id, uint16_t obj_inst_id, uint8_t * obj_data, uint16_t obj_size)
{
	int8_t rc;

	PIOS_Assert(obj_size <= (logfs.cfg->slot_size - sizeof(struct slot_header)));

	if (logfs.driver->start_transaction(logfs.flash_id) != 0) {
		rc = -1;
		goto out_exit;
	}

	if (logfs_delete_object (obj_id, obj_inst_id) != 0) {
		rc = -2;
		goto out_end_trans;
	}

	/*
	 * All old versions of this object + instance have been invalidated.
	 * Write the new object.
	 */

	/* Check if the arena is entirely full. */
	if (logfs_fs_is_full()) {
		/* Note: Filesystem Full means we're full of *active* records so gc won't help at all. */
		rc = -3;
		goto out_end_trans;
	}

	/* Is garbage collection required? */
	if (logfs_log_is_full()) {
		/* Note: Log Full means the log is full but may contain obsolete slots so gc may free some space */
		if (logfs_garbage_collect() != 0) {
			rc = -4;
			goto out_end_trans;
		}
		/* Check one more time just to be sure we actually free'd some space */
		if (logfs_log_is_full()) {
			/*
			 * Log is still full even after gc!
			 * NOTE: This should not happen since the filesystem wasn't full
			 *       when we checked above so gc should have helped.
			 */
			PIOS_DEBUG_Assert(0);
			rc = -5;
			goto out_end_trans;
		}
	}

	/* We have room for our new object.  Append it to the log. */
	if (logfs_append_to_log(obj_id, obj_inst_id, obj_data, obj_size) != 0) {
		/* Error during append */
		rc = -6;
		goto out_end_trans;
	}

	/* Object successfully written to the log */
	rc = 0;

out_end_trans:
	logfs.driver->end_transaction(logfs.flash_id);

out_exit:
	return rc;
}

/**
 * @brief Load one object instance from the filesystem
 * @param[in] fs_id The filesystem to use for this action
 * @param[in] obj UAVObject ID of the object to load
 * @param[in] obj_inst_id The instance of the object to load
 * @param[in] obj_data Buffer to hold the contents of the loaded object
 * @param[in] obj_size Size of the object to be loaded
 * @return 0 if success or error code
 * @retval -1 if failed to start transaction
 * @retval -2 if object not found in filesystem
 * @retval -3 if object size in filesystem does not exactly match buffer size
 * @retval -4 if reading the object data from flash fails
 */
int32_t PIOS_FLASHFS_ObjLoad(uint32_t fs_id, uint32_t obj_id, uint16_t obj_inst_id, uint8_t * obj_data, uint16_t obj_size)
{
	int8_t rc;

	PIOS_Assert(obj_size <= (logfs.cfg->slot_size - sizeof(struct slot_header)));

	if (logfs.driver->start_transaction(logfs.flash_id) != 0) {
		rc = -1;
		goto out_exit;
	}

	/* Find the object in the log */
	uint16_t slot_id = 0;
	struct slot_header slot_hdr;
	if (logfs_object_find_next (&slot_hdr, &slot_id, obj_id, obj_inst_id) != 0) {
		/* Object does not exist in fs */
		rc = -2;
		goto out_end_trans;
	}

	/* Sanity check what we've found */
	if (slot_hdr.obj_size != obj_size) {
		/* Object sizes don't match.  Not safe to copy contents. */
		rc = -3;
		goto out_end_trans;
	}

	/* Read the contents of the object from the log */
	if (obj_size > 0) {
		uintptr_t slot_addr = logfs_get_addr (logfs.active_arena_id, slot_id);
		if (logfs.driver->read_data(logfs.flash_id,
						slot_addr + sizeof(slot_hdr),
						(uint8_t *)obj_data,
						obj_size) != 0) {
			/* Failed to read object data from the log */
			rc = -4;
			goto out_end_trans;
		}
	}

	/* Object successfully loaded */
	rc = 0;

out_end_trans:
	logfs.driver->end_transaction(logfs.flash_id);

out_exit:
	return rc;
}

/**
 * @brief Delete one instance of an object from the filesystem
 * @param[in] fs_id The filesystem to use for this action
 * @param[in] obj UAVObject ID of the object to delete
 * @param[in] obj_inst_id The instance of the object to delete
 * @return 0 if success or error code
 * @retval -1 if failed to start transaction
 * @retval -2 if failed to delete the object from the filesystem
 */
int32_t PIOS_FLASHFS_ObjDelete(uint32_t fs_id, uint32_t obj_id, uint16_t obj_inst_id)
{
	int8_t rc;

	if (logfs.driver->start_transaction(logfs.flash_id) != 0) {
		rc = -1;
		goto out_exit;
	}

	if (logfs_delete_object (obj_id, obj_inst_id) != 0) {
		rc = -2;
		goto out_end_trans;
	}

	/* Object successfully deleted from the log */
	rc = 0;

out_end_trans:
	logfs.driver->end_transaction(logfs.flash_id);

out_exit:
	return rc;
}

/**
 * @brief Erases all filesystem arenas and activate the first arena
 * @param[in] fs_id The filesystem to use for this action
 * @return 0 if success or error code
 * @retval -1 if failed to start transaction
 * @retval -2 if failed to erase all arenas
 * @retval -3 if failed to activate arena 0
 * @retval -4 if failed to mount arena 0
 */
int32_t PIOS_FLASHFS_Format(uint32_t fs_id)
{
	int32_t rc;

	if (logfs.mounted) {
		logfs_unmount_log();
	}

	if (logfs.driver->start_transaction(logfs.flash_id) != 0) {
		rc = -1;
		goto out_exit;
	}

	if (logfs_erase_all_arenas() != 0) {
		rc = -2;
		goto out_end_trans;
	}

	/* Reinitialize arena 0 */
	if (logfs_activate_arena(0) != 0) {
		rc = -3;
		goto out_end_trans;
	}

	/* Mount arena 0 */
	if (logfs_mount_log(0) != 0) {
		rc = -4;
		goto out_end_trans;
	}

	/* Chip erased and log remounted successfully */
	rc = 0;

out_end_trans:
	logfs.driver->end_transaction(logfs.flash_id);

out_exit:
	return rc;
}

#endif /* PIOS_INCLUDE_FLASH */

/**
 * @}
 * @}
 */
