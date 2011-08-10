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
#ifndef CONFIGINPUTWIDGET_H
#define CONFIGINPUTWIDGET_H

#include "ui_input.h"
#include "configtaskwidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include <QtGui/QWidget>
#include <QList>
#include "inputchannelform.h"
#include "ui_inputchannelform.h"
#include <QRadioButton>
#include "manualcontrolcommand.h"
#include "manualcontrolsettings.h"
#include "receiveractivity.h"
class Ui_InputWidget;

class ConfigInputWidget: public ConfigTaskWidget
{
	Q_OBJECT
public:
        ConfigInputWidget(QWidget *parent = 0);
        ~ConfigInputWidget();
        enum wizardSteps{wizardWelcome,wizardChooseMode,wizardIdentifySticks,wizardIdentifyCenter,wizardIdentifyLimits,wizardIdentifyInverted,wizardFinish};
        enum txMode{mode1,mode2};
public slots:

private:
        Ui_InputWidget *m_config;
        wizardSteps wizardStep;
        void setupWizardWidget(int step);
        QList<QWidget*> extraWidgets;
        txMode transmitterMode;
        struct channelsStruct
        {
            bool operator ==(const channelsStruct& rhs) const
            {
                return((group==rhs.group) &&(number==rhs.number));
            }
            int group;
            int number;
        }lastChannel;
        channelsStruct currentChannel;
        QList<channelsStruct> usedChannels;
        QEventLoop * loop;
        bool skipflag;

        int currentCommand;

        ManualControlCommand * manualCommandObj;
        ManualControlCommand::DataFields manualCommandData;
        ManualControlSettings * manualSettingsObj;
        ManualControlSettings::DataFields manualSettingsData;
        ReceiverActivity * receiverActivityObj;
        ReceiverActivity::DataFields receiverActivityData;

        /*
        ManualControlCommand * manualCommandObj = ManualControlCommand::GetInstance(getObjectManager());
        ManualControlCommand::DataFields manualCommandData = manualCommandObj->getData();
        ManualControlSettings * manualSettingsObj = ManualControlSettings::GetInstance(getObjectManager());
        ManualControlSettings::DataFields manualSettingsData;
        ReceiverActivity * receiverActivityObj=ReceiverActivity::GetInstance(getObjectManager());
        ReceiverActivity::DataFields receiverActivityData =receiverActivityObj->getData();
*/
private slots:
        void wzNext();
        void wzBack();
        void wzCancel();
        void goToWizard();
        void openHelp();
        void identifyControls();
        void identifyLimits();
};

#endif
