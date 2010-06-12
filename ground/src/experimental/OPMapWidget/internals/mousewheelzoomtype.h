/**
******************************************************************************
*
* @file       mousewheelzoomtype.h
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
#ifndef MOUSEWHEELZOOMTYPE_H
#define MOUSEWHEELZOOMTYPE_H
#include <QObject>
#include <QStringList>
#include <QMetaType>
 
namespace internals {
    struct MouseWheelZoomType:public QObject
{
    Q_OBJECT
public:
    enum Types
    {
        /// <summary>
        /// zooms map to current mouse position and makes it map center
        /// </summary>
        MousePositionAndCenter,

        /// <summary>
        /// zooms to current mouse position, but doesn't make it map center,
        /// google/bing style ;}
        /// </summary>
        MousePositionWithoutCenter,

        /// <summary>
        /// zooms map to current view center
        /// </summary>
        ViewCenter,
    };
    Q_ENUMS(Types)
    static QStringList TypesStrList(){return strList;}
    static Types TypeByStr(QString const& value){return (Types)MouseWheelZoomType::strList.indexOf(value);}
private:
    static QStringList strList;
};

}
Q_DECLARE_METATYPE(internals::MouseWheelZoomType::Types)
#endif // MOUSEWHEELZOOMTYPE_H

