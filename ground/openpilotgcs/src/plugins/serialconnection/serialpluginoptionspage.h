/**
 ******************************************************************************
 *
 * @file       serialpluginoptionspage.h
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

#ifndef SERIALpluginOPTIONSPAGE_H
#define SERIALpluginOPTIONSPAGE_H

#include "coreplugin/dialogs/ioptionspage.h"
#include "QString"
#include <QStringList>
#include <QDebug>
#include <QFont>

namespace Core {
class IUAVpluginConfiguration;
}

class SerialPluginConfiguration;

namespace Ui {
    class SerialPluginOptionsPage;
}

using namespace Core;

class SerialPluginOptionsPage : public IOptionsPage
{
Q_OBJECT
public:
    explicit SerialPluginOptionsPage(SerialPluginConfiguration *config, QObject *parent = 0);

    QString id() const { return QLatin1String("settings"); }
    QString trName() const { return tr("settings"); }
    QString category() const { return "Serial Telemetry"; }
    QString trCategory() const { return "Serial Telemetry"; }
    QWidget *createPage(QWidget *parent);
    void apply();
    void finish();

private:
    Ui::SerialPluginOptionsPage *options_page;
    SerialPluginConfiguration *m_config;


};

#endif // SERIALpluginOPTIONSPAGE_H
