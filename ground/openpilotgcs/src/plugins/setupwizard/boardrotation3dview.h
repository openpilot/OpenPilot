/**
 ******************************************************************************
 *
 * @file       boardrotation3dview.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2014.
 * @addtogroup
 * @{
 * @addtogroup BoardRotation3DView
 * @{
 * @brief
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

#ifndef BOARDROTATION3DVIEW_H
#define BOARDROTATION3DVIEW_H

#include <QGLWidget>

#include "glc_factory.h"
#include "viewport/glc_viewport.h"
#include "viewport/glc_movercontroller.h"
#include "shading/glc_light.h"
#include "sceneGraph/glc_world.h"
#include "glc_exception.h"


class BoardRotation3DView : public QGLWidget {
    Q_OBJECT

public:
    BoardRotation3DView(QWidget *parent = 0, QString fname = QString());
    ~BoardRotation3DView();

    void rollRotation(int val);
    void pitchRotation(int val);
    void yawRotation(int val);

private:
    GLC_Light m_glcLight;
    GLC_World m_glcWorld;
    GLC_Viewport m_glcView;
    GLC_MoverController m_glcMoverController;
    GLC_BoundingBox m_glcBoundingBox;

    QString m_boardFilename;

    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
    void CreateScene();
    //void wheelEvent(QWheelEvent *e);
};

#endif /* BOARDROTATION3DVIEW_H */

