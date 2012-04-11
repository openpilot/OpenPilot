/**
******************************************************************************
*
* @file       pipxtremegadgetwidget.cpp
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

#include <QDebug>
#include <QtOpenGL/QGLWidget>
#include <QSettings>
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include <stdlib.h>

#include <pipxsettings.h>
#include <pipxstatus.h>
#include "pipxtremegadgetwidget.h"

#define NO_PORT					0
#define SERIAL_PORT				1
#define USB_PORT				2

// ***************************************************************************************
// config packet details

#define MAX_RETRIES					7
#define RETRY_TIME					500		// ms

#define PIPX_HEADER_MARKER					0x76b38a52

enum {
	PIPX_PACKET_TYPE_REQ_DETAILS = 0,
	PIPX_PACKET_TYPE_DETAILS,
	PIPX_PACKET_TYPE_REQ_SETTINGS,
	PIPX_PACKET_TYPE_SETTINGS,
	PIPX_PACKET_TYPE_REQ_STATE,
	PIPX_PACKET_TYPE_STATE,
	PIPX_PACKET_TYPE_SPECTRUM
};

enum {
	FREQBAND_UNKNOWN = 0,
	FREQBAND_434MHz,
	FREQBAND_868MHz,
	FREQBAND_915MHz
};

enum {
	LINK_DISCONNECTED = 0,
	LINK_CONNECTING,
	LINK_CONNECTED
};

// ***************************************************************************************

enum {
	MODE_NORMAL = 0,			// normal 2-way packet mode
	MODE_STREAM_TX,				// 1-way continuous tx packet mode
	MODE_STREAM_RX,				// 1-way continuous rx packet mode
	MODE_PPM_TX,				// PPM tx mode
	MODE_PPM_RX,				// PPM rx mode
	MODE_SCAN_SPECTRUM,			// scan the receiver over the whole band
	MODE_TX_BLANK_CARRIER_TEST,	// blank carrier Tx mode (for calibrating the carrier frequency say)
	MODE_TX_SPECTRUM_TEST		// pseudo random Tx data mode (for checking the Tx carrier spectrum)
};

// ***************************************************************************************

#define Poly32	0x04c11db7		// 32-bit polynomial .. this should produce the same as the STM32 hardware CRC

uint32_t CRC_Table32[] = {
	0x00000000, 0x04c11db7, 0x09823b6e, 0x0d4326d9, 0x130476dc, 0x17c56b6b, 0x1a864db2, 0x1e475005,
	0x2608edb8, 0x22c9f00f, 0x2f8ad6d6, 0x2b4bcb61, 0x350c9b64, 0x31cd86d3, 0x3c8ea00a, 0x384fbdbd,
	0x4c11db70, 0x48d0c6c7, 0x4593e01e, 0x4152fda9, 0x5f15adac, 0x5bd4b01b, 0x569796c2, 0x52568b75,
	0x6a1936c8, 0x6ed82b7f, 0x639b0da6, 0x675a1011, 0x791d4014, 0x7ddc5da3, 0x709f7b7a, 0x745e66cd,
	0x9823b6e0, 0x9ce2ab57, 0x91a18d8e, 0x95609039, 0x8b27c03c, 0x8fe6dd8b, 0x82a5fb52, 0x8664e6e5,
	0xbe2b5b58, 0xbaea46ef, 0xb7a96036, 0xb3687d81, 0xad2f2d84, 0xa9ee3033, 0xa4ad16ea, 0xa06c0b5d,
	0xd4326d90, 0xd0f37027, 0xddb056fe, 0xd9714b49, 0xc7361b4c, 0xc3f706fb, 0xceb42022, 0xca753d95,
	0xf23a8028, 0xf6fb9d9f, 0xfbb8bb46, 0xff79a6f1, 0xe13ef6f4, 0xe5ffeb43, 0xe8bccd9a, 0xec7dd02d,
	0x34867077, 0x30476dc0, 0x3d044b19, 0x39c556ae, 0x278206ab, 0x23431b1c, 0x2e003dc5, 0x2ac12072,
	0x128e9dcf, 0x164f8078, 0x1b0ca6a1, 0x1fcdbb16, 0x018aeb13, 0x054bf6a4, 0x0808d07d, 0x0cc9cdca,
	0x7897ab07, 0x7c56b6b0, 0x71159069, 0x75d48dde, 0x6b93dddb, 0x6f52c06c, 0x6211e6b5, 0x66d0fb02,
	0x5e9f46bf, 0x5a5e5b08, 0x571d7dd1, 0x53dc6066, 0x4d9b3063, 0x495a2dd4, 0x44190b0d, 0x40d816ba,
	0xaca5c697, 0xa864db20, 0xa527fdf9, 0xa1e6e04e, 0xbfa1b04b, 0xbb60adfc, 0xb6238b25, 0xb2e29692,
	0x8aad2b2f, 0x8e6c3698, 0x832f1041, 0x87ee0df6, 0x99a95df3, 0x9d684044, 0x902b669d, 0x94ea7b2a,
	0xe0b41de7, 0xe4750050, 0xe9362689, 0xedf73b3e, 0xf3b06b3b, 0xf771768c, 0xfa325055, 0xfef34de2,
	0xc6bcf05f, 0xc27dede8, 0xcf3ecb31, 0xcbffd686, 0xd5b88683, 0xd1799b34, 0xdc3abded, 0xd8fba05a,
	0x690ce0ee, 0x6dcdfd59, 0x608edb80, 0x644fc637, 0x7a089632, 0x7ec98b85, 0x738aad5c, 0x774bb0eb,
	0x4f040d56, 0x4bc510e1, 0x46863638, 0x42472b8f, 0x5c007b8a, 0x58c1663d, 0x558240e4, 0x51435d53,
	0x251d3b9e, 0x21dc2629, 0x2c9f00f0, 0x285e1d47, 0x36194d42, 0x32d850f5, 0x3f9b762c, 0x3b5a6b9b,
	0x0315d626, 0x07d4cb91, 0x0a97ed48, 0x0e56f0ff, 0x1011a0fa, 0x14d0bd4d, 0x19939b94, 0x1d528623,
	0xf12f560e, 0xf5ee4bb9, 0xf8ad6d60, 0xfc6c70d7, 0xe22b20d2, 0xe6ea3d65, 0xeba91bbc, 0xef68060b,
	0xd727bbb6, 0xd3e6a601, 0xdea580d8, 0xda649d6f, 0xc423cd6a, 0xc0e2d0dd, 0xcda1f604, 0xc960ebb3,
	0xbd3e8d7e, 0xb9ff90c9, 0xb4bcb610, 0xb07daba7, 0xae3afba2, 0xaafbe615, 0xa7b8c0cc, 0xa379dd7b,
	0x9b3660c6, 0x9ff77d71, 0x92b45ba8, 0x9675461f, 0x8832161a, 0x8cf30bad, 0x81b02d74, 0x857130c3,
	0x5d8a9099, 0x594b8d2e, 0x5408abf7, 0x50c9b640, 0x4e8ee645, 0x4a4ffbf2, 0x470cdd2b, 0x43cdc09c,
	0x7b827d21, 0x7f436096, 0x7200464f, 0x76c15bf8, 0x68860bfd, 0x6c47164a, 0x61043093, 0x65c52d24,
	0x119b4be9, 0x155a565e, 0x18197087, 0x1cd86d30, 0x029f3d35, 0x065e2082, 0x0b1d065b, 0x0fdc1bec,
	0x3793a651, 0x3352bbe6, 0x3e119d3f, 0x3ad08088, 0x2497d08d, 0x2056cd3a, 0x2d15ebe3, 0x29d4f654,
	0xc5a92679, 0xc1683bce, 0xcc2b1d17, 0xc8ea00a0, 0xd6ad50a5, 0xd26c4d12, 0xdf2f6bcb, 0xdbee767c,
	0xe3a1cbc1, 0xe760d676, 0xea23f0af, 0xeee2ed18, 0xf0a5bd1d, 0xf464a0aa, 0xf9278673, 0xfde69bc4,
	0x89b8fd09, 0x8d79e0be, 0x803ac667, 0x84fbdbd0, 0x9abc8bd5, 0x9e7d9662, 0x933eb0bb, 0x97ffad0c,
	0xafb010b1, 0xab710d06, 0xa6322bdf, 0xa2f33668, 0xbcb4666d, 0xb8757bda, 0xb5365d03, 0xb1f740b4
};

// ***************************************************************************************

// constructor
PipXtremeGadgetWidget::PipXtremeGadgetWidget(QWidget *parent) :
	QWidget(parent),
	m_widget(NULL),
	m_ioDevice(NULL),
	m_stage(PIPX_IDLE)
{
	m_widget = new Ui_PipXtremeWidget();
	m_widget->setupUi(this);

#if QT_VERSION >= 0x040700
	qsrand(QDateTime::currentDateTime().toMSecsSinceEpoch());
#else
	qsrand(QDateTime::currentDateTime().toTime_t());
#endif
	// Connect to the PipXStatus object updates
	ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
	UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
	pipxStatusObj = dynamic_cast<UAVDataObject*>(objManager->getObject("PipXStatus"));
	if (pipxStatusObj != NULL ) {
		connect(pipxStatusObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updateStatus(UAVObject*)));
	} else {
		qDebug() << "Error: Object is unknown (PipXStatus).";
	}

	device_input_buffer.size = 8192;
	device_input_buffer.used = 0;
	device_input_buffer.buffer = new quint8 [device_input_buffer.size];

	memset(&pipx_config_details, 0, sizeof(pipx_config_details));
	memset(&pipx_config_settings, 0, sizeof(pipx_config_settings));

	m_widget->comboBox_SerialBaudrate->clear();
	m_widget->comboBox_SerialBaudrate->addItem("1200", 1200);
	m_widget->comboBox_SerialBaudrate->addItem("2400", 2400);
	m_widget->comboBox_SerialBaudrate->addItem("4800", 4800);
	m_widget->comboBox_SerialBaudrate->addItem("9600", 9600);
	m_widget->comboBox_SerialBaudrate->addItem("19200", 19200);
	m_widget->comboBox_SerialBaudrate->addItem("38400", 38400);
	m_widget->comboBox_SerialBaudrate->addItem("57600", 57600);
	m_widget->comboBox_SerialBaudrate->addItem("115200", 115200);
#if (defined Q_OS_WIN)
	m_widget->comboBox_SerialBaudrate->addItem("230400", 230400);
	m_widget->comboBox_SerialBaudrate->addItem("460800", 460800);
	m_widget->comboBox_SerialBaudrate->addItem("921600", 921600);
#endif
	m_widget->comboBox_SerialBaudrate->setCurrentIndex(m_widget->comboBox_SerialBaudrate->findText("57600"));

	m_widget->comboBox_Mode->clear();
	m_widget->comboBox_Mode->addItem("Normal", MODE_NORMAL);
	m_widget->comboBox_Mode->addItem("Continuous Stream Tx", MODE_STREAM_TX);
	m_widget->comboBox_Mode->addItem("Continuous Stream Rx", MODE_STREAM_RX);
	m_widget->comboBox_Mode->addItem("PPM Tx", MODE_PPM_TX);
	m_widget->comboBox_Mode->addItem("PPM Rx", MODE_PPM_RX);
	m_widget->comboBox_Mode->addItem("Scan Spectrum", MODE_SCAN_SPECTRUM);
	m_widget->comboBox_Mode->addItem("Test Tx Blank Carrier Frequency", MODE_TX_BLANK_CARRIER_TEST);
	m_widget->comboBox_Mode->addItem("Test Tx Spectrum", MODE_TX_SPECTRUM_TEST);

	m_widget->comboBox_SerialPortSpeed->clear();
	for (int i = 0; i < m_widget->comboBox_SerialBaudrate->count(); i++)
		m_widget->comboBox_SerialPortSpeed->addItem(m_widget->comboBox_SerialBaudrate->itemText(i), m_widget->comboBox_SerialBaudrate->itemData(i));
	m_widget->comboBox_SerialPortSpeed->setCurrentIndex(m_widget->comboBox_SerialPortSpeed->findText("57600"));

	m_widget->comboBox_MaxRFBandwidth->clear();
	m_widget->comboBox_MaxRFBandwidth->addItem("500", 500);
	m_widget->comboBox_MaxRFBandwidth->addItem("1000", 1000);
	m_widget->comboBox_MaxRFBandwidth->addItem("2000", 2000);
	m_widget->comboBox_MaxRFBandwidth->addItem("4000", 4000);
	m_widget->comboBox_MaxRFBandwidth->addItem("8000", 8000);
	m_widget->comboBox_MaxRFBandwidth->addItem("9600", 9600);
	m_widget->comboBox_MaxRFBandwidth->addItem("16000", 16000);
	m_widget->comboBox_MaxRFBandwidth->addItem("19200", 19200);
	m_widget->comboBox_MaxRFBandwidth->addItem("24000", 24000);
	m_widget->comboBox_MaxRFBandwidth->addItem("32000", 32000);
	m_widget->comboBox_MaxRFBandwidth->addItem("64000", 64000);
	m_widget->comboBox_MaxRFBandwidth->addItem("128000", 128000);
	m_widget->comboBox_MaxRFBandwidth->addItem("192000", 192000);
	m_widget->comboBox_MaxRFBandwidth->addItem("256000", 256000);
	m_widget->comboBox_MaxRFBandwidth->setCurrentIndex(m_widget->comboBox_MaxRFBandwidth->findText("128000"));

	m_widget->comboBox_MaxRFTxPower->clear();
	m_widget->comboBox_MaxRFTxPower->addItem("1.25mW", 0);
	m_widget->comboBox_MaxRFTxPower->addItem("1.6mW", 1);
	m_widget->comboBox_MaxRFTxPower->addItem("3.16mW", 2);
	m_widget->comboBox_MaxRFTxPower->addItem("6.3mW", 3);
	m_widget->comboBox_MaxRFTxPower->addItem("12.6mW", 4);
	m_widget->comboBox_MaxRFTxPower->addItem("25mW", 5);
	m_widget->comboBox_MaxRFTxPower->addItem("50mW", 6);
	m_widget->comboBox_MaxRFTxPower->addItem("100mW", 7);
	m_widget->comboBox_MaxRFTxPower->setCurrentIndex(m_widget->comboBox_MaxRFTxPower->findText("12.6mW"));

	m_widget->doubleSpinBox_Frequency->setSingleStep(0.00015625);

	m_widget->widgetRSSI->setMinimum(-120);
	m_widget->widgetRSSI->setMaximum(0);
	m_widget->widgetRSSI->setValue(m_widget->widgetRSSI->minimum());

	m_widget->label_RSSI->setText("RSSI");

	m_widget->graphicsView_Spectrum->setScene(new QGraphicsScene(this));
	m_widget->graphicsView_Spectrum->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
//	m_widget->graphicsView_Spectrum->setViewport(new QWidget);
	m_widget->graphicsView_Spectrum->setCacheMode(QGraphicsView::CacheBackground);
	m_widget->graphicsView_Spectrum->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform | QPainter::HighQualityAntialiasing);
	QGraphicsScene *spec_scene = m_widget->graphicsView_Spectrum->scene();
	if (spec_scene)
	{
		spec_scene->setBackgroundBrush(QColor(80, 80, 80));
		spec_scene->clear();

//        spec_scene->addItem(m_background);
//        spec_scene->addItem(m_joystickEnd);
//        spec_scene->setSceneRect(m_background->boundingRect());
	}

	m_widget->pushButton_Save->setEnabled(false);
	m_widget->pushButton_ScanSpectrum->setEnabled(false);
	m_widget->pushButton_Import->setEnabled(false);
	m_widget->pushButton_Export->setEnabled(false);

	QIcon rbi;
	rbi.addFile(QString(":pipxtreme/images/view-refresh.svg"));
	m_widget->refreshPorts->setIcon(rbi);

	// Listen to telemetry connection events
	ExtensionSystem::PluginManager *pluginManager = ExtensionSystem::PluginManager::instance();
	if (pluginManager)
	{
		TelemetryManager *telemetryManager = pluginManager->getObject<TelemetryManager>();
		if (telemetryManager)
		{
			connect(telemetryManager, SIGNAL(myStart()), this, SLOT(onTelemetryStart()));
			connect(telemetryManager, SIGNAL(myStop()), this, SLOT(onTelemetryStop()));
			connect(telemetryManager, SIGNAL(connected()), this, SLOT(onTelemetryConnect()));
			connect(telemetryManager, SIGNAL(disconnected()), this, SLOT(onTelemetryDisconnect()));
		}
	}

	getPorts();

	connect(m_widget->connectButton, SIGNAL(clicked()), this, SLOT(connectDisconnect()));
	connect(m_widget->refreshPorts, SIGNAL(clicked()), this, SLOT(getPorts()));
	connect(m_widget->pushButton_AESKeyRandom, SIGNAL(clicked()), this, SLOT(randomiseAESKey()));
	connect(m_widget->pushButton_ScanSpectrum, SIGNAL(clicked()), this, SLOT(scanSpectrum()));
	connect(m_widget->pushButton_Save, SIGNAL(clicked()), this, SLOT(saveToFlash()));
	connect(m_widget->lineEdit_AESKey, SIGNAL(textChanged(const QString &)), this, SLOT(textChangedAESKey(const QString &)));
	connect(m_widget->pushButton_Import, SIGNAL(clicked()), this, SLOT(importSettings()));
	connect(m_widget->pushButton_Export, SIGNAL(clicked()), this, SLOT(exportSettings()));
}

// destructor .. this never gets called :(
PipXtremeGadgetWidget::~PipXtremeGadgetWidget()
{
	disconnectPort(false);

	device_input_buffer.mutex.lock();
	if (device_input_buffer.buffer)
	{
		delete [] device_input_buffer.buffer;
		device_input_buffer.buffer = NULL;
		device_input_buffer.size = 0;
		device_input_buffer.used = 0;
	}
	device_input_buffer.mutex.unlock();
}

/*!
  \brief Called by updates to @PipXStatus
  */
