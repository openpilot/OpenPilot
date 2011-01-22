/**
 ******************************************************************************
 *
 * @file       GCSControlgadgetfactory.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup GCSControlGadgetPlugin GCSControl Gadget Plugin
 * @{
 * @brief A gadget to control the UAV, either from the keyboard or a joystick
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
#include "logginggadgetfactory.h"
#include "logginggadgetwidget.h"
#include "logginggadget.h"
#include <coreplugin/iuavgadget.h>

LoggingGadgetFactory::LoggingGadgetFactory(QObject *parent) :
        IUAVGadgetFactory(QString("LoggingGadget"),
                          tr("Logging"),
                          parent)
{
    loggingPlugin = (LoggingPlugin *) parent;
}

LoggingGadgetFactory::~LoggingGadgetFactory()
{

}

IUAVGadget* LoggingGadgetFactory::createGadget(QWidget *parent) {
    LoggingGadgetWidget* gadgetWidget = new LoggingGadgetWidget(parent);
    gadgetWidget->setPlugin(loggingPlugin);
    return new LoggingGadget(QString("LoggingGadget"), gadgetWidget, parent);
}
