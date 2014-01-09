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
    m_oplink = new Ui_OPLinkWidget();
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
    addUAVObjectToWidgetRelation("OPLinkSettings", "ComSpeed", m_oplink->ComSpeed);
    addUAVObjectToWidgetRelation("OPLinkSettings", "MaxRFPower", m_oplink->MaxRFTxPower);
    addUAVObjectToWidgetRelation("OPLinkSettings", "MinChannel", m_oplink->MinimumChannel);
    addUAVObjectToWidgetRelation("OPLinkSettings", "MaxChannel", m_oplink->MaximumChannel);
    addUAVObjectToWidgetRelation("OPLinkSettings", "ChannelSet", m_oplink->ChannelSet);
    addUAVObjectToWidgetRelation("OPLinkSettings", "CoordID", m_oplink->CoordID);
    addUAVObjectToWidgetRelation("OPLinkSettings", "Coordinator", m_oplink->Coordinator);
    addUAVObjectToWidgetRelation("OPLinkSettings", "OneWay", m_oplink->OneWayLink);
    addUAVObjectToWidgetRelation("OPLinkSettings", "PPMOnly", m_oplink->PPMOnly);
    addUAVObjectToWidgetRelation("OPLinkSettings", "PPM", m_oplink->PPM);

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

    // Connect the bind buttons
    connect(m_oplink->Bind1, SIGNAL(clicked()), this, SLOT(bind1()));
    connect(m_oplink->Bind2, SIGNAL(clicked()), this, SLOT(bind2()));
    connect(m_oplink->Bind3, SIGNAL(clicked()), this, SLOT(bind3()));
    connect(m_oplink->Bind4, SIGNAL(clicked()), this, SLOT(bind3()));

    // Connect the selection changed signals.
    connect(m_oplink->PPMOnly, SIGNAL(toggled(bool)), this, SLOT(ppmOnlyToggled(bool)));
    connect(m_oplink->ComSpeed, SIGNAL(currentIndexChanged(int)), this, SLOT(comSpeedChanged(int)));

    ppmOnlyToggled(m_oplink->PPMOnly->isChecked());

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
{}

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
        m_oplink->Bind1->setEnabled(pairid1);
        quint32 pairid2 = pairIdField->getValue(1).toUInt();
        m_oplink->PairID2->setText(QString::number(pairIdField->getValue(1).toUInt(), 16).toUpper());
        m_oplink->PairID2->setEnabled(false);
        m_oplink->Bind2->setEnabled(pairid2);
        quint32 pairid3 = pairIdField->getValue(2).toUInt();
        m_oplink->PairID3->setText(QString::number(pairIdField->getValue(2).toUInt(), 16).toUpper());
        m_oplink->PairID3->setEnabled(false);
        m_oplink->Bind3->setEnabled(pairid3);
        quint32 pairid4 = pairIdField->getValue(3).toUInt();
        m_oplink->PairID4->setText(QString::number(pairIdField->getValue(3).toUInt(), 16).toUpper());
        m_oplink->PairID4->setEnabled(false);
        m_oplink->Bind4->setEnabled(pairid4);
    } else {
        qDebug() << "ConfigPipXtremeWidget: Count not read PairID field.";
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
        qDebug() << "ConfigPipXtremeWidget: Count not read PairID field.";
    }

    // Update the Description field
    // TODO use  UAVObjectUtilManager::descriptionToStructure()
    UAVObjectField *descField = object->getField("Description");
    if (descField) {
        if (descField->getValue(0) != QChar(255)) {
            /*
             * This looks like a binary with a description at the end:
             *   4 bytes: header: "OpFw".
             *   4 bytes: GIT commit tag (short version of SHA1).
             *   4 bytes: Unix timestamp of compile time.
             *   2 bytes: target platform. Should follow same rule as BOARD_TYPE and BOARD_REVISION in board define files.
             *  26 bytes: commit tag if it is there, otherwise branch name. '-dirty' may be added if needed. Zero-padded.
             *  20 bytes: SHA1 sum of the firmware.
             *  20 bytes: SHA1 sum of the uavo definitions.
             *  20 bytes: free for now.
             */
            char buf[OPLinkStatus::DESCRIPTION_NUMELEM];
            for (unsigned int i = 0; i < 26; ++i) {
                buf[i] = descField->getValue(i + 14).toChar().toLatin1();
            }
            buf[26] = '\0';
            QString descstr(buf);
            quint32 gitDate = descField->getValue(11).toChar().toLatin1() & 0xFF;
            for (int i = 1; i < 4; i++) {
                gitDate  = gitDate << 8;
                gitDate += descField->getValue(11 - i).toChar().toLatin1() & 0xFF;
            }
            QString date = QDateTime::fromTime_t(gitDate).toUTC().toString("yyyy-MM-dd HH:mm");
            m_oplink->FirmwareVersion->setText(descstr + " " + date);
        } else {
            m_oplink->FirmwareVersion->setText(tr("Unknown"));
        }
    } else {
        qDebug() << "ConfigPipXtremeWidget: Failed to read Description field.";
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
        qDebug() << "ConfigPipXtremeWidget: Failed to read CPUSerial field.";
    }

    // Update the link state
    UAVObjectField *linkField = object->getField("LinkState");
    if (linkField) {
        m_oplink->LinkState->setText(linkField->getValue().toString());
    } else {
        qDebug() << "ConfigPipXtremeWidget: Failed to read LinkState field.";
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

        // Enable components based on the board type connected.
        UAVObjectField *board_type_field = oplinkStatusObj->getField("BoardType");
        if (board_type_field) {
            switch (board_type_field->getValue().toInt()) {
            case 0x09: // Revolution
                m_oplink->MainPort->setVisible(false);
                m_oplink->MainPortLabel->setVisible(false);
                m_oplink->FlexiPort->setVisible(false);
                m_oplink->FlexiPortLabel->setVisible(false);
                m_oplink->VCPPort->setVisible(false);
                m_oplink->VCPPortLabel->setVisible(false);
                m_oplink->FlexiIOPort->setVisible(false);
                m_oplink->FlexiIOPortLabel->setVisible(false);
                m_oplink->PPM->setVisible(true);
                break;
            case 0x03: // OPLinkMini
                m_oplink->MainPort->setVisible(true);
                m_oplink->MainPortLabel->setVisible(true);
                m_oplink->FlexiPort->setVisible(true);
                m_oplink->FlexiPortLabel->setVisible(true);
                m_oplink->VCPPort->setVisible(true);
                m_oplink->VCPPortLabel->setVisible(true);
                m_oplink->FlexiIOPort->setVisible(false);
                m_oplink->FlexiIOPortLabel->setVisible(false);
                m_oplink->PPM->setVisible(false);
                break;
            case 0x0A:
                m_oplink->MainPort->setVisible(true);
                m_oplink->MainPortLabel->setVisible(true);
                m_oplink->FlexiPort->setVisible(true);
                m_oplink->FlexiPortLabel->setVisible(true);
                m_oplink->VCPPort->setVisible(true);
                m_oplink->VCPPortLabel->setVisible(true);
                m_oplink->FlexiIOPort->setVisible(true);
                m_oplink->FlexiIOPortLabel->setVisible(true);
                m_oplink->PPM->setVisible(false);
                break;
            default:
                // This shouldn't happen.
                break;
            }
        } else {
            qDebug() << "BoardType not found.";
        }

        // Enable the push buttons.
        enableControls(true);
    }
}

