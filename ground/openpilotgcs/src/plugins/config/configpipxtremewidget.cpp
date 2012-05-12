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

	addApplySaveButtons(m_pipx->Apply, m_pipx->Save);
	//connect(m_pipx->Apply, SIGNAL(clicked()), this, SLOT(applySettings()));
	//connect(m_pipx->Save, SIGNAL(clicked()), this, SLOT(saveSettings()));

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
	addUAVObjectToWidgetRelation("PipXStatus", "AFC", m_pipx->RxAFC);
	addUAVObjectToWidgetRelation("PipXStatus", "Retries", m_pipx->Retries);
	addUAVObjectToWidgetRelation("PipXStatus", "Errors", m_pipx->Errors);
	addUAVObjectToWidgetRelation("PipXStatus", "UAVTalkErrors", m_pipx->UAVTalkErrors);
	addUAVObjectToWidgetRelation("PipXStatus", "Resets", m_pipx->Resets);
	addUAVObjectToWidgetRelation("PipXStatus", "RXRate", m_pipx->RXRate);
	addUAVObjectToWidgetRelation("PipXStatus", "TXRate", m_pipx->TXRate);

	// Create the timer that is used to timeout the connection to the PipX.
	timeOut = new QTimer(this);
	connect(timeOut, SIGNAL(timeout()),this,SLOT(disconnected()));

	// Request and update of the setting object.
	settingsUpdated = false;
	pipxSettingsObj->requestUpdate();
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

	// Restart the disconnection timer.
	timeOut->start(5000);

	// Request and update of the setting object if we haven't received it yet.
	if (!settingsUpdated)
		pipxSettingsObj->requestUpdate();

	// Update the detected devices.
	UAVObjectField* pairIdField = object->getField("PairIDs");
	if (pairIdField) {
		quint32 pairid1 = pairIdField->getValue(0).toUInt();
		m_pipx->PairID1->setText(QString::number(pairid1, 16).toUpper());
		m_pipx->PairID1->setEnabled(false);
		m_pipx->PairSelect1->setChecked(pairID == pairid1);
		m_pipx->PairSelect1->setEnabled(pairid1);
		quint32 pairid2 = pairIdField->getValue(1).toUInt();
		m_pipx->PairID2->setText(QString::number(pairIdField->getValue(1).toUInt(), 16).toUpper());
		m_pipx->PairID2->setEnabled(false);
		m_pipx->PairSelect2->setChecked(pairID == pairid2);
		m_pipx->PairSelect2->setEnabled(pairid2);
		quint32 pairid3 = pairIdField->getValue(2).toUInt();
		m_pipx->PairID3->setText(QString::number(pairIdField->getValue(2).toUInt(), 16).toUpper());
		m_pipx->PairID3->setEnabled(false);
		m_pipx->PairSelect3->setChecked(pairID == pairid3);
		m_pipx->PairSelect3->setEnabled(pairid3);
		quint32 pairid4 = pairIdField->getValue(3).toUInt();
		m_pipx->PairID4->setText(QString::number(pairIdField->getValue(3).toUInt(), 16).toUpper());
		m_pipx->PairID4->setEnabled(false);
		m_pipx->PairSelect4->setChecked(pairID == pairid4);
		m_pipx->PairSelect4->setEnabled(pairid4);
	} else {
		qDebug() << "PipXtremeGadgetWidget: Count not read PairID field.";
	}
	UAVObjectField* pairRssiField = object->getField("PairSignalStrengths");
	if (pairRssiField) {
		m_pipx->PairSignalStrength1->setValue(pairRssiField->getValue(0).toInt());
		m_pipx->PairSignalStrength2->setValue(pairRssiField->getValue(1).toInt());
		m_pipx->PairSignalStrength3->setValue(pairRssiField->getValue(2).toInt());
		m_pipx->PairSignalStrength4->setValue(pairRssiField->getValue(3).toInt());
	} else {
		qDebug() << "PipXtremeGadgetWidget: Count not read PairID field.";
	}

	// Update the Description field
	UAVObjectField* descField = object->getField("Description");
	if (descField) {
		char buf[PipXStatus::DESCRIPTION_NUMELEM];
		for (unsigned int i = 0; i < PipXStatus::DESCRIPTION_NUMELEM; ++i)
			buf[i] = descField->getValue(i).toChar().toAscii();
		m_pipx->FirmwareVersion->setText(buf);
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

 	// Update the link state
 	UAVObjectField* linkField = object->getField("LinkState");
 	if (linkField) {
		const char *msg = "Unknown";
		switch (linkField->getValue().toInt())
		{
		case PipXStatus::LINKSTATE_DISCONNECTED:
			msg = "Disconnected";
			break;
		case PipXStatus::LINKSTATE_CONNECTING:
			msg = "Connecting";
			break;
		case PipXStatus::LINKSTATE_CONNECTED:
			msg = "Connected";
			break;
		}
		m_pipx->LinkState->setText(msg);
	} else {
		qDebug() << "PipXtremeGadgetWidget: Count not read link state field.";
	}
}

/*!
  \brief Called by updates to @PipXSettings
  */
void ConfigPipXtremeWidget::updateSettings(UAVObject *object)
{
	settingsUpdated = true;
        enableControls(true);

	// Get the settings object.
	PipXSettings *pipxSettings = PipXSettings::GetInstance(getObjectManager());
	PipXSettings::DataFields pipxSettingsData = pipxSettings->getData();
	pairID = pipxSettingsData.PairID;
}

void ConfigPipXtremeWidget::disconnected()
{
	settingsUpdated = false;
	enableControls(false);
}

/**
   @}
   @}
*/
