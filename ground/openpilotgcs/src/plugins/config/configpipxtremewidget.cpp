/**
 ******************************************************************************
 *
 * @file       configtxpidswidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief The Configuration Gadget used to configure the PipXtreme
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

#include "configpipxtremewidget.h"

#include <coreplugin/generalsettings.h>
#include <oplinksettings.h>
#include <oplinkstatus.h>

ConfigPipXtremeWidget::ConfigPipXtremeWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_oplink = new Ui_PipXtremeWidget();
    m_oplink->setupUi(this);

    // Connect to the OPLinkStatus object updates
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    oplinkStatusObj = dynamic_cast<UAVDataObject *>(objManager->getObject("OPLinkStatus"));
    if (oplinkStatusObj != NULL) {
        connect(oplinkStatusObj, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(updateStatus(UAVObject *)));
    } else {
        qDebug() << "Error: Object is unknown (OPLinkStatus).";
    }

    // Connect to the OPLinkSettings object updates
    oplinkSettingsObj = dynamic_cast<OPLinkSettings *>(objManager->getObject("OPLinkSettings"));
    if (oplinkSettingsObj != NULL) {
        connect(oplinkSettingsObj, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(updateSettings(UAVObject *)));
    } else {
        qDebug() << "Error: Object is unknown (OPLinkSettings).";
    }
    autoLoadWidgets();
    Core::Internal::GeneralSettings *settings = pm->getObject<Core::Internal::GeneralSettings>();
    if (!settings->useExpertMode()) {
        m_oplink->Apply->setVisible(false);
    }
    addApplySaveButtons(m_oplink->Apply, m_oplink->Save);

    addUAVObjectToWidgetRelation("OPLinkSettings", "MainPort", m_oplink->MainPort);
    addUAVObjectToWidgetRelation("OPLinkSettings", "FlexiPort", m_oplink->FlexiPort);
    addUAVObjectToWidgetRelation("OPLinkSettings", "VCPPort", m_oplink->VCPPort);
    addUAVObjectToWidgetRelation("OPLinkSettings", "MaxRFPower", m_oplink->MaxRFTxPower);
    addUAVObjectToWidgetRelation("OPLinkSettings", "MinFrequency", m_oplink->MinimumFrequency);
    addUAVObjectToWidgetRelation("OPLinkSettings", "MaxFrequency", m_oplink->MaximumFrequency);
    addUAVObjectToWidgetRelation("OPLinkSettings", "InitFrequency", m_oplink->InitFrequency);
    addUAVObjectToWidgetRelation("OPLinkSettings", "ChannelSpacing", m_oplink->StepSize);

    addUAVObjectToWidgetRelation("OPLinkStatus", "DeviceID", m_oplink->DeviceID);
    addUAVObjectToWidgetRelation("OPLinkStatus", "RxGood", m_oplink->Good);
    addUAVObjectToWidgetRelation("OPLinkStatus", "RxCorrected", m_oplink->Corrected);
    addUAVObjectToWidgetRelation("OPLinkStatus", "RxErrors", m_oplink->Errors);
    addUAVObjectToWidgetRelation("OPLinkStatus", "RxMissed", m_oplink->Missed);
    addUAVObjectToWidgetRelation("OPLinkStatus", "RxFailure", m_oplink->RxFailure);
    addUAVObjectToWidgetRelation("OPLinkStatus", "UAVTalkErrors", m_oplink->UAVTalkErrors);
    addUAVObjectToWidgetRelation("OPLinkStatus", "TxDropped", m_oplink->Dropped);
    addUAVObjectToWidgetRelation("OPLinkStatus", "TxResent", m_oplink->Resent);
    addUAVObjectToWidgetRelation("OPLinkStatus", "TxFailure", m_oplink->TxFailure);
    addUAVObjectToWidgetRelation("OPLinkStatus", "Resets", m_oplink->Resets);
    addUAVObjectToWidgetRelation("OPLinkStatus", "Timeouts", m_oplink->Timeouts);
    addUAVObjectToWidgetRelation("OPLinkStatus", "RSSI", m_oplink->RSSI);
    addUAVObjectToWidgetRelation("OPLinkStatus", "HeapRemaining", m_oplink->FreeHeap);
    addUAVObjectToWidgetRelation("OPLinkStatus", "LinkQuality", m_oplink->LinkQuality);
    addUAVObjectToWidgetRelation("OPLinkStatus", "RXSeq", m_oplink->RXSeq);
    addUAVObjectToWidgetRelation("OPLinkStatus", "TXSeq", m_oplink->TXSeq);
    addUAVObjectToWidgetRelation("OPLinkStatus", "RXRate", m_oplink->RXRate);
    addUAVObjectToWidgetRelation("OPLinkStatus", "TXRate", m_oplink->TXRate);

    signalMapperAddBinding = new QSignalMapper(this);
    signalMapperRemBinding = new QSignalMapper(this);
    connect(m_oplink->BindingAdd_1, SIGNAL(clicked()), signalMapperAddBinding, SLOT(map()));
    signalMapperAddBinding->setMapping(m_oplink->BindingAdd_1, (QWidget *)(m_oplink->BindingID_1));
    connect(m_oplink->BindingRemove_1, SIGNAL(clicked()), signalMapperRemBinding, SLOT(map()));
    signalMapperRemBinding->setMapping(m_oplink->BindingRemove_1, (QWidget *)(m_oplink->BindingID_1));
    addUAVObjectToWidgetRelation("OPLinkSettings", "Bindings", m_oplink->BindingID_1, "0");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteMainPort", m_oplink->RemoteMainPort_1, "0");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteFlexiPort", m_oplink->RemoteFlexiPort_1, "0");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteVCPPort", m_oplink->RemoteVCPPort_1, "0");
    addUAVObjectToWidgetRelation("OPLinkSettings", "ComSpeed", m_oplink->ComSpeed_1, "0");
    connect(m_oplink->BindingAdd_2, SIGNAL(clicked()), signalMapperAddBinding, SLOT(map()));
    signalMapperAddBinding->setMapping(m_oplink->BindingAdd_2, (QWidget *)(m_oplink->BindingID_2));
    connect(m_oplink->BindingRemove_2, SIGNAL(clicked()), signalMapperRemBinding, SLOT(map()));
    signalMapperRemBinding->setMapping(m_oplink->BindingRemove_2, (QWidget *)(m_oplink->BindingID_2));
    addUAVObjectToWidgetRelation("OPLinkSettings", "Bindings", m_oplink->BindingID_2, "1");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteMainPort", m_oplink->RemoteMainPort_2, "1");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteFlexiPort", m_oplink->RemoteFlexiPort_2, "1");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteVCPPort", m_oplink->RemoteVCPPort_2, "1");
    addUAVObjectToWidgetRelation("OPLinkSettings", "ComSpeed", m_oplink->ComSpeed_2, "1");
    connect(m_oplink->BindingAdd_3, SIGNAL(clicked()), signalMapperAddBinding, SLOT(map()));
    signalMapperAddBinding->setMapping(m_oplink->BindingAdd_3, (QWidget *)(m_oplink->BindingID_3));
    connect(m_oplink->BindingRemove_3, SIGNAL(clicked()), signalMapperRemBinding, SLOT(map()));
    signalMapperRemBinding->setMapping(m_oplink->BindingRemove_3, (QWidget *)(m_oplink->BindingID_3));
    addUAVObjectToWidgetRelation("OPLinkSettings", "Bindings", m_oplink->BindingID_3, "2");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteMainPort", m_oplink->RemoteMainPort_3, "2");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteFlexiPort", m_oplink->RemoteFlexiPort_3, "2");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteVCPPort", m_oplink->RemoteVCPPort_3, "2");
    addUAVObjectToWidgetRelation("OPLinkSettings", "ComSpeed", m_oplink->ComSpeed_3, "2");
    connect(m_oplink->BindingAdd_4, SIGNAL(clicked()), signalMapperAddBinding, SLOT(map()));
    signalMapperAddBinding->setMapping(m_oplink->BindingAdd_4, (QWidget *)(m_oplink->BindingID_4));
    connect(m_oplink->BindingRemove_4, SIGNAL(clicked()), signalMapperRemBinding, SLOT(map()));
    signalMapperRemBinding->setMapping(m_oplink->BindingRemove_4, (QWidget *)(m_oplink->BindingID_4));
    addUAVObjectToWidgetRelation("OPLinkSettings", "Bindings", m_oplink->BindingID_4, "3");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteMainPort", m_oplink->RemoteMainPort_4, "3");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteFlexiPort", m_oplink->RemoteFlexiPort_4, "3");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteVCPPort", m_oplink->RemoteVCPPort_4, "3");
    addUAVObjectToWidgetRelation("OPLinkSettings", "ComSpeed", m_oplink->ComSpeed_4, "3");
    connect(m_oplink->BindingAdd_5, SIGNAL(clicked()), signalMapperAddBinding, SLOT(map()));
    signalMapperAddBinding->setMapping(m_oplink->BindingAdd_5, (QWidget *)(m_oplink->BindingID_5));
    connect(m_oplink->BindingRemove_5, SIGNAL(clicked()), signalMapperRemBinding, SLOT(map()));
    signalMapperRemBinding->setMapping(m_oplink->BindingRemove_5, (QWidget *)(m_oplink->BindingID_5));
    addUAVObjectToWidgetRelation("OPLinkSettings", "Bindings", m_oplink->BindingID_5, "4");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteMainPort", m_oplink->RemoteMainPort_5, "4");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteFlexiPort", m_oplink->RemoteFlexiPort_5, "4");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteVCPPort", m_oplink->RemoteVCPPort_5, "4");
    addUAVObjectToWidgetRelation("OPLinkSettings", "ComSpeed", m_oplink->ComSpeed_5, "4");
    connect(m_oplink->BindingAdd_6, SIGNAL(clicked()), signalMapperAddBinding, SLOT(map()));
    signalMapperAddBinding->setMapping(m_oplink->BindingAdd_6, (QWidget *)(m_oplink->BindingID_6));
    connect(m_oplink->BindingRemove_6, SIGNAL(clicked()), signalMapperRemBinding, SLOT(map()));
    signalMapperRemBinding->setMapping(m_oplink->BindingRemove_6, (QWidget *)(m_oplink->BindingID_6));
    addUAVObjectToWidgetRelation("OPLinkSettings", "Bindings", m_oplink->BindingID_6, "5");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteMainPort", m_oplink->RemoteMainPort_6, "5");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteFlexiPort", m_oplink->RemoteFlexiPort_6, "5");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteVCPPort", m_oplink->RemoteVCPPort_6, "5");
    addUAVObjectToWidgetRelation("OPLinkSettings", "ComSpeed", m_oplink->ComSpeed_6, "5");
    connect(m_oplink->BindingAdd_7, SIGNAL(clicked()), signalMapperAddBinding, SLOT(map()));
    signalMapperAddBinding->setMapping(m_oplink->BindingAdd_7, (QWidget *)(m_oplink->BindingID_7));
    connect(m_oplink->BindingRemove_7, SIGNAL(clicked()), signalMapperRemBinding, SLOT(map()));
    signalMapperRemBinding->setMapping(m_oplink->BindingRemove_7, (QWidget *)(m_oplink->BindingID_7));
    addUAVObjectToWidgetRelation("OPLinkSettings", "Bindings", m_oplink->BindingID_7, "6");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteMainPort", m_oplink->RemoteMainPort_7, "6");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteFlexiPort", m_oplink->RemoteFlexiPort_7, "6");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteVCPPort", m_oplink->RemoteVCPPort_7, "6");
    addUAVObjectToWidgetRelation("OPLinkSettings", "ComSpeed", m_oplink->ComSpeed_7, "6");
    connect(m_oplink->BindingAdd_8, SIGNAL(clicked()), signalMapperAddBinding, SLOT(map()));
    signalMapperAddBinding->setMapping(m_oplink->BindingAdd_8, (QWidget *)(m_oplink->BindingID_8));
    connect(m_oplink->BindingRemove_8, SIGNAL(clicked()), signalMapperRemBinding, SLOT(map()));
    signalMapperRemBinding->setMapping(m_oplink->BindingRemove_8, (QWidget *)(m_oplink->BindingID_8));
    addUAVObjectToWidgetRelation("OPLinkSettings", "Bindings", m_oplink->BindingID_8, "7");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteMainPort", m_oplink->RemoteMainPort_8, "7");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteFlexiPort", m_oplink->RemoteFlexiPort_8, "7");
    addUAVObjectToWidgetRelation("OPLinkSettings", "RemoteVCPPort", m_oplink->RemoteVCPPort_8, "7");
    addUAVObjectToWidgetRelation("OPLinkSettings", "ComSpeed", m_oplink->ComSpeed_8, "7");
    connect(signalMapperAddBinding, SIGNAL(mapped(QWidget *)), this, SLOT(addBinding(QWidget *)));
    connect(signalMapperRemBinding, SIGNAL(mapped(QWidget *)), this, SLOT(removeBinding(QWidget *)));

    // Add scroll bar when necessary
    QScrollArea *scroll = new QScrollArea;
    scroll->setWidget(m_oplink->frame_3);
    scroll->setWidgetResizable(true);
    m_oplink->verticalLayout_3->addWidget(scroll);

    // Request and update of the setting object.
    settingsUpdated = false;

    disableMouseWheelEvents();
}

ConfigPipXtremeWidget::~ConfigPipXtremeWidget()
{
    delete signalMapperAddBinding;
    delete signalMapperRemBinding;
}

/*!
   \brief Called by updates to @OPLinkStatus
 */
