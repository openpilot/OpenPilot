/**
 ******************************************************************************
 *
 * @file       modelviewgadgetwidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   modelviewplugin
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
#include "modelviewgadgetwidget.h"
#include "extensionsystem/pluginmanager.h"
#include <iostream>

ModelViewGadgetWidget::ModelViewGadgetWidget(QWidget *parent) 
: QGLWidget(parent)
, m_pFactory(GLC_Factory::instance(this->context()))
, m_Light()
, m_World()
, m_GlView(this)
, m_MoverController()
, m_ModelBoundingBox()
, m_MotionTimer()
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);


    m_Light.setPosition(4000.0, 40000.0, 80000.0);
    m_Light.setAmbientColor(Qt::lightGray);

    QColor repColor;
    repColor.setRgbF(1.0, 0.11372, 0.11372, 0.0);
    m_MoverController= m_pFactory->createDefaultMoverController(repColor, &m_GlView);

    m_GlView.cameraHandle()->setDefaultUpVector(glc::Y_AXIS);
    m_GlView.cameraHandle()->setRightView();
    //m_GlView.cameraHandle()->setIsoView();

    // Get required UAVObjects
    ExtensionSystem::PluginManager* pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager* objManager = pm->getObject<UAVObjectManager>();
    attActual = AttitudeActual::GetInstance(objManager);

    // Create objects to display
    CreateScene();
    connect(&m_MotionTimer, SIGNAL(timeout()), this, SLOT(updateAttitude()));
}

ModelViewGadgetWidget::~ModelViewGadgetWidget()
{
    delete m_pFactory;
}

//// Public funcitons ////
void ModelViewGadgetWidget::reloadScene()
{
    CreateScene();
}

//// Private functions ////
void ModelViewGadgetWidget::initializeGL()
{
    // OpenGL initialization
    m_GlView.initGl();
    if (!vboEnable) 
    {
	GLC_State::setVboUsage(false); 
	qDebug("VBOs disabled.  Enable for better performance if GPU supports it.");
    }

    m_GlView.reframe(m_ModelBoundingBox);
    // Calculate camera depth of view
    m_GlView.setDistMinAndMax(m_World.boundingBox());
    glEnable(GL_NORMALIZE);

    m_MotionTimer.start(100);
}

void ModelViewGadgetWidget::resizeEvent(QResizeEvent *event)
{
//   m_GlView.setWinGLSize(width(), height());
   QWidget::resizeEvent(event);
}

void ModelViewGadgetWidget::paintGL()
{
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Load identity matrix
    glLoadIdentity();

    // define the light
    m_Light.enable();

    // define view matrix
    m_GlView.glExecuteCam();
    m_Light.glExecute();

    // Display the collection of GLC_Object
    m_World.glExecute(0, false);

    // Display UI Info (orbit circle)
    m_MoverController.drawActiveMoverRep();
}

void ModelViewGadgetWidget::resizeGL(int width, int height)
{
    m_GlView.setWinGLSize(width, height);   // Compute window aspect ratio
}

// Create GLC_Object to display
void ModelViewGadgetWidget::CreateScene()
{
    if (QFile::exists(bgFilename))
    {
        m_GlView.loadBackGroundImage(bgFilename);
    }
    //else { std::cout << "File doesn't exist" << bgFilename.toStdString() << std::endl; }

    if (QFile::exists(acFilename))
    {
        QFile aircraft(acFilename);

        GLC_World* pWorld= m_pFactory->createWorld(aircraft);
        m_World= *pWorld;
        delete pWorld;

        m_ModelBoundingBox= m_World.boundingBox();
    }
    //else { std::cout << "File doesn't exist" << acFilename.toStdString() << std::endl; }
}

void ModelViewGadgetWidget::mousePressEvent(QMouseEvent *e)
{
        if (m_MoverController.hasActiveMover()) return;

        switch (e->button())
        {
        case (Qt::LeftButton):
                m_MotionTimer.stop();
                m_MoverController.setActiveMover(GLC_MoverController::TurnTable, e->x(), e->y());
                updateGL();
                break;
        default:
                break;
        }
}

void ModelViewGadgetWidget::mouseMoveEvent(QMouseEvent * e)
{
        if (not m_MoverController.hasActiveMover()) return;
        m_MoverController.move(e->x(), e->y());
        m_GlView.setDistMinAndMax(m_World.boundingBox());
        updateGL();
}

void ModelViewGadgetWidget::mouseReleaseEvent(QMouseEvent*)
{
        if (not m_MoverController.hasActiveMover()) return;
        m_MoverController.setNoMover();
        m_MotionTimer.start();
        updateGL();
}

//////////////////////////////////////////////////////////////////////
// Private slots Functions
//////////////////////////////////////////////////////////////////////
void ModelViewGadgetWidget::updateAttitude()
{
    // Reset view to zero angles
    // NOTE: Some aircraft 3D models have different orientation and axis
    m_GlView.cameraHandle()->setTopView();
    m_GlView.cameraHandle()->rotateAroundTarget(glc::X_AXIS, 180.0*glc::PI/180.0);
    m_GlView.cameraHandle()->rotateAroundTarget(glc::Z_AXIS, 90.0*glc::PI/180.0);
    // Rotate to the actual angles (if a gimbal lock at yaw 90/270 deg, change sequence of rotations)
    AttitudeActual::DataFields data = attActual->getData();
    m_GlView.cameraHandle()->rotateAroundTarget(glc::Z_AXIS, data.Yaw*glc::PI/180.0);
    m_GlView.cameraHandle()->rotateAroundTarget(glc::X_AXIS, data.Roll*glc::PI/180.0);
    m_GlView.cameraHandle()->rotateAroundTarget(glc::Y_AXIS, -data.Pitch*glc::PI/180.0);
    updateGL();
}