void PipXtremeGadgetWidget::updateStatus(UAVObject *object) {
	UAVObjectField* rssiField = object->getField("RSSI");
	if (rssiField) {
		m_widget->widgetRSSI->setValue(rssiField->getDouble());
	} else {
		qDebug() << "PipXtremeGadgetWidget: Count not read RSSI field.";
	}
	UAVObjectField* idField = object->getField("DeviceID");
	if (idField) {
		m_widget->lineEdit_SerialNumber->setText(QString::number(idField->getValue().toInt(), 16).toUpper());
	} else {
		qDebug() << "PipXtremeGadgetWidget: Count not read RSSI field.";
	}
	UAVObjectField* linkField = object->getField("linkState");
	if (idField) {
		char *msg = "Unknown";
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
		m_widget->lineEdit_LinkState->setText(msg);
	} else {
		qDebug() << "PipXtremeGadgetWidget: Count not read link state field.";
	}
}

// ***************************************************************************************

void PipXtremeGadgetWidget::resizeEvent(QResizeEvent *event)
{
	if (m_widget)
	{
		if (m_widget->graphicsView_Spectrum)
		{
			QGraphicsScene *spec_scene = m_widget->graphicsView_Spectrum->scene();
			if (spec_scene)
			{
//              spec_scene->setSceneRect(QRect(QPoint(0, 0), event->size()));
//              spec_scene->setBackgroundBrush(Qt::black);
			}
		}
	}

//    PipXtremeGadgetWidget::resizeEvent(event);
}

