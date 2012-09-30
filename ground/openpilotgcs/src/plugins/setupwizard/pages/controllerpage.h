/**
 ******************************************************************************
 *
 * @file       controllerpage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup ControllerPage
 * @{
 * @brief
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

#ifndef CONTROLLERPAGE_H
#define CONTROLLERPAGE_H

#include <coreplugin/icore.h>
#include <coreplugin/connectionmanager.h>
#include "setupwizard.h"
#include "uavtalk/telemetrymanager.h"
#include "abstractwizardpage.h"

namespace Ui {
class ControllerPage;
}

class ControllerPage : public AbstractWizardPage
{
    Q_OBJECT
    
public:
    explicit ControllerPage(SetupWizard *wizard, QWidget *parent = 0);
    ~ControllerPage();
    void initializePage();
    bool isComplete() const;
    bool validatePage();
    
private:
    Ui::ControllerPage *ui;
    bool anyControllerConnected();
    SetupWizard::CONTROLLER_TYPE getControllerType();
    void setupDeviceList();
    void setupBoardTypes();
    void setControllerType(SetupWizard::CONTROLLER_TYPE type);
    Core::ConnectionManager *m_connectionManager;
    TelemetryManager *m_telemtryManager;

private slots:
    void devicesChanged(QLinkedList<Core::DevListItem> devices);
    void connectionStatusChanged();
    void connectDisconnect();
};

#endif // CONTROLLERPAGE_H
