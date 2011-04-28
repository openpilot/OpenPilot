/**
 ******************************************************************************
 *
 * @file       configgadgetplugin.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief The Configuration Gadget used to update settings in the firmware
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
#ifndef CONFIGPLUGIN_H
#define CONFIGPLUGIN_H

#include <extensionsystem/iplugin.h>
#include <coreplugin/icore.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include "uavtalk/telemetrymanager.h"
#include "objectpersistence.h"


#include <QMessageBox>


class ConfigGadgetFactory;

class ConfigPlugin : public ExtensionSystem::IPlugin
{
    Q_OBJECT

public:
    ConfigPlugin();
    ~ConfigPlugin();

    void extensionsInitialized();
    bool initialize(const QStringList & arguments, QString * errorString);
    void shutdown();

private slots:
    void eraseAllSettings();
    void onAutopilotConnect();
    void onAutopilotDisconnect();
    void eraseDone(UAVObject *);
    void eraseFailed();

 private:
    ConfigGadgetFactory *cf;
    Core::Command* cmd;
    bool settingsErased;

};

#endif // CONFIGPLUGIN_H
