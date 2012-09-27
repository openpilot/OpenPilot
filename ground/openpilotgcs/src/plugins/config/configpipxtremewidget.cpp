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

#include <pipxsettings.h>
#include <pipxstatus.h>

ConfigPipXtremeWidget::ConfigPipXtremeWidget(QWidget *parent) : ConfigTaskWidget(parent)
{
	m_pipx = new Ui_PipXtremeWidget();
	m_pipx->setupUi(this);

	// Connect to the PipXStatus object updates
	ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
	UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
	pipxStatusObj = dynamic_cast<UAVDataObject*>(objManager->getObject("PipXStatus"));
	if (pipxStatusObj != NULL ) {
		connect(pipxStatusObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updateStatus(UAVObject*)));
	} else {
		qDebug() << "Error: Object is unknown (PipXStatus).";
	}

	// Connect to the PipXSettings object updates
	pipxSettingsObj = dynamic_cast<UAVDataObject*>(objManager->getObject("PipXSettings"));
	if (pipxSettingsObj != NULL ) {
		connect(pipxSettingsObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updateSettings(UAVObject*)));
	} else {
		qDebug() << "Error: Object is unknown (PipXSettings).";
	}
    autoLoadWidgets();
	addApplySaveButtons(m_pipx->Apply, m_pipx->Save);

	addUAVObjectToWidgetRelation("PipXSettings", "TelemetryConfig", m_pipx->TelemPortConfig);
	addUAVObjectToWidgetRelation("PipXSettings", "TelemetrySpeed", m_pipx->TelemPortSpeed);
	addUAVObjectToWidgetRelation("PipXSettings", "FlexiConfig", m_pipx->FlexiPortConfig);
	addUAVObjectToWidgetRelation("PipXSettings", "FlexiSpeed", m_pipx->FlexiPortSpeed);
	addUAVObjectToWidgetRelation("PipXSettings", "VCPConfig", m_pipx->VCPConfig);
	addUAVObjectToWidgetRelation("PipXSettings", "VCPSpeed", m_pipx->VCPSpeed);
	addUAVObjectToWidgetRelation("PipXSettings", "RFSpeed", m_pipx->MaxRFDatarate);
	addUAVObjectToWidgetRelation("PipXSettings", "MaxRFPower", m_pipx->MaxRFTxPower);
	addUAVObjectToWidgetRelation("PipXSettings", "SendTimeout", m_pipx->SendTimeout);
	addUAVObjectToWidgetRelation("PipXSettings", "MinPacketSize", m_pipx->MinPacketSize);
	addUAVObjectToWidgetRelation("PipXSettings", "FrequencyCalibration", m_pipx->FrequencyCalibration);
	addUAVObjectToWidgetRelation("PipXSettings", "Frequency", m_pipx->Frequency);

	addUAVObjectToWidgetRelation("PipXStatus", "MinFrequency", m_pipx->MinFrequency);
	addUAVObjectToWidgetRelation("PipXStatus", "MaxFrequency", m_pipx->MaxFrequency);
	addUAVObjectToWidgetRelation("PipXStatus", "FrequencyStepSize", m_pipx->FrequencyStepSize);
	addUAVObjectToWidgetRelation("PipXStatus", "FrequencyBand", m_pipx->FreqBand);
	addUAVObjectToWidgetRelation("PipXStatus", "RSSI", m_pipx->RSSI);
	addUAVObjectToWidgetRelation("PipXStatus", "AFC", m_pipx->RxAFC);
	addUAVObjectToWidgetRelation("PipXStatus", "Retries", m_pipx->Retries);
	addUAVObjectToWidgetRelation("PipXStatus", "Errors", m_pipx->Errors);
	addUAVObjectToWidgetRelation("PipXStatus", "UAVTalkErrors", m_pipx->UAVTalkErrors);
	addUAVObjectToWidgetRelation("PipXStatus", "Resets", m_pipx->Resets);
	addUAVObjectToWidgetRelation("PipXStatus", "Dropped", m_pipx->Dropped);
	addUAVObjectToWidgetRelation("PipXStatus", "RXRate", m_pipx->RXRate);
	addUAVObjectToWidgetRelation("PipXStatus", "TXRate", m_pipx->TXRate);

	// Connect to the pair ID radio buttons.
	connect(m_pipx->PairSelectB, SIGNAL(toggled(bool)), this, SLOT(pairBToggled(bool)));
	connect(m_pipx->PairSelect1, SIGNAL(toggled(bool)), this, SLOT(pair1Toggled(bool)));
	connect(m_pipx->PairSelect2, SIGNAL(toggled(bool)), this, SLOT(pair2Toggled(bool)));
	connect(m_pipx->PairSelect3, SIGNAL(toggled(bool)), this, SLOT(pair3Toggled(bool)));
	connect(m_pipx->PairSelect4, SIGNAL(toggled(bool)), this, SLOT(pair4Toggled(bool)));

	//Add scroll bar when necessary
	QScrollArea *scroll = new QScrollArea;
	scroll->setWidget(m_pipx->frame_3);
	scroll->setWidgetResizable(true);
	m_pipx->verticalLayout_3->addWidget(scroll);

	// Request and update of the setting object.
	settingsUpdated = false;
	//pipxSettingsObj->requestUpdate();

	disableMouseWheelEvents();
}

