/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_COM COM layer functions
 * @brief Hardware communication layer
 * @{
 *
 * @file       pios_com.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      COM layer functions header
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

#ifndef PIOS_COM_H
#define PIOS_COM_H

#include <stdint.h>		/* uint*_t */
#include <stdbool.h>		/* bool */

typedef uint16_t (*pios_com_callback)(uint32_t context, uint8_t * buf, uint16_t buf_len, uint16_t * headroom, bool * task_woken);

struct pios_com_driver {
	void (*init)(uint32_t id);
	void (*set_baud)(uint32_t id, uint32_t baud);
	void (*tx_start)(uint32_t id, uint16_t tx_bytes_avail);
	void (*rx_start)(uint32_t id, uint16_t rx_bytes_avail);
	void (*bind_rx_cb)(uint32_t id, pios_com_callback rx_in_cb, uint32_t context);
	void (*bind_tx_cb)(uint32_t id, pios_com_callback tx_out_cb, uint32_t context);
};

/* Public Functions */
extern int32_t PIOS_COM_ChangeBaud(uint32_t com_id, uint32_t baud);
extern int32_t PIOS_COM_SendCharNonBlocking(uint32_t com_id, char c);
extern int32_t PIOS_COM_SendChar(uint32_t com_id, char c);
extern int32_t PIOS_COM_SendBufferNonBlocking(uint32_t com_id, const uint8_t *buffer, uint16_t len);
extern int32_t PIOS_COM_SendBuffer(uint32_t com_id, const uint8_t *buffer, uint16_t len);
extern int32_t PIOS_COM_SendStringNonBlocking(uint32_t com_id, const char *str);
extern int32_t PIOS_COM_SendString(uint32_t com_id, const char *str);
extern int32_t PIOS_COM_SendFormattedStringNonBlocking(uint32_t com_id, const char *format, ...);
extern int32_t PIOS_COM_SendFormattedString(uint32_t com_id, const char *format, ...);
extern uint16_t PIOS_COM_ReceiveBuffer(uint32_t com_id, uint8_t * buf, uint16_t buf_len, uint32_t timeout_ms);

#endif /* PIOS_COM_H */

/**
  * @}
  * @}
  */