// ***************************************************************************************

void PipXtremeGadgetWidget::onComboBoxPorts_currentIndexChanged(int index)
{
	if (index < 0)
	{
		m_widget->comboBox_SerialBaudrate->setEnabled(false);
		return;
	}

	int type = m_widget->comboBox_Ports->itemData(index).toInt();

	m_widget->comboBox_SerialBaudrate->setEnabled(type == SERIAL_PORT);
}

// ***************************************************************************************

uint32_t PipXtremeGadgetWidget::updateCRC32(uint32_t crc, uint8_t b)
{
	return (crc << 8) ^ CRC_Table32[(crc >> 24) ^ b];
}

uint32_t PipXtremeGadgetWidget::updateCRC32Data(uint32_t crc, void *data, int len)
{
	register uint8_t *p = (uint8_t *)data;
	register uint32_t _crc = crc;
	for (int i = len; i > 0; i--)
		_crc = (_crc << 8) ^ CRC_Table32[(_crc >> 24) ^ *p++];
	return _crc;
}

// Generate the CRC table
void PipXtremeGadgetWidget::makeCRC_Table32()
{
	for (int i = 0; i < 256; i++)
	{
		uint32_t crc = i;
		for (int j = 8; j > 0; j--)
			crc = (crc & 1) ? (crc << 1) ^ Poly32 : crc << 1;
		CRC_Table32[i] = crc;
	}
}

// ***************************************************************************************

QString PipXtremeGadgetWidget::getSerialPortDevice(const QString &friendName)
{
	QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();

	foreach (QextPortInfo port, ports)
	{
#ifdef Q_OS_WIN
		if (port.friendName == friendName)
			return port.portName;
#else
		if (port.friendName == friendName)
			return port.physName;
#endif
	}

	return "";
}

bool sortSerialPorts(const QextPortInfo &s1, const QextPortInfo &s2)
{
	return (s1.portName < s2.portName);
}

void PipXtremeGadgetWidget::getPorts()
{
	disconnect(m_widget->comboBox_Ports, 0, 0, 0);

	m_widget->comboBox_Ports->clear();

	// ********************************
	// Populate the telemetry combo box with serial ports

	QList<QextPortInfo> serial_ports = QextSerialEnumerator::getPorts();

	qSort(serial_ports.begin(), serial_ports.end(), sortSerialPorts);

	QStringList list;

	foreach (QextPortInfo port, serial_ports)
		list.append(port.friendName);

	for (int i = 0; i < list.count(); i++)
		m_widget->comboBox_Ports->addItem("com: " + list.at(i), SERIAL_PORT);

	// ********************************
	// Populate the telemetry combo box with usb ports

	pjrc_rawhid *rawHidHandle = new pjrc_rawhid();
	if (rawHidHandle)
	{
		int opened = rawHidHandle->open(10, 0x20A0, 0x415C, 0xFF9C, 0x0001);
		if (opened > 0)
		{
			QList<QString> usb_ports;

			for (int i = 0; i < opened; i++)
				usb_ports.append(rawHidHandle->getserial(i));

			qSort(usb_ports.begin(), usb_ports.end());

			for (int i = 0; i < usb_ports.count(); i++)
				m_widget->comboBox_Ports->addItem("usb: " + usb_ports.at(i), USB_PORT);
		}

		delete rawHidHandle;
	}

	// ********************************

	connect(m_widget->comboBox_Ports, SIGNAL(currentIndexChanged(int)), this, SLOT(onComboBoxPorts_currentIndexChanged(int)));

	onComboBoxPorts_currentIndexChanged(m_widget->comboBox_Ports->currentIndex());

	// ********************************
}

