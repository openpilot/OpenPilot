/**
 ******************************************************************************
 *
 * @file       gpsdisplaygadgetfactory.cpp
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GPSGadgetPlugin GPS Gadget Plugin
 * @{
 * @brief A gadget that displays GPS status and enables basic configuration 
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
#include "gpsdisplaygadgetfactory.h"
#include "gpsdisplaywidget.h"
#include "gpsdisplaygadget.h"
#include "gpsdisplaygadgetconfiguration.h"
#include "gpsdisplaygadgetoptionspage.h"
#include <coreplugin/iuavgadget.h>

GpsDisplayGadgetFactory::GpsDisplayGadgetFactory(QObject *parent) :
        IUAVGadgetFactory(QString("GpsDisplayGadget"),
                          tr("GPS Display"),
                          parent)
{
}

GpsDisplayGadgetFactory::~GpsDisplayGadgetFactory()
{
}

Core::IUAVGadget* GpsDisplayGadgetFactory::createGadget(QWidget *parent)
{
    GpsDisplayWidget* gadgetWidget = new GpsDisplayWidget(parent);
    return new GpsDisplayGadget(QString("GpsDisplayGadget"), gadgetWidget, parent);
}

IUAVGadgetConfiguration *GpsDisplayGadgetFactory::createConfiguration(QSettings* qSettings)
{
    return new GpsDisplayGadgetConfiguration(QString("GpsDisplayGadget"), qSettings);
}

IOptionsPage *GpsDisplayGadgetFactory::createOptionsPage(IUAVGadgetConfiguration *config)
{
    return new GpsDisplayGadgetOptionsPage(qobject_cast<GpsDisplayGadgetConfiguration*>(config));
}

