/**
 ******************************************************************************
 *
 * @file       defaultccattitudewidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief Placeholder for attitude settings widget until board connected.
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
#ifndef DEFAULTHWSETTINGSt_H
#define DEFAULTHWSETTINGSt_H

#include "ui_defaulthwsettings.h"
#include "../uavobjectwidgetutils/configtaskwidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include <QtGui/QWidget>
#include <QTimer>
#include <QMutex>

class Ui_Widget;

class DefaultHwSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DefaultHwSettingsWidget(QWidget *parent = 0);
    ~DefaultHwSettingsWidget();

private slots:

private:
    Ui_defaulthwsettings *ui;
};

#endif // DEFAULTHWSETTINGSt_H
