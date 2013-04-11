/**
 ******************************************************************************
 *
 * @file       gcscontrolgadgetconfiguration.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GCSControlGadgetPlugin GCSControl Gadget Plugin
 * @{
 * @brief A gadget to control the UAV, either from the keyboard or a joystick
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

#ifndef GCSCONTROLGADGETCONFIGURATION_H
#define GCSCONTROLGADGETCONFIGURATION_H

#include <coreplugin/iuavgadgetconfiguration.h>
#include <QtNetwork/QHostAddress>

typedef struct{
    int     ActionID;
    int     FunctionID;
    double  Amount;
}buttonSettingsStruct;

typedef struct{
    int port;
    QHostAddress address;
}portSettingsStruct;


using namespace Core;



class GCSControlGadgetConfiguration : public IUAVGadgetConfiguration
{
    Q_OBJECT
    public:
        explicit GCSControlGadgetConfiguration(QString classId, QSettings* qSettings = 0, QObject *parent = 0);

    void setControlsMode(int mode) { controlsMode = mode; }
    void setRPYTchannels(int roll, int pitch, int yaw, int throttle);
    void setUDPControlSettings(int port, QString host);
    int getUDPControlPort();
    QHostAddress getUDPControlHost();
    int getControlsMode() { return controlsMode; }
    QList<int>  getChannelsMapping();
    QList<bool>  getChannelsReverse();

    buttonSettingsStruct getbuttonSettings(int i){return buttonSettings[i];}
    void setbuttonSettingsAction(int i, int ActionID ){buttonSettings[i].ActionID=ActionID;return;}
    void setbuttonSettingsFunction(int i, int FunctionID ){buttonSettings[i].FunctionID=FunctionID;return;}
    void setbuttonSettingsAmount(int i, double Amount ){buttonSettings[i].Amount=Amount;return;}
    void setChannelReverse(int i, bool Reverse ){channelReverse[i]=Reverse;return;}


        void saveConfig(QSettings* settings) const;
        IUAVGadgetConfiguration *clone();

    private:
        int controlsMode; // Mode1 to Mode4
        // Joystick mappings for roll/pitch/yaw/throttle:
        int rollChannel;
        int pitchChannel;
        int yawChannel;
        int throttleChannel;
        buttonSettingsStruct buttonSettings[8];
        bool channelReverse[8];
        int udp_port;
        QHostAddress udp_host;


};

#endif // GCSCONTROLGADGETCONFIGURATION_H
