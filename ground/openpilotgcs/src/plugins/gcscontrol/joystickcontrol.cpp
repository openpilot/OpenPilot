/**
 ******************************************************************************
 *
 * @file       joystickcontrol.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GCSControlGadgetPlugin GCSControl Gadget Plugin
 * @{
 * @brief The plugin that mimics a transmitter joystick and updates the MCC
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

#include "joystickcontrol.h"
#include "extensionsystem/pluginmanager.h"
#include <QDebug>
#include <QtGui/QWidget>
#include <QtOpenGL/QGLWidget>
#include <QMouseEvent>

/**
  * @brief Constructor for JoystickControl widget.  Sets up the image of a joystick
  */
JoystickControl::JoystickControl(QWidget *parent) : QGraphicsView(parent)
{
    setMinimumSize(64, 64);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setScene(new QGraphicsScene(this));
    setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    m_renderer = new QSvgRenderer();
    bool test = m_renderer->load(QString(":/gcscontrol/images/joystick.svg"));
    Q_UNUSED(test);
    Q_ASSERT(test);

    m_background = new QGraphicsSvgItem();
    m_background->setSharedRenderer(m_renderer);
    m_background->setElementId(QString("background"));

    m_joystickEnd = new QGraphicsSvgItem();
    m_joystickEnd->setSharedRenderer(m_renderer);
    m_joystickEnd->setElementId(QString("joystickEnd"));

    m_joystickArea = new QGraphicsSvgItem();
    m_joystickArea->setSharedRenderer(m_renderer);
    m_joystickArea->setElementId(QString("joystickArea"));
    m_joystickArea->setPos(
        (m_background->boundingRect().width() - m_joystickArea->boundingRect().width()) * 0.5,
        (m_background->boundingRect().height() - m_joystickArea->boundingRect().height()) * 0.5
    );
    m_joystickArea->setVisible(false);

    QGraphicsScene *l_scene = scene();
    l_scene->clear(); // This also deletes all items contained in the scene.
    l_scene->addItem(m_background);
    l_scene->addItem(m_joystickArea);
    l_scene->addItem(m_joystickEnd);
    l_scene->setSceneRect(m_background->boundingRect());

    changePosition(0.0, 0.0);
}

JoystickControl::~JoystickControl()
{
    // Do nothing
}

/**
  * @brief Enables/Disables OpenGL
  */
void JoystickControl::enableOpenGL(bool flag)
{
    if (flag)
        setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
    else
        setViewport(new QWidget);
}

/**
  * @brief Update the displayed position based on an MCC update
  */
void JoystickControl::changePosition(double x, double y)
{
    QRectF areaSize = m_joystickArea->boundingRect();
    QPointF point(
        ((1.0 + x) * areaSize.width() - m_joystickEnd->boundingRect().width()) * 0.5,
        ((1.0 - y) * areaSize.height() - m_joystickEnd->boundingRect().height()) * 0.5
    );
    m_joystickEnd->setPos(m_joystickArea->mapToScene(point));
}

/**
  * @brief Redirect mouse move events to control position
  */
void JoystickControl::mouseMoveEvent(QMouseEvent *event)
{
    QPointF point = m_joystickArea->mapFromScene(mapToScene(event->pos()));
    QSizeF areaSize = m_joystickArea->boundingRect().size();

    double y = - (point.y() / areaSize.height() - 0.5) * 2.0;
    double x = (point.x() / areaSize.width() - 0.5) * 2.0;
    if (y < -1.0) y = -1.0;
    if (y >  1.0) y =  1.0;
    if (x < -1.0) x = -1.0;
    if (x >  1.0) x =  1.0;

    emit positionClicked(x, y);
}

/**
  * @brief Redirect mouse move clicks to control position
  */
void JoystickControl::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        mouseMoveEvent(event);
    }
}

void JoystickControl::paint()
{
    update();
}

void JoystickControl::paintEvent(QPaintEvent *event)
{
    // Skip painting until the image file is loaded
    if (!m_renderer->isValid()) {
        qDebug() << "Image file not loaded, not rendering";
    }

    QGraphicsView::paintEvent(event);
}

void JoystickControl::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    fitInView(m_background, Qt::KeepAspectRatio);
}

/**
  * @}
  * @}
  */
