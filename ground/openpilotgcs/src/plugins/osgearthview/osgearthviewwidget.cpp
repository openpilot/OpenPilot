/********************************************************************************
* @file       osgearthviewwidget.cpp
* @author     The OpenPilot Team Copyright (C) 2012.
* @addtogroup GCSPlugins GCS Plugins
* @{
* @addtogroup OsgEarthview Plugin Widget
* @{
* @brief Osg Earth view of UAV
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

#include "osgearthviewwidget.h"
#include <utils/stylehelper.h>
#include <utils/cachedsvgitem.h>
#include <iostream>
#include <QDebug>
#include <QPainter>
#include <QtOpenGL/QGLWidget>
#include <cmath>

OsgEarthviewWidget::OsgEarthviewWidget(QWidget *parent) : QGraphicsView(parent)
{
}

OsgEarthviewWidget::~OsgEarthviewWidget()
{
}

void OsgEarthviewWidget::paint()
{
}

void OsgEarthviewWidget::paintEvent(QPaintEvent *event)
{
}

void OsgEarthviewWidget::resizeEvent(QResizeEvent *event)
{
}
