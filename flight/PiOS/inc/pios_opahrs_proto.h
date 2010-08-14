/**
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_OPAHRS OPAHRS Functions
 * @{
 *
 * @file       pios_opahrs_proto.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      PPM Input functions
 * @see        The GNU Public License (GPL) Version 3
 * 
 */

#ifndef PIOS_OPAHRS_PROTO_H
#define PIOS_OPAHRS_PROTO_H

#include <stdint.h>

enum opahrs_msg_type {
  OPAHRS_MSG_TYPE_LINK = 0x00,

  OPAHRS_MSG_TYPE_USER_V0,
  OPAHRS_MSG_TYPE_USER_V1,
};

enum opahrs_msg_link_state {
  OPAHRS_MSG_LINK_STATE_INACTIVE,
  OPAHRS_MSG_LINK_STATE_BUSY,
  OPAHRS_MSG_LINK_STATE_READY,
};

#define OPAHRS_MSG_MAGIC_HEAD  0x53524841 /* ASCII "AHRS" */
struct opahrs_msg_link_head {
  uint32_t             magic;	/* Set to OPAHRS_MSG_MAGIC_HEAD */
  enum opahrs_msg_type type;
} __attribute__((__packed__));

#define OPAHRS_MSG_MAGIC_TAIL  0xFEFE
struct opahrs_msg_link_tail {
  uint16_t magic;
  uint8_t  crc8;
} __attribute__((__packed__));

enum opahrs_msg_link_tag {
  OPAHRS_MSG_LINK_TAG_NOP,
};

#define SPI_MSG_LINK_ERROR_BADCRC 0x00000001
struct opahrs_msg_link {
  enum opahrs_msg_link_state state;
  uint16_t                   errors;

  enum opahrs_msg_link_tag   t;
  union {
  } v;
} __attribute__((__packed__));

/********
 * SPI protocol v0 definitions
 *
 * This protocol version is NOT to be changed after release since it is the
 * protocol used for upgrading the firmware of AHRS boards that are already
 * in end user hands.
 *
 ********/

struct opahrs_msg_v0_req_nop {
} __attribute__((__packed__));

struct opahrs_msg_v0_req_sync {
  uint32_t cookie;
} __attribute__((__packed__));

struct opahrs_msg_v0_req_serial {
} __attribute__((__packed__));

struct opahrs_msg_v0_req_reset {
  uint32_t reset_delay_in_ms;
} __attribute__((__packed__));

struct opahrs_msg_v0_req_switch_proto {
} __attribute__((__packed__));

struct opahrs_msg_v0_req_fwup_start {
} __attribute__((__packed__));

struct opahrs_msg_v0_req_fwup_data {
} __attribute__((__packed__));

struct opahrs_msg_v0_req_fwup_verify {
} __attribute__((__packed__));

union opahrs_msg_v0_req {
  /* Mandatory for all bootloader and all application loads */
  struct opahrs_msg_v0_req_nop          nop;
  struct opahrs_msg_v0_req_sync         sync;
  struct opahrs_msg_v0_req_serial       serial;
  struct opahrs_msg_v0_req_reset        reset;

  /* Only implemented by the application */
  struct opahrs_msg_v0_req_switch_proto switch_proto;

  /* Only implemented by bootloaders */
  struct opahrs_msg_v0_req_fwup_start   fwup_start;
  struct opahrs_msg_v0_req_fwup_data    fwup_data;
  struct opahrs_msg_v0_req_fwup_verify  fwup_verify;
} __attribute__((__packed__));

struct opahrs_msg_v0_rsp_sync {
  uint8_t  i_am_a_bootloader;
  uint8_t  hw_version;
  uint16_t bl_version;
  uint32_t fw_version;

  uint32_t cookie;
  uint32_t alternate_proto_version;
} __attribute__((__packed__));

struct opahrs_msg_v0_rsp_serial {
  uint8_t  serial_bcd[24];
} __attribute__((__packed__));

struct opahrs_msg_v0_rsp_fwup_status {
} __attribute__((__packed__));

union opahrs_msg_v0_rsp {
  /* Mandatory for all bootloader and all application loads */
  struct opahrs_msg_v0_rsp_sync        sync;
  struct opahrs_msg_v0_rsp_serial      serial;

  /* Only implemented by bootloaders */
  struct opahrs_msg_v0_rsp_fwup_status fwup_status;
} __attribute__((__packed__));

enum opahrs_msg_v0_tag {
  OPAHRS_MSG_V0_REQ_NOP = 0x00,
  OPAHRS_MSG_V0_REQ_SYNC,
  OPAHRS_MSG_V0_REQ_RESET,
  OPAHRS_MSG_V0_REQ_SERIAL,
  OPAHRS_MSG_V0_REQ_SWITCH_PROTO,

  OPAHRS_MSG_V0_REQ_FWUP_START,
  OPAHRS_MSG_V0_REQ_FWUP_DATA,
  OPAHRS_MSG_V0_REQ_FWUP_VERIFY,

  OPAHRS_MSG_V0_RSP_SYNC,
  OPAHRS_MSG_V0_RSP_SERIAL,
  OPAHRS_MSG_V0_RSP_FWUP_STATUS,
};

