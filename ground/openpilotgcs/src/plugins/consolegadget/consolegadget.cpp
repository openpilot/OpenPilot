/**
 ******************************************************************************
 *
 * @file       consolegadget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConsolePlugin Console Plugin
 * @{
 * @brief The Console Gadget impliments a console view 
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
#include "consolegadget.h"
#include "consolegadgetwidget.h"
#include "texteditloggerengine.h"

ConsoleGadget::ConsoleGadget(QString classId, ConsoleGadgetWidget *widget, QWidget *parent) :
        IUAVGadget(classId, parent),
        m_widget(widget)
{
    m_logger = new TextEditLoggerEngine(widget);
    bool suitableName = false;
    int i = 0;
    QString loggerName;
    while (!suitableName) {
        loggerName = "TextEditLogger" + QVariant(i).toString();
        if (!qxtLog->isLoggerEngine(loggerName))
            suitableName = true;
        ++i;
    }
    qxtLog->addLoggerEngine(loggerName, m_logger);
}

ConsoleGadget::~ConsoleGadget()
{
    qxtLog->removeLoggerEngine(m_logger);
}
