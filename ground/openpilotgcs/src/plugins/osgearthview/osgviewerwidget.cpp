/********************************************************************************
* @file       osgviewerwidget.cpp
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

#include "osgviewerwidget.h"
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
#include <osgGA/NodeTrackerManipulator>

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

#include "utils/stylehelper.h"
#include "utils/homelocationutil.h"
#include "utils/worldmagmodel.h"
#include "utils/coordinateconversions.h"
#include "attitudeactual.h"
#include "gpsposition.h"
#include "homelocation.h"
#include "positionactual.h"
#include "systemsettings.h"

using namespace Utils;

OsgViewerWidget::OsgViewerWidget(QWidget *parent) : QWidget(parent)
{
    setThreadingModel(osgViewer::ViewerBase::CullThreadPerCameraDrawThreadPerContext);
    setAttribute(Qt::WA_PaintOnScreen, true);

    osg::Group* root = new osg::Group;
    osg::Node* earth = osgDB::readNodeFile("/Users/Cotton/Programming/osg/osgearth/tests/boston.earth");
    mapNode = osgEarth::MapNode::findMapNode( earth );
    if (!mapNode)
    {
        qDebug() <<"Uhoh";
    }

    root->addChild(earth);

    osg::Node* airplane = createAirplane();
    uavPos = new osgEarth::Util::ObjectLocatorNode(mapNode->getMap());
    uavPos->getLocator()->setPosition( osg::Vec3d(-71.100549, 42.349273, 150) );
    uavPos->addChild(airplane);

    root->addChild(uavPos);

    osgUtil::Optimizer optimizer;
    optimizer.optimize(root);

    QWidget* viewWidget = createViewWidget( createCamera(0,0,200,200,"Earth",false), root);
    viewWidget->show();
    setLayout(new QVBoxLayout(this));

    viewWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    layout()->addWidget(viewWidget);


    connect( &_timer, SIGNAL(timeout()), this, SLOT(update()) );
    _timer.start( 10 );
}

OsgViewerWidget::~OsgViewerWidget()
{
}

QWidget* OsgViewerWidget::createViewWidget( osg::Camera* camera, osg::Node* scene )
{
    osgViewer::View* view = new osgViewer::View;
    view->setCamera( camera );

    addView( view );

    view->setSceneData( scene );
    view->addEventHandler( new osgViewer::StatsHandler );
    view->getDatabasePager()->setDoPreCompile( true );

    manip = new EarthManipulator();
    view->setCameraManipulator( manip );

//    osgGA::NodeTrackerManipulator *camTracker = new osgGA::NodeTrackerManipulator();
//    camTracker->setTrackNode(uavPos);
//    camTracker->setMinimumDistance(0.0001f);
//    camTracker->setDistance(0.001f);
//    camTracker->setTrackerMode(osgGA::NodeTrackerManipulator::NODE_CENTER);
//    view->setCameraManipulator(camTracker);

    Grid* grid = new Grid();
    grid->setControl(0,0,new LabelControl("OpenPilot"));
    ControlCanvas::get(view, true)->addControl(grid);

    // zoom to a good startup position
    manip->setViewpoint( Viewpoint(-71.100549, 42.349273, 0, 24.261, -21.6, 350.0), 5.0 );
    //manip->setViewpoint( Viewpoint(-71.100549, 42.349273, 0, 24.261, -81.6, 650.0), 5.0 );
    //manip->setHomeViewpoint(Viewpoint("Boston", osg::Vec3d(-71.0763, 42.34425, 0), 24.261, -21.6, 3450.0));

    manip->setTetherNode(uavPos);

    osgQt::GraphicsWindowQt* gw = dynamic_cast<osgQt::GraphicsWindowQt*>( camera->getGraphicsContext() );
    return gw ? gw->getGLWidget() : NULL;
}

osg::Camera* OsgViewerWidget::createCamera( int x, int y, int w, int h, const std::string& name="", bool windowDecoration=false )
{
    osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->windowName = name;
    traits->windowDecoration = windowDecoration;
    traits->x = x;
    traits->y = y;
    traits->width = w;
    traits->height = h;
    traits->doubleBuffer = true;
    traits->alpha = ds->getMinimumNumAlphaBits();
    traits->stencil = ds->getMinimumNumStencilBits();
    traits->sampleBuffers = ds->getMultiSamples();
    traits->samples = ds->getNumMultiSamples();

    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setGraphicsContext( new osgQt::GraphicsWindowQt(traits.get()) );

    camera->setClearColor( osg::Vec4(0.2, 0.2, 0.6, 1.0) );
    camera->setViewport( new osg::Viewport(0, 0, traits->width, traits->height) );
    camera->setProjectionMatrixAsPerspective(
                30.0f, static_cast<double>(traits->width)/static_cast<double>(traits->height), 1.0f, 10000.0f );
    return camera.release();
}

osg::Node* OsgViewerWidget::createAirplane()
{
    osg::Group* model = new osg::Group;
    osg::Node *uav;

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    Q_ASSERT(pm);
    UAVObjectManager * objMngr = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objMngr);

    SystemSettings *systemSettingsObj = SystemSettings::GetInstance(objMngr);
    SystemSettings::DataFields systemSettings = systemSettingsObj->getData();

    qDebug() << "Frame type:" << systemSettingsObj->getField("AirframeType")->getValue().toString();
    // Get model that matches airframe type
    switch(systemSettings.AirframeType) {
    case SystemSettings::AIRFRAMETYPE_FIXEDWING:
    case SystemSettings::AIRFRAMETYPE_FIXEDWINGELEVON:
    case  SystemSettings::AIRFRAMETYPE_FIXEDWINGVTAIL:
        uav = osgDB::readNodeFile("/Users/Cotton/Programming/OpenPilot/artwork/3D Model/planes/Easystar/easystar.3ds");
        break;
    default:
        uav = osgDB::readNodeFile("/Users/Cotton/Programming/OpenPilot/artwork/3D Model/multi/joes_cnc/J14-QT_+.3DS");
    }

    if(uav) {
        uavAttitudeAndScale = new osg::MatrixTransform();
        uavAttitudeAndScale->setMatrix(osg::Matrixd::scale(0.2e0,0.2e0,0.2e0));

        // Apply a rotation so model is NED before any other rotations
        osg::MatrixTransform *rotateModelNED = new osg::MatrixTransform();
        rotateModelNED->setMatrix(osg::Matrixd::scale(0.05e0,0.05e0,0.05e0) * osg::Matrixd::rotate(M_PI, osg::Vec3d(0,0,1)));
        rotateModelNED->addChild( uav );

        uavAttitudeAndScale->addChild( rotateModelNED );

        model->addChild(uavAttitudeAndScale);
    } else
        qDebug() << "Bad model file";
    return model;
}

void OsgViewerWidget::paintEvent( QPaintEvent* event )
{
    Q_UNUSED(event);
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager * objMngr = pm->getObject<UAVObjectManager>();

    PositionActual *positionActualObj = PositionActual::GetInstance(objMngr);
    PositionActual::DataFields positionActual = positionActualObj->getData();
    double NED[3] = {positionActual.North, positionActual.East, positionActual.Down};

    bool positionActualUpdate = true;
    if(positionActualUpdate) {
        HomeLocation *homeLocationObj = HomeLocation::GetInstance(objMngr);
        HomeLocation::DataFields homeLocation = homeLocationObj->getData();
        double homeLLA[3] = {homeLocation.Latitude / 10.0e6, homeLocation.Longitude / 10.0e6, homeLocation.Altitude};

        double LLA[3];
		CoordinateConversions().NED2LLA_HomeLLA(homeLLA, NED, LLA);
        uavPos->getLocator()->setPosition( osg::Vec3d(LLA[1], LLA[0], LLA[2]) );  // Note this takes longtitude first
    } else {
        GPSPosition *gpsPosObj = GPSPosition::GetInstance(objMngr);
        GPSPosition::DataFields gpsPos = gpsPosObj->getData();
        uavPos->getLocator()->setPosition( osg::Vec3d(gpsPos.Longitude / 10.0e6, gpsPos.Latitude / 10.0e6, gpsPos.Altitude));
    }

    // Set the attitude (reverse the attitude)
    AttitudeActual *attitudeActualObj = AttitudeActual::GetInstance(objMngr);
    AttitudeActual::DataFields attitudeActual = attitudeActualObj->getData();
    osg::Quat quat(attitudeActual.q2, attitudeActual.q3, attitudeActual.q4, attitudeActual.q1);

    // Have to rotate the axes from OP NED frame to OSG frame (X east, Y north, Z down)
    double angle;
    osg::Vec3d axis;
    quat.getRotate(angle,axis);
    quat.makeRotate(angle, osg::Vec3d(axis[1],axis[0],-axis[2]));
    osg::Matrixd rot = osg::Matrixd::rotate(quat);

    uavAttitudeAndScale->setMatrix(rot);

    frame();
}
