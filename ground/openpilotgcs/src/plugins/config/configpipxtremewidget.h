/**
 ******************************************************************************
 *
 * @file       configpipxtremewidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief The Configuration Gadget used to configure PipXtreme
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
#ifndef CONFIGPIPXTREMEWIDGET_H
#define CONFIGPIPXTREMEWIDGET_H

#include <oplinksettings.h>

#include "ui_pipxtreme.h"
#include "configtaskwidget.h"

class ConfigPipXtremeWidget : public ConfigTaskWidget {
    Q_OBJECT

public:
    ConfigPipXtremeWidget(QWidget *parent = 0);
    ~ConfigPipXtremeWidget();

public slots:
    void updateStatus(UAVObject *object1);
    void updateSettings(UAVObject *object1);

private:
    Ui_PipXtremeWidget *m_oplink;

    // The OPLink status UAVObject
    UAVDataObject *oplinkStatusObj;

    // The OPLink ssettins UAVObject
    OPLinkSettings *oplinkSettingsObj;

    // Are the settings current?
    bool settingsUpdated;

    // Signal mappers to add arguments to signals.
    QSignalMapper *signalMapperAddBinding;
    QSignalMapper *signalMapperRemBinding;

private slots:
    void disconnected();
    void addBinding(QWidget *w);
    void removeBinding(QWidget *w);
};

#endif // CONFIGTXPIDWIDGET_H
