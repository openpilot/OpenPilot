/**
 ******************************************************************************
 *
 * @file       systemhealthgadgetfactory.cpp
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup SystemHealthPlugin System Health Plugin
 * @{
 * @brief The System Health gadget plugin
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
#include "systemhealthgadgetfactory.h"
#include "systemhealthgadgetwidget.h"
#include "systemhealthgadget.h"
#include "systemhealthgadgetconfiguration.h"
#include "systemhealthgadgetoptionspage.h"
#include <coreplugin/iuavgadget.h>

SystemHealthGadgetFactory::SystemHealthGadgetFactory(QObject *parent) :
        IUAVGadgetFactory(QString("SystemHealthGadget"),
                          tr("System Health"),
                          parent)
{
}

SystemHealthGadgetFactory::~SystemHealthGadgetFactory()
{
}

Core::IUAVGadget* SystemHealthGadgetFactory::createGadget(QWidget *parent)
{
    SystemHealthGadgetWidget* gadgetWidget = new SystemHealthGadgetWidget(parent);
    return new SystemHealthGadget(QString("SystemHealthGadget"), gadgetWidget, parent);
}

IUAVGadgetConfiguration *SystemHealthGadgetFactory::createConfiguration(QSettings* qSettings)
{
    return new SystemHealthGadgetConfiguration(QString("SystemHealthGadget"), qSettings);
}

IOptionsPage *SystemHealthGadgetFactory::createOptionsPage(IUAVGadgetConfiguration *config)
{
    return new SystemHealthGadgetOptionsPage(qobject_cast<SystemHealthGadgetConfiguration*>(config));
}

