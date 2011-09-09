/**
 ******************************************************************************
 *
 * @file       cachedsvgitem.h
 * @author     Dmytro Poplavskiy Copyright (C) 2011.
 * @{
 * @brief OpenGL texture cached SVG item
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

#ifndef CACHEDSVGITEM_H
#define CACHEDSVGITEM_H

#include <QGraphicsSvgItem>
#include <QGLContext>

#include "utils_global.h"

class QGLContext;

//Cache Svg item as GL Texture.
//Texture is regenerated each time item is scaled
//but it's reused during rotation, unlike DeviceCoordinateCache mode
class QTCREATOR_UTILS_EXPORT CachedSvgItem: public QGraphicsSvgItem
{
    Q_OBJECT
public:
    CachedSvgItem(QGraphicsItem * parent = 0);
    CachedSvgItem(const QString & fileName, QGraphicsItem * parent = 0);
    ~CachedSvgItem();

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    QGLContext *m_context;
    GLuint m_texture;
    qreal m_scale;
};

#endif
