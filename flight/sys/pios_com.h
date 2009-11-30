/**
 ******************************************************************************
 *
 * @file       pios_com.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2009.   
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

/* GLobal Types */
typedef enum {
	COM_DEBUG_UART = 0,
	COM_GPS_UART = 1,
	COM_TELEM_UART = 2,
	COM_AUX_UART = 3
} COMPortTypeDef;

/* Public Functions */
extern int32_t COMInit(void);

extern int32_t COMCheckAvailable(COMPortTypeDef port);

extern int32_t COMSendChar_NonBlocking(COMPortTypeDef port, char c);
extern int32_t COMSendChar(COMPortTypeDef port, char c);
extern int32_t COMSendBuffer_NonBlocking(COMPortTypeDef port, uint8_t *buffer, uint16_t len);
extern int32_t COMSendBuffer(COMPortTypeDef port, uint8_t *buffer, uint16_t len);
extern int32_t COMSendStringNonBlocking(COMPortTypeDef port, char *str);
extern int32_t COMSendString(COMPortTypeDef port, char *str);
extern int32_t COMSendFormattedStringNonBlocking(COMPortTypeDef port, char *format, ...);
extern int32_t COMSendFormattedString(COMPortTypeDef port, char *format, ...);

extern int32_t COMReceiveHandler(void);
extern int32_t COMReceiveCallback_Init(void *callback_receive);

#endif /* PIOS_COM_H */
