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
#include "gcscontrolgadgetwidget.h"
#include "ui_gcscontrol.h"

#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QTextEdit>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>

#include "uavobjects/uavobject.h"
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/manualcontrolcommand.h"
#include "extensionsystem/pluginmanager.h"

GCSControlGadgetWidget::GCSControlGadgetWidget(QWidget *parent) : QLabel(parent)
{
    m_gcscontrol = new Ui_GCSControl();
    m_gcscontrol->setupUi(this);

    // Set up the drop down box for the flightmode
    m_gcscontrol->comboBoxFlightMode->addItem(QString("Manual"));
    m_gcscontrol->comboBoxFlightMode->addItem(QString("Stabilized"));
    m_gcscontrol->comboBoxFlightMode->addItem(QString("Auto"));

    // Set up slots and signals
    connect(m_gcscontrol->checkBoxGcsControl, SIGNAL(stateChanged(int)), this, SLOT(gcsControlToggle(int)));
    connect(m_gcscontrol->comboBoxFlightMode, SIGNAL(currentIndexChanged(int)), this, SLOT(flightModeChanged(int)));

}

GCSControlGadgetWidget::~GCSControlGadgetWidget()
{
   // Do nothing
}

/*!
  \brief Returns the ManualControlCommand UAVObject
  */
ManualControlCommand* GCSControlGadgetWidget::getMCC()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    ManualControlCommand* obj = dynamic_cast<ManualControlCommand*>( objManager->getObject(QString("ManualControlCommand")) );
    return obj;
}

/*!
  \brief Called when the gcs control is toggled and enabled or disables flight write access to manual control command
  */
void GCSControlGadgetWidget::gcsControlToggle(int state)
{
    UAVObject::Metadata mdata = getMCC()->getMetadata();
    if (state)
    {
        mdata.flightAccess = UAVObject::ACCESS_READONLY;
    }
    else
    {
        mdata.flightAccess = UAVObject::ACCESS_READWRITE;
    }
    getMCC()->setMetadata(mdata);
}

/*!
  \brief Called when the flight mode drop down is changed and sets the ManualControlCommand->FlightMode accordingly
  */
void GCSControlGadgetWidget::flightModeChanged(int state)
{
    ManualControlCommand::DataFields data = getMCC()->getData();
    if( state == 0 )
    {
        data.FlightMode = ManualControlCommand::FLIGHTMODE_MANUAL;
    }
    else if ( state == 1 )
    {
        data.FlightMode = ManualControlCommand::FLIGHTMODE_STABILIZED;
    }
    else if ( state == 2 )
    {
        data.FlightMode = ManualControlCommand::FLIGHTMODE_AUTO;
    }
    getMCC()->setData(data);
}

/**
  * @}
  * @}
  */
