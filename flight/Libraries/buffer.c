/**
 ******************************************************************************
 *
 * @addtogroup OpenPilotModules OpenPilot Modules
 * @{ 
 * @addtogroup Flight_Libraries Miscellaneous library functions
 * @brief Miscellaneous library functions shared between PIOS / OpenPilot / AHRS
 * @{ 
 *
 * @file       buffer.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Simplies buffering data
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

//*****************************************************************************
//
// File Name	: 'buffer.c'
// Title		: Multipurpose byte buffer structure and methods
// Author		: Pascal Stang - Copyright (C) 2001-2002
// Created		: 9/23/2001
// Revised		: 9/23/2001
// Version		: 1.0
// Target MCU	: any
// Editor Tabs	: 4
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************

#include "buffer.h"

/**
 * @brief Initialize a cBuffer structure
 * @param[in] buffer Points to the buffer structure
 * @param[in] start Allocated memory to store data
 * @param[in] size Maximum size of buffer
 * @return None
 */
void bufferInit(cBuffer* buffer, uint8_t *start, uint32_t size)
{
	// set start pointer of the buffer
	buffer->dataptr = start;
	buffer->size = size;
	// initialize index and length
	buffer->dataindex = 0;
	buffer->datalength = 0;
}

/**
 * @brief Return remaining space in buffer
 * @param[in] buffer Pointer to buffer structure
 * @return Amount of space remaining on buffer
 */
uint32_t bufferRemainingSpace(cBuffer* buffer) 
{
    return buffer->size - buffer->datalength;
}

/**
 * @brief Return amount of data
 * @param[in] buffer Pointer to buffer structure
 * @return Amount of data queued in buffer
 */
uint32_t bufferBufferedData(cBuffer* buffer) 
{
    return buffer->datalength;
}


/**
 * @brief Pop one element from buffer
 * @param[in] buffer Pointer to the buffer structure
 * @return None
 */
uint8_t bufferGetFromFront(cBuffer* buffer)
{
	unsigned char data = 0;

	// check to see if there's data in the buffer
	if(buffer->datalength)
	{
		// get the first character from buffer
		data = buffer->dataptr[buffer->dataindex];
		// move index down and decrement length
		buffer->dataindex++;
		if(buffer->dataindex >= buffer->size)
		{
			buffer->dataindex %= buffer->size;
		}
		buffer->datalength--;
	}
	// return
	return data;
}

/**
 * @brief Copy number of elements into another buffer
 * @param[in] buffer Pointer to the buffer structure
 * @param[in] dest Point to destimation, must be allocated enough space for size
 * @param[in] size Number of elements to get
 * @return 
 *  @arg -1 for success
 *  @arg 0 error 
 */
uint8_t bufferGetChunkFromFront(cBuffer* buffer, uint8_t * dest, uint32_t size)
{
    if(size > buffer->datalength)
        return -1;

    for(uint32_t i = 0; i < size; i++) 
    {
        dest[i] = bufferGetFromFront(buffer);
    }
    
    return 0;
}    

/**
 * @brief Shift index to trash data from front of buffer
 * @param[in] buffer Pointer to buffer structure
 * @param[in] numbytes Number of bytes to drop
 * @return None
 */
void bufferDumpFromFront(cBuffer* buffer, uint32_t numbytes)
{
	// dump numbytes from the front of the buffer
	// are we dumping less than the entire buffer?
	if(numbytes < buffer->datalength)
	{
		// move index down by numbytes and decrement length by numbytes
		buffer->dataindex += numbytes;
		if(buffer->dataindex >= buffer->size)
		{
			buffer->dataindex %= buffer->size;
		}
		buffer->datalength -= numbytes;
	}
	else
	{
		// flush the whole buffer
		buffer->datalength = 0;
	}
}

/**
 * @brief Get element indexed from the front of buffer
 * @param[in] buffer Point to the buffer structure
 * @param[in] index Index into the buffer relative to front
 * @return None
 */
uint8_t bufferGetAtIndex(cBuffer* buffer, uint32_t index)
{
	// return character at index in buffer
	return buffer->dataptr[(buffer->dataindex+index)%(buffer->size)];
}

/**
 * @brief Queue a character to end of buffer
 * @param[in] buffer Point to the buffer structure
 * @param[in] data Byte to add
 * @return 
 *  @arg -1 for success
 *  @arg 0 error 
 */
uint8_t bufferAddToEnd(cBuffer* buffer, uint8_t data)
{
	// make sure the buffer has room
	if(buffer->datalength < buffer->size)
	{
		// save data byte at end of buffer
		buffer->dataptr[(buffer->dataindex + buffer->datalength) % buffer->size] = data;
		// increment the length
		buffer->datalength++;
		// return success
        return -1;
    }
    else return 0;
}

/**
 * @brief Queue a block of character to end of buffer
 * @param[in] buffer Point to the buffer structure
 * @param[in] data Pointer to data to add
 * @param[in] size Number of bytes to add
 * @return 
 *  @arg -1 for success
 *  @arg 0 error 
 */
uint8_t bufferAddChunkToEnd(cBuffer* buffer, const uint8_t * data, uint32_t size)
{
    // TODO: replace with memcpy and logic, for now keeping it simple
    for(uint32_t i = 0; i < size; i++) 
    {
        if(bufferAddToEnd(buffer,data[i]) == 0)
            return 0;
    }
    return -1;
}

/**
 * @brief Check to see if the buffer has room
 * @param[in] buffer Point to the buffer structure
 * @return
 *  @arg True there is room available in buffer
 *  @arg False buffer is full
 */
unsigned char bufferIsNotFull(cBuffer* buffer)
{
    return (buffer->datalength < buffer->size);
}

/**
 * @brief Trash all data in buffer
 * @param[in] buffer Point to the buffer structure
 */
void bufferFlush(cBuffer* buffer)
{
	// flush contents of the buffer
    buffer->datalength = 0;
}

/** 
  * @}
  * @}
  */
