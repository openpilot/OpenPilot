/**
 ******************************************************************************
 *
 * @file       AntennaTracgadgetoptionspage.cpp
 * @author     Sami Korhonen & the OpenPilot team Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup AntennaTrackGadgetPlugin Antenna Track Gadget Plugin
 * @{
 * @brief A gadget that communicates with antenna tracker and enables basic configuration
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

#include "antennatrackgadgetoptionspage.h"
#include "antennatrackgadgetconfiguration.h"
#include "ui_antennatrackgadgetoptionspage.h"

#include <QFileDialog>
#include <QtAlgorithms>
#include <QStringList>

AntennaTrackGadgetOptionsPage::AntennaTrackGadgetOptionsPage(AntennaTrackGadgetConfiguration *config, QObject *parent) :
        IOptionsPage(parent),
        m_config(config)
{
//  Taken from the uploader gadget, since we also can use a serial port for this
//    Gadget

    //the begining of some ugly code
//diferent OS's have diferent serial port capabilities
#ifdef Q_OS_WIN
//load windows port capabilities
BaudRateTypeString
        <<"BAUD110"
        <<"BAUD300"
        <<"BAUD600"
        <<"BAUD1200"
        <<"BAUD2400"
        <<"BAUD4800"
        <<"BAUD9600"
        <<"BAUD14400"
        <<"BAUD19200"
        <<"BAUD38400"
        <<"BAUD56000"
        <<"BAUD57600"
        <<"BAUD115200"
        <<"BAUD128000"
        <<"BAUD256000";
DataBitsTypeString
        <<"DATA_5"
        <<"DATA_6"
        <<"DATA_7"
        <<"DATA_8";
ParityTypeString
        <<"PAR_NONE"
        <<"PAR_ODD"
        <<"PAR_EVEN"
        <<"PAR_MARK"               //WINDOWS ONLY
        <<"PAR_SPACE";
StopBitsTypeString
        <<"STOP_1"
        <<"STOP_1_5"               //WINDOWS ONLY
        <<"STOP_2";
#else
//load POSIX port capabilities
BaudRateTypeString

        <<"BAUD50"                //POSIX ONLY
        <<"BAUD75"                //POSIX ONLY
        <<"BAUD110"
        <<"BAUD134"               //POSIX ONLY
        <<"BAUD150"               //POSIX ONLY
        <<"BAUD200"             //POSIX ONLY
        <<"BAUD300"
        <<"BAUD600"
        <<"BAUD1200"
        <<"BAUD1800"            //POSIX ONLY
        <<"BAUD2400"
        <<"BAUD4800"
        <<"BAUD9600"
        <<"BAUD19200"
        <<"BAUD38400"
        <<"BAUD57600"
        <<"BAUD76800"             //POSIX ONLY
        <<"BAUD115200";
DataBitsTypeString
        <<"DATA_5"
        <<"DATA_6"
        <<"DATA_7"
        <<"DATA_8";
ParityTypeString
        <<"PAR_NONE"
        <<"PAR_ODD"
        <<"PAR_EVEN"
        <<"PAR_SPACE";
StopBitsTypeString
        <<"STOP_1"
        <<"STOP_2";
#endif
//load all OS's capabilities
BaudRateTypeStringALL
        <<"BAUD50"                //POSIX ONLY
        <<"BAUD75"                //POSIX ONLY
        <<"BAUD110"
        <<"BAUD134"               //POSIX ONLY
        <<"BAUD150"               //POSIX ONLY
        <<"BAUD200"             //POSIX ONLY
        <<"BAUD300"
        <<"BAUD600"
        <<"BAUD1200"
        <<"BAUD1800"            //POSIX ONLY
        <<"BAUD2400"
        <<"BAUD4800"
        <<"BAUD9600"
        <<"BAUD14400"
        <<"BAUD19200"
        <<"BAUD38400"
        <<"BAUD56000"
        <<"BAUD57600"
        <<"BAUD76800"             //POSIX ONLY
        <<"BAUD115200"
        <<"BAUD128000"
        <<"BAUD256000";
DataBitsTypeStringALL
        <<"DATA_5"
        <<"DATA_6"
        <<"DATA_7"
        <<"DATA_8";
ParityTypeStringALL
        <<"PAR_NONE"
        <<"PAR_ODD"
        <<"PAR_EVEN"
        <<"PAR_MARK"               //WINDOWS ONLY
        <<"PAR_SPACE";
StopBitsTypeStringALL
        <<"STOP_1"
        <<"STOP_1_5"               //WINDOWS ONLY
        <<"STOP_2";

FlowTypeString
        <<"FLOW_OFF"
        <<"FLOW_HARDWARE"
        <<"FLOW_XONXOFF";
}
bool sortPorts(QextPortInfo const& s1,QextPortInfo const& s2)
{
return s1.portName<s2.portName;
}


//creates options page widget (uses the UI file)
QWidget *AntennaTrackGadgetOptionsPage::createPage(QWidget *parent)
{
    options_page = new Ui::AntennaTrackGadgetOptionsPage();
    QWidget *optionsPageWidget = new QWidget;
    options_page->setupUi(optionsPageWidget);


    // PORTS
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    qSort(ports.begin(), ports.end(),sortPorts);
    foreach( QextPortInfo port, ports ) {
        qDebug() << "Adding port: " << port.friendName << " (" << port.portName << ")";
        options_page->portComboBox->addItem(port.friendName, port.friendName);
    }

    int portIndex = options_page->portComboBox->findData(m_config->port());
    if(portIndex!=-1){
        qDebug() << "createPage(): port is " << m_config->port();
        options_page->portComboBox->setCurrentIndex(portIndex);
    }

    // BAUDRATES
    options_page->portSpeedComboBox->addItems(BaudRateTypeString);

    int portSpeedIndex = options_page->portSpeedComboBox->findText(BaudRateTypeStringALL.at((int)m_config->speed()));
    if(portSpeedIndex != -1){
       options_page->portSpeedComboBox->setCurrentIndex(portSpeedIndex);
    }

    // FLOW CONTROL
    options_page->flowControlComboBox->addItems(FlowTypeString);

    int flowControlIndex = options_page->flowControlComboBox->findText(FlowTypeString.at((int)m_config->flow()));
    if(flowControlIndex != -1){
       options_page->flowControlComboBox->setCurrentIndex(flowControlIndex);
    }

    // DATABITS
    options_page->dataBitsComboBox->addItems(DataBitsTypeString);

    int dataBitsIndex = options_page->dataBitsComboBox->findText(DataBitsTypeStringALL.at((int)m_config->dataBits()));
    if(dataBitsIndex != -1){
       options_page->dataBitsComboBox->setCurrentIndex(dataBitsIndex);
    }

    // STOPBITS
    options_page->stopBitsComboBox->addItems(StopBitsTypeString);

    int stopBitsIndex = options_page->stopBitsComboBox->findText(StopBitsTypeStringALL.at((int)m_config->stopBits()));
    if(stopBitsIndex != -1){
       options_page->stopBitsComboBox->setCurrentIndex(stopBitsIndex);
    }

    // PARITY
    options_page->parityComboBox->addItems(ParityTypeString);

    int parityIndex = options_page->parityComboBox->findText(ParityTypeStringALL.at((int)m_config->parity()));
    if(parityIndex != -1){
       options_page->parityComboBox->setCurrentIndex(parityIndex);
    }

    // TIMEOUT
    options_page->timeoutSpinBox->setValue(m_config->timeOut());

    QStringList connectionModes;
    connectionModes << "Serial";
    options_page->connectionMode->addItems(connectionModes);
    int conMode = options_page->connectionMode->findText(m_config->connectionMode());
    if (conMode != -1)
       options_page->connectionMode->setCurrentIndex(conMode);


    return optionsPageWidget;
}

/**
 * Called when the user presses apply or OK.
 *
 * Saves the current values
 *
 */
void AntennaTrackGadgetOptionsPage::apply()
{
    int portIndex = options_page->portComboBox->currentIndex();
    m_config->setPort(options_page->portComboBox->itemData(portIndex).toString());
    qDebug() << "apply(): port is " << m_config->port();

    m_config->setSpeed((BaudRateType)BaudRateTypeStringALL.indexOf(options_page->portSpeedComboBox->currentText()));
    m_config->setFlow((FlowType)FlowTypeString.indexOf(options_page->flowControlComboBox->currentText()));
    m_config->setDataBits((DataBitsType)DataBitsTypeStringALL.indexOf(options_page->dataBitsComboBox->currentText()));
    m_config->setStopBits((StopBitsType)StopBitsTypeStringALL.indexOf(options_page->stopBitsComboBox->currentText()));
    m_config->setParity((ParityType)ParityTypeStringALL.indexOf(options_page->parityComboBox->currentText()));
    m_config->setTimeOut( options_page->timeoutSpinBox->value());
    m_config->setConnectionMode(options_page->connectionMode->currentText());

}

void AntennaTrackGadgetOptionsPage::finish()
{
    delete options_page;
}
