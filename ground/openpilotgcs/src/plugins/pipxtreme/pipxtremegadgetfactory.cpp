/**
 ******************************************************************************
 *
 * @file       pipxtremegadgetfactory.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @{
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

#include "pipxtremegadgetfactory.h"
#include "pipxtremegadgetwidget.h"
#include "pipxtremegadget.h"
#include "pipxtremegadgetconfiguration.h"
#include "pipxtremegadgetoptionspage.h"
#include <coreplugin/iuavgadget.h>

PipXtremeGadgetFactory::PipXtremeGadgetFactory(QObject *parent) :
                IUAVGadgetFactory(QString("PipXtreme"), tr("PipXtreme"), parent)
{
}

PipXtremeGadgetFactory::~PipXtremeGadgetFactory()
{
}

Core::IUAVGadget* PipXtremeGadgetFactory::createGadget(QWidget *parent)
{
	PipXtremeGadgetWidget *gadgetWidget = new PipXtremeGadgetWidget(parent);
	return new PipXtremeGadget(QString("PipXtreme"), gadgetWidget, parent);
}

IUAVGadgetConfiguration * PipXtremeGadgetFactory::createConfiguration(QSettings *qSettings)
{
	return new PipXtremeGadgetConfiguration(QString("PipXtreme"), qSettings);
}
