#include <pios.h>
#include <pios_board.h>

#include "pios_board_info.h"

const struct pios_board_info __attribute__((__used__)) __attribute__((__section__(".boardinfo"))) pios_board_info_blob = {
  .magic      = PIOS_BOARD_INFO_BLOB_MAGIC,
  .board_type = BOARD_TYPE,
  .board_rev  = BOARD_REVISION,
  .bl_rev     = BOOTLOADER_VERSION,
  .hw_type    = HW_TYPE,
  .fw_base    = START_OF_USER_CODE,
  .fw_size    = SIZE_OF_CODE,
  .desc_base  = START_OF_USER_CODE + SIZE_OF_CODE,
  .desc_size  = SIZE_OF_DESCRIPTION,
};
