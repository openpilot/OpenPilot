/**
 ******************************************************************************
 *
 * @file       configautotunewidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief The Configuration Gadget used to adjust or recalculate autotuning
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
#ifndef CONFIGAUTOTUNE_H
#define CONFIGAUTOTUNE_H

#include "ui_autotune.h"
#include "../uavobjectwidgetutils/configtaskwidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include "stabilizationsettings.h"
#include "relaytuningsettings.h"
#include "relaytuning.h"
#include <QtGui/QWidget>
#include <QTimer>

class ConfigAutotuneWidget : public ConfigTaskWidget
{
    Q_OBJECT
public:
    explicit ConfigAutotuneWidget(QWidget *parent = 0);

private:
    Ui_AutotuneWidget *m_autotune;
    StabilizationSettings::DataFields stabSettings;

signals:

public slots:
    void refreshWidgetsValues(UAVObject *obj);
    void updateObjectsFromWidgets();
private slots:
    void recomputeStabilization();
    void saveStabilization();
};

#endif // CONFIGAUTOTUNE_H
