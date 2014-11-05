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

#include "configoplinkwidget.h"

#include <coreplugin/generalsettings.h>
#include <oplinksettings.h>
#include <oplinkstatus.h>
#include <QMessageBox>

ConfigOPLinkWidget::ConfigOPLinkWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
    m_oplink = new Ui_OPLinkWidget();
    m_oplink->setupUi(this);

    // Connect to the OPLinkStatus object updates
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    oplinkStatusObj = dynamic_cast<UAVDataObject *>(objManager->getObject("OPLinkStatus"));
    Q_ASSERT(oplinkStatusObj);
    connect(oplinkStatusObj, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(updateStatus(UAVObject *)));

    // Connect to the OPLinkSettings object updates
    oplinkSettingsObj = dynamic_cast<OPLinkSettings *>(objManager->getObject("OPLinkSettings"));
    Q_ASSERT(oplinkSettingsObj);
    connect(oplinkSettingsObj, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(updateSettings(UAVObject *)));

    Core::Internal::GeneralSettings *settings = pm->getObject<Core::Internal::GeneralSettings>();
    if (!settings->useExpertMode()) {
        m_oplink->Apply->setVisible(false);
    }
    addApplySaveButtons(m_oplink->Apply, m_oplink->Save);

    addWidgetBinding("OPLinkSettings", "MainPort", m_oplink->MainPort);
    addWidgetBinding("OPLinkSettings", "FlexiPort", m_oplink->FlexiPort);
    addWidgetBinding("OPLinkSettings", "VCPPort", m_oplink->VCPPort);
    addWidgetBinding("OPLinkSettings", "MaxRFPower", m_oplink->MaxRFTxPower);
    addWidgetBinding("OPLinkSettings", "MinChannel", m_oplink->MinimumChannel);
    addWidgetBinding("OPLinkSettings", "MaxChannel", m_oplink->MaximumChannel);
    addWidgetBinding("OPLinkSettings", "ChannelSet", m_oplink->ChannelSet);
    addWidgetBinding("OPLinkSettings", "CoordID", m_oplink->CoordID);
    addWidgetBinding("OPLinkSettings", "Coordinator", m_oplink->Coordinator);
    addWidgetBinding("OPLinkSettings", "OneWay", m_oplink->OneWayLink);
    addWidgetBinding("OPLinkSettings", "PPMOnly", m_oplink->PPMOnly);
    addWidgetBinding("OPLinkSettings", "PPM", m_oplink->PPM);
    addWidgetBinding("OPLinkSettings", "ComSpeed", m_oplink->ComSpeed);

    addWidgetBinding("OPLinkStatus", "DeviceID", m_oplink->DeviceID);
    addWidgetBinding("OPLinkStatus", "RxGood", m_oplink->Good);
    addWidgetBinding("OPLinkStatus", "RxCorrected", m_oplink->Corrected);
    addWidgetBinding("OPLinkStatus", "RxErrors", m_oplink->Errors);
    addWidgetBinding("OPLinkStatus", "RxMissed", m_oplink->Missed);
    addWidgetBinding("OPLinkStatus", "RxFailure", m_oplink->RxFailure);
    addWidgetBinding("OPLinkStatus", "UAVTalkErrors", m_oplink->UAVTalkErrors);
    addWidgetBinding("OPLinkStatus", "TxDropped", m_oplink->Dropped);
    addWidgetBinding("OPLinkStatus", "TxFailure", m_oplink->TxFailure);
    addWidgetBinding("OPLinkStatus", "Resets", m_oplink->Resets);
    addWidgetBinding("OPLinkStatus", "Timeouts", m_oplink->Timeouts);
    addWidgetBinding("OPLinkStatus", "RSSI", m_oplink->RSSI);
    addWidgetBinding("OPLinkStatus", "HeapRemaining", m_oplink->FreeHeap);
    addWidgetBinding("OPLinkStatus", "LinkQuality", m_oplink->LinkQuality);
    addWidgetBinding("OPLinkStatus", "RXSeq", m_oplink->RXSeq);
    addWidgetBinding("OPLinkStatus", "TXSeq", m_oplink->TXSeq);
    addWidgetBinding("OPLinkStatus", "RXRate", m_oplink->RXRate);
    addWidgetBinding("OPLinkStatus", "TXRate", m_oplink->TXRate);

    // Connect the bind buttons
    connect(m_oplink->Bind1, SIGNAL(clicked()), this, SLOT(bind()));
    connect(m_oplink->Bind2, SIGNAL(clicked()), this, SLOT(bind()));
    connect(m_oplink->Bind3, SIGNAL(clicked()), this, SLOT(bind()));
    connect(m_oplink->Bind4, SIGNAL(clicked()), this, SLOT(bind()));

    // Connect the selection changed signals.
    connect(m_oplink->PPMOnly, SIGNAL(toggled(bool)), this, SLOT(ppmOnlyChanged()));

    // Request and update of the setting object.
    settingsUpdated = false;
    autoLoadWidgets();
    disableMouseWheelEvents();
    updateEnableControls();
}