// ***************************************************************************************

void PipXtremeGadgetWidget::onTelemetryStart()
{
	setEnabled(false);

//	m_widget->connectButton->setEnabled(false);
//	m_widget->comboBox_Ports->setEnabled(false);
//	m_widget->refreshPorts->setEnabled(false);
//	m_widget->comboBox_SerialBaudrate->setEnabled(false);
}

void PipXtremeGadgetWidget::onTelemetryStop()
{
	setEnabled(true);

//	m_widget->connectButton->setEnabled(true);
//	m_widget->comboBox_Ports->setEnabled(true);
//	m_widget->refreshPorts->setEnabled(true);
//	m_widget->comboBox_SerialBaudrate->setEnabled(true);
}

// ***************************************************************************************

void PipXtremeGadgetWidget::onTelemetryConnect()
{
}

void PipXtremeGadgetWidget::onTelemetryDisconnect()
{
}

// ***************************************************************************************

void PipXtremeGadgetWidget::disableTelemetry()
{	// Suspend telemetry & polling

	Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();
	if (!cm) return;

	cm->disconnectDevice();
	cm->suspendPolling();
}

void PipXtremeGadgetWidget::enableTelemetry()
{	// Restart the polling thread

	Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();
	if (!cm) return;

	cm->resumePolling();
}

// ***************************************************************************************

void PipXtremeGadgetWidget::randomiseAESKey()
{
#if QT_VERSION >= 0x040700
	uint32_t crc = ((uint32_t)qrand() << 16) ^ qrand() ^ QDateTime::currentDateTime().toMSecsSinceEpoch();	// only available with Qt 4.7.1 and later
#else
	uint32_t crc = ((uint32_t)qrand() << 16) ^ qrand() ^ QDateTime::currentDateTime().toTime_t();
#endif

	QString key = "";
	for (int i = 0; i < 4; i++)
	{
		for (int i = 0; i < (27 + (qrand() & 0x7f)); i++)
			crc = updateCRC32(crc, qrand());

		key += QString::number(crc, 16).rightJustified(8, '0');
	}

	m_widget->lineEdit_AESKey->setText(key);
}

void PipXtremeGadgetWidget::scanSpectrum()
{
}

void PipXtremeGadgetWidget::saveToFlash()
{
	QMutexLocker locker_dev(&device_mutex);
	QMutexLocker locker_inbuf(&device_input_buffer.mutex);

	if (!m_ioDevice) return;
	if (!m_ioDevice->isOpen()) return;

	if (pipx_config_details.serial_number == 0) return;	// we first need some details before sending new setings

	QString s;
	bool ok;

	t_pipx_config_settings settings;

	settings.mode = m_widget->comboBox_Mode->itemData(m_widget->comboBox_Mode->currentIndex()).toUInt();

	s = m_widget->lineEdit_PairedSerialNumber->text().trimmed().toLower();
	s.replace(' ', "");	// remove all spaces
	if (s.startsWith("0x"))
	{
		s.remove(0, 2);
		s = s.trimmed();
	}
	settings.destination_id = s.toUInt(&ok, 16);
	if (s.isEmpty() || !ok)
	{
		error("Check your \"Paired Serial Number\" entry!", 0);
		return;
	}

	settings.rf_xtal_cap = m_widget->spinBox_FrequencyCalibration->value();

	s = m_widget->doubleSpinBox_Frequency->text().trimmed();
	s.replace(' ', "");	// remove all spaces
	settings.frequency_Hz = (uint32_t)(s.toDouble(&ok) * 1e6);
	if (ok)	// round it to the 'frequency step size'
		settings.frequency_Hz = (uint32_t)(((double)settings.frequency_Hz + pipx_config_details.frequency_step_size / 2) / pipx_config_details.frequency_step_size) * pipx_config_details.frequency_step_size;
	if (s.isEmpty() || !ok || settings.frequency_Hz < pipx_config_details.min_frequency_Hz || settings.frequency_Hz > pipx_config_details.max_frequency_Hz)
	{
		error("Check your \"Frequency\" entry!", 0);
		return;
	}

	settings.max_rf_bandwidth = m_widget->comboBox_MaxRFBandwidth->itemData(m_widget->comboBox_MaxRFBandwidth->currentIndex()).toInt();
	settings.max_tx_power = m_widget->comboBox_MaxRFTxPower->itemData(m_widget->comboBox_MaxRFTxPower->currentIndex()).toInt();
	settings.serial_baudrate = m_widget->comboBox_SerialPortSpeed->itemData(m_widget->comboBox_SerialPortSpeed->currentIndex()).toInt();
	settings.aes_enable = m_widget->checkBox_AESEnable->isChecked();
	settings.rts_time = m_widget->spinBox_RTSTime->value();

	memset(settings.aes_key, 0, sizeof(settings.aes_key));
	s = m_widget->lineEdit_AESKey->text().trimmed();
	s.replace(' ', "");	// remove all spaces
	while (s.length() < 32) s = '0' + s;
	if (settings.aes_enable && s.length() != 32)
	{
		error("Check your \"AES Key\" entry! .. it must be 32 hex characters long", 0);
		return;
	}
	for (int i = 0; i < (int)sizeof(settings.aes_key); i++)
	{
		QString s2 = s.left(2);
		s.remove(0, 2);
		s = s.trimmed();
		settings.aes_key[i] = s2.toUInt(&ok, 16);
		if (!ok)
		{
			error("Check your \"AES Key\" entry! .. it must contain only hex characters (0-9, a-f)", 0);
			return;
		}
	}

	// send the new settings to the modem
	sendSettings(pipx_config_details.serial_number, &settings);

	// retrieve them back
	m_stage_retries = 0;
	m_stage = PIPX_REQ_SETTINGS;
}

void PipXtremeGadgetWidget::textChangedAESKey(const QString &text)
{
	QString s = text;
	s.replace(' ', "");
	m_widget->label_AESkey->setText("AES Encryption Key (" + QString::number(s.length()) + ")");
}

// ***************************************************************************************
// send various packets

void PipXtremeGadgetWidget::sendRequestDetails(uint32_t serial_number)
{
	// *****************
	// create the packet

	t_pipx_config_header header;
	header.marker = PIPX_HEADER_MARKER;
	header.serial_number = serial_number;
	header.type = PIPX_PACKET_TYPE_REQ_DETAILS;
	header.spare = 0;
	header.data_size = 0;
	header.data_crc = 0xffffffff;
	header.header_crc = 0;
	header.header_crc = updateCRC32Data(0xffffffff, &header, sizeof(t_pipx_config_header));

	// *****************
	// send the packet

//	QMutexLocker locker_dev(&device_mutex);
	if (!m_ioDevice) return;
	if (!m_ioDevice->isOpen()) return;
	qint64 bytes_written = m_ioDevice->write((const char*)&header, sizeof(t_pipx_config_header));

	Q_UNUSED(bytes_written)

		// *****************
		}

void PipXtremeGadgetWidget::sendRequestSettings(uint32_t serial_number)
{
	if (serial_number == 0)
		return;

	// *****************
	// create the packet

	t_pipx_config_header header;
	header.marker = PIPX_HEADER_MARKER;
	header.serial_number = serial_number;
	header.type = PIPX_PACKET_TYPE_REQ_SETTINGS;
	header.spare = 0;
	header.data_size = 0;
	header.data_crc = 0xffffffff;
	header.header_crc = 0;
	header.header_crc = updateCRC32Data(0xffffffff, &header, sizeof(t_pipx_config_header));

	// *****************
	// send the packet

//	QMutexLocker locker_dev(&device_mutex);
	if (!m_ioDevice) return;
	if (!m_ioDevice->isOpen()) return;
	qint64 bytes_written = m_ioDevice->write((const char*)&header, sizeof(t_pipx_config_header));

//	qDebug() << "PipX m_ioDevice->write: " << QString::number(bytes_written) << endl;

	Q_UNUSED(bytes_written)

//	error("Bytes written", bytes_written);	// TEST ONLY

		// *****************
		}