struct opahrs_msg_v0_payload {
  enum opahrs_msg_v0_tag t;
  union {
    union opahrs_msg_v0_req req;
    union opahrs_msg_v0_rsp rsp;
  } v;
} __attribute__((__packed__));

struct opahrs_msg_v0 {
  struct opahrs_msg_link_head head;
  union {
    struct opahrs_msg_link       link;
    struct opahrs_msg_v0_payload user;
  } payload;
  struct opahrs_msg_link_tail    tail;
} __attribute__((__packed__));


/********
 * SPI protocol v1 definitions
 ********/

struct opahrs_msg_v1_req_nop {
} __attribute__((__packed__));

struct opahrs_msg_v1_req_sync {
  uint32_t cookie;
} __attribute__((__packed__));

struct opahrs_msg_v1_req_reset {
  uint32_t reset_delay_in_ms;
} __attribute__((__packed__));

struct opahrs_msg_v1_req_serial {
} __attribute__((__packed__));

struct opahrs_msg_v1_req_attituderaw {
} __attribute__((__packed__));

struct opahrs_msg_v1_req_attitude {
} __attribute__((__packed__));

union opahrs_msg_v1_req {
  struct opahrs_msg_v1_req_nop          nop;
  struct opahrs_msg_v1_req_sync         sync;
  struct opahrs_msg_v1_req_reset        reset;
  struct opahrs_msg_v1_req_serial       serial;
  struct opahrs_msg_v1_req_attituderaw  attituderaw;
  struct opahrs_msg_v1_req_attitude     attitude;
} __attribute__((__packed__));

struct opahrs_msg_v1_rsp_sync {
  uint8_t  i_am_a_bootloader;
  uint8_t  hw_version;
  uint16_t bl_version;
  uint32_t fw_version;

  uint32_t cookie;
} __attribute__((__packed__));

struct opahrs_msg_v1_rsp_serial {
  uint8_t  serial_bcd[25];
} __attribute__((__packed__));

struct opahrs_msg_v1_rsp_attituderaw {
  struct {
    int16_t x;
    int16_t y;
    int16_t z;
  } mags;
  struct {
    uint16_t x;
    uint16_t y;
    uint16_t z;
    uint16_t xy_temp;
    uint16_t z_temp;
  } gyros;
  struct {
    uint16_t x;
    uint16_t y;
    uint16_t z;
  } accels;
} __attribute__((__packed__));

struct opahrs_msg_v1_rsp_attitude {
  struct {
    float q1;
    float q2;
    float q3;
    float q4;
  } quaternion;
  struct {
    float roll;
    float pitch;
    float yaw;
  } euler;
} __attribute__((__packed__));

union opahrs_msg_v1_rsp {
  struct opahrs_msg_v1_rsp_sync         sync;
  struct opahrs_msg_v1_rsp_serial       serial;
  struct opahrs_msg_v1_rsp_attituderaw  attituderaw;
  struct opahrs_msg_v1_rsp_attitude     attitude;
} __attribute__((__packed__));

enum opahrs_msg_v1_tag {
  OPAHRS_MSG_V1_REQ_NOP = 0x02000000,
  OPAHRS_MSG_V1_REQ_SYNC,
  OPAHRS_MSG_V1_REQ_RESET,
  OPAHRS_MSG_V1_REQ_SERIAL,
  OPAHRS_MSG_V1_REQ_ATTITUDERAW,
  OPAHRS_MSG_V1_REQ_ATTITUDE,

  OPAHRS_MSG_V1_RSP_SYNC,
  OPAHRS_MSG_V1_RSP_SERIAL,
  OPAHRS_MSG_V1_RSP_ATTITUDERAW,
  OPAHRS_MSG_V1_RSP_ATTITUDE,
};

struct opahrs_msg_v1_payload {
  enum opahrs_msg_v1_tag t;
  union {
    union opahrs_msg_v1_req req;
    union opahrs_msg_v1_rsp rsp;
  } v;
} __attribute__((__packed__));

struct opahrs_msg_v1 {
  struct opahrs_msg_link_head head;
  union {
    struct opahrs_msg_link       link;
    struct opahrs_msg_v1_payload user;
  } payload;
  struct opahrs_msg_link_tail tail;
} __attribute__((__packed__));

/* Helper functions for setting up messages */
extern void opahrs_msg_v0_init_rx (struct opahrs_msg_v0 * msg);
extern void opahrs_msg_v0_init_user_tx (struct opahrs_msg_v0 * msg, enum opahrs_msg_v0_tag tag);
extern void opahrs_msg_v0_init_link_tx (struct opahrs_msg_v0 * msg, enum opahrs_msg_link_tag tag);
extern void opahrs_msg_v1_init_rx (struct opahrs_msg_v1 * msg);
extern void opahrs_msg_v1_init_user_tx (struct opahrs_msg_v1 * msg, enum opahrs_msg_v1_tag tag);
extern void opahrs_msg_v1_init_link_tx (struct opahrs_msg_v1 * msg, enum opahrs_msg_link_tag tag);

#endif /* PIOS_OPAHRS_PROTO_H */

/**
  * @}
  * @}
  */
