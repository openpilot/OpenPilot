#include <stdio.h>		/* printf */
#include <stdlib.h>		/* abort */
#include <string.h>		/* memset */

#include "pios_flash.h"		/* PIOS_FLASH_* API */
#include "pios_flash_ut_priv.h"

const struct pios_flash_ut_cfg flash_config = {
	.size_of_flash  = 0x00200000,
	.size_of_sector = 0x00010000,
};

#include "pios_flashfs_logfs_priv.h"

const struct flashfs_logfs_cfg flashfs_config = {
	.fs_magic      = 0x89abceef,
	.total_fs_size = 0x00200000, /* 2M bytes (32 sectors = entire chip) */
	.arena_size    = 0x00010000, /* 256 * slot size */
	.slot_size     = 0x00000100, /* 256 bytes */

	.start_offset  = 0,	     /* start at the beginning of the chip */
	.sector_size   = 0x00010000, /* 64K bytes */
	.page_size     = 0x00000100, /* 256 bytes */
};

#include "pios_flashfs.h"	/* PIOS_FLASHFS_* */

int main (int argc, char * argv[])
{
	int32_t rc;

	/* dd if=/dev/zero bs=1 count=2MiB | tr '\000' '\377' > theflash.bin */

	uintptr_t flash_id;
	rc = PIOS_Flash_UT_Init(&flash_id, &flash_config);
	if (rc != 0) {
		printf ("flash init failed (%d)\n", rc);
		abort();
	}

	uintptr_t fs_id;
	rc = PIOS_FLASHFS_Logfs_Init(&fs_id, &flashfs_config, &pios_ut_flash_driver, flash_id);
	if (rc != 0) {
		printf ("flash filesystem init failed (%d)\n", rc);
		abort();
	}

#define OBJ1_ID 0x12345678
#define OBJ1_SIZE 76
	unsigned char obj1[OBJ1_SIZE];
	memset(obj1, 0xA5, sizeof(obj1));
	for (uint32_t i = 0; i < 10000; i++) {
		rc = PIOS_FLASHFS_ObjSave(fs_id, OBJ1_ID, 0, obj1, sizeof(obj1));
		if (rc != 0) {
			printf ("failed to save obj1 (%d)\n", rc);
			abort();
		}
	}

	unsigned char obj1_check[OBJ1_SIZE];
	memset(obj1_check, 0, sizeof(obj1_check));
	rc = PIOS_FLASHFS_ObjLoad(fs_id, OBJ1_ID, 0, obj1_check, sizeof(obj1_check));
	if (rc != 0) {
		printf ("failed to load obj1 (%d)\n", rc);
		abort();
	}

	if (memcmp(obj1, obj1_check, sizeof(obj1)) != 0) {
		printf ("read-back of obj1 failed\n");
		abort();
	}


	rc = PIOS_FLASHFS_ObjDelete(fs_id, OBJ1_ID, 0);
	if (rc != 0) {
		printf ("failed to delete obj1 (%d)\n", rc);
		abort();
	}

	rc = PIOS_FLASHFS_ObjLoad(fs_id, OBJ1_ID, 0, obj1_check, sizeof(obj1_check));
	if (rc == 0) {
		printf ("was able to load obj1 after delete!\n");
		abort();
	}

}

