/**
 ******************************************************************************
 *
 * @file       systemhealthgadgetconfiguration.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup SystemHealthPlugin System Health Plugin
 * @{
 * @brief The System Health gadget plugin
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

#ifndef SYSTEMHEALTHGADGETCONFIGURATION_H
#define SYSTEMHEALTHGADGETCONFIGURATION_H

#include <coreplugin/iuavgadgetconfiguration.h>

using namespace Core;

/* This is a generic system health gadget displaying
   system alarms for one or more components.
  */
class SystemHealthGadgetConfiguration : public IUAVGadgetConfiguration
{
Q_OBJECT
public:
    explicit SystemHealthGadgetConfiguration(QString classId, QSettings* qSettings = 0, QObject *parent = 0);

    //set system health configuration functions
    void setSystemFile(QString filename){systemFile=filename;}

    //get dial configuration functions
    QString getSystemFile() {return systemFile;}

    void saveConfig(QSettings* settings) const;
    IUAVGadgetConfiguration *clone();

private:
    // systemFile contains the source SVG:
    QString systemFile;

};

#endif // SYSTEMHEALTHGADGETCONFIGURATION_H
