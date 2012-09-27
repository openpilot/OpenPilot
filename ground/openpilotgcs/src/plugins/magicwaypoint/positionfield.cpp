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

#include "positionfield.h"
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
PositionField::PositionField(QWidget *parent) :
    QGraphicsView(parent)
{
    setMinimumSize(64,64);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setScene(new QGraphicsScene(this));
    setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);

    m_renderer = new QSvgRenderer();
    Q_ASSERT( m_renderer->load(QString(":/magicwaypoint/images/positionfield.svg")) != 0 );

    m_background = new QGraphicsSvgItem();
    m_background->setSharedRenderer(m_renderer);
    m_background->setElementId(QString("background"));

    m_positiondesired = new QGraphicsSvgItem();
    m_positiondesired->setSharedRenderer(m_renderer);
    m_positiondesired->setElementId(QString("desiredPosition"));
    m_positiondesired->setPos(0,0);

    m_positionactual = new QGraphicsSvgItem();
    m_positionactual->setSharedRenderer(m_renderer);
    m_positionactual->setElementId(QString("actualPosition"));
    m_positionactual->setPos(0,0);

    QGraphicsScene *l_scene = scene();
    l_scene->clear(); // This also deletes all items contained in the scene.
    l_scene->addItem(m_background);
    l_scene->addItem(m_positiondesired);
    l_scene->addItem(m_positionactual);
    l_scene->setSceneRect(m_background->boundingRect());
}

PositionField::~PositionField()
{

}

/**
  * @brief Update aircraft position on image (values go from -1 to 1)
  */
void PositionField::updateDesiredIndicator(double north, double east)
{
    QRectF sceneSize = scene()->sceneRect();

    m_positiondesired->setPos(
            (east+1)/2*sceneSize.width() - m_positiondesired->boundingRect().width() / 2,
            (-north+1)/2*sceneSize.height() - m_positiondesired->boundingRect().height() / 2);
}

void PositionField::updateActualIndicator(double north, double east)
{
    QRectF sceneSize = scene()->sceneRect();

    m_positionactual->setPos(
            (east+1)/2*sceneSize.width() - m_positionactual->boundingRect().width() / 2,
            (-north+1)/2*sceneSize.height() - m_positionactual->boundingRect().height() / 2);
}

/**
  * @brief Redirect mouse move events to control position
  */
void PositionField::mouseMoveEvent(QMouseEvent *event)
{
    QPointF point = mapToScene(event->pos());
    QRectF sceneSize = scene()->sceneRect();

    double north = - (point.y() / sceneSize.height() - .5) * 2;
    double east = (point.x() / sceneSize.width() - .5) * 2;
    emit positionClicked(north, east);
}

/**
  * @brief Redirect mouse move clicks to control position
  */
void PositionField::mousePressEvent(QMouseEvent *event)
{
    if( event->button() == Qt::LeftButton ) {
        mouseMoveEvent(event);
    }
}

void PositionField::paint()
{
    update();
}

void PositionField::paintEvent(QPaintEvent *event)
{
    // Skip painting until the dial file is loaded
    if (! m_renderer->isValid()) {
        qDebug()<<"Image file not loaded, not rendering";
    }

    QGraphicsView::paintEvent(event);
}

void PositionField::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    fitInView(m_background, Qt::IgnoreAspectRatio );
}


/**
  * @}
  * @}
  */
