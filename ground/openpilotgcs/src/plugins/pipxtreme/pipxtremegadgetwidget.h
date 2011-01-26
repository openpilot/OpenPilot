/**
 ******************************************************************************
 *
 * @file       pipxtremegadgetwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @{
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

#ifndef PIPXTREMEGADGETWIDGET_H
#define PIPXTREMEGADGETWIDGET_H

#include "ui_pipxtreme.h"
#include "delay.h"

#include <qextserialport.h>
#include <qextserialenumerator.h>

#include "uavtalk/telemetrymanager.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/uavobject.h"

#include "rawhid/rawhidplugin.h"

#include <QtGui/QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QThread>
#include <QMessageBox>
#include <QTimer>
#include <QtCore/QVector>
#include <QtCore/QIODevice>
#include <QtCore/QLinkedList>

// *************************
// pipx config comms packets

#pragma pack(push, 1)

typedef struct
{
    uint32_t    marker;
    uint32_t    serial_number;
    uint8_t     type;
    uint8_t     spare;
    uint16_t    data_size;
    uint32_t    crc;
    uint8_t     data[0];
} t_pipx_header;

typedef struct
{
    uint8_t     mode;
    uint8_t     state;
} t_pipx_data_mode_state;

typedef struct
{
    uint32_t    serial_baudrate;    // serial uart baudrate

    uint32_t    destination_id;

    uint32_t    min_frequency_Hz;
    uint32_t    max_frequency_Hz;
    uint32_t    frequency_Hz;

    uint32_t    max_rf_bandwidth;

    uint8_t     max_tx_power;

    uint8_t     frequency_band;

    uint8_t     rf_xtal_cap;

    bool        aes_enable;
    uint8_t     aes_key[16];

    uint16_t    frequency_step_size;
} t_pipx_data_settings;

typedef struct
{
    uint32_t    start_frequency;
    uint16_t    frequency_step_size;
    uint16_t    magnitudes;
    int8_t      magnitude[0];
} t_pipx_data_spectrum;

#pragma pack(pop)

// *************************

class PipXtremeGadgetWidget : public QWidget
{
    Q_OBJECT

public:
    PipXtremeGadgetWidget(QWidget *parent = 0);
   ~PipXtremeGadgetWidget();

    typedef enum { IAP_STATE_READY, IAP_STATE_STEP_1} IAPStep;
    typedef enum { RESCUE_STEP0, RESCUE_STEP1, RESCUE_STEP2, RESCUE_STEP3, RESCUE_POWER1, RESCUE_POWER2, RESCUE_DETECT } RescueStep;

public slots:
    void onTelemetryStart();
    void onTelemetryStop();
    void onTelemetryConnect();
    void onTelemetryDisconnect();

    void onModemConnect();
    void onModemDisconnect();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    Ui_PipXtremeWidget *m_config;

    IAPStep currentStep;
    RescueStep rescueStep;
    bool resetOnly;

    pjrc_rawhid     rawHidHandle;
//    RawHIDPlugin    *rawHidPlugin;

    // currently connected QIODevice
    QIODevice *m_ioDevice;


    QString getPortDevice(const QString &friendName);

    void suspendTelemetry();
    void restartTelemetryPolling();

    void processOutputStream();
    void processInputStream();

private slots:
    void error(QString errorString,int errorNumber);
    void goToAPIMode(UAVObject* = NULL, bool = false);
    void systemBoot();
    void getPorts();
};

#endif
