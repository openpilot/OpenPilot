/**
 ******************************************************************************
 *
 * @file       defaultattitudewidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief Placeholder for attitude panel until board is connected.
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
#include "defaultattitudewidget.h"
#include "ui_defaultattitude.h"
#include <QMutexLocker>
#include <QErrorMessage>
#include <QDebug>

DefaultAttitudeWidget::DefaultAttitudeWidget(QWidget *parent) :
        QWidget(parent),
        ui(new Ui_defaultattitude)
{
    ui->setupUi(this);
}

DefaultAttitudeWidget::~DefaultAttitudeWidget()
{
    delete ui;
}

