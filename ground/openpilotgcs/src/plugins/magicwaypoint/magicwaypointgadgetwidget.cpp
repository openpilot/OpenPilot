/**
 ******************************************************************************
 *
 * @file       GCSControlgadgetwidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GCSControlGadgetPlugin GCSControl Gadget Plugin
 * @{
 * @brief A gadget to control the UAV, either from the keyboard or a joystick
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
#include "magicwaypointgadgetwidget.h"
#include "ui_magicwaypoint.h"

#include <QDebug>
#include <QString>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>

#include "uavobject.h"
#include "uavobjectmanager.h"
#include "manualcontrolcommand.h"
#include "extensionsystem/pluginmanager.h"
#include "extensionsystem/pluginmanager.h"

MagicWaypointGadgetWidget::MagicWaypointGadgetWidget(QWidget *parent) : QLabel(parent)
{
    m_magicwaypoint = new Ui_MagicWaypoint();
    m_magicwaypoint->setupUi(this);

    // Connect object updated event from UAVObject to also update check boxes
    connect(getPathDesired(), SIGNAL(objectUpdated(UAVObject*)), this, SLOT(positionObjectChanged(UAVObject*)));
    connect(getPositionActual(), SIGNAL(objectUpdated(UAVObject*)), this, SLOT(positionObjectChanged(UAVObject*)));

    // Connect updates from the position widget to this widget
    connect(m_magicwaypoint->widgetPosition, SIGNAL(positionClicked(double,double)), this, SLOT(positionSelected(double,double)));
    connect(this, SIGNAL(positionActualObjectChanged(double,double)), m_magicwaypoint->widgetPosition, SLOT(updateActualIndicator(double,double)));
    connect(this, SIGNAL(positionDesiredObjectChanged(double,double)), m_magicwaypoint->widgetPosition, SLOT(updateDesiredIndicator(double,double)));

    // Catch changes in scale for visualization
    connect(m_magicwaypoint->horizontalSliderScale, SIGNAL(valueChanged(int)), this, SLOT(scaleChanged(int)));

    // Make sure the scale is correctly visualized
    m_magicwaypoint->horizontalSliderScale->setValue(10);
}

MagicWaypointGadgetWidget::~MagicWaypointGadgetWidget()
{
   // Do nothing
}

/*!
  \brief Returns the @ref PositionDesired UAVObject
  */
PathDesired* MagicWaypointGadgetWidget::getPathDesired()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    PathDesired* obj = PathDesired::GetInstance(objManager);
    Q_ASSERT(obj != NULL); // Save crashes later
    return obj;
}

/*!
  \brief Returns the @ref PositionActual UAVObject
  */
PositionActual* MagicWaypointGadgetWidget::getPositionActual()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    PositionActual *obj = PositionActual::GetInstance(objManager);
    Q_ASSERT(obj != NULL);
    return obj;
}

/**
  * Detect changes in scale and update visualization (does not change position)
  */
void MagicWaypointGadgetWidget::scaleChanged(int scale) {
    Q_UNUSED(scale);
    pathDesiredChanged(getPathDesired());
    positionActualChanged(getPositionActual());
}

/**
  * Emit a position changed signal when @ref PositionActual object is changed
  */
void MagicWaypointGadgetWidget::positionActualChanged(UAVObject *)
{
    PositionActual::DataFields positionActual = getPositionActual()->getData();
    double scale = m_magicwaypoint->horizontalSliderScale->value();

    emit positionActualObjectChanged(positionActual.North / scale,
                                     positionActual.East / scale);
}

/**
  * Emit a position changed signal when @ref PathDesired is changed
  */
void MagicWaypointGadgetWidget::pathDesiredChanged(UAVObject *)
{
    PathDesired::DataFields pathDesired = getPathDesired()->getData();
    double scale = m_magicwaypoint->horizontalSliderScale->value();

    emit positionDesiredObjectChanged(pathDesired.End[PathDesired::END_NORTH] / scale,
                                      pathDesired.End[PathDesired::END_EAST] / scale);
}

/**
  * Slot called by visualization when a new @ref PathDesired is requested
  */
void MagicWaypointGadgetWidget::positionSelected(double north, double east) {
    double scale = m_magicwaypoint->horizontalSliderScale->value();

    PathDesired::DataFields pathDesired = getPathDesired()->getData();
    pathDesired.End[PathDesired::END_NORTH] = north * scale;
    pathDesired.End[PathDesired::END_EAST] = east * scale;
    pathDesired.Mode = PathDesired::MODE_FLYENDPOINT;
    getPathDesired()->setData(pathDesired);
}

/**
  * @}
  * @}
  */
