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

#include "pipxtremegadgetwidget.h"

//#include <aggregation/aggregate.h>

// constructor
PipXtremeGadgetWidget::PipXtremeGadgetWidget(QWidget *parent) : QWidget(parent)
{
    m_config = new Ui_PipXtremeWidget();
    m_config->setupUi(this);

    m_ioDevice = NULL;

    currentStep = IAP_STATE_READY;
    rescueStep = RESCUE_STEP0;
    resetOnly = false;

	m_config->comboBox_Mode->clear();
	m_config->comboBox_Mode->addItem("Normal", 0);
	m_config->comboBox_Mode->addItem("Scan Spectrum", 1);
	m_config->comboBox_Mode->addItem("Tx Carrier Calibrate", 2);
	m_config->comboBox_Mode->addItem("Tx Spectrum Test", 3);

	m_config->comboBox_SerialBaudrate->clear();
	m_config->comboBox_SerialBaudrate->addItem("1200", 1200);
	m_config->comboBox_SerialBaudrate->addItem("2400", 2400);
	m_config->comboBox_SerialBaudrate->addItem("4800", 4800);
	m_config->comboBox_SerialBaudrate->addItem("9600", 9600);
	m_config->comboBox_SerialBaudrate->addItem("19200", 19200);
	m_config->comboBox_SerialBaudrate->addItem("38400", 38400);
	m_config->comboBox_SerialBaudrate->addItem("57600", 57600);
	m_config->comboBox_SerialBaudrate->addItem("115200", 115200);
//	m_config->comboBox_SerialBaudrate->addItem("230400", 230400);
//	m_config->comboBox_SerialBaudrate->addItem("460800", 460800);
//	m_config->comboBox_SerialBaudrate->addItem("921600", 921600);
	m_config->comboBox_SerialBaudrate->setCurrentIndex(m_config->comboBox_SerialBaudrate->findText("57600"));

	m_config->comboBox_SerialPortSpeed->clear();
	m_config->comboBox_SerialPortSpeed->addItem("1200", 1200);
	m_config->comboBox_SerialPortSpeed->addItem("2400", 2400);
	m_config->comboBox_SerialPortSpeed->addItem("4800", 4800);
	m_config->comboBox_SerialPortSpeed->addItem("9600", 9600);
	m_config->comboBox_SerialPortSpeed->addItem("19200", 19200);
	m_config->comboBox_SerialPortSpeed->addItem("38400", 38400);
	m_config->comboBox_SerialPortSpeed->addItem("57600", 57600);
	m_config->comboBox_SerialPortSpeed->addItem("115200", 115200);
//	m_config->comboBox_SerialPortSpeed->addItem("230400", 230400);
//	m_config->comboBox_SerialPortSpeed->addItem("460800", 460800);
//	m_config->comboBox_SerialPortSpeed->addItem("921600", 921600);
	m_config->comboBox_SerialPortSpeed->setCurrentIndex(m_config->comboBox_SerialPortSpeed->findText("57600"));

    m_config->doubleSpinBox_Frequency->setSingleStep(0.00015625);

    m_config->graphicsView_Spectrum->setScene(new QGraphicsScene(this));
    QGraphicsScene *spec_scene = m_config->graphicsView_Spectrum->scene();
    if (spec_scene)
    {
        spec_scene->setBackgroundBrush(Qt::black);
        spec_scene->clear();
//        spec_scene->addItem(m_background);
//        spec_scene->addItem(m_joystickEnd);
//        spec_scene->setSceneRect(m_background->boundingRect());
    }

    m_config->pushButton_ScanSpectrum->setEnabled(false);

    // Listen to autopilot connection events
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    TelemetryManager *telMngr = pm->getObject<TelemetryManager>();
    connect(telMngr, SIGNAL(myStart()), this, SLOT(onTelemetryStart()));
    connect(telMngr, SIGNAL(myStop()), this, SLOT(onTelemetryStop()));
    connect(telMngr, SIGNAL(connected()), this, SLOT(onTelemetryConnect()));
    connect(telMngr, SIGNAL(disconnected()), this, SLOT(onTelemetryDisconnect()));

    // Note: remove listening to the connection manager, it overlaps with
    // listening to the telemetry manager, we should only listen to one, not both.

    // Also listen to disconnect actions from the user:
    // Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();
    // connect(cm, SIGNAL(deviceDisconnected()), this, SLOT(onModemDisconnect()));

//    int opened = rawHidHandle.open(10, 0x20A0, 0x4117, 0xFF9C, 0x0001);
//    rawHidPlugin = new RawHIDPlugin();

	connect(m_config->connectButton, SIGNAL(clicked()), this, SLOT(connectDisconnect()));

    getPorts();

    QIcon rbi;
    rbi.addFile(QString(":pipxtreme/images/view-refresh.svg"));
    m_config->refreshPorts->setIcon(rbi);

    connect(m_config->refreshPorts, SIGNAL(clicked()), this, SLOT(getPorts()));

    if (m_ioDevice)
        connect(m_ioDevice, SIGNAL(readyRead()), this, SLOT(processInputStream()));

//    delay::msleep(600);   // just for pips reference
}

