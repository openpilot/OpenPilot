/**
 ******************************************************************************
 *
 * @file       configgadgetfactory.cpp
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
#include "configgadgetfactory.h"
#include "configgadget.h"
#include "configgadgetconfiguration.h"
#include "configgadgetoptionspage.h"
#include <coreplugin/iuavgadget.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/modemanager.h>

ConfigGadgetFactory::ConfigGadgetFactory(QObject *parent) :
    IUAVGadgetFactory(QString("ConfigGadget"), tr("Config Gadget"), parent),
    gadgetWidget(0)
{
}

ConfigGadgetFactory::~ConfigGadgetFactory()
{
}

Core::IUAVGadget* ConfigGadgetFactory::createGadget(QWidget *parent)
{
    gadgetWidget = new ConfigGadgetWidget(parent);

    // Add Menu entry
    Core::ActionManager* am = Core::ICore::instance()->actionManager();
    Core::ActionContainer* ac = am->actionContainer(Core::Constants::M_TOOLS);

    Core::Command* cmd = am->registerAction(new QAction(this),
                                            "ConfigPlugin.ShowInputWizard",
                                            QList<int>() <<
                                            Core::Constants::C_GLOBAL_ID);
    cmd->setDefaultKeySequence(QKeySequence("Ctrl+R"));
    cmd->action()->setText(tr("Radio Setup Wizard"));

    Core::ModeManager::instance()->addAction(cmd, 1);

    ac->appendGroup("Wizard");
    ac->addAction(cmd, "Wizard");

    connect(cmd->action(), SIGNAL(triggered(bool)), this, SLOT(startInputWizard()));

    return new ConfigGadget(QString("ConfigGadget"), gadgetWidget, parent);
}

IUAVGadgetConfiguration *ConfigGadgetFactory::createConfiguration(QSettings* qSettings)
{
    return new ConfigGadgetConfiguration(QString("ConfigGadget"), qSettings);
}

IOptionsPage *ConfigGadgetFactory::createOptionsPage(IUAVGadgetConfiguration *config)
{
    return new ConfigGadgetOptionsPage(qobject_cast<ConfigGadgetConfiguration*>(config));
}

void ConfigGadgetFactory::startInputWizard()
{
    if(gadgetWidget)
    {
        Core::ModeManager::instance()->activateModeByWorkspaceName("Configuration");
        gadgetWidget->startInputWizard();
    }
}
