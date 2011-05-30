/**
 ******************************************************************************
 *
 * @file       scopegadgetfactory.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ScopePlugin Scope Gadget Plugin
 * @{
 * @brief The scope Gadget, graphically plots the states of UAVObjects
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
#include "scopegadgetfactory.h"
#include "scopegadgetwidget.h"
#include "scopegadgetconfiguration.h"
#include "scopegadgetoptionspage.h"
#include "scopegadget.h"
#include <coreplugin/iuavgadget.h>

ScopeGadgetFactory::ScopeGadgetFactory(QObject *parent) :
        IUAVGadgetFactory(QString("ScopeGadget"),
                          tr("Scope"),
                          parent)
{
}

ScopeGadgetFactory::~ScopeGadgetFactory()
{
}

void ScopeGadgetFactory::stopPlotting()
{
    emit onStopPlotting();
}

void ScopeGadgetFactory::startPlotting()
{
    emit onStartPlotting();
}


Core::IUAVGadget* ScopeGadgetFactory::createGadget(QWidget *parent)
{
    ScopeGadgetWidget* gadgetWidget = new ScopeGadgetWidget(parent);
    connect(this,SIGNAL(onStartPlotting()), gadgetWidget, SLOT(startPlotting()));
    connect(this,SIGNAL(onStopPlotting()), gadgetWidget, SLOT(stopPlotting()));
    return new ScopeGadget(QString("ScopeGadget"), gadgetWidget, parent);
}

IUAVGadgetConfiguration *ScopeGadgetFactory::createConfiguration(QSettings* qSettings)
{
    return new ScopeGadgetConfiguration(QString("ScopeGadget"), qSettings);
}

IOptionsPage *ScopeGadgetFactory::createOptionsPage(IUAVGadgetConfiguration *config)
{
    return new ScopeGadgetOptionsPage(qobject_cast<ScopeGadgetConfiguration*>(config));
}
