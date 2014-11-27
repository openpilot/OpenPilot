#ifndef PIOS_BOARD_INFO_H
#define PIOS_BOARD_INFO_H

#include <stdint.h> /* uint* */

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

struct __attribute__((packed)) fw_version_info {
    uint8_t magic[4];
    uint32_t commit_hash_prefix;
    uint32_t timestamp;
    uint8_t board_type;
    uint8_t board_revision;
    uint8_t commit_tag_name[26];
    uint8_t sha1sum[20];
    uint8_t uavosha1[20];
    uint8_t pad[20];
};

#endif /* PIOS_BOARD_INFO_H */
