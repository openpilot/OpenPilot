/**
 ******************************************************************************
 *
 * @file       scopegadgetfactory.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Scope Gadget Factory
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   scopeplugin
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
#include "scopegadgetfactory.h"
#include "scopegadgetwidget.h"
#include "scopegadgetconfiguration.h"
#include "scopegadgetoptionspage.h"
#include "scopegadget.h"
#include <coreplugin/iuavgadget.h>

ScopeGadgetFactory::ScopeGadgetFactory(QObject *parent) :
        IUAVGadgetFactory(QString("ScopeGadget"),
                          tr("Scope Gadget"),
                          parent)
{
}

ScopeGadgetFactory::~ScopeGadgetFactory()
{
}

Core::IUAVGadget* ScopeGadgetFactory::createGadget(QWidget *parent)
{
    ScopeGadgetWidget* gadgetWidget = new ScopeGadgetWidget(parent);
    return new ScopeGadget(QString("ScopeGadget"), gadgetWidget, parent);
}

IUAVGadgetConfiguration *ScopeGadgetFactory::createConfiguration(const QByteArray &state)
{
    return new ScopeGadgetConfiguration(QString("ScopeGadget"), state);
}

IOptionsPage *ScopeGadgetFactory::createOptionsPage(IUAVGadgetConfiguration *config)
{
    return new ScopeGadgetOptionsPage(qobject_cast<ScopeGadgetConfiguration*>(config));
}
