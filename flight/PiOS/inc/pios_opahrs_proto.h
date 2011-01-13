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

#define OPAHRS_MSG_MAGIC_HEAD  0x53524841	/* ASCII "AHRS" */
struct opahrs_msg_link_head {
	uint32_t magic;		/* Set to OPAHRS_MSG_MAGIC_HEAD */
	enum opahrs_msg_type type;
} __attribute__ ((__packed__));

#define OPAHRS_MSG_MAGIC_TAIL  0xFEFE
struct opahrs_msg_link_tail {
	uint16_t magic;
	uint8_t crc8;
} __attribute__ ((__packed__));

enum opahrs_msg_link_tag {
	OPAHRS_MSG_LINK_TAG_NOP,
};

#define SPI_MSG_LINK_ERROR_BADCRC 0x00000001
struct opahrs_msg_link {
	enum opahrs_msg_link_state state;
	uint16_t errors;

	enum opahrs_msg_link_tag t;
	union {
	} v;
} __attribute__ ((__packed__));

/********
 * SPI protocol v0 definitions
 *
 * This protocol version is NOT to be changed after release since it is the
 * protocol used for upgrading the firmware of AHRS boards that are already
 * in end user hands.
 *
 ********/

struct opahrs_msg_v0_req_nop {
} __attribute__ ((__packed__));

struct opahrs_msg_v0_req_versions {
} __attribute__ ((__packed__));

struct opahrs_msg_v0_req_reset {
	uint32_t reset_delay_in_ms;
} __attribute__ ((__packed__));

struct opahrs_msg_v0_req_boot {
	uint32_t boot_delay_in_ms;
} __attribute__ ((__packed__));

struct opahrs_msg_v0_req_serial {
} __attribute__ ((__packed__));

struct opahrs_msg_v0_req_fwup_start {
} __attribute__ ((__packed__));

struct opahrs_msg_v0_req_fwup_status {
} __attribute__ ((__packed__));

struct opahrs_msg_v0_req_fwup_data {
	uint32_t adress;
	uint32_t data[14];
	uint8_t size;
} __attribute__ ((__packed__));

struct opahrs_msg_v0_req_fwdn_data {
	uint32_t adress;
} __attribute__ ((__packed__));

struct opahrs_msg_v0_req_mem_map {
} __attribute__ ((__packed__));

struct opahrs_msg_v0_req_fwup_verify {
} __attribute__ ((__packed__));

union opahrs_msg_v0_req {
	struct opahrs_msg_v0_req_nop nop;
	struct opahrs_msg_v0_req_versions versions;
	struct opahrs_msg_v0_req_reset reset;
	struct opahrs_msg_v0_req_boot boot;
	struct opahrs_msg_v0_req_serial serial;
	struct opahrs_msg_v0_req_fwup_status status_req;
	struct opahrs_msg_v0_req_fwdn_data fwdn_data;
	struct opahrs_msg_v0_req_mem_map mem_map;
	struct opahrs_msg_v0_req_fwup_start fwup_start;
	struct opahrs_msg_v0_req_fwup_data fwup_data;
	struct opahrs_msg_v0_req_fwup_verify fwup_verify;
} __attribute__ ((__packed__));

struct opahrs_msg_v0_rsp_fwdn_data {
	uint8_t data[4];
} __attribute__ ((__packed__));

struct opahrs_msg_v0_rsp_versions {
	uint16_t hw_version;
	uint16_t bl_version;
	uint32_t fw_crc;
} __attribute__ ((__packed__));

struct opahrs_msg_v0_rsp_serial {
	uint8_t serial_bcd[24];
} __attribute__ ((__packed__));

enum bootloader_status { idle, started, start_failed, write_error, outside_dev_capabilities, jump_failed };
struct opahrs_msg_v0_rsp_fwup_status {
	enum bootloader_status status;
} __attribute__ ((__packed__));

enum hw_density { high_density, medium_density };
struct opahrs_msg_v0_rsp_mem_map {
	uint32_t start_of_user_code;
	uint32_t size_of_code_memory;
	uint8_t size_of_description;
	uint8_t rw_flags;
	enum hw_density density;
} __attribute__ ((__packed__));

union opahrs_msg_v0_rsp {
	struct opahrs_msg_v0_rsp_versions versions;
	struct opahrs_msg_v0_rsp_serial serial;
	struct opahrs_msg_v0_rsp_fwup_status fwup_status;
	struct opahrs_msg_v0_rsp_mem_map mem_map;
	struct opahrs_msg_v0_rsp_fwdn_data fw_dn;
} __attribute__ ((__packed__));

