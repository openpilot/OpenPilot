/**
 ******************************************************************************
 *
 * @file       modelviewgadgetwidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   modelview
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

    m_GlView.cameraHandle()->setDefaultUpVector(glc::Z_AXIS);
    m_GlView.cameraHandle()->setIsoView();

    // Create objects to display
    CreateScene();
    connect(&m_MotionTimer, SIGNAL(timeout()), this, SLOT(rotateView()));
}

ModelViewGadgetWidget::~ModelViewGadgetWidget()
{
    delete m_pFactory;
}

void ModelViewGadgetWidget::initializeGL()
{
    // OpenGL initialization
    m_GlView.initGl();

    m_GlView.reframe(m_ModelBoundingBox);
    //m_GlView.reframe(m_World.boundingBox());
    // Calculate camera depth of view
    m_GlView.setDistMinAndMax(m_World.boundingBox());
    glEnable(GL_NORMALIZE);

    m_MotionTimer.start(60);
}

void ModelViewGadgetWidget::resizeEvent(QResizeEvent *event)
{
//   printf("%d,%d\n", width(), height());
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
    // TODO: Replace with files from configuration page
    m_GlView.loadBackGroundImage("../artwork/3D\ Model/default_background.png");
    QFile aircraft("../artwork/3D\ Model/quad.dae");

    GLC_World* pWorld= m_pFactory->createWorld(aircraft);
    m_World= *pWorld;
    delete pWorld;

    m_ModelBoundingBox= m_World.boundingBox();
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
// Rotate the view
void ModelViewGadgetWidget::rotateView()
{
        m_GlView.cameraHandle()->rotateAroundTarget(glc::Z_AXIS, 2.0 * glc::PI / static_cast<double>(200));
        updateGL();
}

