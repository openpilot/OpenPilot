/**
 ******************************************************************************
 * @file
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup importexportplugin
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

#ifndef IMPORTEXPORTGADGETCONFIGURATION_H
#define IMPORTEXPORTGADGETCONFIGURATION_H

#include <coreplugin/iuavgadgetconfiguration.h>
#include "importexport_global.h"

using namespace Core;

/* This is a generic bargraph dial
   supporting one indicator.
  */
class IMPORTEXPORT_EXPORT ImportExportGadgetConfiguration : public IUAVGadgetConfiguration
{
    Q_OBJECT
public:
    explicit ImportExportGadgetConfiguration(QString classId, QSettings* qSettings = 0, QObject *parent = 0);

    //set dial configuration functions
    void setDialFile(QString filename) {
        dialFile = filename;
    }

    //get dial configuration functions
    QString getDialFile() const{
        return dialFile;
    }

    void saveConfig(QSettings* settings) const;
    IUAVGadgetConfiguration *clone();

private:
    QString dialFile;
};

#endif // IMPORTEXPORTGADGETCONFIGURATION_H
/**
 * @}
 * @}
 */
