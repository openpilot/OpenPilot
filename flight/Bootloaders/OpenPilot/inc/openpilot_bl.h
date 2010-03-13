/**
 ******************************************************************************
 *
 * @file       openpilot_bl.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Main OpenPilot Bootloader header.
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


#ifndef OPENPILOT_BL_H
#define OPENPILOT_BL_H

#define OPBL_MAGIC_TIMEOUT		2000
#define OPBL_COM_PORT			COM_DEBUG_USART

/* PIOS Includes */
#include <pios.h>
#include "bootloader.h"
#include "ymodem.h"

#endif /* OPENPILOT_BL_H */
