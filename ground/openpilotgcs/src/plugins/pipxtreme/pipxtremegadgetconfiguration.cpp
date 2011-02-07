/**
 ******************************************************************************
 *
 * @file       pipxtremegadgetconfiguration.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @{
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

#include "pipxtremegadgetconfiguration.h"
#include <qextserialport/src/qextserialport.h>

/**
 * Loads a saved configuration or defaults if non exist.
 *
 */
PipXtremeGadgetConfiguration::PipXtremeGadgetConfiguration(QString classId, QSettings* qSettings, QObject *parent) :
	IUAVGadgetConfiguration(classId, parent)
{
    //if a saved configuration exists load it
    if (qSettings != 0)
    {
    }
}

PipXtremeGadgetConfiguration::~PipXtremeGadgetConfiguration()
{
}

/**
 * Clones a configuration.
 *
 */
IUAVGadgetConfiguration *PipXtremeGadgetConfiguration::clone()
{
    PipXtremeGadgetConfiguration *m = new PipXtremeGadgetConfiguration(this->classId());
    return m;
}

/**
 * Saves a configuration.
 *
 */
void PipXtremeGadgetConfiguration::saveConfig(QSettings *qSettings) const
{
	if (qSettings)
	{

	}
}
