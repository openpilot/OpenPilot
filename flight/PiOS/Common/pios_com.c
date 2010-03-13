/**
 ******************************************************************************
 *
 * @file       pios_com.c  
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * 	       Parts by Thorsten Klose (tk@midibox.org)
 * @brief      COM layer functions
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   PIOS_COM COM layer functions
 * @{
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


/* Global Variables */

/* Local Variables */
static int32_t (*receive_callback_func)(COMPortTypeDef port, char c);


/**
* Initialises COM layer
* \param[in] mode currently only mode 0 supported
* \return < 0 if initialisation failed
*/
int32_t PIOS_COM_Init(void)
{
	int32_t ret = 0;

	/* Disable callback by default */
	receive_callback_func = NULL;

	/* If any COM assignment: */
#if defined(PIOS_INCLUDE_USART)
	PIOS_USART_Init();
#endif

#if defined(PIOS_INCLUDE_USB_HID)
	PIOS_USB_HID_Init(0);
#endif

	return ret;
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
int32_t PIOS_COM_SendBufferNonBlocking(COMPortTypeDef port, uint8_t *buffer, uint16_t len)
{
	/* Branch depending on selected port */
	switch(port) {
#if defined(PIOS_INCLUDE_USART)
		case COM_DEBUG_USART:
			return PIOS_USART_TxBufferPutMoreNonBlocking(PIOS_COM_DEBUG_PORT, buffer, len);
		case COM_USART1:
			return PIOS_USART_TxBufferPutMoreNonBlocking(USART_1, buffer, len);
		case COM_USART2:
			return PIOS_USART_TxBufferPutMoreNonBlocking(USART_2, buffer, len);
		case COM_USART3:
			return PIOS_USART_TxBufferPutMoreNonBlocking(USART_3, buffer, len);
#endif
		case COM_USB_HID:
			return PIOS_USB_HID_TxBufferPutMoreNonBlocking(buffer, len);
		default:
			/* Invalid port */
			return -1;
	}
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
int32_t PIOS_COM_SendBuffer(COMPortTypeDef port, uint8_t *buffer, uint16_t len)
{
	/* Branch depending on selected port */
	switch(port) {
#if defined(PIOS_INCLUDE_USART)
		case COM_DEBUG_USART:
			return PIOS_USART_TxBufferPutMore(PIOS_COM_DEBUG_PORT, buffer, len);
		case COM_USART1:
			return PIOS_USART_TxBufferPutMore(USART_1, buffer, len);
		case COM_USART2:
			return PIOS_USART_TxBufferPutMore(USART_2, buffer, len);
		case COM_USART3:
			return PIOS_USART_TxBufferPutMore(USART_3, buffer, len);
#endif
		case COM_USB_HID:
			return PIOS_USB_HID_TxBufferPutMore(buffer, len);
		default:
			/* Invalid port */
			return -1;
	}
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
int32_t PIOS_COM_SendCharNonBlocking(COMPortTypeDef port, char c)
{
	return PIOS_COM_SendBufferNonBlocking(port, (uint8_t *)&c, 1);
}

/**
* Sends a single character over given port
* (blocking function)
* \param[in] port COM port
* \param[in] c character
* \return -1 if port not available
* \return 0 on success
*/
int32_t PIOS_COM_SendChar(COMPortTypeDef port, char c)
{
	return PIOS_COM_SendBuffer(port, (uint8_t *)&c, 1);
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
int32_t PIOS_COM_SendStringNonBlocking(COMPortTypeDef port, char *str)
{
	return PIOS_COM_SendBufferNonBlocking(port, (uint8_t *)str, (uint16_t)strlen(str));
}

/**
* Sends a string over given port
* (blocking function)
* \param[in] port COM port
* \param[in] str zero-terminated string
* \return -1 if port not available
* \return 0 on success
*/
int32_t PIOS_COM_SendString(COMPortTypeDef port, char *str)
{
	return PIOS_COM_SendBuffer(port, (uint8_t *)str, strlen(str));
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
int32_t PIOS_COM_SendFormattedStringNonBlocking(COMPortTypeDef port, char *format, ...)
{
	uint8_t buffer[128]; // TODO: tmp!!! Provide a streamed COM method later!

	va_list args;

	va_start(args, format);
	vsprintf((char *)buffer, format, args);
	return PIOS_COM_SendBufferNonBlocking(port, buffer, (uint16_t)strlen((char *)buffer));
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
int32_t PIOS_COM_SendFormattedString(COMPortTypeDef port, char *format, ...)
{
	uint8_t buffer[128]; // TODO: tmp!!! Provide a streamed COM method later!
	va_list args;

	va_start(args, format);
	vsprintf((char *)buffer, format, args);
	return PIOS_COM_SendBuffer(port, buffer, (uint16_t)strlen((char *)buffer));
}

/**
* Transfer bytes from port buffers into another buffer
* \param[in] port COM port
* \returns Byte from buffer
*/
uint8_t PIOS_COM_ReceiveBuffer(COMPortTypeDef port)
{
	switch(port) {
#if defined(PIOS_INCLUDE_USART)
		case COM_DEBUG_USART:
			return PIOS_USART_RxBufferGet(PIOS_COM_DEBUG_PORT);
		case COM_USART1:
			return PIOS_USART_RxBufferGet(USART_1);
		case COM_USART2:
			return PIOS_USART_RxBufferGet(USART_2);
		case COM_USART3:
			return PIOS_USART_RxBufferGet(USART_3);
#endif
		case COM_USB_HID:
			return PIOS_USB_HID_RxBufferGet();
		/* To suppress warnings */
		default:
			return 0;
	}
}

/**
* Get the number of bytes waiting in the buffer
* \param[in] port COM port
* \return Number of bytes used in buffer
*/
int32_t PIOS_COM_ReceiveBufferUsed(COMPortTypeDef port)
{
	switch(port) {
#if defined(PIOS_INCLUDE_USART)
		case COM_DEBUG_USART:
			return PIOS_USART_RxBufferUsed(PIOS_COM_DEBUG_PORT);
		case COM_USART1:
			return PIOS_USART_RxBufferUsed(USART_1);
		case COM_USART2:
			return PIOS_USART_RxBufferUsed(USART_2);
		case COM_USART3:
			return PIOS_USART_RxBufferUsed(USART_3);
#endif
		case COM_USB_HID:
			return PIOS_USB_HID_DATA_LENGTH;
		/* To suppress warnings */
		default:
			return 0;
	}
}

#endif
