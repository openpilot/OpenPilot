/**
 ******************************************************************************
 *
 * @file       modelviewgadgetfactory.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ModelViewPlugin ModelView Plugin
 * @{
 * @brief A gadget that displays a 3D representation of the UAV 
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
#include "modelviewgadgetfactory.h"
#include "modelviewgadgetwidget.h"
#include "modelviewgadget.h"
#include "modelviewgadgetconfiguration.h"
#include "modelviewgadgetoptionspage.h"
#include <coreplugin/uavgadgetoptionspagedecorator.h>
#include <coreplugin/iuavgadget.h>

ModelViewGadgetFactory::ModelViewGadgetFactory(QObject *parent) :
        IUAVGadgetFactory(QString("ModelViewGadget"), tr("ModelView"), parent)
{
}

ModelViewGadgetFactory::~ModelViewGadgetFactory()
{
}

Core::IUAVGadget* ModelViewGadgetFactory::createGadget(QWidget *parent)
{
	ModelViewGadgetWidget* gadgetWidget = new ModelViewGadgetWidget(parent);        
        return new ModelViewGadget(QString("ModelViewGadget"), gadgetWidget, parent);
}

IUAVGadgetConfiguration *ModelViewGadgetFactory::createConfiguration(QSettings* qSettings)
{
    return new ModelViewGadgetConfiguration(QString("ModelViewGadget"), qSettings);
}

IOptionsPage *ModelViewGadgetFactory::createOptionsPage(IUAVGadgetConfiguration *config)
{
    return new ModelViewGadgetOptionsPage(qobject_cast<ModelViewGadgetConfiguration*>(config));
}

