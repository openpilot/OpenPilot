/**
 ******************************************************************************
 *
 * @file       serialpluginconfiguration.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup SerialPlugin Serial Connection Plugin
 * @{
 * @brief Impliments serial connection to the flight hardware for Telemetry
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

#ifndef SERIALPLUGINCONFIGURATION_H
#define SERIALPLUGINCONFIGURATION_H

#include <coreplugin/iuavgadgetconfiguration.h>

using namespace Core;

/* Despite its name, this is actually a generic analog Serial
   supporting up to two rotating "needle" indicators.
  */
class SerialPluginConfiguration : public IUAVGadgetConfiguration
{
Q_OBJECT
public:
    explicit SerialPluginConfiguration(QString classId, QSettings* qSettings = 0, QObject *parent = 0);
    QString speed() {return m_speed;}
    void saveConfig(QSettings* settings) const;
    IUAVGadgetConfiguration *clone();
    void savesettings() const;
    void restoresettings();
    virtual ~SerialPluginConfiguration();
private:
    QString m_speed;
    QSettings* settings;
public slots:
    void setSpeed(QString speed) { m_speed = speed; }

};

#endif // SERIALPLUGINCONFIGURATION_H
