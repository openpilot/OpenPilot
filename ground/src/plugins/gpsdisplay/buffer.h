/**
 ******************************************************************************
 *
 * @file       buffer.c
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      see below
 * 	       As with all modules only the initialize function is exposed all other
 * 	       interactions with the module take place through the event queue and
 *             objects.
 * @see        The GNU Public License (GPL) Version 3
 *
 *****************************************************************************/

/*! \file buffer.h \brief Multipurpose byte buffer structure and methods. */
//*****************************************************************************
//
// File Name	: 'buffer.h'
// Title		: Multipurpose byte buffer structure and methods
// Author		: Pascal Stang - Copyright (C) 2001-2002
// Created		: 9/23/2001
// Revised		: 11/16/2002
// Version		: 1.1
// Target MCU	: any
// Editor Tabs	: 4
//
///	\ingroup general
/// \defgroup buffer Circular Byte-Buffer Structure and Function Library (buffer.c)
/// \code #include "buffer.h" \endcode
/// \par Overview
///		This byte-buffer structure provides an easy and efficient way to store
///		and process a stream of bytes.  You can create as many buffers as you
///		like (within memory limits), and then use this common set of functions to
///		access each buffer.  The buffers are designed for FIFO operation (first
///		in, first out).  This means that the first byte you put in the buffer
///		will be the first one you get when you read out the buffer.  Supported
///		functions include buffer initialize, get byte from front of buffer, add
///		byte to end of buffer, check if buffer is full, and flush buffer.  The
///		buffer uses a circular design so no copying of data is ever necessary.
///		This buffer is not dynamically allocated, it has a user-defined fixed
///		maximum size.  This buffer is used in many places in the avrlib code.
//
// This code is distributed under the GNU Public License
//		which can be found at http://www.gnu.org/licenses/gpl.txt
//
//*****************************************************************************
//@{

#ifndef BUFFER_HPP
#define BUFFER_HPP

// structure/typdefs

//! cBuffer structure
typedef struct struct_cBuffer
{
	unsigned char *dataptr;			///< the physical memory address where the buffer is stored
	unsigned short size;			///< the allocated size of the buffer
	unsigned short datalength;		///< the length of the data currently in the buffer
	unsigned short dataindex;		///< the index into the buffer where the data starts
} cBuffer;

// function prototypes

//! initialize a buffer to start at a given address and have given size
void			bufferInit(cBuffer* buffer, unsigned char *start, unsigned short size);

//! get the first byte from the front of the buffer
unsigned char	bufferGetFromFront(cBuffer* buffer);

//! dump (discard) the first numbytes from the front of the buffer
void bufferDumpFromFront(cBuffer* buffer, unsigned short numbytes);

//! get a byte at the specified index in the buffer (kind of like array access)
// ** note: this does not remove the byte that was read from the buffer
unsigned char	bufferGetAtIndex(cBuffer* buffer, unsigned short index);

//! add a byte to the end of the buffer
unsigned char	bufferAddToEnd(cBuffer* buffer, unsigned char data);

//! check if the buffer is full/not full (returns non-zero value if not full)
unsigned char	bufferIsNotFull(cBuffer* buffer);

//! flush (clear) the contents of the buffer
void			bufferFlush(cBuffer* buffer);

#endif
//@}
