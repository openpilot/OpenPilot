/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup   PIOS_I2C I2C Functions
 * @brief        Linux dependent I2C functionality
 * @{
 *
 * @file       pios_i2c.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @brief      I2C Enable/Disable routines
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/
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

#ifdef PIOS_INCLUDE_I2C
#include "openpilot.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <pthread.h>

#define RPI_I2C_BUS_1 "/dev/i2c-1"
#define RPI_I2C_MAX_MESSAGES_P_TRANSFER 64

static int i2c_fd = -1;
static pthread_mutex_t i2c_mutex = PTHREAD_MUTEX_INITIALIZER;

#if defined(PIOS_I2C_DIAGNOSTICS)
/*static struct pios_i2c_fault_history i2c_adapter_fault_history;*/
#endif


/**
 * Logs the last N state transitions and N IRQ events due to
 * an error condition
 * \param[out] data address where to copy the pios_i2c_fault_history structure to
 * \param[out] counts three uint16 that receive the bad event, fsm, and error irq
 * counts
 */
void PIOS_I2C_GetDiagnostics(struct pios_i2c_fault_history *data, uint8_t *counts)
{
#if defined(PIOS_I2C_DIAGNOSTICS)
	// for now just fake history
	struct pios_i2c_fault_history i2c_adapter_fault_history;
    i2c_adapter_fault_history.type = PIOS_I2C_ERROR_EVENT;

    memcpy(data, &i2c_adapter_fault_history, sizeof(i2c_adapter_fault_history));
    counts[0] = counts[1] = counts[2] = 0;

#else
    struct pios_i2c_fault_history i2c_adapter_fault_history;
    i2c_adapter_fault_history.type = PIOS_I2C_ERROR_EVENT;

    memcpy(data, &i2c_adapter_fault_history, sizeof(i2c_adapter_fault_history));
    counts[0] = counts[1] = counts[2] = 0;
#endif
}


/**
 * @brief Perform a series of I2C transactions
 * @returns 0 if success or error code
 * @retval -1 for failed transaction
 * @retval -2 for failure to get semaphore
 */
int32_t PIOS_I2C_Transfer(uint32_t i2c_id, const struct pios_i2c_txn txn_list[], uint32_t num_txns)
{
    struct i2c_rdwr_ioctl_data data;
    struct i2c_msg msg[RPI_I2C_MAX_MESSAGES_P_TRANSFER];
	int i;
	int ret;

	(void)i2c_id; // not used for now
	
	if (-1 == i2c_fd)
	{
		return -1;
	}

	if (num_txns > RPI_I2C_MAX_MESSAGES_P_TRANSFER)
	{
		return -1;
	}

	for (i = 0; i < num_txns; ++i)
	{
		msg[i].addr = txn_list[i].addr;
		switch (txn_list[i].rw)
		{
			case PIOS_I2C_TXN_WRITE:
				msg[i].flags = 0;
				break;
			case PIOS_I2C_TXN_READ:
				msg[i].flags = I2C_M_RD/* | I2C_M_NOSTART*/;
				break;
		}
		msg[i].len = txn_list[i].len;
		msg[i].buf = txn_list[i].buf;
	}

	data.msgs = msg;
	data.nmsgs = num_txns;
	ret = -1;

	pthread_mutex_lock(&i2c_mutex);
	ret = ioctl(i2c_fd, I2C_RDWR, &data);
	pthread_mutex_unlock(&i2c_mutex);

	return ret;
}

uint32_t PIOS_I2C_init()
{
	i2c_fd = open(RPI_I2C_BUS_1, O_RDWR);
	if (-1 == i2c_fd)
	{
		// Can't open I2C bus
		AlarmsSet(SYSTEMALARMS_ALARM_I2C, SYSTEMALARMS_ALARM_CRITICAL);
		return -1;
	}

	AlarmsClear(SYSTEMALARMS_ALARM_I2C);
	return 0;
}

#endif /* PIOS_INCLUDE_I2C */

/**
 * @}
 * @}
 */