void PipXtremeGadgetWidget::sendRequestState(uint32_t serial_number)
{
	if (serial_number == 0)
		return;

	// *****************
	// create the packet

	t_pipx_config_header header;
	header.marker = PIPX_HEADER_MARKER;
	header.serial_number = serial_number;
	header.type = PIPX_PACKET_TYPE_REQ_STATE;
	header.spare = 0;
	header.data_size = 0;
	header.data_crc = 0xffffffff;
	header.header_crc = 0;
	header.header_crc = updateCRC32Data(0xffffffff, &header, sizeof(t_pipx_config_header));

	// *****************
	// send the packet

//	QMutexLocker locker_dev(&device_mutex);
	if (!m_ioDevice) return;
	if (!m_ioDevice->isOpen()) return;
	qint64 bytes_written = m_ioDevice->write((const char*)&header, sizeof(t_pipx_config_header));

//	qDebug() << "PipX m_ioDevice->write: " << QString::number(bytes_written) << endl;

	Q_UNUSED(bytes_written)

//	error("Bytes written", bytes_written);	// TEST ONLY

		// *****************
		}

void PipXtremeGadgetWidget::sendSettings(uint32_t serial_number, t_pipx_config_settings *settings)
{
	if (serial_number == 0)
		return;

	if (!settings)
		return;

	uint8_t buffer[sizeof(t_pipx_config_header) + sizeof(t_pipx_config_settings)];

	t_pipx_config_header *header = (t_pipx_config_header *)buffer;
	uint8_t *data = (uint8_t *) header + sizeof(t_pipx_config_header);

	// *****************
	// create the packet

	memcpy(data, settings, sizeof(t_pipx_config_settings));

	header->marker = PIPX_HEADER_MARKER;
	header->serial_number = serial_number;
	header->type = PIPX_PACKET_TYPE_SETTINGS;
	header->spare = 0;
	header->data_size = sizeof(t_pipx_config_settings);
	header->data_crc = updateCRC32Data(0xffffffff, data, header->data_size);
	header->header_crc = 0;
	header->header_crc = updateCRC32Data(0xffffffff, header, sizeof(t_pipx_config_header));

	// *****************
	// send the packet

//	QMutexLocker locker_dev(&device_mutex);
	if (!m_ioDevice) return;
	if (!m_ioDevice->isOpen()) return;
	qint64 bytes_written = m_ioDevice->write((const char*)buffer, sizeof(t_pipx_config_header) + header->data_size);

//	error("Bytes written", bytes_written);	// TEST ONLY

	Q_UNUSED(bytes_written)

		// *****************
		}

// ***************************************************************************************
// process rx stream data

void PipXtremeGadgetWidget::processRxStream()
{
	QMutexLocker locker_dev(&device_mutex);
	QMutexLocker locker_inbuf(&device_input_buffer.mutex);

	if (!m_ioDevice) return;
	if (!m_ioDevice->isOpen()) return;

	QTimer::singleShot(100, this, SLOT(processRxStream()));

	qint64 bytes_available = m_ioDevice->bytesAvailable();
	if (bytes_available <= 0) return;

	// ensure we have buffer space for the new data
	if (!device_input_buffer.buffer)
	{	// allocate a buffer for the data
		device_input_buffer.size = bytes_available * 2;
		device_input_buffer.used = 0;
		device_input_buffer.buffer = new quint8 [device_input_buffer.size];
		if (!device_input_buffer.buffer) return;
	}
	else
	{
		if ((device_input_buffer.used + (bytes_available * 2)) > device_input_buffer.size)
		{	// need to increase the buffer size

			// create a new larger buffer
			int new_size = device_input_buffer.used + bytes_available * 2;
			quint8 *new_buf = new quint8 [new_size];
			if (!new_buf) return;

			// copy the data from the old buffer into the new buffer
			memmove(new_buf, device_input_buffer.buffer, device_input_buffer.used);

			// delete the old buffer
			delete [] device_input_buffer.buffer;

			// keep the new buffer
			device_input_buffer.buffer = new_buf;
			device_input_buffer.size = new_size;
		}
	}

	// add the new data into the buffer
	qint64 bytes_read = m_ioDevice->read((char *)(device_input_buffer.buffer + device_input_buffer.used), device_input_buffer.size - device_input_buffer.used);
	if (bytes_read <= 0) return;
	device_input_buffer.used += bytes_read;

	processRxBuffer();
}

void PipXtremeGadgetWidget::processRxBuffer()
{	// scan the buffer for a valid packet

	if (!device_input_buffer.buffer || device_input_buffer.used < (int)sizeof(t_pipx_config_header))
		return;	// no data as yet or not yet enough data

	while (device_input_buffer.used >= (int)sizeof(t_pipx_config_header))
	{
		uint32_t crc1, crc2;

		t_pipx_config_header *header = (t_pipx_config_header *)device_input_buffer.buffer;
		uint8_t *data = (uint8_t *)header + sizeof(t_pipx_config_header);

		// check packet marker
		if (header->marker != PIPX_HEADER_MARKER)
		{	// marker not yet found
			// remove a byte
			int i = 1;
			if (i < device_input_buffer.used)
				memmove(device_input_buffer.buffer, device_input_buffer.buffer + i, device_input_buffer.used - i);
			device_input_buffer.used -= i;
			continue;
		}

		// check the header CRC
		crc1 = header->header_crc;
		header->header_crc = 0;
		crc2 = updateCRC32Data(0xffffffff, header, sizeof(t_pipx_config_header));
		header->header_crc = crc1;
		if (crc2 != crc1)
		{	// faulty header
			int i = 1;
			if (i < device_input_buffer.used)
				memmove(device_input_buffer.buffer, device_input_buffer.buffer + i, device_input_buffer.used - i);
			device_input_buffer.used -= i;
			continue;
		}

		// valid error free header found!

		int total_packet_size = sizeof(t_pipx_config_header) + header->data_size;

		if (device_input_buffer.used < total_packet_size)
		{	// not yet got a full packet
			break;
		}

		if (header->data_size > 0)
		{
			// check the data crc
			crc1 = header->data_crc;
			crc2 = updateCRC32Data(0xffffffff, data, header->data_size);
			if (crc2 != crc1)
			{	// faulty data
				int i = 1;
				if (i < device_input_buffer.used)
					memmove(device_input_buffer.buffer, device_input_buffer.buffer + i, device_input_buffer.used - i);
				device_input_buffer.used -= i;
				continue;
			}
		}

		processRxPacket(device_input_buffer.buffer, total_packet_size);

		// remove the packet from the buffer
		if (total_packet_size < device_input_buffer.used)
			memmove(device_input_buffer.buffer, device_input_buffer.buffer + total_packet_size, device_input_buffer.used - total_packet_size);
		device_input_buffer.used -= total_packet_size;
	}
}

