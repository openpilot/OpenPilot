/**
 ******************************************************************************
 *
 * @file       AntennaTracgadgetfactory.cpp
 * @author     Sami Korhonen & the OpenPilot team Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup AntennaTrackGadgetPlugin Antenna Track Gadget Plugin
 * @{
 * @brief A gadget that communicates with antenna tracker and enables basic configuration
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
#include "antennatrackgadgetfactory.h"
#include "antennatrackwidget.h"
#include "antennatrackgadget.h"
#include "antennatrackgadgetconfiguration.h"
#include "antennatrackgadgetoptionspage.h"
#include <coreplugin/iuavgadget.h>

AntennaTrackGadgetFactory::AntennaTrackGadgetFactory(QObject *parent) :
        IUAVGadgetFactory(QString("AntennaTrackGadget"),
                          tr("Antenna Track Gadget"),
                          parent)
{
}

AntennaTrackGadgetFactory::~AntennaTrackGadgetFactory()
{
}

Core::IUAVGadget* AntennaTrackGadgetFactory::createGadget(QWidget *parent)
{
    AntennaTrackWidget* gadgetWidget = new AntennaTrackWidget(parent);
    return new AntennaTrackGadget(QString("AntennaTrackGadget"), gadgetWidget, parent);
}

IUAVGadgetConfiguration *AntennaTrackGadgetFactory::createConfiguration(QSettings* qSettings)
{
    return new AntennaTrackGadgetConfiguration(QString("AntennaTrackGadget"), qSettings);
}

IOptionsPage *AntennaTrackGadgetFactory::createOptionsPage(IUAVGadgetConfiguration *config)
{
    return new AntennaTrackGadgetOptionsPage(qobject_cast<AntennaTrackGadgetConfiguration*>(config));
}

