/**
 ******************************************************************************
 *
 * @file       configairframetwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief Airframe configuration panel
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
#ifndef CONFIGAIRFRAMEWIDGET_H
#define CONFIGAIRFRAMEWIDGET_H

#include "ui_airframe.h"
#include "configtaskwidget.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include "uavtalk/telemetrymanager.h"
#include <QtGui/QWidget>
#include <QList>
#include <QItemDelegate>

class Ui_Widget;

class ConfigAirframeWidget: public ConfigTaskWidget
{
    Q_OBJECT

public:
    ConfigAirframeWidget(QWidget *parent = 0);
    ~ConfigAirframeWidget();

private:
    Ui_AircraftWidget *m_aircraft;
    bool setupFrameFixedWing();
    bool setupFrameElevon();
    bool setupFrameVtail();
    bool setupQuad(bool pLayout);
    bool setupHexa(bool pLayout);
    bool setupOcto();
    void updateCustomAirframeUI();
    bool setupMixer(double mixerFactors[8][3]);
    void setupMotors(QList<QString> motorList);
    virtual void enableControls(bool enable);

    void resetField(UAVObjectField * field);
    void resetMixer (MixerCurveWidget *mixer, int numElements);
    void resetActuators();
    //void setMixerChannel(int channelNumber, bool channelIsMotor, QList<double> vector);
    void setupQuadMotor(int channel, double roll, double pitch, double yaw);

    QStringList mixerTypes;
    QStringList mixerVectors;
    QGraphicsSvgItem *quad;
    bool ffTuningInProgress;
    bool ffTuningPhase;
    UAVObject::Metadata accInitialData;

private slots:
    virtual void refreshValues();
    void sendAircraftUpdate();
    void saveAircraftUpdate();
    void setupAirframeUI(QString type);
    void toggleAileron2(int index);
    void toggleElevator2(int index);
    void toggleRudder2(int index);
    void switchAirframeType(int index);
    void resetFwMixer();
    void resetMrMixer();
    void resetCt1Mixer();
    void resetCt2Mixer();
    void updateFwThrottleCurveValue(QList<double> list, double value);
    void updateMrThrottleCurveValue(QList<double> list, double value);
    void updateCustomThrottle1CurveValue(QList<double> list, double value);
    void updateCustomThrottle2CurveValue(QList<double> list, double value);
    void enableFFTest();
    void openHelp();

protected:
    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent *event);


};

class SpinBoxDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    SpinBoxDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // CONFIGAIRFRAMEWIDGET_H
