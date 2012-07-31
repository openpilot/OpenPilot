/**
 ******************************************************************************
 *
 * @file       controllerpage.cpp
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

#include "controllerpage.h"
#include "ui_controllerpage.h"
#include "setupwizard.h"

#include <extensionsystem/pluginmanager.h>
#include <uavobjectutil/uavobjectutilmanager.h>

ControllerPage::ControllerPage(SetupWizard *wizard, QWidget *parent) :
    AbstractWizardPage(wizard, parent),
    ui(new Ui::ControllerPage)
{
    ui->setupUi(this);

    m_connectionManager = Core::ICore::instance()->connectionManager();
    Q_ASSERT(m_connectionManager);
    connect(m_connectionManager, SIGNAL(availableDevicesChanged(QLinkedList<Core::devListItem>)), this, SLOT(devicesChanged(QLinkedList<Core::devListItem>)));

    connect(m_connectionManager, SIGNAL(deviceConnected(QIODevice*)), this, SLOT(connectionStatusChanged()));
    connect(m_connectionManager, SIGNAL(deviceDisconnected()), this, SLOT(connectionStatusChanged()));

    connect(ui->manualCB, SIGNAL(toggled(bool)), this, SLOT(identificationModeChanged()));
    connect(ui->connectButton, SIGNAL(clicked()), this, SLOT(connectDisconnect()));

    setupBoardTypes();
    setupDeviceList();
}

ControllerPage::~ControllerPage()
{
    delete ui;
}

void ControllerPage::initializePage()
{
    if(anyControllerConnected()) {
        SetupWizard::CONTROLLER_TYPE type = getControllerType();
        setControllerType(type);
    }
    else {
        setControllerType(SetupWizard::CONTROLLER_UNKNOWN);
    }
    emit completeChanged();
}

bool ControllerPage::isComplete() const
{
    return (ui->manualCB->isChecked() && ui->boardTypeCombo->currentIndex() > 0) ||
            (!ui->manualCB->isChecked() && m_connectionManager->isConnected() && ui->boardTypeCombo->currentIndex() > 0);
}

bool ControllerPage::validatePage()
{
    getWizard()->setControllerType((SetupWizard::CONTROLLER_TYPE)ui->boardTypeCombo->itemData(ui->boardTypeCombo->currentIndex()).toInt());
    getWizard()->setControllerSelectionMode(ui->manualCB->isChecked() ? SetupWizard::CONTROLLER_SELECTION_MANUAL : SetupWizard::CONTROLLER_SELECTION_AUTOMATIC);
    return true;
}

bool ControllerPage::anyControllerConnected()
{
    return m_connectionManager->isConnected();
}

SetupWizard::CONTROLLER_TYPE ControllerPage::getControllerType()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectUtilManager* utilMngr = pm->getObject<UAVObjectUtilManager>();
    int id = utilMngr->getBoardModel();

    switch (id) {
    case 0x0301:
        return SetupWizard::CONTROLLER_PIPX;
    case 0x0401:
        return SetupWizard::CONTROLLER_CC;
    case 0x0402:
        return SetupWizard::CONTROLLER_CC3D;
    case 0x0901:
        return SetupWizard::CONTROLLER_REVO;
    default:
        return SetupWizard::CONTROLLER_UNKNOWN;
    }
}

void ControllerPage::setupDeviceList()
{
    devicesChanged(m_connectionManager->getAvailableDevices());
    connectionStatusChanged();
}

void ControllerPage::setupBoardTypes()
{
    QVariant v(0);
    ui->boardTypeCombo->addItem(tr("<Unknown>"), SetupWizard::CONTROLLER_UNKNOWN);
    ui->boardTypeCombo->addItem(tr("OpenPilot CopterControl"), SetupWizard::CONTROLLER_CC);
    ui->boardTypeCombo->addItem(tr("OpenPilot CopterControl 3D"), SetupWizard::CONTROLLER_CC3D);
    ui->boardTypeCombo->addItem(tr("OpenPilot Revolution"), SetupWizard::CONTROLLER_REVO);
    //ui->boardTypeCombo->model()->setData(ui->boardTypeCombo->model()->index(ui->boardTypeCombo->count() - 1, 0), v, Qt::UserRole - 1);
    ui->boardTypeCombo->addItem(tr("OpenPilot PipX Radio Modem"), SetupWizard::CONTROLLER_PIPX);
    //ui->boardTypeCombo->model()->setData(ui->boardTypeCombo->model()->index(ui->boardTypeCombo->count() - 1, 0), v, Qt::UserRole - 1);
}

void ControllerPage::setControllerType(SetupWizard::CONTROLLER_TYPE type)
{
    for(int i = 0; i < ui->boardTypeCombo->count(); ++i) {
        if(ui->boardTypeCombo->itemData(i) == type) {
            ui->boardTypeCombo->setCurrentIndex(i);
            break;
        }
    }
}

void ControllerPage::devicesChanged(QLinkedList<Core::devListItem> devices)
{
    // Get the selected item before the update if any
    QString currSelectedDeviceName = ui->deviceCombo->currentIndex() != -1 ?
                ui->deviceCombo->itemData(ui->deviceCombo->currentIndex(), Qt::ToolTipRole).toString() : "";

    // Clear the box
    ui->deviceCombo->clear();

    int indexOfSelectedItem = -1;
    int i = 0;

    // Loop and fill the combo with items from connectionmanager
    foreach (Core::devListItem device, devices)
    {
        ui->deviceCombo->addItem(device.displayName);
        QString deviceName = (const QString)device.devName;
        ui->deviceCombo->setItemData(ui->deviceCombo->count() - 1, deviceName, Qt::ToolTipRole);
        if(currSelectedDeviceName != "" && currSelectedDeviceName == deviceName) {
            indexOfSelectedItem = i;
        }
        i++;
    }

    // Re select the item that was selected before if any
    if(indexOfSelectedItem != -1) {
        ui->deviceCombo->setCurrentIndex(indexOfSelectedItem);
    }
    connectionStatusChanged();
}

void ControllerPage::connectionStatusChanged()
{
    if(m_connectionManager->isConnected()) {
        ui->deviceCombo->setEnabled(false);
        ui->connectButton->setText(tr("Disconnect"));
        ui->boardTypeCombo->setEnabled(false);
        ui->manualCB->setEnabled(false);
        QString connectedDeviceName = m_connectionManager->getCurrentDevice().devName;
        for(int i = 0; i < ui->deviceCombo->count(); ++i) {
            if(connectedDeviceName == ui->deviceCombo->itemData(i, Qt::ToolTipRole).toString()) {
                ui->deviceCombo->setCurrentIndex(i);
                break;
            }
        }

        SetupWizard::CONTROLLER_TYPE type = getControllerType();
        setControllerType(type);
    }
    else {
        ui->deviceCombo->setEnabled(true);
        ui->connectButton->setText(tr("Connect"));
        ui->boardTypeCombo->setEnabled(false);
        ui->manualCB->setEnabled(true);
        ui->boardTypeCombo->model()->setData(ui->boardTypeCombo->model()->index(0, 0), QVariant(0), Qt::UserRole - 1);
        setControllerType(SetupWizard::CONTROLLER_UNKNOWN);
    }
    emit completeChanged();
}

void ControllerPage::identificationModeChanged()
{
    if(ui->manualCB->isChecked()) {
        ui->deviceCombo->setEnabled(false);
        ui->boardTypeCombo->setEnabled(true);
        ui->connectButton->setEnabled(false);
        ui->boardTypeCombo->setCurrentIndex(1);
        //ui->boardTypeCombo->model()->setData(ui->boardTypeCombo->model()->index(0, 0), QVariant(0), Qt::UserRole - 1);
    }
    else {
        ui->connectButton->setEnabled(ui->deviceCombo->count() > 0);
        ui->deviceCombo->setEnabled(!m_connectionManager->isConnected());
        //ui->boardTypeCombo->model()->setData(ui->boardTypeCombo->model()->index(0, 0), QVariant(1), Qt::UserRole - 1);
        ui->boardTypeCombo->setCurrentIndex(0);
        ui->boardTypeCombo->setEnabled(false);
    }
    emit completeChanged();
}

void ControllerPage::connectDisconnect()
{
    if(m_connectionManager->isConnected()) {
        m_connectionManager->disconnectDevice();
    }
    else {
        m_connectionManager->connectDevice(m_connectionManager->findDevice(ui->deviceCombo->itemData(ui->deviceCombo->currentIndex(), Qt::ToolTipRole).toString()));
    }
    emit completeChanged();
}