ConfigOPLinkWidget::~ConfigOPLinkWidget()
{}

/*!
   \brief Called by updates to @OPLinkStatus
 */
void ConfigOPLinkWidget::updateStatus(UAVObject *object)
{
    // Request and update of the setting object if we haven't received it yet.
    if (!settingsUpdated) {
        oplinkSettingsObj->requestUpdate();
    }

    // Update the link state
    UAVObjectField *linkField = object->getField("LinkState");
    m_oplink->LinkState->setText(linkField->getValue().toString());
    bool linkConnected = (linkField->getValue() == linkField->getOptions().at(OPLinkStatus::LINKSTATE_CONNECTED));
    bool modemEnabled  = linkConnected || (linkField->getValue() == linkField->getOptions().at(OPLinkStatus::LINKSTATE_DISCONNECTED)) ||
                         (linkField->getValue() == linkField->getOptions().at(OPLinkStatus::LINKSTATE_ENABLED));

    UAVObjectField *pairRssiField = object->getField("PairSignalStrengths");

    bool bound;
    bool ok;
    quint32 boundPairId = m_oplink->CoordID->text().toUInt(&ok, 16);

    // Update the detected devices.
    UAVObjectField *pairIdField = object->getField("PairIDs");
    quint32 pairid = pairIdField->getValue(0).toUInt();
    bound = (pairid == boundPairId);
    m_oplink->PairID1->setText(QString::number(pairid, 16).toUpper());
    m_oplink->PairID1->setEnabled(false);
    m_oplink->Bind1->setText(bound ? tr("Unbind") : tr("Bind"));
    m_oplink->Bind1->setEnabled(pairid && modemEnabled);
    m_oplink->PairSignalStrengthBar1->setValue(((bound && !linkConnected) || !modemEnabled) ? -127 : pairRssiField->getValue(0).toInt());
    m_oplink->PairSignalStrengthLabel1->setText(QString("%1dB").arg(m_oplink->PairSignalStrengthBar1->value()));

    pairid = pairIdField->getValue(1).toUInt();
    bound  = (pairid == boundPairId);
    m_oplink->PairID2->setText(QString::number(pairid, 16).toUpper());
    m_oplink->PairID2->setEnabled(false);
    m_oplink->Bind2->setText(bound ? tr("Unbind") : tr("Bind"));
    m_oplink->Bind2->setEnabled(pairid && modemEnabled);
    m_oplink->PairSignalStrengthBar2->setValue(((bound && !linkConnected) || !modemEnabled) ? -127 : pairRssiField->getValue(1).toInt());
    m_oplink->PairSignalStrengthLabel2->setText(QString("%1dB").arg(m_oplink->PairSignalStrengthBar2->value()));

    pairid = pairIdField->getValue(2).toUInt();
    bound  = (pairid == boundPairId);
    m_oplink->PairID3->setText(QString::number(pairid, 16).toUpper());
    m_oplink->PairID3->setEnabled(false);
    m_oplink->Bind3->setText(bound ? tr("Unbind") : tr("Bind"));
    m_oplink->Bind3->setEnabled(pairid && modemEnabled);
    m_oplink->PairSignalStrengthBar3->setValue(((bound && !linkConnected) || !modemEnabled) ? -127 : pairRssiField->getValue(2).toInt());
    m_oplink->PairSignalStrengthLabel3->setText(QString("%1dB").arg(m_oplink->PairSignalStrengthBar3->value()));

    pairid = pairIdField->getValue(3).toUInt();
    bound  = (pairid == boundPairId);
    m_oplink->PairID4->setText(QString::number(pairid, 16).toUpper());
    m_oplink->PairID4->setEnabled(false);
    m_oplink->Bind4->setText(bound ? tr("Unbind") : tr("Bind"));
    m_oplink->Bind4->setEnabled(pairid && modemEnabled);
    m_oplink->PairSignalStrengthBar4->setValue(((bound && !linkConnected) || !modemEnabled) ? -127 : pairRssiField->getValue(3).toInt());
    m_oplink->PairSignalStrengthLabel4->setText(QString("%1dB").arg(m_oplink->PairSignalStrengthBar4->value()));

    // Update the Description field
    // TODO use  UAVObjectUtilManager::descriptionToStructure()
    UAVObjectField *descField = object->getField("Description");
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

    // Update the serial number field
    UAVObjectField *serialField = object->getField("CPUSerial");
    char buf[OPLinkStatus::CPUSERIAL_NUMELEM * 2 + 1];
    for (unsigned int i = 0; i < OPLinkStatus::CPUSERIAL_NUMELEM; ++i) {
        unsigned char val = serialField->getValue(i).toUInt() >> 4;
        buf[i * 2]     = ((val < 10) ? '0' : '7') + val;
        val = serialField->getValue(i).toUInt() & 0xf;
        buf[i * 2 + 1] = ((val < 10) ? '0' : '7') + val;
    }
    buf[OPLinkStatus::CPUSERIAL_NUMELEM * 2] = '\0';
    m_oplink->SerialNumber->setText(buf);

    updateEnableControls();
}

