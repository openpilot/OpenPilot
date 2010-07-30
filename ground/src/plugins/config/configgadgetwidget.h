/**
 ******************************************************************************
 *
 * @file       configgadgetwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief The Configuration Gadget used to update settings in the firmware
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
#ifndef CONFIGGADGETWIDGET_H
#define CONFIGGADGETWIDGET_H

#include "ui_settingswidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/uavobject.h"
#include "uavobjects/objectpersistence.h"
#include <QtGui/QWidget>
#include <QList>

class Ui_SettingsWidget;

class ConfigGadgetWidget: public QWidget
{
    Q_OBJECT

public:
    ConfigGadgetWidget(QWidget *parent = 0);
    ~ConfigGadgetWidget();

protected:
        void resizeEvent(QResizeEvent * event);

private:
        Ui_SettingsWidget *m_config;
        QList<QSlider> sliders;
        void updateChannelSlider(QSlider* slider, QLabel* min, QLabel* Max,  QCheckBox* rev, int value);
        void updateObjectPersistance(ObjectPersistence::OperationOptions op, UAVObject *obj);

        bool firstUpdate;

    private slots:
        void updateChannels(UAVObject* obj);
        void requestRCInputUpdate();
        void sendRCInputUpdate();
        void saveRCInputObject();


};

#endif // CONFIGGADGETWIDGET_H