void PipXtremeGadgetWidget::processRxPacket(quint8 *packet, int packet_size)
{
	if (!packet || packet_size <= 0)
		return;

	t_pipx_config_header *header = (t_pipx_config_header *)packet;
	uint8_t *data = (uint8_t *)header + sizeof(t_pipx_config_header);

	switch (header->type)
	{
	case PIPX_PACKET_TYPE_REQ_DETAILS:
		break;

	case PIPX_PACKET_TYPE_DETAILS:
		if (m_stage == PIPX_REQ_DETAILS)
		{
			m_stage_retries = 0;
			m_stage = PIPX_REQ_SETTINGS;

			if (packet_size < (int)sizeof(t_pipx_config_header) + (int)sizeof(t_pipx_config_details))
				break;	// packet size is too small - error

			memcpy(&pipx_config_details, data, sizeof(t_pipx_config_details));

			if (pipx_config_details.major_version < 0 || (pipx_config_details.major_version <= 0 && pipx_config_details.minor_version < 6))
			{
				QMessageBox msgBox;
				msgBox.setIcon(QMessageBox::Critical);
				msgBox.setText("You need to update your PipX modem firmware to V0.5 or later");
				msgBox.exec();
				disconnectPort(true);
				return;
			}

			m_widget->lineEdit_FirmwareVersion->setText(QString::number(pipx_config_details.major_version) + "." + QString::number(pipx_config_details.minor_version));

			m_widget->lineEdit_SerialNumber->setText(QString::number(pipx_config_details.serial_number, 16).toUpper());

			if (pipx_config_details.frequency_band == FREQBAND_434MHz)
				m_widget->lineEdit_FrequencyBand->setText("434MHz");
			else
				if (pipx_config_details.frequency_band == FREQBAND_868MHz)
					m_widget->lineEdit_FrequencyBand->setText("868MHz");
				else
					if (pipx_config_details.frequency_band == FREQBAND_915MHz)
						m_widget->lineEdit_FrequencyBand->setText("915MHz");
					else
						m_widget->lineEdit_FrequencyBand->setText("UNKNOWN [" + QString::number(pipx_config_details.frequency_band) + "]");

			m_widget->lineEdit_MinFrequency->setText(QString::number((double)pipx_config_details.min_frequency_Hz / 1e6, 'f', 6) + "MHz");
			m_widget->lineEdit_MaxFrequency->setText(QString::number((double)pipx_config_details.max_frequency_Hz / 1e6, 'f', 6) + "MHz");

			m_widget->doubleSpinBox_Frequency->setMinimum((double)pipx_config_details.min_frequency_Hz / 1e6);
			m_widget->doubleSpinBox_Frequency->setMaximum((double)pipx_config_details.max_frequency_Hz / 1e6);
			m_widget->doubleSpinBox_Frequency->setSingleStep(((double)pipx_config_details.frequency_step_size * 4) / 1e6);

			m_widget->lineEdit_FrequencyStepSize->setText(QString::number(pipx_config_details.frequency_step_size, 'f', 2) + "Hz");

			m_widget->pushButton_Save->setEnabled(true);
			m_widget->pushButton_ScanSpectrum->setEnabled(true);
			m_widget->pushButton_Import->setEnabled(true);
			m_widget->pushButton_Export->setEnabled(true);
		}
		break;

	case PIPX_PACKET_TYPE_REQ_SETTINGS:
		break;

	case PIPX_PACKET_TYPE_SETTINGS:
		if (m_stage == PIPX_REQ_SETTINGS && pipx_config_details.serial_number != 0)
		{
			if (packet_size < (int)sizeof(t_pipx_config_header) + (int)sizeof(t_pipx_config_settings))
				break;	// packet size is too small - error

			m_stage_retries = 0;
			m_stage = PIPX_REQ_STATE;

			memcpy(&pipx_config_settings, data, sizeof(t_pipx_config_settings));

			m_widget->comboBox_Mode->setCurrentIndex(m_widget->comboBox_Mode->findData(pipx_config_settings.mode));
			m_widget->lineEdit_PairedSerialNumber->setText(QString::number(pipx_config_settings.destination_id, 16).toUpper());
			m_widget->spinBox_FrequencyCalibration->setValue(pipx_config_settings.rf_xtal_cap);
			m_widget->doubleSpinBox_Frequency->setValue((double)pipx_config_settings.frequency_Hz / 1e6);
			m_widget->comboBox_MaxRFBandwidth->setCurrentIndex(m_widget->comboBox_MaxRFBandwidth->findData(pipx_config_settings.max_rf_bandwidth));
			m_widget->comboBox_MaxRFTxPower->setCurrentIndex(m_widget->comboBox_MaxRFTxPower->findData(pipx_config_settings.max_tx_power));
			m_widget->comboBox_SerialPortSpeed->setCurrentIndex(m_widget->comboBox_SerialPortSpeed->findData(pipx_config_settings.serial_baudrate));
			m_widget->spinBox_RTSTime->setValue(pipx_config_settings.rts_time);

			QString key = "";
			for (int i = 0; i < (int)sizeof(pipx_config_settings.aes_key); i++)
				key += QString::number(pipx_config_settings.aes_key[i], 16).rightJustified(2, '0');
			m_widget->lineEdit_AESKey->setText(key);
			m_widget->checkBox_AESEnable->setChecked(pipx_config_settings.aes_enable);
		}

		break;

	case PIPX_PACKET_TYPE_REQ_STATE:
		break;

	case PIPX_PACKET_TYPE_STATE:
		if (m_stage == PIPX_REQ_STATE && pipx_config_details.serial_number != 0)
		{
			if (packet_size < (int)sizeof(t_pipx_config_header) + (int)sizeof(t_pipx_config_state))
				break;	// packet size is too small - error

			m_stage_retries = 0;
//				m_stage = PIPX_REQ_STATE;

			memcpy(&pipx_config_state, data, sizeof(t_pipx_config_state));

			if (pipx_config_state.rssi < m_widget->widgetRSSI->minimum())
				m_widget->widgetRSSI->setValue(m_widget->widgetRSSI->minimum());
			else
				if (pipx_config_state.rssi > m_widget->widgetRSSI->maximum())
					m_widget->widgetRSSI->setValue(m_widget->widgetRSSI->maximum());
				else
					m_widget->widgetRSSI->setValue(pipx_config_state.rssi);
			m_widget->label_RSSI->setText("RSSI " + QString::number(pipx_config_state.rssi) + "dBm");
			m_widget->lineEdit_RxAFC->setText(QString::number(pipx_config_state.afc) + "Hz");
			m_widget->lineEdit_Retries->setText(QString::number(pipx_config_state.retries));
		}

		break;

	case PIPX_PACKET_TYPE_SPECTRUM:	// a packet with scanned spectrum data
		if (pipx_config_details.serial_number != 0)
		{
			if (packet_size < (int)sizeof(t_pipx_config_header) + (int)sizeof(t_pipx_config_spectrum))
				break;	// packet size is too small - error

			memcpy(&pipx_config_spectrum, data, sizeof(t_pipx_config_spectrum));
			int8_t *spec_data = (int8_t *)(data + sizeof(t_pipx_config_spectrum));

			if (pipx_config_spectrum.magnitudes > 0)
			{
				m_widget->label_19->setText(QString::number(pipx_config_spectrum.start_frequency) + " " + QString::number(pipx_config_spectrum.frequency_step_size) + " " + QString::number(pipx_config_spectrum.magnitudes));
/*
  QGraphicsScene *spec_scene = m_widget->graphicsView_Spectrum->scene();
  if (spec_scene)
  {
  if (pipx_config_spectrum.start_frequency - pipx_config_details.min_frequency_Hz <= 0)
  spec_scene->clear();

  int w = 500;
  int h = 500;

  float xscale = (float)w / (pipx_config_details.max_frequency_Hz - pipx_config_details.min_frequency_Hz);
  float yscale = h / 128.0f;

  float xs = xscale * (pipx_config_spectrum.start_frequency - pipx_config_details.min_frequency_Hz);

  for (int i = 0; i < pipx_config_spectrum.magnitudes; i++)
  {
  int x = -(w / 2) + xs + (xscale * i * pipx_config_spectrum.frequency_step_size);
  int rssi = (int)spec_data[i] + 128;
  int y = yscale * rssi;
  spec_scene->addLine(x, -h / 2, x, (-h / 2) - y, QPen(Qt::green, 1, Qt::SolidLine, Qt::SquareCap));
  }
  }
*/
			}
		}

		break;

	default:
		break;
	}
}


