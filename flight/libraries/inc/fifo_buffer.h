/**
 ******************************************************************************
 *
 * @file       fifo_buffer.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      GPIO functions header.
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

#ifndef _FIFO_BUFFER_H_
#define _FIFO_BUFFER_H_

#include "stdint.h"

// *********************

typedef struct
{
    uint8_t *buf_ptr;
    volatile uint16_t rd;
    volatile uint16_t wr;
    uint16_t buf_size;
} t_fifo_buffer;

// *********************

uint16_t fifoBuf_getSize(t_fifo_buffer *buf);

uint16_t fifoBuf_getUsed(t_fifo_buffer *buf);
uint16_t fifoBuf_getFree(t_fifo_buffer *buf);

void fifoBuf_clearData(t_fifo_buffer *buf);
void fifoBuf_removeData(t_fifo_buffer *buf, uint16_t len);

int16_t fifoBuf_getBytePeek(t_fifo_buffer *buf);
int16_t fifoBuf_getByte(t_fifo_buffer *buf);

uint16_t fifoBuf_getDataPeek(t_fifo_buffer *buf, void *data, uint16_t len);
uint16_t fifoBuf_getData(t_fifo_buffer *buf, void *data, uint16_t len);

uint16_t fifoBuf_putByte(t_fifo_buffer *buf, const uint8_t b);

uint16_t fifoBuf_putData(t_fifo_buffer *buf, const void *data, uint16_t len);

void fifoBuf_init(t_fifo_buffer *buf, const void *buffer, const uint16_t buffer_size);

// *********************

#endif
