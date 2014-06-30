/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_MPU6000 MPU6000 Functions
 * @brief Deals with the hardware interface to the 3-axis gyro
 * @{
 *
 * @file       pios_mpu000.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      MPU6000 6-axis gyro and accel chip
 * @see        The GNU Public License (GPL) Version 3
 *
 ******************************************************************************
 */
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

#ifdef PIOS_INCLUDE_MPU6000

#include "fifo_buffer.h"
#include "pios_i2c.h"
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h"
#include "inv_mpu_misc.h"

/* Linux includes */
#include <sys/time.h>

#define GRAV                 9.81f
#define STACK_SIZE_BYTES     2048
#define TASK_PRIORITY        (tskIDLE_PRIORITY + 4) // device driver
/* in Hz */
#define MPU_9150_SAMPLE_RATE 20

static xTaskHandle taskHandle;
xQueueHandle queue;
static void mpuTask(void *parameters);

int32_t MpuStart()
{
	/* no queue, no task */
	if (NULL == queue) {
        return 0;
    }
    // Start main task
    xTaskCreate(mpuTask, (signed char *)"MPU_9150", STACK_SIZE_BYTES / 4, NULL, TASK_PRIORITY, &taskHandle);


    //PIOS_TASK_MONITOR_RegisterTask(TASKINFO_RUNNING_???, taskHandle);
#ifdef PIOS_INCLUDE_WDG
//    PIOS_WDG_RegisterFlag(PIOS_WDG_???);
#endif

    return 0;
}


/**
 * @brief Module initialization
 * @return 0
 */
int32_t MpuInitialize()
{
	signed char gyro_orientation[9] = {1, 0, 0,
                                       0, 1, 0,
                                       0, 0, 1};

	queue = NULL;
	queue = xQueueCreate(MPU_9150_SAMPLE_RATE, sizeof(struct pios_mpu6000_data));
	if (NULL == queue) {
        return 0;
    }

	printf("MPU-9150 init ...");
	fflush(stdout);
	mpu_init(NULL);
	mpu_set_sensors(INV_XYZ_GYRO | INV_XYZ_ACCEL | INV_XYZ_COMPASS);
	mpu_configure_fifo(INV_XYZ_GYRO | INV_XYZ_ACCEL);
	mpu_set_sample_rate(MPU_9150_SAMPLE_RATE);
	mpu_set_compass_sample_rate(MPU_9150_SAMPLE_RATE);
	dmp_load_motion_driver_firmware();
	dmp_set_orientation(inv_orientation_matrix_to_scaler(gyro_orientation));
	dmp_enable_feature(DMP_FEATURE_6X_LP_QUAT | DMP_FEATURE_SEND_RAW_ACCEL | DMP_FEATURE_SEND_RAW_GYRO);
	dmp_set_fifo_rate(MPU_9150_SAMPLE_RATE);
	mpu_set_dmp_state(1);
	printf(" done.\n");
    return 0;
}

MODULE_INITCALL(MpuInitialize, MpuStart);

/**
 * \brief Reads the queue handle
 * \return Handle to the queue or null if invalid device
 */
xQueueHandle PIOS_MPU6000_GetQueue()
{
    return queue;
}

#define MAX_I2C_TRANSACTION_LEN 255
static uint8_t i2c_buf[MAX_I2C_TRANSACTION_LEN + 1];
int mpu_i2c_write(unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char const *data) {
	struct pios_i2c_txn i2c_transaction[1];
	int i;

	if (length > MAX_I2C_TRANSACTION_LEN) {
		return -1;
	}

	i2c_transaction[0].info = NULL;
	i2c_transaction[0].addr = slave_addr;
	i2c_transaction[0].rw = PIOS_I2C_TXN_WRITE;
	i2c_transaction[0].len = length + 1;
	i2c_transaction[0].buf = i2c_buf;

	i2c_buf[0] = reg_addr;
	for (i = 0; i < length; ++i) {
		i2c_buf[i + 1] = data[i];
	}

	PIOS_I2C_Transfer(1, i2c_transaction, 1);

	return 0;
}

int mpu_i2c_read(unsigned char slave_addr, unsigned char reg_addr, unsigned char length, unsigned char *data) {
	struct pios_i2c_txn i2c_transaction[2];

	i2c_transaction[0].info = NULL;
	i2c_transaction[0].addr = slave_addr;
	i2c_transaction[0].rw = PIOS_I2C_TXN_WRITE;
	i2c_transaction[0].len = 1;
	i2c_transaction[0].buf = &reg_addr;

	i2c_transaction[1].info = NULL;
	i2c_transaction[1].addr = slave_addr;
	i2c_transaction[1].rw = PIOS_I2C_TXN_READ;
	i2c_transaction[1].len = length;
	i2c_transaction[1].buf = (uint8_t*) data;


	PIOS_I2C_Transfer(1, i2c_transaction, 2);

	return 0;
}

/* internal data type */
typedef struct mpu_9150_data_t_tag{
	short raw_gyro[3];
	short raw_accel[3];
	long raw_quat[4];
	short raw_mag[3];
	long temperature; // in q16 format
} mpu_9150_data_t;

int mpu_read_data(mpu_9150_data_t *mpu_data)
{
	short data_ready;
	unsigned char more;
	short sensors;

	if (mpu_get_int_status(&data_ready) < 0) {
		return -1;
	}

	if ((MPU_INT_STATUS_DATA_READY | MPU_INT_STATUS_DMP | MPU_INT_STATUS_DMP_0) == data_ready) {
		do {
			if (dmp_read_fifo(mpu_data->raw_gyro, mpu_data->raw_accel, mpu_data->raw_quat, NULL, &sensors, &more) < 0) {
				return -1;
			}
		} while (more); /* read until fifo is empty */
	}
	else {
		return -1;
	}

	if (mpu_get_compass_reg(mpu_data->raw_mag, NULL) < 0) {
		return -1;
	}

	if (mpu_get_temperature(&(mpu_data->temperature), NULL) != 0) {
		return -1;
	}

	return 0;
}

/**
 * @brief Main Mpu driver task
 *
 * MPU 9150 driver.
 * 
 * @return -1 if error, 0 if success
 */
static void mpuTask(__attribute__((unused)) void *parameters)
{
	mpu_9150_data_t mpu_data;
	struct pios_mpu6000_data mpu6000_data;

	/* no queue, no task, many problems */
	if (NULL == queue) {
		/* this should not happen BTW */
        return;
    }

	while (1) {
		if (mpu_read_data(&mpu_data) == 0) {
			// 180DEG rotation for Z
			mpu6000_data.gyro_x = mpu_data.raw_gyro[0];
			mpu6000_data.gyro_y = mpu_data.raw_gyro[1];
			mpu6000_data.gyro_z = - mpu_data.raw_gyro[2];
#if defined(PIOS_MPU6000_ACCEL)
			mpu6000_data.accel_x = mpu_data.raw_accel[0];
			mpu6000_data.accel_y = mpu_data.raw_accel[1];
			mpu6000_data.accel_z = - mpu_data.raw_accel[2];
#endif /* PIOS_MPU6000_ACCEL */
			mpu6000_data.temperature = (int16_t) ((mpu_data.temperature & 0x00FFFF00) >> 8);

			xQueueSendToBack(queue, (void *)&mpu6000_data, 0);
		}

		//* Looks like vTaskDelay in posix works like sleep_ms() */
		vTaskDelay(35);
	}
}

int32_t PIOS_MPU6000_ReadGyros(struct pios_mpu6000_data *buffer) {
	/* ******************************************
	 * need to fix this function
	 * stub for now
	 * ******************************************/

	buffer->gyro_x = 0;
	buffer->gyro_y = 0;
	buffer->gyro_z = 0;
#if defined(PIOS_MPU6000_ACCEL)
	buffer->accel_x = 0;
	buffer->accel_y = 0;
	buffer->accel_z = 0;
#endif /* PIOS_MPU6000_ACCEL */
	buffer->temperature = 0;

	return 0;
}
int32_t PIOS_MPU6000_Test() {
	return 0;
}
float PIOS_MPU6000_GetScale() {
	//PIOS_MPU6000_SCALE_2000_DEG:
	return 1.0f / 16.4f;
}
float PIOS_MPU6000_GetAccelScale() {
	//PIOS_MPU6000_ACCEL_8G:
	return GRAV / 4096.0f;
}

#endif /* PIOS_INCLUDE_MPU6000 */

/**
 * @}
 * @}
 */
