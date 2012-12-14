#include <stdint.h>

struct pios_flash_ut_cfg {
	uint32_t size_of_flash;
	uint32_t size_of_sector;
};

int32_t PIOS_Flash_UT_Init(uint32_t * flash_id, const struct pios_flash_ut_cfg * cfg);

extern const struct pios_flash_driver pios_ut_flash_driver;
