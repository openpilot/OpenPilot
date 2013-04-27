/**
 ******************************************************************************
 *
 * @file       WorldMagModel.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Include file of the WorldMagModel exposed functionality.
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

#ifndef WORLDMAGMODEL_H_
#define WORLDMAGMODEL_H_

	//  Exposed Function Prototypes
int WMM_Initialize();
int WMM_GetMagVector(float Lat, float Lon, float AltEllipsoid, uint16_t Month, uint16_t Day, uint16_t Year, float B[3]);

#endif /* WORLDMAGMODEL_H_ */
