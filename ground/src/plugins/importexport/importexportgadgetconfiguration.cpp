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
#include <QtCore/QDataStream>

/**
 * Loads a saved configuration or defaults if non exist.
 *
 */
ImportExportGadgetConfiguration::ImportExportGadgetConfiguration(QString classId, const QByteArray &state, QObject *parent) :
        IUAVGadgetConfiguration(classId, parent),
        dialFile("gcs.ini")
{
    //if a saved configuration exists load it
    if (state.count() > 0) {
        QDataStream stream(state);
        stream >> dialFile;
    }
}
/**
 * Clones a configuration.
 *
 */
IUAVGadgetConfiguration *ImportExportGadgetConfiguration::clone()
{
    ImportExportGadgetConfiguration *m = new ImportExportGadgetConfiguration(this->classId());
    m->dialFile = dialFile;
    return m;
}
/**
 * Saves a configuration.
 *
 */
QByteArray ImportExportGadgetConfiguration::saveState() const
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << dialFile;
    return bytes;
}

/**
 * @}
 * @}
 */
