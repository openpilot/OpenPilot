/**
 ******************************************************************************
 *
 * @file       pios_usart.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	       Parts by Thorsten Klose (tk@midibox.org)
 * @brief      UDP functions header.
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

#ifndef PIOS_UDP_H
#define PIOS_UDP_H


/* Global Types */

/* Public Functions */
//extern void PIOS_UDP_Init(void);
void PIOS_UDP_Init(void);
void PIOS_UDP_Close(void);
extern void PIOS_UDP_ChangeBaud(uint8_t usart, uint32_t baud);

extern int32_t PIOS_UDP_RxBufferFree(uint8_t usart);
extern int32_t PIOS_UDP_RxBufferUsed(uint8_t usart);
extern int32_t PIOS_UDP_RxBufferGet(uint8_t usart);
extern int32_t PIOS_UDP_RxBufferPeek(uint8_t usart);
extern int32_t PIOS_UDP_RxBufferPut(uint8_t usart, uint8_t b);

extern int32_t PIOS_UDP_TxBufferFree(uint8_t usart);
extern int32_t PIOS_UDP_TxBufferGet(uint8_t usart);
extern int32_t PIOS_UDP_TxBufferPutMoreNonBlocking(uint8_t usart, char *buffer, uint16_t len);
extern int32_t PIOS_UDP_TxBufferPutMore(uint8_t usart, char *buffer, uint16_t len);
extern int32_t PIOS_UDP_TxBufferPutNonBlocking(uint8_t usart, char b);
extern int32_t PIOS_UDP_TxBufferPut(uint8_t usart, char b);

#endif /* PIOS_UDP_H */
