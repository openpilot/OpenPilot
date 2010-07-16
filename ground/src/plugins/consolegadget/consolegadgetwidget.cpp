/**
 ******************************************************************************
 *
 * @file       consolegadgetwidget.cpp
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
#include "consolegadgetwidget.h"
#include "qxtlogger.h"

#include <QDebug>
#include <QtGui/QTextEdit>

ConsoleGadgetWidget::ConsoleGadgetWidget(QWidget *parent) : QTextEdit(parent)
{
    setMinimumSize(64,64);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setReadOnly(true);
}

ConsoleGadgetWidget::~ConsoleGadgetWidget()
{
   // Do nothing
}

