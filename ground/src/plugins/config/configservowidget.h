/**
 ******************************************************************************
 *
 * @file       configservowidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief Servo input/output configuration panel for the config gadget
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
#ifndef CONFIGSERVOWIDGET_H
#define CONFIGSERVOWIDGET_H

#include "ui_settingswidget.h"
#include "configtaskwidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/uavobject.h"
#include <QtGui/QWidget>
#include <QList>

class Ui_SettingsWidget;

class ConfigServoWidget: public ConfigTaskWidget
{
    Q_OBJECT

public:
    ConfigServoWidget(QWidget *parent = 0);
    ~ConfigServoWidget();


private:
        Ui_SettingsWidget *m_config;
        QList<QSlider> sliders;
        void updateChannelSlider(QSlider* slider, QLabel* min, QLabel* Max,  QCheckBox* rev, int value);
        void assignChannel(UAVDataObject *obj, UAVObjectField *field, QString str);
        void assignOutputChannel(UAVDataObject *obj, UAVObjectField *field, QString str);
        int mccDataRate;
        UAVObject::Metadata accInitialData;

        bool firstUpdate;

    private slots:
        void updateChannels(UAVObject* obj);
        void requestRCInputUpdate();
        void sendRCInputUpdate();
        void saveRCInputObject();
        void requestRCOutputUpdate();
        void sendRCOutputUpdate();
        void saveRCOutputObject();
        void runChannelTests(bool state);
        void sendChannelTest(int value);


        void setch0OutRange();
        void setch1OutRange();
        void setch2OutRange();
        void setch3OutRange();
        void setch4OutRange();
        void setch5OutRange();
        void setch6OutRange();
        void setch7OutRange();

};

#endif // CONFIGSERVOWIDGET_H
