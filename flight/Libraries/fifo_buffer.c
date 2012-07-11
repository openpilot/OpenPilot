/**
 ******************************************************************************
 *
 * @file       fifo_buffer.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      GPIO input functions
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

#include <string.h>

#include "fifo_buffer.h"

// *****************************************************************************
// circular buffer functions

uint16_t fifoBuf_getSize(t_fifo_buffer *buf)
{       // return the usable size of the buffer

    uint16_t buf_size = buf->buf_size;

    if (buf_size > 0)
      return (buf_size - 1);
    else
      return 0;
}

uint16_t fifoBuf_getUsed(t_fifo_buffer *buf)
{       // return the number of bytes available in the rx buffer

    uint16_t rd = buf->rd;
    uint16_t wr = buf->wr;
    uint16_t buf_size = buf->buf_size;

    uint16_t num_bytes = wr - rd;
    if (wr < rd)
        num_bytes = (buf_size - rd) + wr;

    return num_bytes;
}

uint16_t fifoBuf_getFree(t_fifo_buffer *buf)
{       // return the free space size in the buffer

    uint16_t buf_size = buf->buf_size;

    uint16_t num_bytes = fifoBuf_getUsed(buf);

    return ((buf_size - num_bytes) - 1);
}

void fifoBuf_clearData(t_fifo_buffer *buf)
{       // remove all data from the buffer
	buf->rd = buf->wr;
}

void fifoBuf_removeData(t_fifo_buffer *buf, uint16_t len)
{       // remove a number of bytes from the buffer

    uint16_t rd = buf->rd;
    uint16_t buf_size = buf->buf_size;

    // get number of bytes available
    uint16_t num_bytes = fifoBuf_getUsed(buf);

    if (num_bytes > len)
        num_bytes = len;

    if (num_bytes < 1)
        return;                         // nothing to remove

    rd += num_bytes;
    if (rd >= buf_size)
        rd -= buf_size;

    buf->rd = rd;
}

int16_t fifoBuf_getBytePeek(t_fifo_buffer *buf)
{	// get a data byte from the buffer without removing it

    uint16_t rd = buf->rd;

    // get number of bytes available
    uint16_t num_bytes = fifoBuf_getUsed(buf);

    if (num_bytes < 1)
        return -1;                      // no byte retuened

    return buf->buf_ptr[rd];            // return the byte
}

int16_t fifoBuf_getByte(t_fifo_buffer *buf)
{       // get a data byte from the buffer

    uint16_t rd = buf->rd;
    uint16_t buf_size = buf->buf_size;
    uint8_t *buff = buf->buf_ptr;

    // get number of bytes available
    uint16_t num_bytes = fifoBuf_getUsed(buf);

    if (num_bytes < 1)
        return -1;                      // no byte returned

    uint8_t b = buff[rd];
    if (++rd >= buf_size)
        rd = 0;

    buf->rd = rd;

    return b;                           // return the byte
}

uint16_t fifoBuf_getDataPeek(t_fifo_buffer *buf, void *data, uint16_t len)
{       // get data from the buffer without removing it

    uint16_t rd = buf->rd;
    uint16_t buf_size = buf->buf_size;
    uint8_t *buff = buf->buf_ptr;

    // get number of bytes available
    uint16_t num_bytes = fifoBuf_getUsed(buf);

    if (num_bytes > len)
        num_bytes = len;

    if (num_bytes < 1)
        return 0;		// return number of bytes copied

    uint8_t *p = (uint8_t *)data;
    uint16_t i = 0;

    while (num_bytes > 0)
    {
        uint16_t j = buf_size - rd;
        if (j > num_bytes)
            j = num_bytes;
        memcpy(p + i, buff + rd, j);
        i += j;
        num_bytes -= j;
        rd += j;
        if (rd >= buf_size)
            rd = 0;
    }

    return i;                   // return number of bytes copied
}

uint16_t fifoBuf_getData(t_fifo_buffer *buf, void *data, uint16_t len)
{       // get data from our rx buffer

    uint16_t rd = buf->rd;
    uint16_t buf_size = buf->buf_size;
    uint8_t *buff = buf->buf_ptr;

    // get number of bytes available
    uint16_t num_bytes = fifoBuf_getUsed(buf);

    if (num_bytes > len)
        num_bytes = len;

    if (num_bytes < 1)
        return 0;               // return number of bytes copied

    uint8_t *p = (uint8_t *)data;
    uint16_t i = 0;

    while (num_bytes > 0)
    {
        uint16_t j = buf_size - rd;
        if (j > num_bytes)
            j = num_bytes;
        memcpy(p + i, buff + rd, j);
        i += j;
        num_bytes -= j;
        rd += j;
        if (rd >= buf_size)
            rd = 0;
    }

    buf->rd = rd;

    return i;                   // return number of bytes copied
}

uint16_t fifoBuf_putByte(t_fifo_buffer *buf, const uint8_t b)
{       // add a data byte to the buffer

    uint16_t wr = buf->wr;
    uint16_t buf_size = buf->buf_size;
    uint8_t *buff = buf->buf_ptr;

    uint16_t num_bytes = fifoBuf_getFree(buf);
    if (num_bytes < 1)
        return 0;

    buff[wr] = b;
    if (++wr >= buf_size)
        wr = 0;

    buf->wr = wr;

    return 1;                   // return number of bytes copied
}

uint16_t fifoBuf_putData(t_fifo_buffer *buf, const void *data, uint16_t len)
{       // add data to the buffer

    uint16_t wr = buf->wr;
    uint16_t buf_size = buf->buf_size;
    uint8_t *buff = buf->buf_ptr;

    uint16_t num_bytes = fifoBuf_getFree(buf);
    if (num_bytes > len)
        num_bytes = len;

    if (num_bytes < 1)
        return 0;               // return number of bytes copied

    uint8_t *p = (uint8_t *)data;
    uint16_t i = 0;

    while (num_bytes > 0)
    {
        uint16_t j = buf_size - wr;
        if (j > num_bytes)
            j = num_bytes;
        memcpy(buff + wr, p + i, j);
        i += j;
        num_bytes -= j;
        wr += j;
        if (wr >= buf_size)
            wr = 0;
    }

    buf->wr = wr;

    return i;                   // return number of bytes copied
}

void fifoBuf_init(t_fifo_buffer *buf, const void *buffer, const uint16_t buffer_size)
{
    buf->buf_ptr = (uint8_t *)buffer;
    buf->rd = 0;
    buf->wr = 0;
    buf->buf_size = buffer_size;
}

// *****************************************************************************
