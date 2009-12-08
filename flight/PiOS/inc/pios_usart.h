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
extern void PIOS_USART_Init(void);
extern void PIOS_USART_EnableAux(void);
extern void PIOS_USART_DisableAux(void);
extern void PIOS_USART_ChangeBaud(USART_TypeDef* USARTx, uint32_t Baud);

extern int PIOS_USART_RxBufferFree(USARTNumTypeDef uart);
extern int PIOS_USART_RxBufferUsed(USARTNumTypeDef uart);
extern int PIOS_USART_RxBufferGet(USARTNumTypeDef uart);
extern int PIOS_USART_RxBufferPeek(USARTNumTypeDef uart);
extern int PIOS_USART_RxBufferPut(USARTNumTypeDef uart, uint8_t b);

extern int PIOS_USART_TxBufferFree(USARTNumTypeDef uart);
extern int PIOS_USART_TxBufferGet(USARTNumTypeDef uart);
extern int PIOS_USART_TxBufferPutMoreNonBlocking(USARTNumTypeDef uart, uint8_t *buffer, uint16_t len);
extern int PIOS_USART_TxBufferPutMore(USARTNumTypeDef uart, uint8_t *buffer, uint16_t len);
extern int PIOS_USART_TxBufferPutNonBlocking(uint8_t uart, uint8_t b);
extern int PIOS_USART_TxBufferPut(USARTNumTypeDef uart, uint8_t b);

#endif /* PIOS_USART_H */