// destructor .. this never gets called :(
PipXtremeGadgetWidget::~PipXtremeGadgetWidget()
{
	disconnect(false);
}

void PipXtremeGadgetWidget::resizeEvent(QResizeEvent *event)
{
    if (m_config)
    {
        if (m_config->graphicsView_Spectrum)
        {
            QGraphicsScene *spec_scene = m_config->graphicsView_Spectrum->scene();
            if (spec_scene)
            {
//              spec_scene->setSceneRect(QRect(QPoint(0, 0), event->size()));
//              spec_scene->setBackgroundBrush(Qt::black);
            }
        }
    }

//    PipXtremeGadgetWidget::resizeEvent(event);
}

bool sortSerialPorts(const QextPortInfo &s1, const QextPortInfo &s2)
{
    return (s1.portName < s2.portName);
}

// ***************************************************************************************

void PipXtremeGadgetWidget::getPorts()
{
    QStringList list;

//    m_config->refreshPorts->setEnabled(false);
//    m_config->comboBox_Ports->setEnabled(false);

    m_config->comboBox_Ports->clear();

    // ********************************
    // Populate the telemetry combo box with serial ports

    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();

    qSort(ports.begin(), ports.end(), sortSerialPorts);

    foreach (QextPortInfo port, ports)
       list.append(port.friendName);

	for (int i = 0; i < list.count(); i++)
		m_config->comboBox_Ports->addItem(list.at(i), 0);

//    m_config->comboBox_Ports->addItems(list);

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

			for (int i = 0; i < usb_ports.count(); i++)
				m_config->comboBox_Ports->addItem(usb_ports.at(i), 1);

//            m_config->comboBox_Ports->addItems(usb_ports);
        }

        delete rawHidHandle;
    }

    // ********************************

//    m_config->refreshPorts->setEnabled(true);
//    m_config->comboBox_Ports->setEnabled(true);
}

QString PipXtremeGadgetWidget::getPortDevice(const QString &friendName)
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

// *************************************************************************

void PipXtremeGadgetWidget::onTelemetryStart()
{
    m_config->connectButton->setEnabled(false);
    m_config->comboBox_Ports->setEnabled(false);
}

void PipXtremeGadgetWidget::onTelemetryStop()
{
    m_config->connectButton->setEnabled(true);
    m_config->comboBox_Ports->setEnabled(true);
}

// *************************************************************************

void PipXtremeGadgetWidget::onTelemetryConnect()
{
}

void PipXtremeGadgetWidget::onTelemetryDisconnect()
{
}

// *************************************************************************

void PipXtremeGadgetWidget::onModemConnect()
{
    m_config->connectButton->setText(tr(" Disconnect "));
    m_config->comboBox_Ports->setEnabled(false);
    m_config->pushButton_ScanSpectrum->setEnabled(true);
}

void PipXtremeGadgetWidget::onModemDisconnect()
{
    m_config->connectButton->setText(tr(" Connect "));
    m_config->comboBox_Ports->setEnabled(true);
    m_config->pushButton_ScanSpectrum->setEnabled(false);
}

// *************************************************************************

void PipXtremeGadgetWidget::suspendTelemetry()
{
	Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();
	if (!cm) return;

	// Suspend telemety & polling
	cm->disconnectDevice();
	cm->suspendPolling();
}

void PipXtremeGadgetWidget::restartTelemetryPolling()
{
	Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();
	if (!cm) return;

	// Restart the polling thread
	cm->resumePolling();
}

// *************************************************************************

void PipXtremeGadgetWidget::processOutputStream()
{
    if (!m_ioDevice)
        return;

	if (!m_ioDevice->isOpen())
		return;

//    if (m_ioDevice->bytesToWrite() < TX_BUFFER_SIZE )
    {
//        m_ioDevice->write((const char*)txBuffer, dataOffset+length+CHECKSUM_LENGTH);
    }
}

void PipXtremeGadgetWidget::processInputStream()
{
    while (m_ioDevice && m_ioDevice->bytesAvailable() > 0)
    {
        quint8 tmp;
        m_ioDevice->read((char*)&tmp, 1);

//        processInputByte(tmp);
    }
}

// *************************************************************************

void PipXtremeGadgetWidget::disconnect(bool resume_polling)
{	// disconnect the comms port

	if (!m_ioDevice)
		return;

	m_ioDevice->close();

	delete m_ioDevice;
	m_ioDevice = NULL;

	m_config->connectButton->setText("Connect");
	m_config->comboBox_SerialBaudrate->setEnabled(true);
	m_config->comboBox_Ports->setEnabled(true);
	m_config->refreshPorts->setEnabled(true);

	restartTelemetryPolling();
}

