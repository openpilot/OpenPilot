/**
 ******************************************************************************
 *
 * @file       configplugin.h
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
#include "configplugin.h"
#include "configgadgetfactory.h"
#include <QtPlugin>
#include <QStringList>
#include <QTimer>
#include <extensionsystem/pluginmanager.h>


ConfigPlugin::ConfigPlugin()
{
   // Do nothing
}

ConfigPlugin::~ConfigPlugin()
{
   // Do nothing
}

bool ConfigPlugin::initialize(const QStringList& args, QString *errMsg)
{
   Q_UNUSED(args);
   Q_UNUSED(errMsg);
  cf = new ConfigGadgetFactory(this);
  addAutoReleasedObject(cf);

  // Add Menu entry to erase all settings
  Core::ActionManager* am = Core::ICore::instance()->actionManager();
  Core::ActionContainer* ac = am->actionContainer(Core::Constants::M_TOOLS);

  // Command to erase all settings from the board
  cmd = am->registerAction(new QAction(this),
                                          "ConfigPlugin.EraseAll",
                                          QList<int>() <<
                                          Core::Constants::C_GLOBAL_ID);
  cmd->action()->setText("Erase all settings from board...");

  ac->menu()->addSeparator();
  ac->appendGroup("Utilities");
  ac->addAction(cmd, "Utilities");

  connect(cmd->action(), SIGNAL(triggered(bool)), this, SLOT(eraseAllSettings()));

  // *********************
  // Listen to autopilot connection events
  ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
  TelemetryManager* telMngr = pm->getObject<TelemetryManager>();
  connect(telMngr, SIGNAL(connected()), this, SLOT(onAutopilotConnect()));
  connect(telMngr, SIGNAL(disconnected()), this, SLOT(onAutopilotDisconnect()));

  // And check whether by any chance we are not already connected
  if (telMngr->isConnected())
      onAutopilotConnect();

   return true;
}

void ConfigPlugin::extensionsInitialized()
{
    cmd->action()->setEnabled(false);

}

void ConfigPlugin::shutdown()
{
   // Do nothing
}

/**
  * Enable the menu entry when the autopilot connects
  */
void ConfigPlugin::onAutopilotConnect()
{
    cmd->action()->setEnabled(true);
}

/**
  * Enable the menu entry when the autopilot connects
  */
void ConfigPlugin::onAutopilotDisconnect()
{
    cmd->action()->setEnabled(false);
}


/**
  * Erase all settings from the board
  */
void ConfigPlugin::eraseAllSettings()
{
    QMessageBox msgBox;
    msgBox.setText(tr("Are you sure you want to erase all board settings?."));
    msgBox.setInformativeText(tr("All settings stored in your board flash will be deleted."));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Ok);
    if (msgBox.exec() != QMessageBox::Ok)
            return;

    settingsErased = false;
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager * objMngr = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objMngr);
    ObjectPersistence* objper = dynamic_cast<ObjectPersistence*>( objMngr->getObject(ObjectPersistence::NAME) );
    Q_ASSERT(objper);
    connect(objper, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(eraseDone(UAVObject *)));
    ObjectPersistence::DataFields data;
    data.Operation = ObjectPersistence::OPERATION_DELETE;
    data.Selection = ObjectPersistence::SELECTION_ALLSETTINGS;
    objper->setData(data);
    objper->updated();
    QTimer::singleShot(1500,this,SLOT(eraseFailed()));

}

void ConfigPlugin::eraseFailed()
{
    if (settingsErased)
        return;
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager * objMngr = pm->getObject<UAVObjectManager>();
    Q_ASSERT(objMngr);
    ObjectPersistence* objper = dynamic_cast<ObjectPersistence*>( objMngr->getObject(ObjectPersistence::NAME));
    disconnect(objper, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(eraseDone(UAVObject *)));
    QMessageBox msgBox;
    msgBox.setText(tr("Error trying to erase settings."));
    msgBox.setInformativeText(tr("Power-cycle your board. Settings might be inconsistent."));
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

void ConfigPlugin::eraseDone(UAVObject * obj)
{
    QMessageBox msgBox;
    ObjectPersistence* objper = dynamic_cast<ObjectPersistence*>(sender());
    Q_ASSERT(obj->getName().compare("ObjectPersistence") == 0);
    QString tmp = obj->getField("Operation")->getValue().toString();
    if (obj->getField("Operation")->getValue().toString().compare(QString("Delete")) == 0 ) {
        return;
    }

    disconnect(objper, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(eraseDone(UAVObject *)));
    if (obj->getField("Operation")->getValue().toString().compare(QString("Completed")) == 0) {
        settingsErased = true;
        msgBox.setText(tr("Settings are now erased."));
        msgBox.setInformativeText(tr("Please now power-cycle your board to complete reset."));
    } else {
        msgBox.setText(tr("Error trying to erase settings."));
        msgBox.setInformativeText(tr("Power-cycle your board. Settings might be inconsistent."));
    }
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

Q_EXPORT_PLUGIN(ConfigPlugin)
