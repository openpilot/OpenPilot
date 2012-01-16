/**
 ******************************************************************************
 * @addtogroup PIOS PIOS Core hardware abstraction layer
 * @{
 * @addtogroup PIOS_COM COM MSG layer functions
 * @brief Hardware communication layer
 * @{
 *
 * @file       pios_com_msg.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      COM MSG layer functions header
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

#ifndef PIOS_COM_MSG_H
#define PIOS_COM_MSG_H

#include <stdint.h>		/* uint*_t */

/* Public Functions */
extern int32_t PIOS_COM_MSG_Send(uint32_t com_id, const uint8_t *msg, uint16_t msg_len);
extern uint16_t PIOS_COM_MSG_Receive(uint32_t com_id, uint8_t * buf, uint16_t buf_len);

#endif /* PIOS_COM_MSG_H */

/**
  * @}
  * @}
  */
