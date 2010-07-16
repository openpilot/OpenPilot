/**
 ******************************************************************************
 *
 * @file       gpsdisplaygadgetconfiguration.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GPSGadgetPlugin GPS Gadget Plugin
 * @{
 * @brief A gadget that displays GPS status and enables basic configuration 
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

#ifndef GPSDISPLAYGADGETCONFIGURATION_H
#define GPSDISPLAYGADGETCONFIGURATION_H

#include <coreplugin/iuavgadgetconfiguration.h>

using namespace Core;

/* This is a generic system health gadget displaying
   system alarms for one or more components.
  */
class GpsDisplayGadgetConfiguration : public IUAVGadgetConfiguration
{
Q_OBJECT
public:
    explicit GpsDisplayGadgetConfiguration(QString classId, const QByteArray &state = 0, QObject *parent = 0);

    //set gps display configuration functions
    void setSystemFile(QString filename){systemFile=filename;}

    //get dial configuration functions
    QString getSystemFile() {return systemFile;}

    QByteArray saveState() const;
    IUAVGadgetConfiguration *clone();

private:
    // systemFile contains the source SVG:
    QString systemFile;

};

#endif // GPSDISPLAYGADGETCONFIGURATION_H