void ConfigPipXtremeWidget::updateStatus(UAVObject *object)
{
    // Request and update of the setting object if we haven't received it yet.
    if (!settingsUpdated) {
        oplinkSettingsObj->requestUpdate();
    }

    // Update the detected devices.
    UAVObjectField *pairIdField = object->getField("PairIDs");
    if (pairIdField) {
        quint32 pairid1 = pairIdField->getValue(0).toUInt();
        m_oplink->PairID1->setText(QString::number(pairid1, 16).toUpper());
        m_oplink->PairID1->setEnabled(false);
        m_oplink->PairSelect1->setEnabled(pairid1);
        quint32 pairid2 = pairIdField->getValue(1).toUInt();
        m_oplink->PairID2->setText(QString::number(pairIdField->getValue(1).toUInt(), 16).toUpper());
        m_oplink->PairID2->setEnabled(false);
        m_oplink->PairSelect2->setEnabled(pairid2);
        quint32 pairid3 = pairIdField->getValue(2).toUInt();
        m_oplink->PairID3->setText(QString::number(pairIdField->getValue(2).toUInt(), 16).toUpper());
        m_oplink->PairID3->setEnabled(false);
        m_oplink->PairSelect3->setEnabled(pairid3);
        quint32 pairid4 = pairIdField->getValue(3).toUInt();
        m_oplink->PairID4->setText(QString::number(pairIdField->getValue(3).toUInt(), 16).toUpper());
        m_oplink->PairID4->setEnabled(false);
        m_oplink->PairSelect4->setEnabled(pairid4);
    } else {
        qDebug() << "PipXtremeGadgetWidget: Count not read PairID field.";
    }
    UAVObjectField *pairRssiField = object->getField("PairSignalStrengths");
    if (pairRssiField) {
        m_oplink->PairSignalStrengthBar1->setValue(pairRssiField->getValue(0).toInt());
        m_oplink->PairSignalStrengthBar2->setValue(pairRssiField->getValue(1).toInt());
        m_oplink->PairSignalStrengthBar3->setValue(pairRssiField->getValue(2).toInt());
        m_oplink->PairSignalStrengthBar4->setValue(pairRssiField->getValue(3).toInt());
        m_oplink->PairSignalStrengthLabel1->setText(QString("%1dB").arg(pairRssiField->getValue(0).toInt()));
        m_oplink->PairSignalStrengthLabel2->setText(QString("%1dB").arg(pairRssiField->getValue(1).toInt()));
        m_oplink->PairSignalStrengthLabel3->setText(QString("%1dB").arg(pairRssiField->getValue(2).toInt()));
        m_oplink->PairSignalStrengthLabel4->setText(QString("%1dB").arg(pairRssiField->getValue(3).toInt()));
    } else {
        qDebug() << "PipXtremeGadgetWidget: Count not read PairID field.";
    }

    // Update the Description field
    UAVObjectField *descField = object->getField("Description");
    if (descField) {
        /*
         * This looks like a binary with a description at the end
         *  4 bytes: header: "OpFw"
         *  4 bytes: git commit hash (short version of SHA1)
         *  4 bytes: Unix timestamp of last git commit
         *  2 bytes: target platform. Should follow same rule as BOARD_TYPE and BOARD_REVISION in board define files.
         *  26 bytes: commit tag if it is there, otherwise "Unreleased". Zero-padded
         *   ---- 40 bytes limit ---
         *  20 bytes: SHA1 sum of the firmware.
         *  40 bytes: free for now.
         */
        char buf[OPLinkStatus::DESCRIPTION_NUMELEM];
        for (unsigned int i = 0; i < 26; ++i) {
            buf[i] = descField->getValue(i + 14).toChar().toAscii();
        }
        buf[26] = '\0';
        QString descstr(buf);
        quint32 gitDate = descField->getValue(11).toChar().toAscii() & 0xFF;
        for (int i = 1; i < 4; i++) {
            gitDate  = gitDate << 8;
            gitDate += descField->getValue(11 - i).toChar().toAscii() & 0xFF;
        }
        QString date = QDateTime::fromTime_t(gitDate).toUTC().toString("yyyy-MM-dd HH:mm");
        m_oplink->FirmwareVersion->setText(descstr + " " + date);
    } else {
        qDebug() << "PipXtremeGadgetWidget: Count not read Description field.";
    }

    // Update the serial number field
    UAVObjectField *serialField = object->getField("CPUSerial");
    if (serialField) {
        char buf[OPLinkStatus::CPUSERIAL_NUMELEM * 2 + 1];
        for (unsigned int i = 0; i < OPLinkStatus::CPUSERIAL_NUMELEM; ++i) {
            unsigned char val = serialField->getValue(i).toUInt() >> 4;
            buf[i * 2]     = ((val < 10) ? '0' : '7') + val;
            val = serialField->getValue(i).toUInt() & 0xf;
            buf[i * 2 + 1] = ((val < 10) ? '0' : '7') + val;
        }
        buf[OPLinkStatus::CPUSERIAL_NUMELEM * 2] = '\0';
        m_oplink->SerialNumber->setText(buf);
    } else {
        qDebug() << "PipXtremeGadgetWidget: Count not read Description field.";
    }

    // Update the link state
    UAVObjectField *linkField = object->getField("LinkState");
    if (linkField) {
        m_oplink->LinkState->setText(linkField->getValue().toString());
    } else {
        qDebug() << "PipXtremeGadgetWidget: Count not read link state field.";
    }
}

