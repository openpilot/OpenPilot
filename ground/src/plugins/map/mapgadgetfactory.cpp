/**
 ******************************************************************************
 *
 * @file       mapgadgetfactory.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   map
 * @{
 *
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
#include "mapgadgetfactory.h"
#include "mapgadgetwidget.h"
#include "mapgadget.h"
#include "mapgadgetconfiguration.h"
#include "mapgadgetoptionspage.h"
#include <coreplugin/uavgadgetoptionspagedecorator.h>
#include <coreplugin/iuavgadget.h>

MapGadgetFactory::MapGadgetFactory(QObject *parent) :
        IUAVGadgetFactory(QString("MapGadget"), tr("Map Gadget"), parent)
{
}

MapGadgetFactory::~MapGadgetFactory()
{
}

Core::IUAVGadget* MapGadgetFactory::createGadget(QList<IUAVGadgetConfiguration*> *configurations, QWidget *parent)
{
	MapGadgetWidget* gadgetWidget = new MapGadgetWidget(parent);        
        return new MapGadget(QString("MapGadget"), configurations, gadgetWidget);
}

IUAVGadgetConfiguration *MapGadgetFactory::createConfiguration(bool locked,
                                                               const QString configName,
                                                               const QByteArray &state)
{
    return new MapGadgetConfiguration(locked, QString("MapGadget"), configName, state);
}

IOptionsPage *MapGadgetFactory::createOptionsPage(IUAVGadgetConfiguration *config)
{
    MapGadgetOptionsPage *page = new MapGadgetOptionsPage(config);
    return new UAVGadgetOptionsPageDecorator(page, config);
}

