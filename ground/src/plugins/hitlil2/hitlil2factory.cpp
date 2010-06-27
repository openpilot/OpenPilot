/**
 ******************************************************************************
 *
 * @file       hitlil2factory.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   hitlil2plugin
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
#include "hitlil2factory.h"
#include "hitlil2widget.h"
#include "hitlil2.h"
#include "hitlil2configuration.h"
#include "hitlil2optionspage.h"
#include <coreplugin/iuavgadget.h>

HITLIL2Factory::HITLIL2Factory(QObject *parent) :
        IUAVGadgetFactory(QString("HITLIL2"), tr("HITL Simulation with IL2"), parent)
{
}

HITLIL2Factory::~HITLIL2Factory()
{
}

Core::IUAVGadget* HITLIL2Factory::createGadget(QWidget *parent)
{
    HITLIL2Widget* gadgetWidget = new HITLIL2Widget(parent);
    return new HITLIL2(QString("HITLIL2"), gadgetWidget, parent);
}

IUAVGadgetConfiguration *HITLIL2Factory::createConfiguration(const QByteArray &state)
{
    return new HITLIL2Configuration(QString("HITLIL2"), state);
}

IOptionsPage *HITLIL2Factory::createOptionsPage(IUAVGadgetConfiguration *config)
{
    return new HITLIL2OptionsPage(qobject_cast<HITLIL2Configuration*>(config));
}

