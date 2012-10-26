/**
 ******************************************************************************
 *
 * @file       configgadgetwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief The Configuration Gadget used to update settings in the firmware
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
#ifndef CONFIGGADGETWIDGET_H
#define CONFIGGADGETWIDGET_H

#include "uavtalk/telemetrymanager.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include "objectpersistence.h"
#include <QtGui/QWidget>
#include <QList>
#include <QTextBrowser>
#include "utils/pathutils.h"
#include <QMessageBox>
#include "utils/mytabbedstackwidget.h"
#include "../uavobjectwidgetutils/configtaskwidget.h"

class ConfigGadgetWidget: public QWidget
{
    Q_OBJECT
    QTextBrowser* help;

public:
    ConfigGadgetWidget(QWidget *parent = 0);
    ~ConfigGadgetWidget();
    enum widgetTabs {hardware=0, aircraft, input, output, sensors, stabilization, camerastabilization, txpid, pipxtreme};
    void startInputWizard();

public slots:
    void onAutopilotConnect();
    void onAutopilotDisconnect();
    void tabAboutToChange(int i,bool *);
    void updatePipXStatus(UAVObject *object);
    void onPipxtremeDisconnect();

signals:
    void autopilotConnected();
    void autopilotDisconnected();
    void pipxtremeConnect();
    void pipxtremeDisconnect();

protected:
        void resizeEvent(QResizeEvent * event);
        MyTabbedStackWidget *ftw;

private:
	UAVDataObject* pipxStatusObj;

	// A timer that timesout the connction to the PipX.
	QTimer *pipxTimeout;
	bool pipxConnected;
};

#endif // CONFIGGADGETWIDGET_H
