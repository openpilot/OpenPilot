/**
 ******************************************************************************
 *
 * @file       uploadergadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Uploader Plugin Gadget options page
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   uploaderplugin
 * @{
 *
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

#include "uploadergadgetoptionspage.h"
#include "uploadergadgetconfiguration.h"
#include <QtGui/QLabel>
#include <QtGui/QSpinBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QVBoxLayout>
#include <QtGui/QTextEdit>
#include <QtGui/QComboBox>
#include <QtAlgorithms>
#include <QStringList>


UploaderGadgetOptionsPage::UploaderGadgetOptionsPage(UploaderGadgetConfiguration *config, QObject *parent) :
        IOptionsPage(parent),
        m_config(config)
{
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
//creates options page widget
QWidget *UploaderGadgetOptionsPage::createPage(QWidget *parent)
{
    //main widget
    QWidget *widget = new QWidget;
    //main layout
    QVBoxLayout *vl = new QVBoxLayout();
    widget->setLayout(vl);

    //port layout and widget
    QHBoxLayout *portLayout = new QHBoxLayout();
    QWidget *x = new QWidget;
    x->setLayout(portLayout);
    QWidget *label = new QLabel("Port:");
    m_portCB = new QComboBox(parent);
    m_portCB->setMinimumSize(200,22);
    portLayout->addWidget(label);
    portLayout->addWidget(m_portCB);

    //port speed layout and widget
    QHBoxLayout *speedLayout = new QHBoxLayout();
    QWidget *y = new QWidget;
    y->setLayout(speedLayout);
    label = new QLabel("Port Speed:");
    m_speedCB = new QComboBox();
    m_speedCB->setMinimumSize(200,22);
    speedLayout->addWidget(label);
    speedLayout->addWidget(m_speedCB);

    //flow control layout and widget
    QHBoxLayout *flowLayout = new QHBoxLayout();
    QWidget *z = new QWidget;
    z->setLayout(flowLayout);
    label = new QLabel("Flow Control:");
    m_flowCB = new QComboBox();
    m_flowCB->setMinimumSize(200,22);
    flowLayout->addWidget(label);
    flowLayout->addWidget(m_flowCB);

    //databits layout and widget
    QHBoxLayout *databitsLayout = new QHBoxLayout();
    QWidget *a = new QWidget;
    a->setLayout(databitsLayout);
    label = new QLabel("Data Bits:");
    m_databitsCB = new QComboBox();
    m_databitsCB->setMinimumSize(200,22);
    databitsLayout->addWidget(label);
    databitsLayout->addWidget(m_databitsCB);

    //stopbits layout and widget
    QHBoxLayout *stopbitsLayout = new QHBoxLayout();
    QWidget *b = new QWidget;
    b->setLayout(stopbitsLayout);
    label = new QLabel("Stop Bits:");
    m_stopbitsCB = new QComboBox();
    m_stopbitsCB->setMinimumSize(200,22);
    stopbitsLayout->addWidget(label);
    stopbitsLayout->addWidget(m_stopbitsCB);

    //parity layout and widget
    QHBoxLayout *parityLayout = new QHBoxLayout();
    QWidget *c = new QWidget;
    c->setLayout(parityLayout);
    label = new QLabel("Parity:");
    m_parityCB = new QComboBox();
    m_parityCB->setMinimumSize(200,22);
    parityLayout->addWidget(label);
    parityLayout->addWidget(m_parityCB);

    //timeout layout and widget
    QHBoxLayout *timeoutLayout = new QHBoxLayout();
    QWidget *d = new QWidget;
    d->setLayout(timeoutLayout);
    label = new QLabel("TimeOut(ms):");
    m_timeoutSpin = new QSpinBox();
    m_timeoutSpin->setMaximum(100000);
    m_timeoutSpin->setMinimumSize(200,22);
    timeoutLayout->addWidget(label);
    timeoutLayout->addWidget(m_timeoutSpin);


    QSpacerItem *spacer = new QSpacerItem(100, 100, QSizePolicy::Expanding, QSizePolicy::Expanding);

    //add partial widget to main widget
    vl->addWidget(x);
    vl->addWidget(y);
    vl->addWidget(z);
    vl->addWidget(a);
    vl->addWidget(b);
    vl->addWidget(c);
    vl->addWidget(d);
    vl->addSpacerItem(spacer);

    //clears comboboxes, if not every time the user enters options page the lists
    //duplicate
    m_portCB->clear();
    m_speedCB->clear();
    m_databitsCB->clear();
    m_flowCB->clear();
    m_parityCB->clear();
    m_stopbitsCB->clear();

    //gets available serial ports
    QList<QextPortInfo> ports =QextSerialEnumerator ::getPorts();
    qSort(ports.begin(), ports.end(),sortPorts);
    qDebug() << "List of ports:";
    for (int i = 0; i < ports.size(); i++) {
        qDebug() << "port name:" << ports.at(i).portName;
        qDebug() << "friendly name:" << ports.at(i).friendName;
        qDebug() << "physical name:" << ports.at(i).physName;
        qDebug() << "enumerator name:" << ports.at(i).enumName;
        qDebug() << "vendor ID:" << QString::number(ports.at(i).vendorID, 16);
        qDebug() << "product ID:" << QString::number(ports.at(i).productID, 16);
        qDebug() << "===================================";
    }
#ifdef Q_OS_WIN
    //on windows populate ports combobox with ports name
    for (int i = 0; i < ports.size(); i++) {
        m_portCB->addItem((QString)ports.at(i).portName.toLocal8Bit().constData());
    }
#else
    //on other OS's populate ports combobox with ports physical name
    for (int i = 0; i < ports.size(); i++) {
        m_portCB->addItem((QString)ports.at(i).physName.toLocal8Bit().constData());
    }
#endif
    //The next selections of comboboxe's saved value are ugly as hell
    //There must be a better wat for doing this.
    //select saved port
    if(m_portCB->findText(m_config->Port())!=-1){
        m_portCB->setCurrentIndex(m_portCB->findText(m_config->Port()));
    }
    //populate serial speed combobox
    for (int i=0;i<BaudRateTypeString.size();i++){
        m_speedCB->addItem(BaudRateTypeString.at(i).toLocal8Bit().constData() );
    }
    //select saved speed
    if(m_speedCB->findText(BaudRateTypeStringALL.at((int)m_config->Speed()).toLocal8Bit().constData())!=-1){
        m_speedCB->setCurrentIndex(m_speedCB->findText(BaudRateTypeStringALL.at((int)m_config->Speed()).toLocal8Bit().constData()));
    }
    //populate databits combobox
    for (int i=0;i<DataBitsTypeString.size();i++){
        m_databitsCB->addItem(DataBitsTypeString.at(i).toLocal8Bit().constData() );
    }
    //select saved databits
    if(m_databitsCB->findText(DataBitsTypeStringALL.at((int)m_config->DataBits()).toLocal8Bit().constData())!=-1){
        m_databitsCB->setCurrentIndex(m_databitsCB->findText(DataBitsTypeStringALL.at((int)m_config->DataBits()).toLocal8Bit().constData()));
    }
    //populate parity combobox
    for (int i=0;i<ParityTypeString.size();i++){
        m_parityCB->addItem(ParityTypeString.at(i).toLocal8Bit().constData() );
    }
    //select saved parity
    if(m_parityCB->findText(ParityTypeStringALL.at((int)m_config->Parity()).toLocal8Bit().constData())!=-1){
        m_parityCB->setCurrentIndex(m_parityCB->findText(ParityTypeStringALL.at((int)m_config->Parity()).toLocal8Bit().constData()));
    }
    //populate stopbits combobox
    for (int i=0;i<StopBitsTypeString.size();i++){
        m_stopbitsCB->addItem(StopBitsTypeString.at(i).toLocal8Bit().constData() );
    }
    //select saved stopbits
    if(m_stopbitsCB->findText(StopBitsTypeStringALL.at((int)m_config->StopBits()).toLocal8Bit().constData())!=-1){
        m_stopbitsCB->setCurrentIndex(m_stopbitsCB->findText(StopBitsTypeStringALL.at((int)m_config->StopBits()).toLocal8Bit().constData()));
    }
    //populate flow control combobox
    for (int i=0;i<FlowTypeString.size();i++){
        m_flowCB->addItem(FlowTypeString.at(i).toLocal8Bit().constData() );
    }
    //select saved flow control
    if(m_flowCB->findText(FlowTypeString.at((int)m_config->Flow()).toLocal8Bit().constData())!=-1){
        m_flowCB->setCurrentIndex(m_flowCB->findText(FlowTypeString.at((int)m_config->Flow()).toLocal8Bit().constData()));
    }
    //fill time out spinbox with saved value
    m_timeoutSpin->setValue(m_config->TimeOut());
    return widget;
}
/**
 * Called when the user presses apply or OK.
 *
 * Saves the current values
 *
 */
void UploaderGadgetOptionsPage::apply()
{
    m_config->setPort(m_portCB->currentText());
    m_config->setSpeed((BaudRateType)BaudRateTypeStringALL.indexOf(m_speedCB->currentText()));
    m_config->setDataBits((DataBitsType)DataBitsTypeStringALL.indexOf(m_databitsCB->currentText()));
    m_config->setStopBits((StopBitsType)StopBitsTypeStringALL.indexOf(m_stopbitsCB->currentText()));
    m_config->setParity((ParityType)ParityTypeStringALL.indexOf(m_parityCB->currentText()));
    m_config->setFlow((FlowType)FlowTypeString.indexOf(m_flowCB->currentText()));
    m_config->setTimeOut( m_timeoutSpin->value());

}

void UploaderGadgetOptionsPage::finish()
{

}


