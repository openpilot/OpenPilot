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

/* Public Functions */
extern int32_t PIOS_COM_Init(void);
extern int32_t PIOS_COM_Close(void);
extern int32_t PIOS_COM_ChangeBaud(uint8_t port, uint32_t baud);
extern int32_t PIOS_COM_SendCharNonBlocking(uint8_t port, char c);
extern int32_t PIOS_COM_SendChar(uint8_t port, char c);
extern int32_t PIOS_COM_SendBufferNonBlocking(uint8_t port, uint8_t *buffer, uint16_t len);
extern int32_t PIOS_COM_SendBuffer(uint8_t port, uint8_t *buffer, uint16_t len);
extern int32_t PIOS_COM_SendStringNonBlocking(uint8_t port, char *str);
extern int32_t PIOS_COM_SendString(uint8_t port, char *str);
extern int32_t PIOS_COM_SendFormattedStringNonBlocking(uint8_t port, char *format, ...);
extern int32_t PIOS_COM_SendFormattedString(uint8_t port, char *format, ...);
extern uint8_t PIOS_COM_ReceiveBuffer(uint8_t port);
extern int32_t PIOS_COM_ReceiveBufferUsed(uint8_t port);

extern int32_t PIOS_COM_ReceiveHandler(void);

struct pios_com_driver {
  void    (*init)(uint8_t id);
  void    (*set_baud)(uint8_t id, uint32_t baud);
  int32_t (*tx_nb)(uint8_t id, char *buffer, uint16_t len);
  int32_t (*tx)(uint8_t id, char *buffer, uint16_t len);
  int32_t (*rx)(uint8_t id);
  int32_t (*rx_avail)(uint8_t id);
};

#endif /* PIOS_COM_H */
