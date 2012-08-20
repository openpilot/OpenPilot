/**
******************************************************************************
*
* @file       trailitem.cpp
* @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
* @brief      A graphicsItem representing a trail point
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
#include "traillineitem.h"

namespace mapcontrol
{
TrailLineItem::TrailLineItem(internals::PointLatLng const& coord1,internals::PointLatLng const& coord2, QBrush color, MapGraphicItem * map):QGraphicsLineItem(map),coord1(coord1),coord2(coord2),m_brush(color),m_map(map)
    {
        QPen pen;
        pen.setBrush(m_brush);
        pen.setWidth(1);
        this->setPen(pen);
    }

    int TrailLineItem::type()const
    {
        return Type;
    }

    void TrailLineItem::setLineSlot()
    {
        setLine(m_map->FromLatLngToLocal(this->coord1).X(),m_map->FromLatLngToLocal(this->coord1).Y(),m_map->FromLatLngToLocal(this->coord2).X(),m_map->FromLatLngToLocal(this->coord2).Y());
    }
}
