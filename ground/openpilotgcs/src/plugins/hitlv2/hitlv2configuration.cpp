/**
 ******************************************************************************
 *
 * @file       hitlv2configuration.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010-2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup HITLPlugin HITLv2 Plugin
 * @{
 * @brief The Hardware In The Loop plugin version 2
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

#include "hitlv2configuration.h"

HITLConfiguration::HITLConfiguration(QString classId,
                                     QSettings* qSettings,
                                     QObject *parent)
    : IUAVGadgetConfiguration(classId, parent)
{
//    qDebug() << "HITLV2Configuration";
    // default values
    QString simulatorId     = "ASimRC";
    QString hostAddress     = "127.0.0.1";
    int inPort              = 40100;
    QString remoteAddress   = "127.0.0.1";
    int outPort             = 40200;
    QString binPath         = "";
    QString dataPath        = "";

    bool homeLocation       = true;
    quint16 homeLocRate     = 0;

    bool attRaw             = true;
    quint8 attRawRate       = 20;

    bool attActual          = true;
    bool attActHW           = false;
    bool attActSim          = true;
    bool attActCalc         = false;

    bool sonarAltitude      = false;
    float sonarMaxAlt       = 5.0;
    quint16 sonarAltRate    = 50;

    bool gpsPosition        = true;
    quint16 gpsPosRate      = 200;

    bool inputCommand       = true;
    bool gcsReciever        = true;
    bool manualControl      = false;

    bool manualOutput       = false;
    quint8 outputRate       = 20;

    // if a saved configuration exists load it
    if (qSettings != 0) {
        settings.simulatorId        = qSettings->value("simulatorId", simulatorId).toString();
        settings.hostAddress        = qSettings->value("hostAddress", hostAddress).toString();
        settings.inPort             = qSettings->value("inPort", inPort).toInt();
        settings.remoteAddress      = qSettings->value("remoteAddress", remoteAddress).toString();
        settings.outPort            = qSettings->value("outPort", outPort).toInt();
        settings.binPath            = qSettings->value("binPath", binPath).toString();
        settings.dataPath           = qSettings->value("dataPath", dataPath).toString();

        settings.homeLocation       = qSettings->value("homeLocation", homeLocation).toBool();
        settings.homeLocRate        = qSettings->value("homeLocRate", homeLocRate).toInt();

        settings.attRaw             = qSettings->value("attRaw", attRaw).toBool();
        settings.attRawRate         = qSettings->value("attRawRate", attRawRate).toInt();

        settings.attActual          = qSettings->value("attActual", attActual).toBool();
        settings.attActHW           = qSettings->value("attActHW", attActHW).toBool();
        settings.attActSim          = qSettings->value("attActSim", attActSim).toBool();
        settings.attActCalc         = qSettings->value("attActCalc", attActCalc).toBool();

        settings.sonarAltitude      = qSettings->value("sonarAltitude", sonarAltitude).toBool();
        settings.sonarMaxAlt        = qSettings->value("sonarMaxAlt", sonarMaxAlt).toFloat();
        settings.sonarAltRate       = qSettings->value("sonarAltRate", sonarAltRate).toInt();

        settings.gpsPosition        = qSettings->value("gpsPosition", gpsPosition).toBool();
        settings.gpsPosRate         = qSettings->value("gpsPosRate", gpsPosRate).toInt();

        settings.inputCommand       = qSettings->value("inputCommand", inputCommand).toBool();
        settings.gcsReciever        = qSettings->value("gcsReciever", gcsReciever).toBool();
        settings.manualControl      = qSettings->value("manualControl", manualControl).toBool();
        settings.manualOutput       = qSettings->value("manualOutput", manualOutput).toBool();
        settings.outputRate         = qSettings->value("outputRate", outputRate).toInt();
    } else {
        settings.simulatorId        = simulatorId;
        settings.hostAddress        = hostAddress;
        settings.inPort             = inPort;
        settings.remoteAddress      = remoteAddress;
        settings.outPort            = outPort;
        settings.binPath            = binPath;
        settings.dataPath           = dataPath;

        settings.homeLocation       = homeLocation;
        settings.homeLocRate        = homeLocRate;

        settings.attRaw             = attRaw;
        settings.attRawRate         = attRawRate;

        settings.attActual          = attActual;
        settings.attActHW           = attActHW;
        settings.attActSim          = attActSim;
        settings.attActCalc         = attActCalc;

        settings.sonarAltitude      = sonarAltitude;
        settings.sonarMaxAlt        = sonarMaxAlt;
        settings.sonarAltRate       = sonarAltRate;

        settings.gpsPosition        = gpsPosition;
        settings.gpsPosRate         = gpsPosRate;

        settings.inputCommand       = inputCommand;
        settings.gcsReciever        = gcsReciever;
        settings.manualControl      = manualControl;
        settings.manualOutput       = manualOutput;
        settings.outputRate         = outputRate;
    }
}

IUAVGadgetConfiguration *HITLConfiguration::clone()
{
    HITLConfiguration *m = new HITLConfiguration(this->classId());
    m->settings = settings;
    return m;
}

void HITLConfiguration::saveConfig(QSettings* qSettings) const
{
    qSettings->setValue("simulatorId", settings.simulatorId);
    qSettings->setValue("hostAddress", settings.hostAddress);
    qSettings->setValue("inPort", settings.inPort);
    qSettings->setValue("remoteAddress", settings.remoteAddress);
    qSettings->setValue("outPort", settings.outPort);
    qSettings->setValue("binPath", settings.binPath);
    qSettings->setValue("dataPath", settings.dataPath);

    qSettings->setValue("homeLocation", settings.homeLocation);
    qSettings->setValue("homeLocRate", settings.homeLocRate);

    qSettings->setValue("attRaw", settings.attRaw);
    qSettings->setValue("attRawRate", settings.attRawRate);

    qSettings->setValue("attActual", settings.attActual);
    qSettings->setValue("attActHW", settings.attActHW);
    qSettings->setValue("attActSim", settings.attActSim);
    qSettings->setValue("attActCalc", settings.attActCalc);

    qSettings->setValue("sonarAltitude", settings.sonarAltitude);
    qSettings->setValue("sonarMaxAlt", settings.sonarMaxAlt);
    qSettings->setValue("sonarAltRate", settings.sonarAltRate);

    qSettings->setValue("gpsPosition", settings.gpsPosition);
    qSettings->setValue("gpsPosRate", settings.gpsPosRate);

    qSettings->setValue("inputCommand", settings.inputCommand);
    qSettings->setValue("gcsReciever", settings.gcsReciever);
    qSettings->setValue("manualControl", settings.manualControl);
    qSettings->setValue("manualOutput", settings.manualOutput);
    qSettings->setValue("outputRate", settings.outputRate);
}
