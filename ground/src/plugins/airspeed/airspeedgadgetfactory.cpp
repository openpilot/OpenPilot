/**
 ******************************************************************************
 *
 * @file       airspeedgadgetfactory.cpp
 * @author     David "Buzz" Carlson Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   dialplugin 
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
#include "airspeedgadgetfactory.h"
#include "airspeedgadgetwidget.h"
#include "airspeedgadget.h"
#include "airspeedgadgetconfiguration.h"
#include "airspeedgadgetoptionspage.h"
#include <coreplugin/iuavgadget.h>

AirspeedGadgetFactory::AirspeedGadgetFactory(QObject *parent) :
        IUAVGadgetFactory(QString("AirspeedGadget"),
                          tr("Analog Dial Gadget"),
                          parent)
{
}

AirspeedGadgetFactory::~AirspeedGadgetFactory()
{
}

Core::IUAVGadget* AirspeedGadgetFactory::createGadget(QWidget *parent)
{
    AirspeedGadgetWidget* gadgetWidget = new AirspeedGadgetWidget(parent);
    return new AirspeedGadget(QString("AirspeedGadget"), gadgetWidget, parent);
}

IUAVGadgetConfiguration *AirspeedGadgetFactory::createConfiguration(const QByteArray &state)
{
    return new AirspeedGadgetConfiguration(QString("AirspeedGadget"), state);
}

IOptionsPage *AirspeedGadgetFactory::createOptionsPage(IUAVGadgetConfiguration *config)
{
    return new AirspeedGadgetOptionsPage(qobject_cast<AirspeedGadgetConfiguration*>(config));
}

