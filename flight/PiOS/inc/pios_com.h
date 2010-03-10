/**
 ******************************************************************************
 *
 * @file       pios_com.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	       Parts by Thorsten Klose (tk@midibox.org)
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

/* Global Types */
typedef enum {
	COM_DEBUG_USART = 0,
	COM_USART1 = 1,
	COM_USART2 = 2,
	COM_USART3 = 3,
	COM_USB_HID = 4
} COMPortTypeDef;

/* Public Functions */
extern int32_t PIOS_COM_Init(void);

extern int32_t PIOS_COM_SendCharNonBlocking(COMPortTypeDef port, char c);
extern int32_t PIOS_COM_SendChar(COMPortTypeDef port, char c);
extern int32_t PIOS_COM_SendBufferNonBlocking(COMPortTypeDef port, uint8_t *buffer, uint16_t len);
extern int32_t PIOS_COM_SendBuffer(COMPortTypeDef port, uint8_t *buffer, uint16_t len);
extern int32_t PIOS_COM_SendStringNonBlocking(COMPortTypeDef port, char *str);
extern int32_t PIOS_COM_SendString(COMPortTypeDef port, char *str);
extern int32_t PIOS_COM_SendFormattedStringNonBlocking(COMPortTypeDef port, char *format, ...);
extern int32_t PIOS_COM_SendFormattedString(COMPortTypeDef port, char *format, ...);
extern uint8_t PIOS_COM_ReceiveBuffer(COMPortTypeDef port);
extern int32_t PIOS_COM_ReceiveBufferUsed(COMPortTypeDef port);

extern int32_t PIOS_COM_ReceiveHandler(void);

#endif /* PIOS_COM_H */
