/**
 ******************************************************************************
 *
 * @file       importexportgadgetconfiguration.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @brief      Configuration for Import/Export Plugin
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup   importexportplugin
 * @{
 *
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

#include "importexportgadgetconfiguration.h"

static const QString VERSION = "1.0.1";

/**
 * Loads a saved configuration or defaults if non exist.
 *
 */
ImportExportGadgetConfiguration::ImportExportGadgetConfiguration(QString classId, QSettings* qSettings, UAVConfigInfo *configInfo, QObject *parent) :
        IUAVGadgetConfiguration(classId, parent)
{
    if ( ! qSettings )
        return;

    if ( configInfo->version() == UAVConfigVersion() )
        configInfo->setVersion("1.0.0");

    if ( !configInfo->standardVersionHandlingOK(VERSION))
        return;

    iniFile = qSettings->value("dialFile", "gcs.ini").toString(); // TODO Delete with next minor version.
    iniFile = qSettings->value("iniFile", iniFile).toString();
}

/**
 * Clones a configuration.
 *
 */
IUAVGadgetConfiguration *ImportExportGadgetConfiguration::clone()
{
    ImportExportGadgetConfiguration *m = new ImportExportGadgetConfiguration(this->classId());
    m->iniFile = iniFile;
    return m;
}

/**
 * Saves a configuration.
 *
 */
void ImportExportGadgetConfiguration::saveConfig(QSettings* qSettings, Core::UAVConfigInfo *configInfo) const {
    configInfo->setVersion(VERSION);
    qSettings->setValue("dialFile", iniFile);
    qSettings->setValue("iniFile", iniFile);
}

/**
 * @}
 * @}
 */
