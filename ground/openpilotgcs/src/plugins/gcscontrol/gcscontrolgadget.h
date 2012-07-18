/**
 ******************************************************************************
 *
 * @file       GCSControlgadget.h
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

#ifndef GCSControlGADGET_H_
#define GCSControlGADGET_H_

#include <coreplugin/iuavgadget.h>
#include "manualcontrolcommand.h"
#include "gcscontrolgadgetconfiguration.h"
#include "sdlgamepad/sdlgamepad.h"
#include <QTime>
#include "gcscontrolplugin.h"
#include <QUdpSocket>
#include <QHostAddress>


namespace Core {
class IUAVGadget;
}
//class QWidget;
//class QString;
class GCSControlGadgetWidget;

using namespace Core;

class GCSControlGadget : public Core::IUAVGadget
{
    Q_OBJECT
public:
    GCSControlGadget(QString classId, GCSControlGadgetWidget *widget, QWidget *parent = 0, QObject *plugin=0);
    ~GCSControlGadget();

    QList<int> context() const { return m_context; }
    QWidget *widget() { return m_widget; }
    QString contextHelpId() const { return QString(); }

    void loadConfiguration(IUAVGadgetConfiguration* config);

private:
    ManualControlCommand* getManualControlCommand();
    double constrain(double value);
    QTime joystickTime;
    QWidget *m_widget;
    QList<int> m_context;
    UAVObject::Metadata mccInitialData;
    int rollChannel;
    int pitchChannel;
    int yawChannel;
    int throttleChannel;
    int controlsMode;
    buttonSettingsStruct buttonSettings[8];
    double bound(double input);
    double wrap(double input);
    bool channelReverse[8];
    QUdpSocket *control_sock;


signals:
    void sticksChangedRemotely(double leftX, double leftY, double rightX, double rightY);

protected slots:
    void manualControlCommandUpdated(UAVObject *);
    void sticksChangedLocally(double leftX, double leftY, double rightX, double rightY);
    void readUDPCommand();

    // signals from joystick
    void gamepads(quint8 count);
    void buttonState(ButtonNumber number, bool pressed);
    void axesValues(QListInt16 values);
};


#endif // GCSControlGADGET_H_