void ConfigPipXtremeWidget::disconnected()
{
    if (settingsUpdated) {
        settingsUpdated = false;

        // Enable the push buttons.
        enableControls(false);
    }
}

void ConfigPipXtremeWidget::SetPairID(QLineEdit *pairIdWidget)
{
    // Get the pair ID out of the selection widget
    quint32 pairID = 0;
    bool okay;

    pairID = pairIdWidget->text().toUInt(&okay, 16);

    // Store the ID in the coord ID field.
    m_oplink->CoordID->setText(QString::number(pairID, 16).toUpper());
}

void ConfigPipXtremeWidget::bind1()
{
    SetPairID(m_oplink->PairID1);
}

void ConfigPipXtremeWidget::bind2()
{
    SetPairID(m_oplink->PairID1);
}

void ConfigPipXtremeWidget::bind3()
{
    SetPairID(m_oplink->PairID1);
}

void ConfigPipXtremeWidget::bind4()
{
    SetPairID(m_oplink->PairID1);
}

void ConfigPipXtremeWidget::ppmOnlyToggled(bool on)
{
    if (on) {
        m_oplink->PPM->setEnabled(false);
        m_oplink->OneWayLink->setEnabled(false);
        m_oplink->ComSpeed->setEnabled(false);
    } else {
        m_oplink->PPM->setEnabled(true);
        m_oplink->OneWayLink->setEnabled(true);
        m_oplink->ComSpeed->setEnabled(true);
        // Change the comspeed from 4800 of PPM only is turned off.
        if (m_oplink->ComSpeed->currentIndex() == OPLinkSettings::COMSPEED_4800) {
            m_oplink->ComSpeed->setCurrentIndex(OPLinkSettings::COMSPEED_9600);
        }
    }
}

void ConfigPipXtremeWidget::comSpeedChanged(int index)
{
    qDebug() << "comSpeedChanged: " << index;
    switch (index) {
    case OPLinkSettings::COMSPEED_4800:
        m_oplink->PPMOnly->setChecked(true);
        break;
    default:
        m_oplink->PPMOnly->setChecked(false);
        break;
    }
}

/**
   @}
   @}
 */