ConfigPipXtremeWidget::~ConfigPipXtremeWidget()
{
	// Do nothing
}

void ConfigPipXtremeWidget::refreshValues()
{
}

void ConfigPipXtremeWidget::applySettings()
{
	PipXSettings *pipxSettings = PipXSettings::GetInstance(getObjectManager());
	PipXSettings::DataFields pipxSettingsData = pipxSettings->getData();

	// Get the pair ID.
	quint32 pairID = 0;
	bool okay;
	if (m_pipx->PairSelect1->isChecked())
		pairID = m_pipx->PairID1->text().toUInt(&okay, 16);
	else if (m_pipx->PairSelect2->isChecked())
		pairID = m_pipx->PairID2->text().toUInt(&okay, 16);
	else if (m_pipx->PairSelect3->isChecked())
		pairID = m_pipx->PairID3->text().toUInt(&okay, 16);
	else if (m_pipx->PairSelect4->isChecked())
		pairID = m_pipx->PairID4->text().toUInt(&okay, 16);
	pipxSettingsData.PairID = pairID;
	pipxSettings->setData(pipxSettingsData);
}

void ConfigPipXtremeWidget::saveSettings()
{
	//applySettings();
	UAVObject *obj = PipXSettings::GetInstance(getObjectManager());
	saveObjectToSD(obj);
}

/*!
  \brief Called by updates to @PipXStatus
  */
