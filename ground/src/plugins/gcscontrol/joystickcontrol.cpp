/**
 ******************************************************************************
 *
 * @file       joystickcontrol.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GCSControlGadgetPlugin GCSControl Gadget Plugin
 * @{
 * @brief A that mimics a transmitter joystick and updates the MCC
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
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>
#include <QtGui/QMessageBox>
#include <QMouseEvent>
#include <QtGlobal>

/**
  * @brief Constructor for JoystickControl widget.  Sets up the image of a joystick
  */
JoystickControl::JoystickControl(QWidget *parent) :
    QGraphicsView(parent)
{
    setMinimumSize(64,64);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setScene(new QGraphicsScene(this));
    setRenderHints(QPainter::Antialiasing);

    m_renderer = new QSvgRenderer();
    bool test = m_renderer->load(QString(":/gcscontrol/images/joystick.svg"));
    Q_ASSERT( test );

    m_background = new QGraphicsSvgItem();
    m_background->setSharedRenderer(m_renderer);
    m_background->setElementId(QString("background"));

    m_joystickEnd = new QGraphicsSvgItem();
    m_joystickEnd->setSharedRenderer(m_renderer);
    m_joystickEnd->setElementId(QString("joystickEnd"));
    m_joystickEnd->setPos(0,0);

    QGraphicsScene *l_scene = scene();
    l_scene->clear(); // This also deletes all items contained in the scene.
    l_scene->addItem(m_background);
    l_scene->addItem(m_joystickEnd);
    l_scene->setSceneRect(m_background->boundingRect());
}

JoystickControl::~JoystickControl()
{

}

/**
  * @brief Update the displayed position based on an MCC update
  */
void JoystickControl::changePosition(double X, double Y)
{
    QRectF sceneSize = scene()->sceneRect();
    m_joystickEnd->setPos((X+1)/2*sceneSize.width(),(-Y+1)/2*sceneSize.height());
}

/**
  * @brief Redirect mouse move events to control position
  */
void JoystickControl::mouseMoveEvent(QMouseEvent *event)
{
    QPointF point = mapToScene(event->pos());
    QRectF sceneSize = scene()->sceneRect();

    double Y = - (point.y() / sceneSize.height() - .5) * 2;
    double X = (point.x() / sceneSize.width() - .5) * 2;
    emit positionClicked(X, Y);
}

/**
  * @brief Redirect mouse move clicks to control position
  */
void JoystickControl::mousePressEvent(QMouseEvent *event)
{
    if( event->button() == Qt::LeftButton ) {
        mouseMoveEvent(event);
    }
}


void JoystickControl::paint()
{
    update();
}

void JoystickControl::paintEvent(QPaintEvent *event)
{
    // Skip painting until the dial file is loaded
    if (! m_renderer->isValid()) {
        qDebug()<<"Image file not loaded, not rendering";
    }

    QGraphicsView::paintEvent(event);
}

void JoystickControl::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    fitInView(m_background, Qt::IgnoreAspectRatio );
}


/**
  * @}
  * @}
  */
