/**
 ******************************************************************************
 *
 * @file       videogadgetfactory.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup VideoGadgetPlugin Video Gadget Plugin
 * @{
 * @brief A place holder gadget plugin 
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
#include "videogadgetfactory.h"
#include "videogadgetwidget.h"
#include "videogadget.h"
#include "videogadgetconfiguration.h"
#include "videogadgetoptionspage.h"
#include <coreplugin/uavgadgetoptionspagedecorator.h>
#include <coreplugin/iuavgadget.h>

VideoGadgetFactory::VideoGadgetFactory(QObject *parent) :
        IUAVGadgetFactory(QString("VideoGadget"),
                          tr("VideoGadget"),
                          parent)
{
}

VideoGadgetFactory::~VideoGadgetFactory()
{

}

Core::IUAVGadget* VideoGadgetFactory::createGadget(QWidget *parent) {
    VideoGadgetWidget* gadgetWidget = new VideoGadgetWidget(parent);
    return new VideoGadget(QString("VideoGadget"), gadgetWidget, parent);
}

IUAVGadgetConfiguration *VideoGadgetFactory::createConfiguration(QSettings* qSettings)
{
    return new VideoGadgetConfiguration(QString("VideoGadget"), qSettings);
}

IOptionsPage *VideoGadgetFactory::createOptionsPage(IUAVGadgetConfiguration *config)
{
    return new VideoGadgetOptionsPage(qobject_cast<VideoGadgetConfiguration*>(config));
}
