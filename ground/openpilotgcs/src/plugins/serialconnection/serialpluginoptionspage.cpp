/**
 ******************************************************************************
 *
 * @file       serialpluginoptionspage.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup SerialPlugin Serial Connection Plugin
 * @{
 * @brief Impliments serial connection to the flight hardware for Telemetry
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

#include "serialpluginoptionspage.h"
#include "serialpluginconfiguration.h"
#include "ui_serialpluginoptions.h"
#include "extensionsystem/pluginmanager.h"

SerialPluginOptionsPage::SerialPluginOptionsPage(SerialPluginConfiguration *config, QObject *parent) :
        IOptionsPage(parent),
        m_config(config)
{
}

//creates options page widget (uses the UI file)
QWidget *SerialPluginOptionsPage::createPage(QWidget *parent)
{

    Q_UNUSED(parent);
    options_page = new Ui::SerialPluginOptionsPage();
    //main widget
    QWidget *optionsPageWidget = new QWidget;
    //main layout
    options_page->setupUi(optionsPageWidget);
    QStringList allowedSpeeds;
    allowedSpeeds<<"1200"
#ifdef Q_OS_UNIX
                 <<"1800"              //POSIX ONLY
#endif
                 <<"2400"
                 <<"4800"
                 <<"9600"
#ifdef Q_OS_WIN
                 <<"14400"             //WINDOWS ONLY
#endif
                 <<"19200"
                 <<"38400"
#ifdef Q_OS_WIN
                 <<"56000"             //WINDOWS ONLY
#endif
                 <<"57600"
#ifdef Q_OS_UNIX
                 <<"76800"             //POSIX ONLY
#endif
                 <<"115200"
#ifdef Q_OS_WIN
                 <<"128000"            //WINDOWS ONLY
                 <<"230400"            //WINDOWS ONLY
                 <<"256000"            //WINDOWS ONLY
                 <<"460800"            //WINDOWS ONLY
                 <<"921600"            //WINDOWS ONLY
#endif
                   ;


    options_page->cb_speed->addItems(allowedSpeeds);
    options_page->cb_speed->setCurrentIndex(options_page->cb_speed->findText(m_config->speed()));
    return optionsPageWidget;
}

/**
 * Called when the user presses apply or OK.
 *
 * Saves the current values
 *
 */
void SerialPluginOptionsPage::apply()
{
    m_config->setSpeed(options_page->cb_speed->currentText());
    m_config->savesettings();
}



void SerialPluginOptionsPage::finish()
{
    delete options_page;
}
