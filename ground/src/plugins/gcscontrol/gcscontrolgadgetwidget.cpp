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
    connect(m_gcscontrol->pushButton, SIGNAL(clicked()), this, SLOT(buttonPressed()));
}

GCSControlGadgetWidget::~GCSControlGadgetWidget()
{
   // Do nothing
}

void GCSControlGadgetWidget::buttonPressed()
{
    // Get access to the ManualControlObject
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    ManualControlCommand* obj = dynamic_cast<ManualControlCommand*>( objManager->getObject(QString("ManualControlCommand")) );

    // Need to set the metadata to let GCS override OpenPilot
    UAVObject::Metadata mdata = obj->getMetadata();
    mdata.gcsAccess = UAVObject::ACCESS_READWRITE;
    mdata.flightAccess = UAVObject::ACCESS_READONLY;
    obj->setMetadata(mdata);

    // Set values to some constants for now
    ManualControlCommand::DataFields data = obj->getData();
    data.FlightMode = ManualControlCommand::FLIGHTMODE_STABILIZED;
    data.Pitch = .5;
    data.Roll = .3;
    data.Throttle = .2;
    data.Yaw = .3;
    obj->setData(data);

    // Visual confirmation
    m_gcscontrol->pushButton->setText(obj->toString());

    //Q_ASSERT( 0 );
}