void PipXtremeGadgetWidget::connectDisconnect()
{
	if (m_ioDevice)
	{	// disconnect
		disconnect(true);
		return;
	}

	// ****************
	// connect

	int device_idx = m_config->comboBox_Ports->currentIndex();
	if (device_idx < 0)
		return;

	QString device_str = m_config->comboBox_Ports->currentText().trimmed();
	if (device_str.isEmpty())
		return;

	int type = m_config->comboBox_Ports->itemData(device_idx).toInt();

//	qDebug() << QString::number(type) << ": " << device_str;

	// Suspend telemety & polling in case it is not done yet
	suspendTelemetry();

	switch (type)
	{
		case 0:		// serial port
			{
				QString str = getPortDevice(device_str);
				if (str.isEmpty())
					break;

				int br_idx = m_config->comboBox_SerialBaudrate->currentIndex();
				if (br_idx < 0)
					break;

				int baudrate = m_config->comboBox_SerialBaudrate->itemData(br_idx).toInt();

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

				QextSerialPort *serial_dev = new QextSerialPort(str, settings, QextSerialPort::Polling);
				if (!serial_dev)
					break;

				if (!serial_dev->open(QIODevice::ReadWrite | QIODevice::Unbuffered))
				{
					delete serial_dev;
					break;
				}

				m_ioDevice = serial_dev;

				m_config->connectButton->setText("Disconnect");
				m_config->comboBox_SerialBaudrate->setEnabled(false);
				m_config->comboBox_Ports->setEnabled(false);
				m_config->refreshPorts->setEnabled(false);
				return;
			}

		case 1:		// USB port
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

				m_config->connectButton->setText("Disconnect");
				m_config->comboBox_SerialBaudrate->setEnabled(false);
				m_config->comboBox_Ports->setEnabled(false);
				m_config->refreshPorts->setEnabled(false);
				return;
			}

		default:	// unknown
			break;
	}

	restartTelemetryPolling();
}

// *************************************************************************

// Ask the modem to go into API mode
void PipXtremeGadgetWidget::goToAPIMode(UAVObject* callerObj, bool success)
{
    Q_UNUSED(callerObj);
/*
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVObject *fwIAP = dynamic_cast<UAVDataObject*>(objManager->getObject(QString("FirmwareIAPObj")));

    switch (currentStep)
    {
        case IAP_STATE_READY:
            getPorts(); // Useful in case a new serial port appeared since the initial list,
                          // otherwise we won't find it when we stop the board.

            // The board is running, send the 1st IAP Reset order:
            fwIAP->getField("Command")->setValue("1122");
            connect(fwIAP,SIGNAL(transactionCompleted(UAVObject*,bool)),this,SLOT(goToAPIMode(UAVObject*, bool)));
            currentStep = IAP_STATE_STEP_1;
            fwIAP->updated();

            break;

        case IAP_STATE_STEP_1:
            if (!success)
            {
                currentStep = IAP_STATE_READY;
                disconnect(fwIAP, SIGNAL(transactionCompleted(UAVObject*,bool)),this,SLOT(goToAPIMode(UAVObject*, bool)));
                break;
            }

            delay::msleep(600);

            fwIAP->getField("Command")->setValue("2233");
            currentStep = IAP_STATE_READY;
            fwIAP->updated();

            break;
    }
*/
}

// Tells the system to boot (from Bootloader state)
void PipXtremeGadgetWidget::systemBoot()
{
/*
    // Suspend telemety & polling in case it is not done yet
    Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();
    cm->disconnectDevice();
    cm->suspendPolling();

    QString devName = m_config->comboBox_Ports->currentText();
    repaint();

    if (!dfu)
    {
        if (devName == "USB")
            dfu = new DFUObject(DFU_DEBUG, false, QString());
        else
            dfu = new DFUObject(DFU_DEBUG,true,getPortDevice(devName));
    }
    dfu->AbortOperation();
    if (!dfu->enterDFU(0))
    {
        delete dfu;
        dfu = NULL;

        return;
    }
    dfu->JumpToApp();
    // Restart the polling thread
    cm->resumePolling();

    m_config->comboBox_Ports->setEnabled(true);
    if (currentStep == IAP_STATE_BOOTLOADER )
    {
    }
    currentStep = IAP_STATE_READY;
    delete dfu; // Frees up the USB/Serial port too
    dfu = NULL;
*/
}

// ***************************************************************************************

// Shows a message box with an error string.
void PipXtremeGadgetWidget::error(QString errorString, int errorNumber)
{
    Q_UNUSED(errorNumber);
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(errorString + " [" + QString::number(errorNumber) + "]");
    msgBox.exec();
}
