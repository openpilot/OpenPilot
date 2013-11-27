/**
 ******************************************************************************
 *
 * @file       pointlatlng.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   OPMapWidget
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
#include "pointlatlng.h"

namespace internals {
PointLatLng PointLatLng::Empty = PointLatLng();
PointLatLng::PointLatLng() : lat(0), lng(0), empty(true)
{}

bool PointLatLng::operator==(PointLatLng const & rhs)
{
    return (this->Lng() == rhs.Lng()) && (this->Lat() == rhs.Lat());
}

bool PointLatLng::operator!=(PointLatLng const & right)
{
    return !(*this == right);
}

PointLatLng PointLatLng::operator+(SizeLatLng sz)
{
    return Add(*this, sz);
}

PointLatLng PointLatLng::operator-(SizeLatLng sz)
{
    return Subtract(*this, sz);
}
}
