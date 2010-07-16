/**
 ******************************************************************************
 *
 * @file       uploadergadget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup YModemUploader YModem Serial Uploader Plugin
 * @{
 * @brief The YModem protocol serial uploader plugin
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
#include "uploadergadget.h"
#include "uploadergadgetwidget.h"
#include "uploadergadgetconfiguration.h"
#include <qextserialport/src/qextserialport.h>
#include <qextserialport/src/qextserialenumerator.h>

UploaderGadget::UploaderGadget(QString classId,  UploaderGadgetWidget *widget, QWidget *parent) :
        IUAVGadget(classId, parent),
        m_widget(widget)
{
}

UploaderGadget::~UploaderGadget()
{

}
/**
 * Loads a configuration.
 *
 */
void UploaderGadget::loadConfiguration(IUAVGadgetConfiguration* config)
{
    UploaderGadgetConfiguration *m = qobject_cast< UploaderGadgetConfiguration*>(config);
    PortSettings portsettings;
    portsettings.BaudRate=m->Speed();
    portsettings.DataBits=m->DataBits();
    portsettings.FlowControl=m->Flow();
    portsettings.Parity=m->Parity();
    portsettings.StopBits=m->StopBits();
    portsettings.Timeout_Millisec=m->TimeOut();
    //Creates new serial port with the user configuration and passes it to the widget
    QextSerialPort *port=new QextSerialPort(m->Port(),portsettings,QextSerialPort::Polling);
    m_widget->setPort(port);
}

