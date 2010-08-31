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
#include <QtCore/QDataStream>

HITLConfiguration::HITLConfiguration(QString classId, const QByteArray &state, QObject *parent) :
	IUAVGadgetConfiguration(classId, parent)
{
	settings.simulatorId = "";
	settings.binPath = "";
	settings.dataPath = "";
	settings.manual = false;
	settings.hostAddress = "127.0.0.1";
	settings.outPort = 0;
	settings.inPort = 0;
	settings.latitude = "";
	settings.longitude = "";

    if (state.count() > 0) {
        QDataStream stream(state);

		stream >> settings.simulatorId;
		stream >> settings.binPath;
		stream >> settings.dataPath;
		stream >> settings.manual;
		stream >> settings.hostAddress;
		stream >> settings.outPort;
		stream >> settings.inPort;
		stream >> settings.latitude;
		stream >> settings.longitude;
    }
}

IUAVGadgetConfiguration *HITLConfiguration::clone()
{
	HITLConfiguration *m = new HITLConfiguration(this->classId());

	m->settings = settings;
    return m;
}

QByteArray HITLConfiguration::saveState() const
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);

	stream << settings.simulatorId;
	stream << settings.binPath;
	stream << settings.dataPath;
	stream << settings.manual;
	stream << settings.hostAddress;
	stream << settings.outPort;
	stream << settings.inPort;
	stream << settings.latitude;
	stream << settings.longitude;
    return bytes;
}