enum opahrs_msg_v0_tag {
	OPAHRS_MSG_V0_REQ_NOP = 0x00,
	OPAHRS_MSG_V0_REQ_VERSIONS,
	OPAHRS_MSG_V0_REQ_RESET,
	OPAHRS_MSG_V0_REQ_BOOT,
	OPAHRS_MSG_V0_REQ_SERIAL,
	OPAHRS_MSG_V0_REQ_FWDN_DATA,
	OPAHRS_MSG_V0_REQ_MEM_MAP,

	OPAHRS_MSG_V0_REQ_FWUP_START,
	OPAHRS_MSG_V0_REQ_FWUP_DATA,
	OPAHRS_MSG_V0_REQ_FWUP_VERIFY,
	OPAHRS_MSG_V0_REQ_FWUP_STATUS,

	OPAHRS_MSG_V0_RSP_VERSIONS,
	OPAHRS_MSG_V0_RSP_SERIAL,
	OPAHRS_MSG_V0_RSP_FWUP_STATUS,
	OPAHRS_MSG_V0_RSP_FWDN_DATA,
	OPAHRS_MSG_V0_RSP_MEM_MAP,
};

struct opahrs_msg_v0_payload {
	enum opahrs_msg_v0_tag t;
	union {
		union opahrs_msg_v0_req req;
		union opahrs_msg_v0_rsp rsp;
	} v;
} __attribute__ ((__packed__));

struct opahrs_msg_v0 {
	struct opahrs_msg_link_head head;
	union {
		struct opahrs_msg_link link;
		struct opahrs_msg_v0_payload user;
	} payload;
	struct opahrs_msg_link_tail tail;
} __attribute__ ((__packed__));

/********
 * SPI protocol v1 definitions
 ********/

struct opahrs_msg_v1_req_nop {
} __attribute__ ((__packed__));

struct opahrs_msg_v1_req_versions {
} __attribute__ ((__packed__));

struct opahrs_msg_v1_req_reset {
	uint32_t reset_delay_in_ms;
} __attribute__ ((__packed__));

struct opahrs_msg_v1_req_serial {
} __attribute__ ((__packed__));

enum initialized_mode { AHRS_UNINITIALIZED, AHRS_INITIALIZED, AHRS_INIT_QUERY };

struct opahrs_msg_v1_req_initialized {
	enum initialized_mode initialized;
} __attribute__ ((__packed__));

struct opahrs_msg_v1_req_north {
	float Be[3];
} __attribute__ ((__packed__));

enum algorithms { SIMPLE_Algo, INSGPS_Algo };

struct opahrs_msg_v1_req_algorithm {
	enum algorithms algorithm;
} __attribute__ ((__packed__));

struct opahrs_msg_v1_req_update {
	struct {
		uint8_t updated;
		float NED[3];
		float groundspeed;
		float heading;
		float quality;
	} gps;
	struct {
		uint8_t updated;
		float altitude;
	} barometer;
} __attribute__ ((__packed__));

struct opahrs_msg_v1_req_attituderaw {
} __attribute__ ((__packed__));

enum measure_mode { AHRS_SET, AHRS_MEASURE, AHRS_ECHO };

struct opahrs_msg_v1_req_calibration {
	enum measure_mode measure_var;
	float accel_bias[3];
	float accel_scale[3];
	float accel_var[3];
	float gyro_bias[3];
	float gyro_scale[3];
	float gyro_var[3];
	float mag_bias[3];
	float mag_scale[3];
	float mag_var[3];
} __attribute__ ((__packed__));

union opahrs_msg_v1_req {
	struct opahrs_msg_v1_req_nop nop;
	struct opahrs_msg_v1_req_versions versions;
	struct opahrs_msg_v1_req_reset reset;
	struct opahrs_msg_v1_req_initialized initialized;
	struct opahrs_msg_v1_req_serial serial;
	struct opahrs_msg_v1_req_update update;
	struct opahrs_msg_v1_req_algorithm algorithm;
	struct opahrs_msg_v1_req_north north;
	struct opahrs_msg_v1_req_attituderaw attituderaw;
	struct opahrs_msg_v1_req_calibration calibration;
} __attribute__ ((__packed__));

struct opahrs_msg_v1_rsp_versions {
	uint8_t hw_version;
	uint16_t bl_version;
	uint32_t fw_crc;
} __attribute__ ((__packed__));

