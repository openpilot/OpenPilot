#include <pios.h>
#include <pios_board.h>

#include "pios_board_info.h"

const struct pios_board_info __attribute__((__used__)) __attribute__((__section__(".boardinfo"))) pios_board_info_blob = {
  .magic      = PIOS_BOARD_INFO_BLOB_MAGIC,
  .board_type = BOARD_TYPE,
  .board_rev  = BOARD_REVISION,
  .bl_rev     = BOOTLOADER_VERSION,
  .hw_type    = HW_TYPE,
  .fw_base    = FW_BANK_BASE,
  .fw_size    = FW_BANK_SIZE - FW_DESC_SIZE,
  .desc_base  = FW_BANK_BASE + FW_BANK_SIZE - FW_DESC_SIZE,
  .desc_size  = FW_DESC_SIZE,
#ifdef EE_BANK_BASE
  .ee_base    = EE_BANK_BASE,
  .ee_size    = EE_BANK_SIZE,
#endif
};
