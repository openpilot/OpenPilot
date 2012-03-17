/********************************************************************************
* @file       osgearthviewwidget.h
* @author     The OpenPilot Team Copyright (C) 2012.
* @addtogroup GCSPlugins GCS Plugins
* @{
* @addtogroup OsgEarthview Plugin
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

#ifndef OSGEARTHVIEWWIDGET_H_
#define OSGEARTHVIEWWIDGET_H_

#include "osgearthviewgadgetconfiguration.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"

#include <QTimer>

#include <osg/Notify>
#include <osg/PositionAttitudeTransform>

#include <osgDB/ReadFile>

#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgGA/GUIEventHandler>

#include <osgUtil/Optimizer>

#include <osgViewer/CompositeViewer>
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

#include <osgQt/GraphicsWindowQt>

#include <iostream>

class OsgEarthviewWidget : public QWidget, public osgViewer::CompositeViewer
{
    Q_OBJECT

public:
    OsgEarthviewWidget(QWidget *parent = 0);
   ~OsgEarthviewWidget();

public slots:

protected: /* Protected methods */
   void paintEvent(QPaintEvent *event);
   void resizeEvent(QResizeEvent *event);

   /* Create a osgQt::GraphicsWindowQt to add to the widget */
   QWidget* createViewWidget( osg::Camera* camera, osg::Node* scene );

   /* Create an osg::Camera which sets up the OSG view */
   osg::Camera* createCamera( int x, int y, int w, int h, const std::string& name, bool windowDecoration );

   /* Get the model to render */
   osg::Node* createAirplane();
private slots:

private: /* Private methods */

private: /* Private variables */
    QTimer _timer;
    EarthManipulator* manip;
};
#endif /* OSGEARTHVIEWWIDGET_H_ */
