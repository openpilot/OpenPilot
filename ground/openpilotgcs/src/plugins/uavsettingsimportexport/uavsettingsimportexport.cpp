/**
 ******************************************************************************
 *
 * @file       uavsettingsimportexport.cpp
 * @author     (C) 2011 The OpenPilot Team, http://www.openpilot.org
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVSettingsImportExport UAVSettings Import/Export Plugin
 * @{
 * @brief UAVSettings Import/Export Plugin
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

#include "uavsettingsimportexport.h"

#include <QtPlugin> 
#include <QStringList> 
#include <QDebug>
#include <QCheckBox>
#include "importsummary.h"
// for menu item
#include <coreplugin/coreconstants.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/icore.h>
#include <QKeySequence>

// for UAVObjects
#include "uavdataobject.h"
#include "uavobjectmanager.h"
#include "extensionsystem/pluginmanager.h"

// for XML object
#include <QDomDocument>

// for file dialog and error messages
#include <QFileDialog>
#include <QMessageBox>

UAVSettingsImportExportPlugin::UAVSettingsImportExportPlugin() 
{ 
   // Do nothing 
} 

UAVSettingsImportExportPlugin::~UAVSettingsImportExportPlugin() 
{ 
   // Do nothing 
} 

bool UAVSettingsImportExportPlugin::initialize(const QStringList& args, QString *errMsg) 
{ 
    Q_UNUSED(args);
    Q_UNUSED(errMsg);
    mf = new UAVSettingsImportExportFactory(this);
    addAutoReleasedObject(mf);
    return true;
}

void UAVSettingsImportExportPlugin::shutdown()
{ 
   // Do nothing 
}
void UAVSettingsImportExportPlugin::extensionsInitialized()
{
}
Q_EXPORT_PLUGIN(UAVSettingsImportExportPlugin)


