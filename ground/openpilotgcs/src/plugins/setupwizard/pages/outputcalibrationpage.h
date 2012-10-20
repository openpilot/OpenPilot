/**
 ******************************************************************************
 *
 * @file       outputcalibrationpage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup
 * @{
 * @addtogroup OutputCalibrationPage
 * @{
 * @brief
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

#ifndef OUTPUTCALIBRATIONPAGE_H
#define OUTPUTCALIBRATIONPAGE_H

#include <QtSvg>
#include "abstractwizardpage.h"
#include "setupwizard.h"
#include "outputcalibrationutil.h"
#include "vehicleconfigurationhelper.h"

namespace Ui {
class OutputCalibrationPage;
}

class OutputCalibrationPage : public AbstractWizardPage
{
    Q_OBJECT
    
public:
    explicit OutputCalibrationPage(SetupWizard *wizard, QWidget *parent = 0);
    ~OutputCalibrationPage();
    void initializePage();
    bool validatePage();

    bool isFinished() { return m_currentWizardIndex >= m_wizardIndexes.size() - 1; }

protected:
    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent *event);
    
public slots:
    void customBackClicked();

private slots:
    void on_motorNeutralButton_toggled(bool checked);
    void on_motorNeutralSlider_valueChanged(int value);

    void on_servoCenterButton_toggled(bool checked);
    void on_servoCenterSlider_valueChanged(int position);

    void on_servoMinAngleButton_toggled(bool checked);
    void on_servoMinAngleSlider_valueChanged(int position);

    void on_servoMaxAngleButton_toggled(bool checked);
    void on_servoMaxAngleSlider_valueChanged(int position);

private:
    void setupVehicle();
    void startWizard();
    void setupVehicleItems();
    void setupVehicleHighlightedPart();
    void setWizardPage();
    void enableButtons(bool enable);
    void onStartButtonToggle(QAbstractButton *button, quint16 channel, quint16 value, quint16 safeValue, QSlider *slider);
    bool checkAlarms();
    void debugLogChannelValues();
    quint16 getCurrentChannel();

    Ui::OutputCalibrationPage *ui;
    QSvgRenderer *m_vehicleRenderer;
    QGraphicsScene *m_vehicleScene;
    QGraphicsSvgItem *m_vehicleBoundsItem;

    qint16 m_currentWizardIndex;

    QList<QString> m_vehicleElementIds;
    QList<QGraphicsSvgItem*> m_vehicleItems;
    QList<quint16> m_vehicleHighlightElementIndexes;
    QList<quint16> m_channelIndex;
    QList<quint16> m_wizardIndexes;

    QList<actuatorChannelSettings> m_actuatorSettings;

    OutputCalibrationUtil *m_calibrationUtil;

};

#endif // OUTPUTCALIBRATIONPAGE_H
