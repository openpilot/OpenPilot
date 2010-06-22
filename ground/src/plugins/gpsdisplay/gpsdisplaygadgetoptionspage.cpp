/**
 ******************************************************************************
 *
 * @file       gpsdisplaygadgetoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      GPS Display Plugin Gadget options page
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   gpsdisplay
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

#include "gpsdisplaygadgetoptionspage.h"
#include "gpsdisplaygadgetconfiguration.h"
#include "ui_gpsdisplaygadgetoptionspage.h"

#include <QFileDialog>
#include <QtAlgorithms>
#include <QStringList>

GpsDisplayGadgetOptionsPage::GpsDisplayGadgetOptionsPage(GpsDisplayGadgetConfiguration *config, QObject *parent) :
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
QWidget *GpsDisplayGadgetOptionsPage::createPage(QWidget *parent)
{

    options_page = new Ui::GpsDisplayGadgetOptionsPage();
    //main widget
    QWidget *optionsPageWidget = new QWidget;
    //main layout
    options_page->setupUi(optionsPageWidget);

    // Restore the contents from the settings:
    options_page->svgFilePathChooser->setExpectedKind(Utils::PathChooser::File);
    options_page->svgFilePathChooser->setPromptDialogFilter(tr("SVG image (*.svg)"));
    options_page->svgFilePathChooser->setPromptDialogTitle(tr("Choose SVG image"));
    options_page->svgFilePathChooser->setPath(m_config->getSystemFile());

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
    m_config->setSystemFile(options_page->svgFilePathChooser->path());
}


void GpsDisplayGadgetOptionsPage::finish()
{
    delete options_page;
}
