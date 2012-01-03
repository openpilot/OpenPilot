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

#include "fifo_buffer.h"
#include <pios_com_priv.h>

#if !defined(PIOS_INCLUDE_FREERTOS)
#include "pios_delay.h"		/* PIOS_DELAY_WaitmS */
#endif

enum pios_com_dev_magic {
  PIOS_COM_DEV_MAGIC = 0xaa55aa55,
};

struct pios_com_dev {
	enum pios_com_dev_magic magic;
	uint32_t lower_id;
	const struct pios_com_driver * driver;

#if defined(PIOS_INCLUDE_FREERTOS)
	xSemaphoreHandle tx_sem;
	xSemaphoreHandle rx_sem;
#endif

	bool has_rx;
	bool has_tx;

	t_fifo_buffer rx;
	t_fifo_buffer tx;
};

static bool PIOS_COM_validate(struct pios_com_dev * com_dev)
{
	return (com_dev && (com_dev->magic == PIOS_COM_DEV_MAGIC));
}

#if defined(PIOS_INCLUDE_FREERTOS)
static struct pios_com_dev * PIOS_COM_alloc(void)
{
	struct pios_com_dev * com_dev;

	com_dev = (struct pios_com_dev *)pvPortMalloc(sizeof(*com_dev));
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

static uint16_t PIOS_COM_TxOutCallback(uint32_t context, uint8_t * buf, uint16_t buf_len, uint16_t * headroom, bool * need_yield);
static uint16_t PIOS_COM_RxInCallback(uint32_t context, uint8_t * buf, uint16_t buf_len, uint16_t * headroom, bool * need_yield);
static void PIOS_COM_UnblockRx(struct pios_com_dev * com_dev, bool * need_yield);
static void PIOS_COM_UnblockTx(struct pios_com_dev * com_dev, bool * need_yield);

/**
  * Initialises COM layer
  * \param[out] handle
  * \param[in] driver
  * \param[in] id
  * \return < 0 if initialisation failed
  */
int32_t PIOS_COM_Init(uint32_t * com_id, const struct pios_com_driver * driver, uint32_t lower_id, uint8_t * rx_buffer, uint16_t rx_buffer_len, uint8_t * tx_buffer, uint16_t tx_buffer_len)
{
	PIOS_Assert(com_id);
	PIOS_Assert(driver);

	bool has_rx = (rx_buffer && rx_buffer_len > 0);
	bool has_tx = (tx_buffer && tx_buffer_len > 0);
	PIOS_Assert(has_rx || has_tx);
	PIOS_Assert(driver->bind_tx_cb || !has_tx);
	PIOS_Assert(driver->bind_rx_cb || !has_rx);

	struct pios_com_dev * com_dev;

	com_dev = (struct pios_com_dev *) PIOS_COM_alloc();
	if (!com_dev) goto out_fail;

	com_dev->driver   = driver;
	com_dev->lower_id = lower_id;

	com_dev->has_rx = has_rx;
	com_dev->has_tx = has_tx;

	if (has_rx) {
		fifoBuf_init(&com_dev->rx, rx_buffer, rx_buffer_len);
#if defined(PIOS_INCLUDE_FREERTOS)
		vSemaphoreCreateBinary(com_dev->rx_sem);
#endif	/* PIOS_INCLUDE_FREERTOS */
		(com_dev->driver->bind_rx_cb)(lower_id, PIOS_COM_RxInCallback, (uint32_t)com_dev);
		if (com_dev->driver->rx_start) {
			/* Start the receiver */
			(com_dev->driver->rx_start)(com_dev->lower_id,
						    fifoBuf_getFree(&com_dev->rx));
		}
	}

	if (has_tx) {
		fifoBuf_init(&com_dev->tx, tx_buffer, tx_buffer_len);
#if defined(PIOS_INCLUDE_FREERTOS)
		vSemaphoreCreateBinary(com_dev->tx_sem);
#endif	/* PIOS_INCLUDE_FREERTOS */
		(com_dev->driver->bind_tx_cb)(lower_id, PIOS_COM_TxOutCallback, (uint32_t)com_dev);
	}

	*com_id = (uint32_t)com_dev;
	return(0);

out_fail:
	return(-1);
}

static void PIOS_COM_UnblockRx(struct pios_com_dev * com_dev, bool * need_yield)
{
#if defined(PIOS_INCLUDE_FREERTOS)
	static signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(com_dev->rx_sem, &xHigherPriorityTaskWoken);

	if (xHigherPriorityTaskWoken != pdFALSE) {
		*need_yield = true;
	} else {
		*need_yield = false;
	}
#else
	*need_yield = false;
#endif
}

static void PIOS_COM_UnblockTx(struct pios_com_dev * com_dev, bool * need_yield)
{
#if defined(PIOS_INCLUDE_FREERTOS)
	static signed portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(com_dev->tx_sem, &xHigherPriorityTaskWoken);

	if (xHigherPriorityTaskWoken != pdFALSE) {
		*need_yield = true;
	} else {
		*need_yield = false;
	}
#else
	*need_yield = false;
#endif
}

static uint16_t PIOS_COM_RxInCallback(uint32_t context, uint8_t * buf, uint16_t buf_len, uint16_t * headroom, bool * need_yield)
{
	struct pios_com_dev * com_dev = (struct pios_com_dev *)context;

	bool valid = PIOS_COM_validate(com_dev);
	PIOS_Assert(valid);
	PIOS_Assert(com_dev->has_rx);

	PIOS_IRQ_Disable();
	uint16_t bytes_into_fifo = fifoBuf_putData(&com_dev->rx, buf, buf_len);
	PIOS_IRQ_Enable();

	if (bytes_into_fifo > 0) {
		/* Data has been added to the buffer */
		PIOS_COM_UnblockRx(com_dev, need_yield);
	}

	if (headroom) {
		*headroom = fifoBuf_getFree(&com_dev->rx);
	}

	return (bytes_into_fifo);
}

static uint16_t PIOS_COM_TxOutCallback(uint32_t context, uint8_t * buf, uint16_t buf_len, uint16_t * headroom, bool * need_yield)
{
	struct pios_com_dev * com_dev = (struct pios_com_dev *)context;

	bool valid = PIOS_COM_validate(com_dev);
	PIOS_Assert(valid);
	PIOS_Assert(buf);
	PIOS_Assert(buf_len);
	PIOS_Assert(com_dev->has_tx);

	PIOS_IRQ_Disable();
	uint16_t bytes_from_fifo = fifoBuf_getData(&com_dev->tx, buf, buf_len);
	PIOS_IRQ_Enable();

	if (bytes_from_fifo > 0) {
		/* More space has been made in the buffer */
		PIOS_COM_UnblockTx(com_dev, need_yield);
	}

	if (headroom) {
		*headroom = fifoBuf_getUsed(&com_dev->tx);
	}

	return (bytes_from_fifo);
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
		com_dev->driver->set_baud(com_dev->lower_id, baud);
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
* \return number of bytes transmitted on success
*/
int32_t PIOS_COM_SendBufferNonBlocking(uint32_t com_id, const uint8_t *buffer, uint16_t len)
{
	struct pios_com_dev * com_dev = (struct pios_com_dev *)com_id;

	if (!PIOS_COM_validate(com_dev)) {
		/* Undefined COM port for this board (see pios_board.c) */
		return -1;
	}

	PIOS_Assert(com_dev->has_tx);

	if (len > fifoBuf_getFree(&com_dev->tx)) {
		/* Buffer cannot accept all requested bytes (retry) */
		return -2;
	}

	PIOS_IRQ_Disable();
	uint16_t bytes_into_fifo = fifoBuf_putData(&com_dev->tx, buffer, len);
	PIOS_IRQ_Enable();

	if (bytes_into_fifo > 0) {
		/* More data has been put in the tx buffer, make sure the tx is started */
		if (com_dev->driver->tx_start) {
			com_dev->driver->tx_start(com_dev->lower_id,
						  fifoBuf_getUsed(&com_dev->tx));
		}
	}

	return (bytes_into_fifo);
}

/**
* Sends a package over given port
* (blocking function)
* \param[in] port COM port
* \param[in] buffer character buffer
* \param[in] len buffer length
* \return -1 if port not available
* \return number of bytes transmitted on success
*/
int32_t PIOS_COM_SendBuffer(uint32_t com_id, const uint8_t *buffer, uint16_t len)
{
	struct pios_com_dev * com_dev = (struct pios_com_dev *)com_id;

	if (!PIOS_COM_validate(com_dev)) {
		/* Undefined COM port for this board (see pios_board.c) */
		return -1;
	}

	PIOS_Assert(com_dev->has_tx);

	uint32_t max_frag_len = fifoBuf_getSize(&com_dev->tx);
	uint32_t bytes_to_send = len;
	while (bytes_to_send) {
		uint32_t frag_size;

		if (bytes_to_send > max_frag_len) {
			frag_size = max_frag_len;
		} else {
			frag_size = bytes_to_send;
		}
		int32_t rc = PIOS_COM_SendBufferNonBlocking(com_id, buffer, frag_size);
		if (rc >= 0) {
			bytes_to_send -= rc;
			buffer += rc;
		} else {
			switch (rc) {
			case -1:
				/* Device is invalid, this will never work */
				return -1;
			case -2:
				/* Device is busy, wait for the underlying device to free some space and retry */
				/* Make sure the transmitter is running while we wait */
				if (com_dev->driver->tx_start) {
					(com_dev->driver->tx_start)(com_dev->lower_id,
								fifoBuf_getUsed(&com_dev->tx));
				}
#if defined(PIOS_INCLUDE_FREERTOS)
				if (xSemaphoreTake(com_dev->tx_sem, 5000) != pdTRUE) {
					return -3;
				}
#endif
				continue;
			default:
				/* Unhandled return code */
				return rc;
			}
		}
	}

	return len;
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
uint16_t PIOS_COM_ReceiveBuffer(uint32_t com_id, uint8_t * buf, uint16_t buf_len, uint32_t timeout_ms)
{
	PIOS_Assert(buf);
	PIOS_Assert(buf_len);

	struct pios_com_dev * com_dev = (struct pios_com_dev *)com_id;

	if (!PIOS_COM_validate(com_dev)) {
		/* Undefined COM port for this board (see pios_board.c) */
		PIOS_Assert(0);
	}
	PIOS_Assert(com_dev->has_rx);

 check_again:
	PIOS_IRQ_Disable();
	uint16_t bytes_from_fifo = fifoBuf_getData(&com_dev->rx, buf, buf_len);
	PIOS_IRQ_Enable();

	if (bytes_from_fifo == 0) {
		/* No more bytes in receive buffer */
		/* Make sure the receiver is running while we wait */
		if (com_dev->driver->rx_start) {
			/* Notify the lower layer that there is now room in the rx buffer */
			(com_dev->driver->rx_start)(com_dev->lower_id,
						    fifoBuf_getFree(&com_dev->rx));
		}
		if (timeout_ms > 0) {
#if defined(PIOS_INCLUDE_FREERTOS)
			if (xSemaphoreTake(com_dev->rx_sem, timeout_ms / portTICK_RATE_MS) == pdTRUE) {
				/* Make sure we don't come back here again */
				timeout_ms = 0;
				goto check_again;
			}
#else
			PIOS_DELAY_WaitmS(1);
			timeout_ms--;
			goto check_again;
#endif
		}
	}

	/* Return received byte */
	return (bytes_from_fifo);
}

#endif

/**
 * @}
 * @}
 */