struct opahrs_msg_v1_rsp_serial {
	uint8_t serial_bcd[25];
} __attribute__ ((__packed__));

struct opahrs_msg_v1_rsp_initialized {
	enum initialized_mode initialized;
} __attribute__ ((__packed__));

struct opahrs_msg_v1_rsp_north {
} __attribute__ ((__packed__));

struct opahrs_msg_v1_rsp_algorithm {
} __attribute__ ((__packed__));

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
		float x;
		float y;
		float z;
	} gyros_filtered;
	struct {
		uint16_t x;
		uint16_t y;
		uint16_t z;
	} accels;
	struct {
		float x;
		float y;
		float z;
	} accels_filtered;
} __attribute__ ((__packed__));

struct opahrs_msg_v1_rsp_update {
	struct {
		float q1;
		float q2;
		float q3;
		float q4;
	} quaternion;
	int32_t NED[3];
	int32_t Vel[3];
	uint8_t load;
	uint8_t idle_time;
	uint8_t run_time;
	uint8_t dropped_updates;
} __attribute__ ((__packed__));

struct opahrs_msg_v1_rsp_calibration {
	float accel_var[3];
	float gyro_var[3];
	float mag_var[3];
} __attribute__ ((__packed__));

union opahrs_msg_v1_rsp {
	struct opahrs_msg_v1_rsp_versions versions;
	struct opahrs_msg_v1_rsp_serial serial;
	struct opahrs_msg_v1_rsp_initialized initialized;
	struct opahrs_msg_v1_rsp_north north;
	struct opahrs_msg_v1_rsp_algorithm algorithm;
	struct opahrs_msg_v1_rsp_attituderaw attituderaw;
	struct opahrs_msg_v1_rsp_update update;
	struct opahrs_msg_v1_rsp_calibration calibration;
} __attribute__ ((__packed__));

enum opahrs_msg_v1_tag {
	OPAHRS_MSG_V1_REQ_NOP = 0x02000000,
	OPAHRS_MSG_V1_REQ_VERSIONS,
	OPAHRS_MSG_V1_REQ_RESET,
	OPAHRS_MSG_V1_REQ_SERIAL,
	OPAHRS_MSG_V1_REQ_INITIALIZED,
	OPAHRS_MSG_V1_REQ_NORTH,
	OPAHRS_MSG_V1_REQ_ALGORITHM,
	OPAHRS_MSG_V1_REQ_UPDATE,
	OPAHRS_MSG_V1_REQ_ATTITUDERAW,
	OPAHRS_MSG_V1_REQ_CALIBRATION,

	OPAHRS_MSG_V1_RSP_VERSIONS,
	OPAHRS_MSG_V1_RSP_SERIAL,
	OPAHRS_MSG_V1_RSP_INITIALIZED,
	OPAHRS_MSG_V1_RSP_NORTH,
	OPAHRS_MSG_V1_RSP_ALGORITHM,
	OPAHRS_MSG_V1_RSP_UPDATE,
	OPAHRS_MSG_V1_RSP_ATTITUDERAW,
	OPAHRS_MSG_V1_RSP_CALIBRATION,
};

struct opahrs_msg_v1_payload {
	enum opahrs_msg_v1_tag t;
	union {
		union opahrs_msg_v1_req req;
		union opahrs_msg_v1_rsp rsp;
	} v;
} __attribute__ ((__packed__));

struct opahrs_msg_v1 {
	struct opahrs_msg_link_head head;
	union {
		struct opahrs_msg_link link;
		struct opahrs_msg_v1_payload user;
	} payload;
	struct opahrs_msg_link_tail tail;
} __attribute__ ((__packed__));

/* Helper functions for setting up messages */
extern void opahrs_msg_v0_init_rx(struct opahrs_msg_v0 *msg);
extern void opahrs_msg_v0_init_user_tx(struct opahrs_msg_v0 *msg, enum opahrs_msg_v0_tag tag);
extern void opahrs_msg_v0_init_link_tx(struct opahrs_msg_v0 *msg, enum opahrs_msg_link_tag tag);
extern void opahrs_msg_v1_init_rx(struct opahrs_msg_v1 *msg);
extern void opahrs_msg_v1_init_user_tx(struct opahrs_msg_v1 *msg, enum opahrs_msg_v1_tag tag);
extern void opahrs_msg_v1_init_link_tx(struct opahrs_msg_v1 *msg, enum opahrs_msg_link_tag tag);

#endif /* PIOS_OPAHRS_PROTO_H */

/**
  * @}
  * @}
  */