// ***************************************************************************************

void PipXtremeGadgetWidget::processStream()
{
	QMutexLocker locker_dev(&device_mutex);
	QMutexLocker locker_inbuf(&device_input_buffer.mutex);

	if (!m_ioDevice) return;
	if (!m_ioDevice->isOpen()) return;

	switch (m_stage)
	{
	case PIPX_IDLE:
		QTimer::singleShot(RETRY_TIME, this, SLOT(processStream()));
		break;

	case PIPX_REQ_DETAILS:
		if (++m_stage_retries > MAX_RETRIES)
		{
			disconnectPort(true, false);
			break;
		}
		sendRequestDetails(0);
		QTimer::singleShot(RETRY_TIME, this, SLOT(processStream()));
		break;

	case PIPX_REQ_SETTINGS:
		if (++m_stage_retries > MAX_RETRIES)
		{
			disconnectPort(true, false);
			break;
		}
		sendRequestSettings(pipx_config_details.serial_number);
		QTimer::singleShot(RETRY_TIME, this, SLOT(processStream()));
		break;

	case PIPX_REQ_STATE:
		if (++m_stage_retries > MAX_RETRIES)
		{
			disconnectPort(true, false);
			break;
		}
		sendRequestState(pipx_config_details.serial_number);
		QTimer::singleShot(RETRY_TIME, this, SLOT(processStream()));
		break;

	default:
		m_stage_retries = 0;
		m_stage = PIPX_IDLE;
		QTimer::singleShot(RETRY_TIME, this, SLOT(processStream()));
		break;
	}
}

// ***************************************************************************************

void PipXtremeGadgetWidget::disconnectPort(bool enable_telemetry, bool lock_stuff)
{	// disconnect the comms port

	if (lock_stuff)
	{
		QMutexLocker locker_dev(&device_mutex);
		QMutexLocker locker_inbuf(&device_input_buffer.mutex);
	}

	m_stage_retries = 0;
	m_stage = PIPX_IDLE;

	device_input_buffer.used = 0;

	memset(&pipx_config_details, 0, sizeof(pipx_config_details));
	memset(&pipx_config_settings, 0, sizeof(pipx_config_settings));

	if (m_ioDevice)
	{
		m_ioDevice->close();
		disconnect(m_ioDevice, 0, 0, 0);
		delete m_ioDevice;
		m_ioDevice = NULL;
	}

	m_widget->connectButton->setText("Connect");
	m_widget->comboBox_SerialBaudrate->setEnabled(true);
	m_widget->comboBox_Ports->setEnabled(true);
	m_widget->refreshPorts->setEnabled(true);
	m_widget->pushButton_ScanSpectrum->setEnabled(false);
	m_widget->pushButton_Save->setEnabled(false);
	m_widget->pushButton_Import->setEnabled(false);
	m_widget->pushButton_Export->setEnabled(false);

	m_widget->lineEdit_FirmwareVersion->setText("");
	m_widget->lineEdit_SerialNumber->setText("");
	m_widget->lineEdit_FrequencyBand->setText("");
	m_widget->lineEdit_MinFrequency->setText("");
	m_widget->lineEdit_MaxFrequency->setText("");
	m_widget->lineEdit_FrequencyStepSize->setText("");
	m_widget->lineEdit_LinkState->setText("");
	m_widget->widgetRSSI->setValue(m_widget->widgetRSSI->minimum());
	m_widget->label_RSSI->setText("RSSI");
	m_widget->lineEdit_RxAFC->setText("");
	m_widget->lineEdit_Retries->setText("");
	m_widget->lineEdit_PairedSerialNumber->setText("");
	m_widget->spinBox_FrequencyCalibration->setValue(0);
	m_widget->doubleSpinBox_Frequency->setValue(0);
	m_widget->lineEdit_AESKey->setText("");
	m_widget->checkBox_AESEnable->setChecked(false);

	if (enable_telemetry)
		enableTelemetry();
}

void PipXtremeGadgetWidget::connectPort()
{	// connect the comms port

	disconnectPort(true);

	QMutexLocker locker_dev(&device_mutex);

	int device_idx = m_widget->comboBox_Ports->currentIndex();
	if (device_idx < 0)
		return;

	QString device_str = m_widget->comboBox_Ports->currentText().trimmed();
	if (device_str.isEmpty())
		return;

	int type = NO_PORT;
	if (device_str.toLower().startsWith("com: "))
	{
		type = SERIAL_PORT;
		device_str.remove(0, 5);
		device_str = device_str.trimmed();
	}
	else
		if (device_str.toLower().startsWith("usb: "))
		{
			type = USB_PORT;
			device_str.remove(0, 5);
			device_str = device_str.trimmed();
		}

//	type = m_widget->comboBox_Ports->itemData(device_idx).toInt();

//	qDebug() << QString::number(type) << ": " << device_str;

	// Suspend GCS telemety & polling
	disableTelemetry();

	switch (type)
	{
	case SERIAL_PORT:	// serial port
	{
		QString str = getSerialPortDevice(device_str);
		if (str.isEmpty())
			break;

		int br_idx = m_widget->comboBox_SerialBaudrate->currentIndex();
		if (br_idx < 0)
			break;

		int baudrate = m_widget->comboBox_SerialBaudrate->itemData(br_idx).toInt();

		BaudRateType bdt = BAUD57600;
		switch (baudrate)
		{
		case 1200: bdt = BAUD1200; break;
		case 2400: bdt = BAUD2400; break;
		case 4800: bdt = BAUD4800; break;
		case 9600: bdt = BAUD9600; break;
		case 19200: bdt = BAUD19200; break;
		case 38400: bdt = BAUD38400; break;
		case 57600: bdt = BAUD57600; break;
		case 115200: bdt = BAUD115200; break;
		case 128000: bdt = BAUD128000; break;
		case 256000: bdt = BAUD256000; break;
#if (defined Q_OS_WIN)
		case 230400: bdt = BAUD230400; break;
		case 460800: bdt = BAUD460800; break;
		case 921600: bdt = BAUD921600; break;
#endif
		}

		PortSettings settings;
		settings.BaudRate = bdt;
		settings.DataBits = DATA_8;
		settings.Parity = PAR_NONE;
		settings.StopBits = STOP_1;
		settings.FlowControl = FLOW_OFF;
		settings.Timeout_Millisec = 1000;
//				settings.setQueryMode(QextSerialBase::EventDriven);

//				QextSerialPort *serial_dev = new QextSerialPort(str, settings, QextSerialPort::Polling);
//				QextSerialPort *serial_dev = new QextSerialPort(str, settings, QextSerialPort::EventDriven);
		QextSerialPort *serial_dev = new QextSerialPort(str, settings);
		if (!serial_dev)
			break;

//				if (!serial_dev->open(QIODevice::ReadWrite | QIODevice::Unbuffered))
		if (!serial_dev->open(QIODevice::ReadWrite))
		{
			delete serial_dev;
			break;
		}

		m_ioDevice = serial_dev;
		break;
	}

	case USB_PORT:	// USB port
	{
		RawHID *usb_dev = new RawHID(device_str);
		if (!usb_dev)
			break;

//				if (!usb_dev->open(QIODevice::ReadWrite | QIODevice::Unbuffered))
		if (!usb_dev->open(QIODevice::ReadWrite))
		{
			delete usb_dev;
			break;
		}

		m_ioDevice = usb_dev;
		break;
	}

	default:	// unknown port type
		break;
	}

	if (!m_ioDevice)
	{	// failed to connect .. restore GCS telemetry and polling
		enableTelemetry();
	}
	else
	{	// connected OK
		memset(&pipx_config_details, 0, sizeof(pipx_config_details));
		memset(&pipx_config_settings, 0, sizeof(pipx_config_settings));

		m_widget->connectButton->setText("Disconnect");
		m_widget->comboBox_SerialBaudrate->setEnabled(false);
		m_widget->comboBox_Ports->setEnabled(false);
		m_widget->refreshPorts->setEnabled(false);

		m_ioDevice->setTextModeEnabled(false);
		QTimer::singleShot(100, this, SLOT(processRxStream()));
//		connect(m_ioDevice, SIGNAL(readyRead()), this, SLOT(processRxStream()));

		m_stage_retries = 0;
		m_stage = PIPX_REQ_DETAILS;
		QTimer::singleShot(100, this, SLOT(processStream()));
	}
}

