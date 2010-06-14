/**
******************************************************************************
*
* @file       tilematrix.cpp
* @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
*             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
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
#include "tilematrix.h"

 
namespace internals {
TileMatrix::TileMatrix()
{
}
void TileMatrix::Clear()
{
    mutex.lock();
    foreach(Tile* t,matrix.values())
    {
        delete t;
        t=0;
    }
    matrix.clear();
    mutex.unlock();
}

void TileMatrix::ClearPointsNotIn(QList<Point>list)
{
    removals.clear();
    mutex.lock();
    foreach(Point p, matrix.keys())
    {
        if(!list.contains(p))
        {
            removals.append(p);
        }
    }
    mutex.unlock();
    foreach(Point p,removals)
    {
        Tile* t=TileAt(p);
        if(t!=0)
        {
            mutex.lock();
            delete t;
            t=0;
            matrix.remove(p);
            mutex.unlock();
        }

    }
    removals.clear();
}
Tile* TileMatrix::TileAt(const Point &p)
{

#ifdef DEBUG_TILEMATRIX
    qDebug()<<"TileMatrix:TileAt:"<<p.ToString();
#endif //DEBUG_TILEMATRIX
    Tile* ret;
    mutex.lock();
    ret=matrix.value(p,0);
    mutex.unlock();
    return ret;
}
void TileMatrix::SetTileAt(const Point &p, Tile* tile)
{
    mutex.lock();
    matrix.insert(p,tile);
    mutex.unlock();
}
}
