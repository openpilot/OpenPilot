#include <stdlib.h>		/* abort */
#include <stdio.h>		/* fopen/fread/fwrite/fseek */
#include <assert.h>		/* assert */
#include <string.h>		/* memset */

#include <stdbool.h>
#include "pios_flash_ut_priv.h"

enum flash_ut_magic {
	FLASH_UT_MAGIC = 0x321dabc1,
};

struct flash_ut_dev {
	enum flash_ut_magic magic;
	const struct pios_flash_ut_cfg * cfg;
	bool transaction_in_progress;
	FILE * flash_file;
};

static struct flash_ut_dev * PIOS_Flash_UT_Alloc(void)
{
	struct flash_ut_dev * flash_dev = malloc(sizeof(struct flash_ut_dev));

	flash_dev->magic = FLASH_UT_MAGIC;

	return flash_dev;
}

int32_t PIOS_Flash_UT_Init(uintptr_t * flash_id, const struct pios_flash_ut_cfg * cfg)
{
	/* Check inputs */
	assert(flash_id);
	assert(cfg);
	assert(cfg->size_of_flash);
	assert(cfg->size_of_sector);
	assert((cfg->size_of_flash % cfg->size_of_sector) == 0);

	struct flash_ut_dev * flash_dev = PIOS_Flash_UT_Alloc();
	assert(flash_dev);

	flash_dev->cfg = cfg;
	flash_dev->transaction_in_progress = false;

	flash_dev->flash_file = fopen ("theflash.bin", "r+");
	if (flash_dev->flash_file == NULL) {
		return -1;
	}

	if (fseek (flash_dev->flash_file, flash_dev->cfg->size_of_flash, SEEK_SET) != 0) {
		return -2;
	}

	*flash_id = (uintptr_t)flash_dev;

	return 0;
}

/**********************************
 *
 * Provide a PIOS flash driver API
 *
 *********************************/
#include "pios_flash.h"

static int32_t PIOS_Flash_UT_StartTransaction(uintptr_t flash_id)
{
	struct flash_ut_dev * flash_dev = (struct flash_ut_dev *)flash_id;

	assert(!flash_dev->transaction_in_progress);

	flash_dev->transaction_in_progress = true;

	return 0;
}

static int32_t PIOS_Flash_UT_EndTransaction(uintptr_t flash_id)
{
	struct flash_ut_dev * flash_dev = (struct flash_ut_dev *)flash_id;

	assert(flash_dev->transaction_in_progress);

	flash_dev->transaction_in_progress = false;

	return 0;
}

static int32_t PIOS_Flash_UT_EraseSector(uintptr_t flash_id, uint32_t addr)
{
	struct flash_ut_dev * flash_dev = (struct flash_ut_dev *)flash_id;

	assert(flash_dev->transaction_in_progress);

	if (fseek (flash_dev->flash_file, addr, SEEK_SET) != 0) {
		assert(0);
	}

	unsigned char * buf = malloc(flash_dev->cfg->size_of_sector);
	assert (buf);
	memset((void *)buf, 0xFF, flash_dev->cfg->size_of_sector);

	size_t s;
	s = fwrite (buf, 1, flash_dev->cfg->size_of_sector, flash_dev->flash_file);

	assert (s == flash_dev->cfg->size_of_sector);

	return 0;
}

static int32_t PIOS_Flash_UT_WriteData(uintptr_t flash_id, uint32_t addr, uint8_t * data, uint16_t len)
{
	/* Check inputs */
	assert(data);

	struct flash_ut_dev * flash_dev = (struct flash_ut_dev *)flash_id;

	assert(flash_dev->transaction_in_progress);

	if (fseek (flash_dev->flash_file, addr, SEEK_SET) != 0) {
		assert(0);
	}

	size_t s;
	s = fwrite (data, 1, len, flash_dev->flash_file);

	assert (s == len);

	return 0;
}

static int32_t PIOS_Flash_UT_ReadData(uintptr_t flash_id, uint32_t addr, uint8_t * data, uint16_t len)
{
	/* Check inputs */
	assert(data);

	struct flash_ut_dev * flash_dev = (struct flash_ut_dev *)flash_id;

	assert(flash_dev->transaction_in_progress);

	if (fseek (flash_dev->flash_file, addr, SEEK_SET) != 0) {
		assert(0);
	}

	size_t s;
	s = fread (data, 1, len, flash_dev->flash_file);

	assert (s == len);

	return 0;
}

/* Provide a flash driver to external drivers */
const struct pios_flash_driver pios_ut_flash_driver = {
	.start_transaction = PIOS_Flash_UT_StartTransaction,
	.end_transaction   = PIOS_Flash_UT_EndTransaction,
	.erase_sector      = PIOS_Flash_UT_EraseSector,
	.write_data        = PIOS_Flash_UT_WriteData,
	.read_data         = PIOS_Flash_UT_ReadData,
};