// ***************************************************************************************

void PipXtremeGadgetWidget::connectDisconnect()
{
	if (m_ioDevice)
		disconnectPort(true);	// disconnect
	else
		connectPort();			// connect
}

// ***************************************************************************************

void PipXtremeGadgetWidget::importSettings()
{
	QString filename = "pipx_" + QString::number(pipx_config_details.serial_number, 16).rightJustified(8, '0') + ".ini";

	QFileDialog::Options options;
	QString selectedFilter;
	filename = QFileDialog::getOpenFileName(this,
						tr("Load from PipX settings file"),
						filename,
						tr("PipX settings (*.ini)"),
						&selectedFilter,
						options
		).trimmed();
	if (filename.isEmpty())
		return;

	if (!QFileInfo(filename).isReadable())
	{
		QMessageBox msgBox;
		msgBox.setText(tr("Can't read file ") + QFileInfo(filename).absoluteFilePath());
		msgBox.exec();
		return;
	}

	QSettings settings(filename, QSettings::IniFormat);

	uint32_t serial_number = settings.value("details/serial_number", 0).toUInt();
	if (serial_number && serial_number != pipx_config_details.serial_number)
	{
//		return;
	}

	pipx_config_settings.mode = settings.value("settings/mode", 0).toUInt();
	pipx_config_settings.destination_id = settings.value("settings/paired_serial_number", 0).toUInt();
	pipx_config_settings.rf_xtal_cap = settings.value("settings/frequency_calibration", 0x7f).toUInt();
	pipx_config_settings.frequency_Hz = settings.value("settings/frequency", (pipx_config_details.min_frequency_Hz + pipx_config_details.max_frequency_Hz) / 2).toUInt();
	pipx_config_settings.max_rf_bandwidth = settings.value("settings/max_rf_bandwidth", 128000).toUInt();
	pipx_config_settings.max_tx_power = settings.value("settings/max_tx_power", 4).toUInt();
	pipx_config_settings.serial_baudrate = settings.value("settings/serial_baudrate", 57600).toUInt();
	pipx_config_settings.aes_enable = settings.value("settings/aes_enable", false).toBool();
	for (int i = 0; i < (int)sizeof(pipx_config_settings.aes_key); i++)
		pipx_config_settings.aes_key[i] = settings.value("settings/aes_key_" + QString::number(i), 0).toUInt();
	pipx_config_settings.rts_time = settings.value("settings/ready_to_send_time", 10).toUInt();

	m_widget->comboBox_Mode->setCurrentIndex(m_widget->comboBox_Mode->findData(pipx_config_settings.mode));
	m_widget->lineEdit_PairedSerialNumber->setText(QString::number(pipx_config_settings.destination_id, 16).toUpper());
	m_widget->spinBox_FrequencyCalibration->setValue(pipx_config_settings.rf_xtal_cap);
	m_widget->doubleSpinBox_Frequency->setValue((double)pipx_config_settings.frequency_Hz / 1e6);
	m_widget->comboBox_MaxRFBandwidth->setCurrentIndex(m_widget->comboBox_MaxRFBandwidth->findData(pipx_config_settings.max_rf_bandwidth));
	m_widget->comboBox_MaxRFTxPower->setCurrentIndex(m_widget->comboBox_MaxRFTxPower->findData(pipx_config_settings.max_tx_power));
	m_widget->comboBox_SerialPortSpeed->setCurrentIndex(m_widget->comboBox_SerialPortSpeed->findData(pipx_config_settings.serial_baudrate));
	m_widget->spinBox_RTSTime->setValue(pipx_config_settings.rts_time);

	QString key = "";
	for (int i = 0; i < (int)sizeof(pipx_config_settings.aes_key); i++)
		key += QString::number(pipx_config_settings.aes_key[i], 16).rightJustified(2, '0');
	m_widget->lineEdit_AESKey->setText(key);
	m_widget->checkBox_AESEnable->setChecked(pipx_config_settings.aes_enable);
}

void PipXtremeGadgetWidget::exportSettings()
{
	QString filename = "pipx_" + QString::number(pipx_config_details.serial_number, 16).rightJustified(8, '0') + ".ini";

	filename = QFileDialog::getSaveFileName(this,
						tr("Save to PipX settings file"),
						filename,
						tr("PipX settings (*.ini)")
		).trimmed();
	if (filename.isEmpty())
		return;
/*
  if (QFileInfo(filename).exists())
  {
  QMessageBox msgBox;
  msgBox.setText(tr("File already exists."));
  msgBox.setInformativeText(tr("Do you want to overwrite the existing file?"));
  msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
  msgBox.setDefaultButton(QMessageBox::Ok);
  if (msgBox.exec() == QMessageBox::Ok)
  QFileInfo(filename).absoluteDir().remove(QFileInfo(filename).fileName());
  else
  return;
  }
*/	QDir dir = QFileInfo(filename).absoluteDir();
	if (!dir.exists())
	{
		QMessageBox msgBox;
		msgBox.setText(tr("Can't write file ") + QFileInfo(filename).absoluteFilePath() + " since directory " + dir.absolutePath() + " doesn't exist!");
		msgBox.exec();
		return;
	}

	QSettings settings(filename, QSettings::IniFormat);
	settings.setValue("details/serial_number", pipx_config_details.serial_number);
	settings.setValue("details/min_frequency", pipx_config_details.min_frequency_Hz);
	settings.setValue("details/max_frequency", pipx_config_details.max_frequency_Hz);
	settings.setValue("details/frequency_band", pipx_config_details.frequency_band);
	settings.setValue("settings/mode", pipx_config_settings.mode);
	settings.setValue("settings/paired_serial_number", pipx_config_settings.destination_id);
	settings.setValue("settings/frequency_calibration", pipx_config_settings.rf_xtal_cap);
	settings.setValue("settings/frequency", pipx_config_settings.frequency_Hz);
	settings.setValue("settings/max_rf_bandwidth", pipx_config_settings.max_rf_bandwidth);
	settings.setValue("settings/max_tx_power", pipx_config_settings.max_tx_power);
	settings.setValue("settings/serial_baudrate", pipx_config_settings.serial_baudrate);
	settings.setValue("settings/aes_enable", pipx_config_settings.aes_enable);
	settings.setValue("settings/ready_to_send_time", pipx_config_settings.rts_time);
	for (int i = 0; i < (int)sizeof(pipx_config_settings.aes_key); i++)
		settings.setValue("settings/aes_key_" + QString::number(i), pipx_config_settings.aes_key[i]);
}

// ***************************************************************************************
// Shows a message box with an error string.

void PipXtremeGadgetWidget::error(QString errorString, int errorNumber)
{
	QMessageBox msgBox;
	msgBox.setIcon(QMessageBox::Critical);
	msgBox.setText(errorString + " [" + QString::number(errorNumber) + "]");
	msgBox.exec();
}

// ***************************************************************************************
