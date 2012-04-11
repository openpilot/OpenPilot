/**
******************************************************************************
*
* @file       pipxtremegadgetwidget.h
* @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
* @addtogroup GCSPlugins GCS Plugins
* @{
* @{
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

#ifndef PIPXTREMEGADGETWIDGET_H
#define PIPXTREMEGADGETWIDGET_H

#include "ui_pipxtreme.h"

#include <qextserialport.h>
#include <qextserialenumerator.h>

#include "uavtalk/telemetrymanager.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/uavobject.h"

#include "coreplugin/icore.h"
#include "coreplugin/connectionmanager.h"

#include "rawhid/rawhidplugin.h"

#include <QtGui/QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QThread>
#include <QMessageBox>
#include <QTimer>
#include <QtCore/QVector>
#include <QtCore/QIODevice>
#include <QtCore/QLinkedList>
#include <QMutex>
#include <QMutexLocker>
#include "widgetbar.h"

// *************************
// pipx config comms packets

#pragma pack(push, 1)

typedef struct
{
	uint32_t    marker;
	uint32_t    serial_number;
	uint8_t     type;
	uint8_t     spare;
	uint16_t    data_size;
	uint32_t    header_crc;
	uint32_t    data_crc;
//	uint8_t     data[0];
} t_pipx_config_header;

typedef struct
{
	uint8_t		major_version;
	uint8_t		minor_version;
	uint32_t	serial_number;
	uint32_t    min_frequency_Hz;
	uint32_t    max_frequency_Hz;
	uint8_t     frequency_band;
	float	    frequency_step_size;
} t_pipx_config_details;

typedef struct
{
	uint8_t     mode;
	uint8_t     link_state;
	int16_t		rssi;
	int32_t		afc;
	uint16_t	retries;
} t_pipx_config_state;

typedef struct
{
	uint8_t		mode;
	uint32_t    serial_baudrate;
	uint32_t    destination_id;
	uint32_t    frequency_Hz;
	uint32_t    max_rf_bandwidth;
	uint8_t     max_tx_power;
	uint8_t     rf_xtal_cap;
	bool        aes_enable;
	uint8_t     aes_key[16];
	uint8_t		rts_time;
	uint8_t		spare[16];
} t_pipx_config_settings;

typedef struct
{
	uint32_t	start_frequency;
	uint16_t    frequency_step_size;
	uint16_t    magnitudes;
//	int8_t      magnitude[0];
} t_pipx_config_spectrum;

#pragma pack(pop)

// *************************

typedef struct
{
	quint8		*buffer;
	int			size;
	int			used;
	QMutex		mutex;
} t_buffer;

// *************************

class PipXtremeGadgetWidget : public QWidget
{
	Q_OBJECT

		public:
	PipXtremeGadgetWidget(QWidget *parent = 0);
	~PipXtremeGadgetWidget();

	public slots:
	void onTelemetryStart();
	void onTelemetryStop();
	void onTelemetryConnect();
	void onTelemetryDisconnect();

	void onComboBoxPorts_currentIndexChanged(int index);

public slots:
	void updateStatus(UAVObject *object1);

protected:
	void resizeEvent(QResizeEvent *event);

private:
	typedef enum { PIPX_IDLE, PIPX_REQ_DETAILS, PIPX_REQ_SETTINGS, PIPX_REQ_STATE} PipXStage;

	Ui_PipXtremeWidget	*m_widget;

	QIODevice			*m_ioDevice;
	QMutex				device_mutex;

	t_buffer			device_input_buffer;

	PipXStage			m_stage;
	int					m_stage_retries;

	// The PipXtreme status UAVObject
	UAVDataObject* pipxStatusObj;

//	QVector<quint8>	buffer;

	t_pipx_config_details	pipx_config_details;
	t_pipx_config_settings	pipx_config_settings;
	t_pipx_config_state		pipx_config_state;
	t_pipx_config_spectrum	pipx_config_spectrum;

	uint32_t updateCRC32(uint32_t crc, uint8_t b);
	uint32_t updateCRC32Data(uint32_t crc, void *data, int len);
	void makeCRC_Table32();

	QString getSerialPortDevice(const QString &friendName);

	void disableTelemetry();
	void enableTelemetry();

	void sendRequestDetails(uint32_t serial_number);
	void sendRequestSettings(uint32_t serial_number);
	void sendRequestState(uint32_t serial_number);
	void sendSettings(uint32_t serial_number, t_pipx_config_settings *settings);

	void processRxBuffer();
	void processRxPacket(quint8 *packet, int packet_size);

	void disconnectPort(bool enable_telemetry, bool lock_stuff = true);
	void connectPort();

	private slots:
	void importSettings();
	void exportSettings();
	void connectDisconnect();
	void error(QString errorString, int errorNumber);
	void getPorts();
	void randomiseAESKey();
	void scanSpectrum();
	void saveToFlash();
	void textChangedAESKey(const QString &text);
	void processStream();
	void processRxStream();

};

#endif
