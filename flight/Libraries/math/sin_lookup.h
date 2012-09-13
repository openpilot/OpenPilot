/**
 ******************************************************************************
 * @addtogroup OpenPilot Math Utilities
 * @{
 * @addtogroup Sine and cosine methods that use a cached lookup table
 * @{
 *
 * @file       sin_lookup.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief      Sine lookup table from flash with 1 degree resolution
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

#ifndef SIN_LOOKUP_H
#define SIN_LOOKUP_H

int sin_lookup_initalize();
float sin_lookup_deg(float angle);
float cos_lookup_deg(float angle);
float sin_lookup_rad(float angle);
float cos_lookup_rad(float angle);

#endif