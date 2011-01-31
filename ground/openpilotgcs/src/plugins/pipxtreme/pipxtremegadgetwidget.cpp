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

#include "pipxtremegadgetwidget.h"

//#include <aggregation/aggregate.h>

#define SERIAL_PORT				1
#define USB_PORT				2

#define pipx_header_marker		0x76b38a52

// ***************************************************************************************

#define Poly32	0x04c11db7				// 32-bit polynomial .. this should produce the same as the STM32 hardware CRC

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
	m_ioDevice(NULL)
{
	m_widget = new Ui_PipXtremeWidget();
	m_widget->setupUi(this);

	device_input_buffer.size = 8192;
	device_input_buffer.used = 0;
	device_input_buffer.buffer = new quint8 [device_input_buffer.size];

	m_widget->comboBox_SerialBaudrate->clear();
	m_widget->comboBox_SerialBaudrate->addItem("1200", 1200);
	m_widget->comboBox_SerialBaudrate->addItem("2400", 2400);
	m_widget->comboBox_SerialBaudrate->addItem("4800", 4800);
	m_widget->comboBox_SerialBaudrate->addItem("9600", 9600);
	m_widget->comboBox_SerialBaudrate->addItem("19200", 19200);
	m_widget->comboBox_SerialBaudrate->addItem("38400", 38400);
	m_widget->comboBox_SerialBaudrate->addItem("57600", 57600);
	m_widget->comboBox_SerialBaudrate->addItem("115200", 115200);
//	m_widget->comboBox_SerialBaudrate->addItem("230400", 230400);
//	m_widget->comboBox_SerialBaudrate->addItem("460800", 460800);
//	m_widget->comboBox_SerialBaudrate->addItem("921600", 921600);
	m_widget->comboBox_SerialBaudrate->setCurrentIndex(m_widget->comboBox_SerialBaudrate->findText("57600"));

	m_widget->comboBox_Mode->clear();
	m_widget->comboBox_Mode->addItem("Normal", 0);
	m_widget->comboBox_Mode->addItem("Scan Spectrum", 1);
	m_widget->comboBox_Mode->addItem("Calibrate Tx Carrier Frequency", 2);
	m_widget->comboBox_Mode->addItem("Test Tx Spectrum", 3);

	m_widget->comboBox_SerialPortSpeed->clear();
	for (int i = 0; i < m_widget->comboBox_SerialBaudrate->count(); i++)
		m_widget->comboBox_SerialPortSpeed->addItem(m_widget->comboBox_SerialBaudrate->itemText(i), m_widget->comboBox_SerialBaudrate->itemData(i));
	m_widget->comboBox_SerialPortSpeed->setCurrentIndex(m_widget->comboBox_SerialPortSpeed->findText("57600"));

	m_widget->doubleSpinBox_Frequency->setSingleStep(0.00015625);

	m_widget->progressBar_RSSI->setMinimum(-120);
	m_widget->progressBar_RSSI->setMaximum(-20);
	m_widget->progressBar_RSSI->setValue(-80);

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

	m_widget->pushButton_ScanSpectrum->setEnabled(false);

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

//    delay::msleep(600);   // just for pips reference
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

uint32_t PipXtremeGadgetWidget::updateCRC32Data(uint32_t crc, void *data, int len)
{
	uint8_t *p = (uint8_t *)data;
	uint32_t _crc = crc;
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
		m_widget->comboBox_Ports->addItem(list.at(i), SERIAL_PORT);

    // ********************************
    // Populate the telemetry combo box with usb ports

    pjrc_rawhid *rawHidHandle = new pjrc_rawhid();
    if (rawHidHandle)
    {
        int opened = rawHidHandle->open(10, 0x20A0, 0x4117, 0xFF9C, 0x0001);
        if (opened > 0)
        {
            QList<QString> usb_ports;

            for (int i = 0; i < opened; i++)
                usb_ports.append(rawHidHandle->getserial(i));

			qSort(usb_ports.begin(), usb_ports.end());

			for (int i = 0; i < usb_ports.count(); i++)
				m_widget->comboBox_Ports->addItem(usb_ports.at(i), USB_PORT);
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

void PipXtremeGadgetWidget::processOutputStream()
{
	QMutexLocker locker_dev(&device_mutex);

	if (!m_ioDevice)
        return;

	if (!m_ioDevice->isOpen())
		return;

//	if (m_ioDevice->bytesToWrite() < TX_BUFFER_SIZE )
	{
//		m_ioDevice->write((const char*)txBuffer, dataOffset+length+CHECKSUM_LENGTH);
	}
}

void PipXtremeGadgetWidget::processInputStream()
{
	QMutexLocker locker_dev(&device_mutex);
	QMutexLocker locker_inbuf(&device_input_buffer.mutex);

	while (m_ioDevice)
    {
		if (!m_ioDevice->isOpen())
			break;

		qint64 bytes_available = m_ioDevice->bytesAvailable();
		if (bytes_available <= 0)
			break;

		if (!device_input_buffer.buffer)
		{	// allocate a buffer for the data
			device_input_buffer.size = bytes_available * 2;
			device_input_buffer.used = 0;
			device_input_buffer.buffer = new quint8 [device_input_buffer.size];
			if (!device_input_buffer.buffer)
				break;
		}
		else
		{
			if ((device_input_buffer.used + (bytes_available * 2)) > device_input_buffer.size)
			{	// need to increase the size of the input buffer

				// create a new larger buffer
				int new_size = device_input_buffer.used + bytes_available * 2;
				quint8 *new_buf = new quint8 [new_size];
				if (!new_buf)
					break;

				// copy the data from the old buffer into the new buffer
				memmove(new_buf, device_input_buffer.buffer, device_input_buffer.used);

				// delete the old buffer
				delete [] device_input_buffer.buffer;

				// keep the new buffer
				device_input_buffer.buffer = new_buf;
				device_input_buffer.size = new_size;
			}
		}

		// add the new data into the input buffer
		qint64 bytes_read = m_ioDevice->read((char *)(device_input_buffer.buffer + device_input_buffer.used), bytes_available);
		if (bytes_read <= 0)
			break;
		device_input_buffer.used += bytes_available;

		processInputBuffer();
    }
}

void PipXtremeGadgetWidget::processInputBuffer()
{	// scan the buffer for a valid packet

	if (!device_input_buffer.buffer || device_input_buffer.used < sizeof(t_pipx_config_header))
		return;	// no data as yet or not yet enough data

	while (device_input_buffer.used >= sizeof(t_pipx_config_header))
	{
		uint32_t crc1, crc2;

		t_pipx_config_header *header = (t_pipx_config_header *)device_input_buffer.buffer;
		uint8_t *data = (uint8_t *)header + sizeof(t_pipx_config_header);

		// check packet marker
		if (header->marker != pipx_header_marker)
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

		int total_packet_size = sizeof(t_pipx_config_header) + header->data_size;

		if (device_input_buffer.used < total_packet_size)
			break;	// not yet got a full packet

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

		processInputPacket(device_input_buffer.buffer, total_packet_size);

		// remove the packet from the buffer
		if (total_packet_size < device_input_buffer.used)
			memmove(device_input_buffer.buffer, device_input_buffer.buffer + total_packet_size, device_input_buffer.used - total_packet_size);
		device_input_buffer.used -= total_packet_size;
	}
}

void PipXtremeGadgetWidget::processInputPacket(quint8 *packet, int packet_size)
{
	if (!packet || packet_size <= 0)
		return;

//	t_pipx_config_header *header = (t_pipx_config_header *)packet;
//	uint8_t *data = (uint8_t *)header + sizeof(t_pipx_config_header);








}


// ***************************************************************************************

void PipXtremeGadgetWidget::disconnectPort(bool enable_telemetry)
{	// disconnect the comms port

	QMutexLocker locker_dev(&device_mutex);
	QMutexLocker locker_inbuf(&device_input_buffer.mutex);

	device_input_buffer.used = 0;

	if (!m_ioDevice)
		return;

	m_ioDevice->close();

	disconnect(m_ioDevice, 0, 0, 0);

	delete m_ioDevice;
	m_ioDevice = NULL;

	m_widget->connectButton->setText("Connect");
	m_widget->comboBox_SerialBaudrate->setEnabled(true);
	m_widget->comboBox_Ports->setEnabled(true);
	m_widget->refreshPorts->setEnabled(true);
	m_widget->pushButton_ScanSpectrum->setEnabled(false);

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

	int type = m_widget->comboBox_Ports->itemData(device_idx).toInt();

//	qDebug() << QString::number(type) << ": " << device_str;

	// Suspend telemety & polling in case it is not done yet
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
				}

				PortSettings settings;
				settings.BaudRate = bdt;
				settings.DataBits = DATA_8;
				settings.Parity = PAR_NONE;
				settings.StopBits = STOP_1;
				settings.FlowControl = FLOW_OFF;
				settings.Timeout_Millisec = 500;

//				QextSerialPort *serial_dev = new QextSerialPort(str, settings, QextSerialPort::Polling);
				QextSerialPort *serial_dev = new QextSerialPort(str, settings);
				if (!serial_dev)
					break;

				if (!serial_dev->open(QIODevice::ReadWrite | QIODevice::Unbuffered))
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

				if (!usb_dev->open(QIODevice::ReadWrite | QIODevice::Unbuffered))
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
	{	// failed to connect
		enableTelemetry();
	}
	else
	{	// connected OK
		m_widget->connectButton->setText("Disconnect");
		m_widget->comboBox_SerialBaudrate->setEnabled(false);
		m_widget->comboBox_Ports->setEnabled(false);
		m_widget->refreshPorts->setEnabled(false);
		m_widget->pushButton_ScanSpectrum->setEnabled(true);

		connect(m_ioDevice, SIGNAL(readyRead()), this, SLOT(processInputStream()));
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

// Shows a message box with an error string.
void PipXtremeGadgetWidget::error(QString errorString, int errorNumber)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(errorString + " [" + QString::number(errorNumber) + "]");
    msgBox.exec();
}

// ***************************************************************************************
