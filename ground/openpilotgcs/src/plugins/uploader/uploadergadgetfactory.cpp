/**
 ******************************************************************************
 *
 * @file       uploadergadgetfactory.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup YModemUploader YModem Serial Uploader Plugin
 * @{
 * @brief The YModem protocol serial uploader plugin
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

#include "uploadergadgetfactory.h"
#include "uploadergadget.h"
#include "uploadergadgetconfiguration.h"
#include "uploadergadgetoptionspage.h"
#include <coreplugin/iuavgadget.h>
#include "uploadergadgetwidget.h"

UploaderGadgetFactory::UploaderGadgetFactory(QObject *parent) :
    IUAVGadgetFactory(QString("Uploader"), tr("Uploader"), parent),isautocapable(false)
{
}

UploaderGadgetFactory::~UploaderGadgetFactory()
{
}

Core::IUAVGadget* UploaderGadgetFactory::createGadget(QWidget *parent)
{
    UploaderGadgetWidget* gadgetWidget = new UploaderGadgetWidget(parent);
    isautocapable=gadgetWidget->autoUpdateCapable();
    connect(this,SIGNAL(autoUpdate()),gadgetWidget,SLOT(autoUpdate()));
    connect(gadgetWidget,SIGNAL(autoUpdateSignal(uploader::AutoUpdateStep,QVariant)),this,SIGNAL(autoUpdateSignal(uploader::AutoUpdateStep,QVariant)));
    return new UploaderGadget(QString("Uploader"), gadgetWidget, parent);
}

IUAVGadgetConfiguration *UploaderGadgetFactory::createConfiguration(QSettings* qSettings)
{
    return new UploaderGadgetConfiguration(QString("Uploader"), qSettings);
}
bool UploaderGadgetFactory::isAutoUpdateCapable()
{
    return isautocapable;
}
