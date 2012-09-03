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

protected:
    void showEvent(QShowEvent *event);
    
private slots:
    void on_nextPageButton_clicked();
    void on_backPageButton_clicked();

    void on_motorNeutralButton_toggled(bool checked);
    void on_motorNeutralSlider_valueChanged(int value);

    void on_motorMaxButton_toggled(bool checked);
    void on_motorMaxSlider_valueChanged(int position);

    void on_servoCenterButton_toggled(bool checked);
    void on_servoCenterSlider_valueChanged(int position);

    void on_servoAngleButton_toggled(bool checked);
    void on_servoMaxAngleSlider_valueChanged(int position);
    void on_servoMinAngleSlider_valueChanged(int position);

private:
    void setupVehicle();
    void startWizard();
    void setupVehicleItems();
    void setupVehicleHighlightedPart();
    void setWizardPage();

    quint16 getEscUpdateRate(){ return getWizard()->getESCType() == VehicleConfigurationSource::ESC_RAPID ?
                    VehicleConfigurationHelper::RAPID_ESC_FREQUENCE : VehicleConfigurationHelper::LEGACY_ESC_FREQUENCE; }

    quint16 getServoUpdateRate(){ return VehicleConfigurationHelper::LEGACY_ESC_FREQUENCE; }

    Ui::OutputCalibrationPage *ui;
    QSvgRenderer *m_vehicleRenderer;
    QGraphicsScene *m_vehicleScene;
    QGraphicsSvgItem *m_vehicleBoundsItem;

    quint16 m_currentWizardIndex;

    QList<QString> m_vehicleElementIds;
    QList<QGraphicsSvgItem*> m_vehicleItems;
    QList<quint16> m_vehicleHighlightElementIndexes;
    QList<quint16> m_wizardIndexes;
    QList<quint16> m_channelUpdateRates;

    OutputCalibrationUtil *m_calibrationUtil;

};

#endif // OUTPUTCALIBRATIONPAGE_H
