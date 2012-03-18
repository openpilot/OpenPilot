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
#include <iostream>
#include <QDebug>
#include <QPainter>
#include <QtOpenGL/QGLWidget>
#include <cmath>
#include <QtGui/QApplication>
#include <QLabel>
#include <QDebug>

#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QGridLayout>


#include <osg/Notify>
#include <osg/PositionAttitudeTransform>

#include <osgUtil/Optimizer>
#include <osgGA/StateSetManipulator>
#include <osgGA/GUIEventHandler>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgEarth/MapNode>
#include <osgEarth/XmlUtils>
#include <osgEarth/Viewpoint>

#include <osgEarthSymbology/Color>

#include <osgEarthAnnotation/AnnotationRegistry>
#include <osgEarthAnnotation/AnnotationData>
#include <osgEarthAnnotation/Decluttering>

#include <osgEarthDrivers/kml/KML>
#include <osgEarthDrivers/ocean_surface/OceanSurface>
#include <osgEarthDrivers/cache_filesystem/FileSystemCache>

#include <osgEarthUtil/EarthManipulator>
#include <osgEarthUtil/AutoClipPlaneHandler>
#include <osgEarthUtil/Controls>
#include <osgEarthUtil/SkyNode>
#include <osgEarthUtil/LatLongFormatter>
#include <osgEarthUtil/MouseCoordsTool>
#include <osgEarthUtil/ObjectLocator>

using namespace osgEarth::Util;
using namespace osgEarth::Util::Controls;
using namespace osgEarth::Symbology;
using namespace osgEarth::Drivers;
using namespace osgEarth::Annotation;

#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>

#include <osgDB/ReadFile>

#include <osgQt/GraphicsWindowQt>

#include <iostream>

#include "ui_osgearthview.h"

#include "utils/stylehelper.h"
#include "utils/homelocationutil.h"
#include "utils/worldmagmodel.h"
#include "utils/coordinateconversions.h"
#include "attitudeactual.h"
#include "homelocation.h"
#include "positionactual.h"

using namespace Utils;

OsgEarthviewWidget::OsgEarthviewWidget(QWidget *parent) : QWidget(parent)
{

    m_widget = new Ui_OsgEarthview();
    m_widget->setupUi(this);

    /*viewWidget = new OsgViewerWidget(this);
    viewWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setLayout(new QVBoxLayout());
    layout()->addWidget(viewWidget);*/
}

OsgEarthviewWidget::~OsgEarthviewWidget()
{
}

void OsgEarthviewWidget::paintEvent( QPaintEvent* event )
{
}

void OsgEarthviewWidget::resizeEvent(QResizeEvent *event)
{
}

