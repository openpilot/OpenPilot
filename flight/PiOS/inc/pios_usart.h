/**
 ******************************************************************************
 *
 * @file       pios_usart.h  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2009.   
 * @brief      USART functions header.
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

#ifndef PIOS_USART_H
#define PIOS_USART_H


/* Global Types */
typedef enum {GPS = 0, TELEM = 1, AUX = 2} USARTNumTypeDef;


/* Public Functions */
extern void USARTInit(void);
extern void USARTEnableAux(void);
extern void USARTDisableAux(void);
extern void USARTChangeBaud(USART_TypeDef* USARTx, uint32_t Baud);

extern int USARTRxBufferFree(USARTNumTypeDef uart);
extern int USARTRxBufferUsed(USARTNumTypeDef uart);
extern int USARTRxBufferGet(USARTNumTypeDef uart);
extern int USARTRxBufferPeek(USARTNumTypeDef uart);
extern int USARTRxBufferPut(USARTNumTypeDef uart, uint8_t b);

extern int USARTTxBufferFree(USARTNumTypeDef uart);
extern int USARTTxBufferGet(USARTNumTypeDef uart);
extern int USARTTxBufferPutMoreNonBlocking(USARTNumTypeDef uart, uint8_t *buffer, uint16_t len);
extern int USARTTxBufferPutMore(USARTNumTypeDef uart, uint8_t *buffer, uint16_t len);
extern int USARTTxBufferPutNonBlocking(uint8_t uart, uint8_t b);
extern int USARTTxBufferPut(USARTNumTypeDef uart, uint8_t b);

#endif /* PIOS_USART_H */
