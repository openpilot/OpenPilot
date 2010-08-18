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

    // Connect object updated event from UAVObject to also animate sticks
    connect(getMCC(), SIGNAL(objectUpdated(UAVObject*)), this, SLOT(mccChanged(UAVObject*)));


    m_renderer = new QSvgRenderer();
    Q_ASSERT( m_renderer->load(QString(":/gcscontrol/images/joystick.svg")) );

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
  * @brief Draw stick positions when ManualControlCommand gets updated
  */
void JoystickControl::mccChanged(UAVObject*)
{
    QPen pen;
    pen.setColor(QColor(Qt::white));
    pen.setWidth(2);

    // draw stick positions
    if( this->objectName() == QString("widgetLeftStick"))
    {
        ManualControlCommand::DataFields data = getMCC()->getData();
        double x = (data.Yaw + 1) / 2 * scene()->sceneRect().width();
        double y = (-data.Pitch + 1) / 2 * scene()->sceneRect().height();
        m_joystickEnd->setPos(x-m_joystickEnd->boundingRect().width()/2,y-m_joystickEnd->boundingRect().height()/2);
    }
    else if( this->objectName() == QString("widgetRightStick"))
    {
        ManualControlCommand::DataFields data = getMCC()->getData();
        double x = (data.Roll + 1) / 2 * scene()->sceneRect().width();
        double y = (-data.Throttle + 1) / 2 * scene()->sceneRect().height();

        m_joystickEnd->setPos(x-m_joystickEnd->boundingRect().width()/2,y-m_joystickEnd->boundingRect().height()/2);
    }

}

/**
  * @brief Get the Manual Control Command UAV Object
  */
ManualControlCommand* JoystickControl::getMCC()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    ManualControlCommand* obj = dynamic_cast<ManualControlCommand*>( objManager->getObject(QString("ManualControlCommand")) );
    return obj;
}

/**
  * @brief Redirect mouse move events to control joystick position
  */
void JoystickControl::mouseMoveEvent(QMouseEvent *event)
{
    updateMCC(mapToScene(event->pos()));
}

/**
  * @brief Redirect mouse move clicks to control joystick position
  */
void JoystickControl::mousePressEvent(QMouseEvent *event)
{
    if( event->button() == Qt::LeftButton )
        updateMCC(mapToScene(event->pos()));
}

void JoystickControl::paint()
{
    update();
}

void JoystickControl::paintEvent(QPaintEvent *event)
{
    // Skip painting until the dial file is loaded
    if (! m_renderer->isValid()) {
        qDebug()<<"Dial file not loaded, not rendering";
//        return;
    }

    QGraphicsView::paintEvent(event);
}

void JoystickControl::resizeEvent(QResizeEvent *event)
{
    fitInView(m_background, Qt::KeepAspectRatio );
}

/**
  * @brief Update the roll,pitch,yaw,throttle for the Manual Control Command based on stick position (from mouse event)
  */
void JoystickControl::updateMCC(QPointF point)
{
    QRectF sceneSize = scene()->sceneRect();

    double x = 2 * ( point.x() / sceneSize.width() - .5 );
    double y = -2 * ( point.y() / sceneSize.height() - .5);
    x = qBound( (double) -1, x, (double) 1);
    y = qBound( (double) -1, y, (double) 1);
    if( this->objectName() == QString("widgetLeftStick"))
    {

        ManualControlCommand::DataFields data = getMCC()->getData();
        data.Pitch = y;
        data.Yaw = x;
        getMCC()->setData(data);
    }

    if( this->objectName() == QString("widgetRightStick"))
    {

        ManualControlCommand::DataFields data = getMCC()->getData();
        data.Throttle = y;
        data.Roll = x;
        getMCC()->setData(data);
    }
}

/**
  * @}
  * @}
  */
