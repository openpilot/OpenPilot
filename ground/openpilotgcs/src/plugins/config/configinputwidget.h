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

class Ui_InputWidget;

class ConfigInputWidget: public ConfigTaskWidget
{
	Q_OBJECT

public:
        ConfigInputWidget(QWidget *parent = 0);
        ~ConfigInputWidget();

public slots:
	void onTelemetryStart();
	void onTelemetryStop();
	void onTelemetryConnect();
	void onTelemetryDisconnect();

	void onInSliderValueChanged0(int value);
	void onInSliderValueChanged1(int value);
	void onInSliderValueChanged2(int value);
	void onInSliderValueChanged3(int value);
	void onInSliderValueChanged4(int value);
	void onInSliderValueChanged5(int value);
	void onInSliderValueChanged6(int value);
	void onInSliderValueChanged7(int value);

private:
        Ui_InputWidget *m_config;

	QList<QSlider> sliders;

        void updateChannelInSlider(QSlider *slider, QLabel *min, QLabel *max, int value, bool reversed);

	void assignChannel(UAVDataObject *obj, QString str);
	void assignOutputChannel(UAVDataObject *obj, QString str);

	int mccDataRate;

	UAVObject::Metadata accInitialData;

        QList<QSlider*> inSliders;
	QList<QLabel*> inMaxLabels;
	QList<QLabel*> inMinLabels;
	QList<QLabel*> inNeuLabels;
        QList<QCheckBox*> inRevCheckboxes;

	bool firstUpdate;

	void enableControls(bool enable);

private slots:
	void updateChannels(UAVObject* obj);
	void requestRCInputUpdate();
	void sendRCInputUpdate();
	void saveRCInputObject();
        void reverseCheckboxClicked(bool state);
};

#endif
