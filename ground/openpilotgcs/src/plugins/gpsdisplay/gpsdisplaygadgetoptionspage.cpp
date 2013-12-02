/**
 ******************************************************************************
 *
 * @file       gpsdisplaygadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GPSGadgetPlugin GPS Gadget Plugin
 * @{
 * @brief A gadget that displays GPS status and enables basic configuration
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

#include "gpsdisplaygadgetoptionspage.h"
#include "gpsdisplaygadgetconfiguration.h"
#include "ui_gpsdisplaygadgetoptionspage.h"

#include <QFileDialog>
#include <QtAlgorithms>
#include <QStringList>

GpsDisplayGadgetOptionsPage::GpsDisplayGadgetOptionsPage(GpsDisplayGadgetConfiguration *config, QObject *parent) :
    IOptionsPage(parent),
    m_config(config)
{}

bool sortPorts(QSerialPortInfo const & s1, QSerialPortInfo const & s2)
{
    return s1.portName() < s2.portName();
}

// creates options page widget (uses the UI file)
QWidget *GpsDisplayGadgetOptionsPage::createPage(QWidget *parent)
{
    Q_UNUSED(parent);

    options_page = new Ui::GpsDisplayGadgetOptionsPage();
    QWidget *optionsPageWidget = new QWidget;
    options_page->setupUi(optionsPageWidget);


    // PORTS
    QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
    qSort(ports.begin(), ports.end(), sortPorts);
    foreach(QSerialPortInfo port, ports) {
        qDebug() << "Adding port: " << port.systemLocation() << " (" << port.portName() << ")";
        options_page->portComboBox->addItem(port.portName(), port.portName());
    }

    int portIndex = options_page->portComboBox->findData(m_config->port());
    if (portIndex != -1) {
        qDebug() << "createPage(): port is " << m_config->port();
        options_page->portComboBox->setCurrentIndex(portIndex);
    }

    // BAUDRATES
    options_page->portSpeedComboBox->addItem("Baud1200", QSerialPort::Baud1200);
    options_page->portSpeedComboBox->addItem("Baud2400", QSerialPort::Baud2400);
    options_page->portSpeedComboBox->addItem("Baud4800", QSerialPort::Baud4800);
    options_page->portSpeedComboBox->addItem("Baud9600", QSerialPort::Baud9600);
    options_page->portSpeedComboBox->addItem("Baud19200", QSerialPort::Baud19200);
    options_page->portSpeedComboBox->addItem("Baud38400", QSerialPort::Baud38400);
    options_page->portSpeedComboBox->addItem("Baud57600", QSerialPort::Baud57600);
    options_page->portSpeedComboBox->addItem("Baud115200", QSerialPort::Baud115200);
    options_page->portSpeedComboBox->addItem("UnknownBaud", QSerialPort::UnknownBaud);

    int portSpeedIndex = options_page->portSpeedComboBox->findData(m_config->speed());
    if (portSpeedIndex != -1) {
        options_page->portSpeedComboBox->setCurrentIndex(portSpeedIndex);
    }

    // FLOW CONTROL
    options_page->flowControlComboBox->addItem("NoFlowControl", QSerialPort::NoFlowControl);
    options_page->flowControlComboBox->addItem("HardwareControl", QSerialPort::HardwareControl);
    options_page->flowControlComboBox->addItem("SoftwareControl", QSerialPort::SoftwareControl);
    options_page->flowControlComboBox->addItem("UnknownFlowControl", QSerialPort::UnknownFlowControl);

    int flowControlIndex = options_page->flowControlComboBox->findData(m_config->flow());
    if (flowControlIndex != -1) {
        options_page->flowControlComboBox->setCurrentIndex(flowControlIndex);
    }

    // DATABITS
    options_page->dataBitsComboBox->addItem("Data5", QSerialPort::Data5);
    options_page->dataBitsComboBox->addItem("Data6", QSerialPort::Data6);
    options_page->dataBitsComboBox->addItem("Data7", QSerialPort::Data7);
    options_page->dataBitsComboBox->addItem("Data8", QSerialPort::Data8);
    options_page->dataBitsComboBox->addItem("UnknownDataBits", QSerialPort::UnknownDataBits);

    int dataBitsIndex = options_page->dataBitsComboBox->findData(m_config->dataBits());
    if (dataBitsIndex != -1) {
        options_page->dataBitsComboBox->setCurrentIndex(dataBitsIndex);
    }

    // STOPBITS
    options_page->stopBitsComboBox->addItem("OneStop", QSerialPort::OneStop);
    options_page->stopBitsComboBox->addItem("OneAndHalfStop", QSerialPort::OneAndHalfStop);
    options_page->stopBitsComboBox->addItem("TwoStop", QSerialPort::TwoStop);
    options_page->stopBitsComboBox->addItem("UnknownStopBits", QSerialPort::UnknownStopBits);

    int stopBitsIndex = options_page->stopBitsComboBox->findData(m_config->stopBits());
    if (stopBitsIndex != -1) {
        options_page->stopBitsComboBox->setCurrentIndex(stopBitsIndex);
    }

    // PARITY
    options_page->parityComboBox->addItem("NoParity", QSerialPort::NoParity);
    options_page->parityComboBox->addItem("EvenParity", QSerialPort::EvenParity);
    options_page->parityComboBox->addItem("OddParity", QSerialPort::OddParity);
    options_page->parityComboBox->addItem("SpaceParity", QSerialPort::SpaceParity);
    options_page->parityComboBox->addItem("MarkParity", QSerialPort::MarkParity);
    options_page->parityComboBox->addItem("UnknownParity", QSerialPort::UnknownParity);

    int parityIndex = options_page->parityComboBox->findData(m_config->parity());
    if (parityIndex != -1) {
        options_page->parityComboBox->setCurrentIndex(parityIndex);
    }

    // TIMEOUT
    options_page->timeoutSpinBox->setValue(m_config->timeOut());

    QStringList connectionModes;
    connectionModes << "Serial" << "Network" << "Telemetry";
    options_page->connectionMode->addItems(connectionModes);
    int conMode = options_page->connectionMode->findText(m_config->connectionMode());
    if (conMode != -1) {
        options_page->connectionMode->setCurrentIndex(conMode);
    }


    return optionsPageWidget;
}

/**
 * Called when the user presses apply or OK.
 *
 * Saves the current values
 *
 */
void GpsDisplayGadgetOptionsPage::apply()
{
    int portIndex = options_page->portComboBox->currentIndex();

    m_config->setPort(options_page->portComboBox->itemData(portIndex).toString());
    qDebug() << "apply(): port is " << m_config->port();

    m_config->setSpeed((QSerialPort::BaudRate)options_page->portSpeedComboBox->itemData(options_page->portSpeedComboBox->currentIndex()).toInt());
    m_config->setFlow((QSerialPort::FlowControl)options_page->flowControlComboBox->itemData(options_page->flowControlComboBox->currentIndex()).toInt());
    m_config->setDataBits((QSerialPort::DataBits)options_page->dataBitsComboBox->itemData(options_page->dataBitsComboBox->currentIndex()).toInt());
    m_config->setStopBits((QSerialPort::StopBits)options_page->stopBitsComboBox->itemData(options_page->stopBitsComboBox->currentIndex()).toInt());
    m_config->setParity((QSerialPort::Parity)options_page->parityComboBox->itemData(options_page->parityComboBox->currentIndex()).toInt());
    m_config->setTimeOut(options_page->timeoutSpinBox->value());
    m_config->setConnectionMode(options_page->connectionMode->currentText());
}

void GpsDisplayGadgetOptionsPage::finish()
{
    delete options_page;
}
