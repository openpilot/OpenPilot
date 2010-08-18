/**
 ******************************************************************************
 *
 * @file       importexportplugin.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @brief      Import/Export Plugin
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @defgroup   importexportplugin
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

#include "importexportplugin.h"
#include "importexportgadgetfactory.h"
#include <QDebug>
#include <QtPlugin>
#include <QStringList>
#include <extensionsystem/pluginmanager.h>


ImportExportPlugin::ImportExportPlugin()
{
    // Do nothing
}

ImportExportPlugin::~ImportExportPlugin()
{
    // Do nothing
}

bool ImportExportPlugin::initialize(const QStringList& args, QString *errMsg)
{
    Q_UNUSED(args);
    Q_UNUSED(errMsg);
    mf = new ImportExportGadgetFactory(this);
    addAutoReleasedObject(mf);

    return true;
}

void ImportExportPlugin::extensionsInitialized()
{
    // Do nothing
}

void ImportExportPlugin::shutdown()
{
    // Do nothing
}
Q_EXPORT_PLUGIN(ImportExportPlugin)

/**
 * @}
 * @}
 */
