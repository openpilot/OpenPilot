/**
 ******************************************************************************
 *
 * @file       boardrotation3dview.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2015.
 * @addtogroup
 * @{
 * @addtogroup BoardRotation3DView
 * @{
 * @brief A widget that displays a 3D representation of the rotation of OpenPilot Controller
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
#include "QtDebug"
#ifdef __APPLE__
    #include "OpenGL/OpenGL.h"
#endif
#include "glc_context.h"
#include "glc_exception.h"
#include "glc_openglexception.h"
#include "viewport/glc_userinput.h"

#include "boardrotation3dview.h"

#define DISTANCE_MIN 70
#define DISTANCE_MAX 200

BoardRotation3DView::BoardRotation3DView(QWidget *parent, QString fname)
    : QGLWidget(new GLC_Context(QGLFormat(QGL::SampleBuffers)), parent),
    m_glcLight(), m_glcWorld(), m_glcView(), m_glcMoverController(),
    m_glcBoundingBox(), m_boardFilename(fname)
{
    connect(&m_glcView, SIGNAL(updateOpenGL()), this, SLOT(updateGL()));
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    m_glcLight.setPosition(1.0, 1.0, 1.0);
    m_glcLight.setAmbientColor(Qt::lightGray);

    m_glcView.cameraHandle()->setDefaultUpVector(glc::Z_AXIS);
    m_glcView.cameraHandle()->setIsoView();

    QColor repColor;
    repColor.setRgbF(1.0, 0.11372, 0.11372, 1.0);
    m_glcMoverController = GLC_Factory::instance()->createDefaultMoverController(repColor, &m_glcView);

    createScene();
}

BoardRotation3DView::~BoardRotation3DView()
{}

void BoardRotation3DView::rollRotation(int val)
{
    m_glcView.cameraHandle()->rotateAroundTarget(glc::Y_AXIS, glc::toRadian(-val));
    updateGL();
}

void BoardRotation3DView::pitchRotation(int val)
{
    m_glcView.cameraHandle()->rotateAroundTarget(glc::X_AXIS, glc::toRadian(-val));
    updateGL();
}

void BoardRotation3DView::yawRotation(int val)
{
    m_glcView.cameraHandle()->rotateAroundTarget(glc::Z_AXIS, glc::toRadian(-val));
    updateGL();
}

void BoardRotation3DView::initializeGL()
{
#if defined(Q_OS_MAC)
    const GLint swapInterval = 1;
    CGLSetParameter(CGLGetCurrentContext(), kCGLCPSwapInterval, &swapInterval);
#endif /* Q_OS_MAC */

    m_glcView.initGl();
    m_glcView.reframe(m_glcBoundingBox);

    glEnable(GL_NORMALIZE);
    glEnable(GL_MULTISAMPLE);
}

void BoardRotation3DView::paintGL()
{
    try {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        {
            GLenum error = glGetError();
            if (error != GL_NO_ERROR) {
                GLC_OpenGlException OpenGlException("BoardRotation3DView::paintGL() ", error);
                throw(OpenGlException);
            }
        }

        GLC_Context::current()->glcLoadIdentity();
        glEnable(GL_MULTISAMPLE);

        m_glcView.setDistMinAndMax(m_glcWorld.boundingBox());
        m_glcView.glExecuteCam();
        m_glcLight.glExecute();

        m_glcWorld.render(0, glc::ShadingFlag);
        m_glcWorld.render(0, glc::TransparentRenderFlag);

        m_glcMoverController.drawActiveMoverRep();
    } catch(GLC_Exception &e) {
        qDebug() << e.what();
    }
}

void BoardRotation3DView::resizeGL(int width, int height)
{
    m_glcView.setWinGLSize(width, height);
    {
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            GLC_OpenGlException OpenGlException("BoardRotation3DView::resizeGL() ", error);
            throw(OpenGlException);
        }
    }
}

void BoardRotation3DView::createScene()
{
    try {
        m_glcView.loadBackGroundImage(QString(":/setupwizard/resources/vehicle_bg.png"));
    } catch(GLC_Exception e) {
        qDebug("BoardRotation3DView::CreateScene() fail to load background image file.");
    }

    try {
        if (QFile::exists(m_boardFilename)) {
            QFile boardFile(m_boardFilename);
            m_glcWorld = GLC_Factory::instance()->createWorldFromFile(boardFile);
            m_glcBoundingBox = m_glcWorld.boundingBox();
            m_glcView.reframe(m_glcBoundingBox);
            // m_glcView.cameraHandle()->setDistEyeTarget(m_glcView.cameraHandle()->distEyeTarget() - 30);
        } else {
            qDebug("BoardRotation3DView::CreateScene() No board image file.");
        }
    } catch(GLC_Exception e) {
        qDebug("BoardRotation3DView::CreateScene() fail to load board image file.");
    }
}

void BoardRotation3DView::wheelEvent(QWheelEvent *e)
{
    double current_dist = m_glcView.cameraHandle()->distEyeTarget();

    if ((current_dist < DISTANCE_MIN) && (e->delta() > 0)) {
        return;
    }
    if ((current_dist > DISTANCE_MAX) && (e->delta() < 0)) {
        return;
    }

    double delta = current_dist - (e->delta() / 8);
    m_glcView.cameraHandle()->setDistEyeTarget(delta);
    m_glcView.setDistMinAndMax(m_glcWorld.boundingBox());
    updateGL();
}
