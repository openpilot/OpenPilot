/**
 ******************************************************************************
 *
 * @file       uavsettingsimportexportfactory.h
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
#ifndef UAVSETTINGSIMPORTEXPORTFACTORY_H
#define UAVSETTINGSIMPORTEXPORTFACTORY_H
#include "uavsettingsimportexport_global.h"
#include "uavobjectutil/uavobjectutilmanager.h"
#include "../../../../../build/ground/openpilotgcs/gcsversioninfo.h"
class UAVSETTINGSIMPORTEXPORT_EXPORT UAVSettingsImportExportFactory : public QObject
{
    Q_OBJECT

public:
   UAVSettingsImportExportFactory(QObject *parent = 0);
   ~UAVSettingsImportExportFactory();

private:
   enum storedData { Settings, Data, Both };
   QString createXMLDocument(const enum storedData, const bool fullExport);

private slots:
   void importUAVSettings();
   void exportUAVSettings();
   void exportUAVData();
signals:
   void importAboutToBegin();
   void importEnded();

};

#endif // UAVSETTINGSIMPORTEXPORTFACTORY_H