void ConfigPipXtremeWidget::updateStatus(UAVObject *object)
{

	// Request and update of the setting object if we haven't received it yet.
	if (!settingsUpdated)
		pipxSettingsObj->requestUpdate();

	// Get the current pairID
	PipXSettings *pipxSettings = PipXSettings::GetInstance(getObjectManager());
	quint32 pairID = 0;
	if (pipxSettings)
		pairID = pipxSettings->getPairID();

	// Update the detected devices.
	UAVObjectField* pairIdField = object->getField("PairIDs");
	if (pairIdField) {
		quint32 pairid1 = pairIdField->getValue(0).toUInt();
		m_pipx->PairID1->setText(QString::number(pairid1, 16).toUpper());
		m_pipx->PairID1->setEnabled(false);
		m_pipx->PairSelect1->setChecked(pairID && (pairID == pairid1));
		m_pipx->PairSelect1->setEnabled(pairid1);
		quint32 pairid2 = pairIdField->getValue(1).toUInt();
		m_pipx->PairID2->setText(QString::number(pairIdField->getValue(1).toUInt(), 16).toUpper());
		m_pipx->PairID2->setEnabled(false);
		m_pipx->PairSelect2->setChecked(pairID && (pairID == pairid2));
		m_pipx->PairSelect2->setEnabled(pairid2);
		quint32 pairid3 = pairIdField->getValue(2).toUInt();
		m_pipx->PairID3->setText(QString::number(pairIdField->getValue(2).toUInt(), 16).toUpper());
		m_pipx->PairID3->setEnabled(false);
		m_pipx->PairSelect3->setChecked(pairID && (pairID == pairid3));
		m_pipx->PairSelect3->setEnabled(pairid3);
		quint32 pairid4 = pairIdField->getValue(3).toUInt();
		m_pipx->PairID4->setText(QString::number(pairIdField->getValue(3).toUInt(), 16).toUpper());
		m_pipx->PairID4->setEnabled(false);
		m_pipx->PairSelect4->setChecked(pairID && (pairID == pairid4));
		m_pipx->PairSelect4->setEnabled(pairid4);
	} else {
		qDebug() << "PipXtremeGadgetWidget: Count not read PairID field.";
	}
	UAVObjectField* pairRssiField = object->getField("PairSignalStrengths");
	if (pairRssiField) {
        m_pipx->PairSignalStrengthBar1->setValue(pairRssiField->getValue(0).toInt());
        m_pipx->PairSignalStrengthBar2->setValue(pairRssiField->getValue(1).toInt());
        m_pipx->PairSignalStrengthBar3->setValue(pairRssiField->getValue(2).toInt());
        m_pipx->PairSignalStrengthBar4->setValue(pairRssiField->getValue(3).toInt());
        m_pipx->PairSignalStrengthLabel1->setText(QString("%1dB").arg(pairRssiField->getValue(0).toInt()));
        m_pipx->PairSignalStrengthLabel2->setText(QString("%1dB").arg(pairRssiField->getValue(1).toInt()));
        m_pipx->PairSignalStrengthLabel3->setText(QString("%1dB").arg(pairRssiField->getValue(2).toInt()));
        m_pipx->PairSignalStrengthLabel4->setText(QString("%1dB").arg(pairRssiField->getValue(3).toInt()));
    } else {
		qDebug() << "PipXtremeGadgetWidget: Count not read PairID field.";
	}

	// Update the Description field
	UAVObjectField* descField = object->getField("Description");
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
		char buf[PipXStatus::DESCRIPTION_NUMELEM];
		for (unsigned int i = 0; i < 26; ++i)
			buf[i] = descField->getValue(i + 14).toChar().toAscii();
		buf[26] = '\0';
		QString descstr(buf);
		quint32 gitDate = descField->getValue(11).toChar().toAscii() & 0xFF;
		for (int i = 1; i < 4; i++) {
			gitDate = gitDate << 8;
			gitDate += descField->getValue(11-i).toChar().toAscii() & 0xFF;
		}
		QString date = QDateTime::fromTime_t(gitDate).toUTC().toString("yyyy-MM-dd HH:mm");
		m_pipx->FirmwareVersion->setText(descstr + " " + date);
 	} else {
 		qDebug() << "PipXtremeGadgetWidget: Count not read Description field.";
 	}

	// Update the serial number field
	UAVObjectField* serialField = object->getField("CPUSerial");
	if (serialField) {
		char buf[PipXStatus::CPUSERIAL_NUMELEM * 2 + 1];
		for (unsigned int i = 0; i < PipXStatus::CPUSERIAL_NUMELEM; ++i)
		{
			unsigned char val = serialField->getValue(i).toUInt() >> 4;
			buf[i * 2] = ((val < 10) ? '0' : '7') + val;
			val = serialField->getValue(i).toUInt() & 0xf;
			buf[i * 2 + 1] = ((val < 10) ? '0' : '7') + val;
		}
		buf[PipXStatus::CPUSERIAL_NUMELEM * 2] = '\0';
		m_pipx->SerialNumber->setText(buf);
 	} else {
 		qDebug() << "PipXtremeGadgetWidget: Count not read Description field.";
 	}

	// Update the DeviceID field
	UAVObjectField* idField = object->getField("DeviceID");
	if (idField) {
		m_pipx->DeviceID->setText(QString::number(idField->getValue().toUInt(), 16).toUpper());
	} else {
		qDebug() << "PipXtremeGadgetWidget: Count not read DeviceID field.";
	}

	// Update the PairID field
	m_pipx->PairID->setText(QString::number(pairID, 16).toUpper());

 	// Update the link state
 	UAVObjectField* linkField = object->getField("LinkState");
 	if (linkField) {
		m_pipx->LinkState->setText(linkField->getValue().toString());
	} else {
		qDebug() << "PipXtremeGadgetWidget: Count not read link state field.";
	}
}

/*!
  \brief Called by updates to @PipXSettings
  */
void ConfigPipXtremeWidget::updateSettings(UAVObject *object)
{
    Q_UNUSED(object);

    if (!settingsUpdated)
	{
		settingsUpdated = true;
		enableControls(true);
	}
}

void ConfigPipXtremeWidget::disconnected()
{
	if (settingsUpdated)
	{
		settingsUpdated = false;
		enableControls(false);
	}
}

void ConfigPipXtremeWidget::pairIDToggled(bool checked, quint8 idx)
{
	if(checked)
	{
		PipXStatus *pipxStatus = PipXStatus::GetInstance(getObjectManager());
		PipXSettings *pipxSettings = PipXSettings::GetInstance(getObjectManager());

		if (pipxStatus && pipxSettings)
		{
			if (idx == 4)
			{
				pipxSettings->setPairID(0);
			}
			else
			{
				quint32 pairID = pipxStatus->getPairIDs(idx);
				if (pairID)
					pipxSettings->setPairID(pairID);
			}
		}
	}
}

void ConfigPipXtremeWidget::pair1Toggled(bool checked)
{
	pairIDToggled(checked, 0);
}

void ConfigPipXtremeWidget::pair2Toggled(bool checked)
{
	pairIDToggled(checked, 1);
}

void ConfigPipXtremeWidget::pair3Toggled(bool checked)
{
	pairIDToggled(checked, 2);
}

void ConfigPipXtremeWidget::pair4Toggled(bool checked)
{
	pairIDToggled(checked, 3);
}

void ConfigPipXtremeWidget::pairBToggled(bool checked)
{
	pairIDToggled(checked, 4);
}

/**
   @}
   @}
*/
