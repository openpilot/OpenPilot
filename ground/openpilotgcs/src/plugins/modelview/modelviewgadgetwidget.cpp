/**
 ******************************************************************************
 *
 * @file       modelviewgadgetwidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ModelViewPlugin ModelView Plugin
 * @{
 * @brief A gadget that displays a 3D representation of the UAV 
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

#include "modelviewgadgetwidget.h"
#include "extensionsystem/pluginmanager.h"
#include "glc_context.h"
#include "glc_exception.h"
#include "glc_openglexception.h"
#include "viewport/glc_userinput.h"

#include <iostream>

ModelViewGadgetWidget::ModelViewGadgetWidget(QWidget *parent) 
: QGLWidget(new GLC_Context(QGLFormat(QGL::SampleBuffers)),parent)
, m_Light()
, m_World()
, m_GlView(this)
, m_MoverController()
, m_ModelBoundingBox()
, m_MotionTimer()
, acFilename()
, bgFilename()
, vboEnable(false)
{

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    CreateScene();

    QColor repColor;
    repColor.setRgbF(1.0, 0.11372, 0.11372, 0.0);
    m_MoverController= GLC_Factory::instance()->createDefaultMoverController(repColor, &m_GlView);

    // Get required UAVObjects
    ExtensionSystem::PluginManager* pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager* objManager = pm->getObject<UAVObjectManager>();
    attActual = AttitudeActual::GetInstance(objManager);

    connect(&m_MotionTimer, SIGNAL(timeout()), this, SLOT(updateAttitude()));
}

ModelViewGadgetWidget::~ModelViewGadgetWidget()
{

}


void ModelViewGadgetWidget::setAcFilename(QString acf)
{
    if(QFile::exists(acf))
        acFilename = acf;
    else
    {
        acFilename= acf= ":/modelview/models/warning_sign.obj";
        m_GlView.cameraHandle()->setFrontView(); // set to front camera to see/read the warning sign
    }
}

void ModelViewGadgetWidget::setBgFilename(QString bgf)
{
    if (QFile::exists(bgFilename))
        bgFilename = bgf;
    else
    {
    	qDebug() << "file " << bgf << " doesn't exists";
    	bgFilename= ":/modelview/models/black.jpg"; // will put a black background if there's no background
    }

}

void ModelViewGadgetWidget::setVboEnable(bool eVbo)
{
	vboEnable = eVbo;
	m_World.collection()->setVboUsage(vboEnable);
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

    m_Light.setPosition(4000.0, -40000.0, 80000.0);
    m_Light.setAmbientColor(Qt::lightGray);

    m_GlView.cameraHandle()->setDefaultUpVector(glc::Z_AXIS);
    m_GlView.cameraHandle()->setFrontView();
    m_GlView.setToOrtho(true); // orthogonal view

    glEnable(GL_NORMALIZE);
    // Enable antialiasing
    glEnable(GL_MULTISAMPLE);

    m_MotionTimer.start(100);
    setFocusPolicy(Qt::StrongFocus); // keyboard capture for camera switching
}

void ModelViewGadgetWidget::paintGL()
{
    try
	{
		// Clear screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// OpenGL error handler
		{
			GLenum error= glGetError();
			if (error != GL_NO_ERROR)
			{
				GLC_OpenGlException OpenGlException("ModelViewGadgetWidget::paintGL() ", error);
				throw(OpenGlException);
			}
		}

		// Load identity matrix
		glLoadIdentity();

		// Enable antialiasing
		glEnable(GL_MULTISAMPLE);

        // Calculate camera depth of view
        m_GlView.setDistMinAndMax(m_World.boundingBox());

        // define view matrix
    	m_Light.glExecute();
        m_GlView.glExecuteCam();

        // Display the collection of GLC_Object
        m_World.render(0, glc::ShadingFlag);
        m_World.render(0, glc::TransparentRenderFlag);

        // Display UI Info (orbit circle)
        m_MoverController.drawActiveMoverRep();
    }
    catch (GLC_Exception &e)
	{
		qDebug() << e.what();
	}


}

void ModelViewGadgetWidget::resizeGL(int width, int height)
{
    m_GlView.setWinGLSize(width, height);   // Compute window aspect ratio
	// OpenGL error handler
	{
		GLenum error= glGetError();
		if (error != GL_NO_ERROR)
		{
			GLC_OpenGlException OpenGlException("ModelViewGadgetWidget::resizeGL() ", error);
			throw(OpenGlException);
		}
	}

}

// Create GLC_Object to display
void ModelViewGadgetWidget::CreateScene()
{
    // put a black background if the 3D model is invalid or if the background image is also invalid
    if (acFilename == ":/modelview/models/warning_sign.obj" or !QFile::exists(bgFilename))
        bgFilename= ":/modelview/models/black.jpg";


    try 
    {
        m_GlView.loadBackGroundImage(bgFilename);
    }
    catch(GLC_Exception e)
    {
        qDebug("ModelView: background image file loading failed.");
    }

    try
    {
        if(QFile::exists(acFilename))
        {
            QFile aircraft(acFilename);
            m_World= GLC_Factory::instance()->createWorldFromFile(aircraft);
            m_ModelBoundingBox= m_World.boundingBox();
            m_GlView.reframe(m_ModelBoundingBox); // center 3D model in the scene
        } else {
        	qDebug("ModelView: aircraft file not found.");
        }
    }
    catch(GLC_Exception e)
    {
        qDebug("ModelView: aircraft file loading failed.");
    }
}

void ModelViewGadgetWidget::wheelEvent(QWheelEvent * e)
{
        double delta = m_GlView.cameraHandle()->distEyeTarget() - (e->delta()/4) ;
        m_GlView.cameraHandle()->setDistEyeTarget(delta);
        m_GlView.setDistMinAndMax(m_World.boundingBox());
}

void ModelViewGadgetWidget::mousePressEvent(QMouseEvent *e)
{
	GLC_UserInput userInput(e->x(), e->y());
	if (m_MoverController.hasActiveMover()) return;

        switch (e->button())
        {
        case (Qt::LeftButton):
                m_MotionTimer.stop();
                m_MoverController.setActiveMover(GLC_MoverController::TurnTable, userInput);
                updateGL();
                break;
        case (Qt::RightButton):
		printf("VBO enabled: %s, VBO supported: %s, VBO used: %s\n",
			vboEnable ? "yes" : "no",
			GLC_State::vboSupported() ? "yes" : "no",
			GLC_State::vboUsed() ? "yes" : "no");
		printf("Renderer - %s \n", (char*)glGetString(GL_RENDERER));
		printf("Extensions - %s\n", (char*)glGetString(GL_EXTENSIONS));
                break;
        default:
                break;
        }
}

void ModelViewGadgetWidget::mouseMoveEvent(QMouseEvent * e)
{
	GLC_UserInput userInput(e->x(), e->y());
	if (not m_MoverController.hasActiveMover()) return;
        m_MoverController.move(userInput);
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

void ModelViewGadgetWidget::keyPressEvent(QKeyEvent * e) // switch between camera
{
    if (e->key() == Qt::Key_1)
    {
        m_GlView.cameraHandle()->setIsoView();
        updateGL();
    }
    if (e->key() == Qt::Key_2)
    {
        m_GlView.cameraHandle()->setFrontView();
        updateGL();
    }
    if (e->key() == Qt::Key_3)
    {
        m_GlView.cameraHandle()->setIsoView();
        m_GlView.cameraHandle()->rotateAroundTarget(glc::Z_AXIS,glc::toRadian(90));
        updateGL();
    }
    if (e->key() == Qt::Key_4)
    {
        m_GlView.cameraHandle()->setLeftView();
        updateGL();
    }
    if (e->key() == Qt::Key_5)
    {
        m_GlView.cameraHandle()->setTopView();
        m_GlView.cameraHandle()->rotateAroundTarget(glc::Z_AXIS,glc::toRadian(180));
        updateGL();
    }
    if (e->key() == Qt::Key_6)
    {
        m_GlView.cameraHandle()->setRightView();;
        updateGL();
    }
    if (e->key() == Qt::Key_7)
    {
        m_GlView.cameraHandle()->setIsoView();
        m_GlView.cameraHandle()->rotateAroundTarget(glc::Z_AXIS,glc::toRadian(-90));
        updateGL();
    }
    if (e->key() == Qt::Key_8)
    {
        m_GlView.cameraHandle()->setRearView();;
        updateGL();
    }
    if (e->key() == Qt::Key_9)
    {
        m_GlView.cameraHandle()->setIsoView();
        m_GlView.cameraHandle()->rotateAroundTarget(glc::Z_AXIS,glc::toRadian(180));
        updateGL();
    }
    if (e->key() == Qt::Key_0)
    {
        m_GlView.cameraHandle()->setBottomView();
        m_GlView.cameraHandle()->rotateAroundTarget(glc::Z_AXIS,glc::toRadian(180));
        updateGL();
    }
}

//////////////////////////////////////////////////////////////////////
// Private slots Functions
//////////////////////////////////////////////////////////////////////
void ModelViewGadgetWidget::updateAttitude()
{
    AttitudeActual::DataFields data = attActual->getData(); // get attitude data
    GLC_StructOccurence* rootObject= m_World.rootOccurence(); // get the full 3D model
    double x= data.q3;
    double y= data.q2;
    double z= data.q4;
    double w= data.q1;
    if (w == 0.0)
        w = 1.0;
    // create and gives the product of 2 4x4 matrices to get the rotation of the 3D model's matrix
    QMatrix4x4 m1;
    m1.setRow(0, QVector4D(w,z,-y,x));
    m1.setRow(1, QVector4D(-z,w,x,y));
    m1.setRow(2, QVector4D(y,-x,w,z));
    m1.setRow(3, QVector4D(-x,-y,-z,w));
    QMatrix4x4 m2;
    m2.setRow(0, QVector4D(w,z,-y,-x));
    m2.setRow(1, QVector4D(-z,w,x,-y));
    m2.setRow(2, QVector4D(y,-x,w,-z));
    m2.setRow(3, QVector4D(x,y,z,w));
    QMatrix4x4 m0= m1 * m2;
    // convert QMatrix4x4 to GLC_Matrix4x4
    GLC_Matrix4x4 rootObjectRotation;
    {
        double* newMatrixData = rootObjectRotation.setData();
	double* oldMatrixData = (double*) m0.data();
        for (int i=0; i<16; i++){
            newMatrixData[i]= oldMatrixData[i];
        }
    }
    // sets and updates the 3D model's matrix
    rootObject->structInstance()->setMatrix(rootObjectRotation);
    rootObject->updateChildrenAbsoluteMatrix();
    updateGL();
}
