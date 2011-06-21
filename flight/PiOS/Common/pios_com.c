/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_COM COM layer functions
 * @brief Hardware communication layer
 * @{
 *
 * @file       pios_com.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	       Parts by Thorsten Klose (tk@midibox.org)
 * @brief      COM layer functions
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

/* Project Includes */
#include "pios.h"

#if defined(PIOS_INCLUDE_COM)

#include <pios_com_priv.h>

static bool PIOS_COM_validate(struct pios_com_dev * com_dev)
{
	return (com_dev && (com_dev->magic == PIOS_COM_DEV_MAGIC));
}

#if defined(PIOS_INCLUDE_FREERTOS) && 0
static struct pios_com_dev * PIOS_COM_alloc(void)
{
	struct pios_com_dev * com_dev;

	com_dev = (struct pios_com_dev *)malloc(sizeof(*com_dev));
	if (!com_dev) return (NULL);

	com_dev->magic = PIOS_COM_DEV_MAGIC;
	return(com_dev);
}
#else
static struct pios_com_dev pios_com_devs[PIOS_COM_MAX_DEVS];
static uint8_t pios_com_num_devs;
static struct pios_com_dev * PIOS_COM_alloc(void)
{
	struct pios_com_dev * com_dev;

	if (pios_com_num_devs >= PIOS_COM_MAX_DEVS) {
		return (NULL);
	}

	com_dev = &pios_com_devs[pios_com_num_devs++];
	com_dev->magic = PIOS_COM_DEV_MAGIC;

	return (com_dev);
}
#endif

/**
  * Initialises COM layer
  * \param[out] handle
  * \param[in] driver
  * \param[in] id
  * \return < 0 if initialisation failed
  */
int32_t PIOS_COM_Init(uint32_t * com_id, const struct pios_com_driver * driver, const uint32_t lower_id)
{
	PIOS_DEBUG_Assert(com_id);
	PIOS_DEBUG_Assert(driver);

	struct pios_com_dev * com_dev;

	com_dev = (struct pios_com_dev *) PIOS_COM_alloc();
	if (!com_dev) goto out_fail;

	com_dev->driver = driver;
	com_dev->id     = lower_id;

	*com_id = (uint32_t)com_dev;
	return(0);

out_fail:
	return(-1);
}

/**
* Change the port speed without re-initializing
* \param[in] port COM port
* \param[in] baud Requested baud rate
* \return -1 if port not available
* \return 0 on success
*/
int32_t PIOS_COM_ChangeBaud(uint32_t com_id, uint32_t baud)
{
	struct pios_com_dev * com_dev = (struct pios_com_dev *)com_id;

	if (!PIOS_COM_validate(com_dev)) {
		/* Undefined COM port for this board (see pios_board.c) */
		return -1;
	}

	/* Invoke the driver function if it exists */
	if (com_dev->driver->set_baud) {
		com_dev->driver->set_baud(com_dev->id, baud);
	}

	return 0;
}

/**
* Sends a package over given port
* \param[in] port COM port
* \param[in] buffer character buffer
* \param[in] len buffer length
* \return -1 if port not available
* \return -2 if non-blocking mode activated: buffer is full
*            caller should retry until buffer is free again
* \return 0 on success
*/
int32_t PIOS_COM_SendBufferNonBlocking(uint32_t com_id, const uint8_t *buffer, uint16_t len)
{
	struct pios_com_dev * com_dev = (struct pios_com_dev *)com_id;

	if (!PIOS_COM_validate(com_dev)) {
		/* Undefined COM port for this board (see pios_board.c) */
		return -1;
	}

	/* Invoke the driver function if it exists */
	if (com_dev->driver->tx_nb) {
		return com_dev->driver->tx_nb(com_dev->id, buffer, len);
	}

	return 0;
}

/**
* Sends a package over given port
* (blocking function)
* \param[in] port COM port
* \param[in] buffer character buffer
* \param[in] len buffer length
* \return -1 if port not available
* \return 0 on success
*/
int32_t PIOS_COM_SendBuffer(uint32_t com_id, const uint8_t *buffer, uint16_t len)
{
	struct pios_com_dev * com_dev = (struct pios_com_dev *)com_id;

	if (!PIOS_COM_validate(com_dev)) {
		/* Undefined COM port for this board (see pios_board.c) */
		return -1;
	}

	/* Invoke the driver function if it exists */
	if (com_dev->driver->tx) {
		return com_dev->driver->tx(com_dev->id, buffer, len);
	}

	return 0;
}

/**
* Sends a single character over given port
* \param[in] port COM port
* \param[in] c character
* \return -1 if port not available
* \return -2 buffer is full
*            caller should retry until buffer is free again
* \return 0 on success
*/
int32_t PIOS_COM_SendCharNonBlocking(uint32_t com_id, char c)
{
	return PIOS_COM_SendBufferNonBlocking(com_id, (uint8_t *)&c, 1);
}

/**
* Sends a single character over given port
* (blocking function)
* \param[in] port COM port
* \param[in] c character
* \return -1 if port not available
* \return 0 on success
*/
int32_t PIOS_COM_SendChar(uint32_t com_id, char c)
{
	return PIOS_COM_SendBuffer(com_id, (uint8_t *)&c, 1);
}

/**
* Sends a string over given port
* \param[in] port COM port
* \param[in] str zero-terminated string
* \return -1 if port not available
* \return -2 buffer is full
*         caller should retry until buffer is free again
* \return 0 on success
*/
int32_t PIOS_COM_SendStringNonBlocking(uint32_t com_id, const char *str)
{
	return PIOS_COM_SendBufferNonBlocking(com_id, (uint8_t *)str, (uint16_t)strlen(str));
}

/**
* Sends a string over given port
* (blocking function)
* \param[in] port COM port
* \param[in] str zero-terminated string
* \return -1 if port not available
* \return 0 on success
*/
int32_t PIOS_COM_SendString(uint32_t com_id, const char *str)
{
	return PIOS_COM_SendBuffer(com_id, (uint8_t *)str, strlen(str));
}

/**
* Sends a formatted string (-> printf) over given port
* \param[in] port COM port
* \param[in] *format zero-terminated format string - 128 characters supported maximum!
* \param[in] ... optional arguments,
*        128 characters supported maximum!
* \return -2 if non-blocking mode activated: buffer is full
*         caller should retry until buffer is free again
* \return 0 on success
*/
int32_t PIOS_COM_SendFormattedStringNonBlocking(uint32_t com_id, const char *format, ...)
{
	uint8_t buffer[128]; // TODO: tmp!!! Provide a streamed COM method later!

	va_list args;

	va_start(args, format);
	vsprintf((char *)buffer, format, args);
	return PIOS_COM_SendBufferNonBlocking(com_id, buffer, (uint16_t)strlen((char *)buffer));
}

/**
* Sends a formatted string (-> printf) over given port
* (blocking function)
* \param[in] port COM port
* \param[in] *format zero-terminated format string - 128 characters supported maximum!
* \param[in] ... optional arguments,
* \return -1 if port not available
* \return 0 on success
*/
int32_t PIOS_COM_SendFormattedString(uint32_t com_id, const char *format, ...)
{
	uint8_t buffer[128]; // TODO: tmp!!! Provide a streamed COM method later!
	va_list args;

	va_start(args, format);
	vsprintf((char *)buffer, format, args);
	return PIOS_COM_SendBuffer(com_id, buffer, (uint16_t)strlen((char *)buffer));
}

/**
* Transfer bytes from port buffers into another buffer
* \param[in] port COM port
* \returns Byte from buffer
*/
uint8_t PIOS_COM_ReceiveBuffer(uint32_t com_id)
{
	struct pios_com_dev * com_dev = (struct pios_com_dev *)com_id;

	if (!PIOS_COM_validate(com_dev)) {
		/* Undefined COM port for this board (see pios_board.c) */
		PIOS_DEBUG_Assert(0);
	}

	PIOS_DEBUG_Assert(com_dev->driver->rx);

	return com_dev->driver->rx(com_dev->id);
}

/**
* Get the number of bytes waiting in the buffer
* \param[in] port COM port
* \return Number of bytes used in buffer
*/
int32_t PIOS_COM_ReceiveBufferUsed(uint32_t com_id)
{
	struct pios_com_dev * com_dev = (struct pios_com_dev *)com_id;

	if (!PIOS_COM_validate(com_dev)) {
		/* Undefined COM port for this board (see pios_board.c) */
		/* This is commented out so com_id=NULL can be used to disable telemetry */
		//PIOS_DEBUG_Assert(0);
		return 0;
	}

	if (!com_dev->driver->rx_avail) {
		return 0;
	}

	return com_dev->driver->rx_avail(com_dev->id);
}

#endif

/**
 * @}
 * @}
 */