/*!
   \brief Called by updates to @OPLinkSettings
 */
void ConfigOPLinkWidget::updateSettings(UAVObject *object)
{
    Q_UNUSED(object);

    if (!settingsUpdated) {
        settingsUpdated = true;

        // Enable components based on the board type connected.
        UAVObjectField *board_type_field = oplinkStatusObj->getField("BoardType");
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
        case 0x0A: // OPLink?
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
        updateEnableControls();
    }
}

void ConfigOPLinkWidget::updateEnableControls()
{
    enableControls(true);
    ppmOnlyChanged();
}

void ConfigOPLinkWidget::disconnected()
{
    if (settingsUpdated) {
        settingsUpdated = false;
    }
}

void ConfigOPLinkWidget::bind()
{
    QPushButton *bindButton = qobject_cast<QPushButton *>(sender());

    if (bindButton) {
        QLineEdit *editField = NULL;
        if (bindButton == m_oplink->Bind1) {
            editField = m_oplink->PairID1;
        } else if (bindButton == m_oplink->Bind2) {
            editField = m_oplink->PairID2;
        } else if (bindButton == m_oplink->Bind3) {
            editField = m_oplink->PairID3;
        } else if (bindButton == m_oplink->Bind4) {
            editField = m_oplink->PairID4;
        }
        Q_ASSERT(editField);
        bool ok;
        quint32 pairid = editField->text().toUInt(&ok, 16);
        if (ok) {
            quint32 boundPairId = m_oplink->CoordID->text().toUInt(&ok, 16);
            (pairid != boundPairId) ? m_oplink->CoordID->setText(QString::number(pairid, 16).toUpper()) : m_oplink->CoordID->setText("0");
        }
        QMessageBox::information(this, tr("Information"), tr("To apply the changes when binding/unbinding the board must be rebooted or power cycled."), QMessageBox::Ok);
    }
}

void ConfigOPLinkWidget::ppmOnlyChanged()
{
    bool is_ppm_only = m_oplink->PPMOnly->isChecked();

    m_oplink->PPM->setEnabled(!is_ppm_only);
    m_oplink->OneWayLink->setEnabled(!is_ppm_only);
    m_oplink->ComSpeed->setEnabled(!is_ppm_only);
}

/**
   @}
   @}
 */
