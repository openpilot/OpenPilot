
/**
 * Project: OpenPilot
 *    
 * @author The OpenPilot Team, http://www.openpilot.org, Copyright (C) 2009.
 *  
 * @file pios_uart.h
 * UART functions header 
 *  
 * @see The GNU Public License (GPL)
 */
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

#ifndef PIOS_UART_H
#define PIOS_UART_H

/* Public Functions */
extern void UARTInit(void);
extern void EnableAuxUART(void);
extern void DisableAuxUART(void);
extern void UARTChangeBaud(USART_TypeDef* USARTx, uint32_t Baud);

/*----------------------------------------------------------------------------------*/
/* WORK IN PROGRESS BELOW */
/*----------------------------------------------------------------------------------*/
typedef enum {GPS = 0, TELEM = 1, AUX = 2} UARTNumTypeDef;

extern int UARTRxBufferFree(UARTNumTypeDef uart);
extern int UARTRxBufferUsed(UARTNumTypeDef uart);
extern int UARTRxBufferGet(UARTNumTypeDef uart);
extern int UARTRxBufferPeek(UARTNumTypeDef uart);
extern int UARTRxBufferPut(UARTNumTypeDef uart, uint8_t b);

extern int UARTTxBufferFree(UARTNumTypeDef uart);
extern int UARTTxBufferGet(UARTNumTypeDef uart);
extern int UARTTxBufferPutMore_NonBlocking(UARTNumTypeDef uart, uint8_t *buffer, uint16_t len);
extern int UARTTxBufferPutMore(UARTNumTypeDef uart, uint8_t *buffer, uint16_t len);
extern int UARTTxBufferPut_NonBlocking(uint8_t uart, uint8_t b);
extern int UARTTxBufferPut(UARTNumTypeDef uart, uint8_t b);

#endif /* PIOS_UART_H */
