#ifndef PIOS_BOARD_INFO_H
#define PIOS_BOARD_INFO_H

#include <stdint.h>		/* uint* */

#define PIOS_BOARD_INFO_BLOB_MAGIC 0xBDBDBDBD

struct pios_board_info {
  uint32_t magic;
  uint8_t  board_type;
  uint8_t  board_rev;
  uint8_t  bl_rev;
  uint8_t  hw_type;
  uint32_t fw_base;
  uint32_t fw_size;
  uint32_t desc_base;
  uint32_t desc_size;
  uint32_t ee_base;
  uint32_t ee_size;
} __attribute__((packed));

extern const struct pios_board_info pios_board_info_blob;

#endif /* PIOS_BOARD_INFO_H */
