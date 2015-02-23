/**
 ******************************************************************************
 * @addtogroup OpenPilot Math Utilities
 * @{
 * @addtogroup Reuseable vector data type and functions
 * @{
 *
 * @file       vectors.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @brief      Reuseable vector data type and functions
 *
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

#ifndef VECTORS_H_
#define VECTORS_H_

#include <math.h>
#include <stdint.h>

#define DECLAREVECTOR3(suffix, datatype) \
    typedef struct Vector3##suffix##_t { \
        datatype x; \
        datatype y; \
        datatype z; \
    } Vector3##suffix

#define DECLAREVECTOR2(suffix, datatype) \
    typedef struct Vector2##suffix##_t { \
        datatype x; \
        datatype y; \
    } Vector2##suffix


DECLAREVECTOR3(i16, int16_t);
DECLAREVECTOR3(i32, int32_t);
DECLAREVECTOR3(u16, uint16_t);
DECLAREVECTOR3(u32, uint32_t);
DECLAREVECTOR3(f, float);

DECLAREVECTOR2(i16, int16_t);
DECLAREVECTOR2(i32, int32_t);
DECLAREVECTOR2(u16, uint16_t);
DECLAREVECTOR2(u32, uint32_t);
DECLAREVECTOR2(f, float);

#endif /* VECTORS_H_ */
