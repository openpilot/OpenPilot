/**
 ******************************************************************************
 *
 * @file       modelviewgadgetwidget.h
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

#ifndef MODELVIEWGADGETWIDGET_H_
#define MODELVIEWGADGETWIDGET_H_

#include <QtOpenGL/QGLWidget>
#include <QTimer>

#include <GLC_Factory>
#include <GLC_Viewport>
#include <GLC_MoverController>
#include <GLC_Light>
#include <GLC_World>

#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/attitudeactual.h"



class ModelViewGadgetWidget : public QGLWidget
{
    Q_OBJECT

public:
    ModelViewGadgetWidget(QWidget *parent = 0);
   ~ModelViewGadgetWidget();
   void setAcFilename(QString acf) { acFilename = acf; }
   void setBgFilename(QString bgf) { bgFilename = bgf; }
   void reloadScene();
   void updateAttitude(int value);

protected:
   void resizeEvent(QResizeEvent *event);

private:
   void initializeGL();
   void paintGL();
   void resizeGL(int width, int height);
   // Create GLC_Object to display
   void CreateScene();

   //Mouse events
   void mousePressEvent(QMouseEvent * e);
   void mouseMoveEvent(QMouseEvent * e);
   void mouseReleaseEvent(QMouseEvent * e);

//////////////////////////////////////////////////////////////////////
// Private slots Functions
//////////////////////////////////////////////////////////////////////
private slots:
    void updateAttitude();

private:
    GLC_Factory* m_pFactory;
    GLC_Light m_Light;
    GLC_World m_World;
    GLC_Viewport m_GlView;
    GLC_MoverController m_MoverController;
    GLC_BoundingBox m_ModelBoundingBox;
    //! The timer used for motion
    QTimer m_MotionTimer;

    QString acFilename;
    QString bgFilename;

    AttitudeActual* attActual;
};

#endif /* MODELVIEWGADGETWIDGET_H_ */
