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

#define SERIAL_PORT	1
#define USB_PORT	2

// ***************************************************************************************

// constructor
PipXtremeGadgetWidget::PipXtremeGadgetWidget(QWidget *parent) :
	QWidget(parent),
	m_widget(NULL),
	m_ioDevice(NULL)
{
	m_widget = new Ui_PipXtremeWidget();
	m_widget->setupUi(this);

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
    QStringList list;

	disconnect(m_widget->comboBox_Ports, 0, 0, 0);

	m_widget->comboBox_Ports->clear();

    // ********************************
    // Populate the telemetry combo box with serial ports

	QList<QextPortInfo> serial_ports = QextSerialEnumerator::getPorts();

	qSort(serial_ports.begin(), serial_ports.end(), sortSerialPorts);

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
	while (m_ioDevice)
    {
		quint8 buf;

		if (!m_ioDevice->isOpen())
			break;

		qint64 bytes_available = m_ioDevice->bytesAvailable();
		if (bytes_available <= 0)
			break;

		qint64 bytes_read = m_ioDevice->read((char *)&buf, sizeof(buf));
		if (bytes_read <= 0)
			break;

		processInputBytes(&buf, sizeof(buf));
    }
}

void PipXtremeGadgetWidget::processInputBytes(quint8 *buf, int count)
{
	if (!buf || count <= 0)
		return;



}

// ***************************************************************************************

void PipXtremeGadgetWidget::disconnectPort(bool enable_telemetry)
{	// disconnect the comms port

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