/*!
   \brief Called by updates to @OPLinkSettings
 */
void ConfigPipXtremeWidget::updateSettings(UAVObject *object)
{
    Q_UNUSED(object);

    if (!settingsUpdated) {
        settingsUpdated = true;
        enableControls(true);
    }
}

void ConfigPipXtremeWidget::disconnected()
{
    if (settingsUpdated) {
        settingsUpdated = false;
        enableControls(false);
    }
}

void ConfigPipXtremeWidget::addBinding(QWidget *w)
{
    if (QLineEdit * le = qobject_cast<QLineEdit *>(w)) {
        // Get the pair ID out of the selection widget
        quint32 pairID = 0;
        bool okay;
        if (m_oplink->PairSelect1->isChecked()) {
            pairID = m_oplink->PairID1->text().toUInt(&okay, 16);
        } else if (m_oplink->PairSelect2->isChecked()) {
            pairID = m_oplink->PairID2->text().toUInt(&okay, 16);
        } else if (m_oplink->PairSelect3->isChecked()) {
            pairID = m_oplink->PairID3->text().toUInt(&okay, 16);
        } else if (m_oplink->PairSelect4->isChecked()) {
            pairID = m_oplink->PairID4->text().toUInt(&okay, 16);
        }

        // Store the ID in the first open slot (or the last slot if all are full).
        le->setText(QString::number(pairID, 16).toUpper());
    }
}

void ConfigPipXtremeWidget::removeBinding(QWidget *w)
{
    if (QLineEdit * le = qobject_cast<QLineEdit *>(w)) {
        le->setText(QString::number(0, 16).toUpper());
    }
}

/**
   @}
   @}
 */
