/**
 ******************************************************************************
 *
 * @file       hitlconfiguration.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup HITLPlugin HITL Plugin
 * @{
 * @brief The Hardware In The Loop plugin 
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

#include "hitlconfiguration.h"

HITLConfiguration::HITLConfiguration(QString classId, QSettings* qSettings, QObject *parent) :
        IUAVGadgetConfiguration(classId, parent)
{
        settings.simulatorId = "";
        settings.binPath = "";
        settings.dataPath = "";
        settings.manual = false;
        settings.startSim = false;
        settings.hostAddress = "127.0.0.1";
        settings.remoteHostAddress = "127.0.0.1";
        settings.outPort = 0;
        settings.inPort = 0;
        settings.latitude = "";
        settings.longitude = "";

        //if a saved configuration exists load it
        if(qSettings != 0) {
                settings.simulatorId = qSettings->value("simulatorId").toString();
                settings.binPath = qSettings->value("binPath").toString();
                settings.dataPath = qSettings->value("dataPath").toString();
                settings.manual = qSettings->value("manual").toBool();
                settings.startSim = qSettings->value("startSim").toBool();
                settings.hostAddress = qSettings->value("hostAddress").toString();
                settings.remoteHostAddress = qSettings->value("remoteHostAddress").toString();
                settings.outPort = qSettings->value("outPort").toInt();
                settings.inPort = qSettings->value("inPort").toInt();
                settings.latitude = qSettings->value("latitude").toString();
                settings.longitude = qSettings->value("longitude").toString();                
        }
}

IUAVGadgetConfiguration *HITLConfiguration::clone()
{
	HITLConfiguration *m = new HITLConfiguration(this->classId());

	m->settings = settings;
    return m;
}

 /**
  * Saves a configuration.
  *
  */
void HITLConfiguration::saveConfig(QSettings* qSettings) const {
    qSettings->setValue("simulatorId", settings.simulatorId);
    qSettings->setValue("binPath", settings.binPath);
    qSettings->setValue("dataPath", settings.dataPath);
    qSettings->setValue("manual", settings.manual);
    qSettings->setValue("startSim", settings.startSim);
    qSettings->setValue("hostAddress", settings.hostAddress);
    qSettings->setValue("remoteHostAddress", settings.remoteHostAddress);
    qSettings->setValue("outPort", settings.outPort);
    qSettings->setValue("inPort", settings.inPort);
    qSettings->setValue("latitude", settings.latitude);
    qSettings->setValue("longitude", settings.longitude);
}

