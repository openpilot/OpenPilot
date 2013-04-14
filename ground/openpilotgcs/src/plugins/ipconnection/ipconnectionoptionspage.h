/**
 ******************************************************************************
 *
 * @file       IPconnectionoptionspage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup IPConnPlugin IP Telemetry Plugin
 * @{
 * @brief IP Connection Plugin impliment telemetry over TCP/IP and UDP/IP
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

#ifndef IPconnectionOPTIONSPAGE_H
#define IPconnectionOPTIONSPAGE_H

#include "coreplugin/dialogs/ioptionspage.h"

class IPconnectionConfiguration;

namespace Core {
    class IUAVGadgetConfiguration;
}

namespace Ui {
    class IPconnectionOptionsPage;
}

using namespace Core;

class IPconnectionOptionsPage : public IOptionsPage
{
Q_OBJECT
public:
    explicit IPconnectionOptionsPage(IPconnectionConfiguration *config, QObject *parent = 0);
    virtual ~IPconnectionOptionsPage();

    QString id() const { return QLatin1String("settings"); }
    QString trName() const { return tr("settings"); }
    QString category() const { return "IP Network Telemetry"; };
    QString trCategory() const { return "IP Network Telemetry"; };

    QWidget *createPage(QWidget *parent);
    void apply();
    void finish();

signals:
    void availableDevChanged();

public slots:
private:
    IPconnectionConfiguration *m_config;
    Ui::IPconnectionOptionsPage *m_page;

};

#endif // IPconnectionOPTIONSPAGE_H
