/**
 ******************************************************************************
 *
 * @file       hitlwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   hitlplugin
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

#ifndef HITLIL2WIDGET_H
#define HITLIL2WIDGET_H

#include <QtGui/QWidget>
#include "il2bridge.h"

class Ui_HITLIL2Widget;

class HITLIL2Widget : public QWidget
{
    Q_OBJECT

public:
    HITLIL2Widget(QWidget *parent = 0);
    ~HITLIL2Widget();

    void setIl2HostName(QString il2HostName);
    void setIl2Latitude(QString il2Latitude);
    void setIl2Longitude(QString il2Longitude);
    void setIl2Port(int il2Port);
    void setIl2ManualControl(bool val);

public slots:

private slots:
    void startButtonClicked();
    void stopButtonClicked();
    void onAutopilotConnect();
    void onAutopilotDisconnect();
    void onIl2Connect();
    void onIl2Disconnect();

private:
    Ui_HITLIL2Widget* widget;
    Il2Bridge* il2Bridge;
    QString il2HostName;
    QString il2Latitude;
    QString il2Longitude;
    int il2Port;
    bool il2ManualControl;
};

#endif /* HITLIL2WIDGET_H */
