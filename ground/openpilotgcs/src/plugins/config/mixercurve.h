/**
 ******************************************************************************
 *
 * @file       mixercurve.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief A MixerCurve Gadget used to update settings in the firmware
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
#ifndef MIXERCURVE_H
#define MIXERCURVE_H

#include <QFrame>
#include <QtGui/QWidget>
#include <QList>
#include <QTableWidget>

#include "ui_mixercurve.h"
#include "mixercurvewidget.h"
#include "dblspindelegate.h"
#include "uavobjectwidgetutils_global.h"
#include "uavobjectwidgetutils/popupwidget.h"


namespace Ui {
class MixerCurve;
}

class  MixerCurve : public QFrame
{
    Q_OBJECT
    
public:
    explicit MixerCurve(QWidget *parent = 0);
    ~MixerCurve();


    /* Enumeration options for ThrottleCurves */
    typedef enum { MIXERCURVE_THROTTLE=0, MIXERCURVE_PITCH=1 } MixerCurveType;

    void setMixerType(MixerCurveType curveType);
    void initCurve (const QList<double>* points);
    QList<double> getCurve();
    void initLinearCurve(int numPoints, double maxValue = 1, double minValue = 0);
    void setCurve(const QList<double>* points);
    void setMin(double value);
    double getMin();
    void setMax(double value);
    double getMax();
    double getCurveMin();
    double getCurveMax();
    double getCurveStep();
    double setRange(double min, double max);

    MixerCurveWidget* getCurveWidget() { return m_curve; }

signals:

protected:
    void showEvent(QShowEvent *event);
    void resizeEvent(QResizeEvent *event);

public slots:
    void ResetCurve();
    void PopupCurve();
    void GenerateCurve();
    void UpdateSettingsTable();

private slots:
    void CommandActivated(MixerNode* node = 0);
    void SettingsTableChanged();
    void CurveTypeChanged();
    void CurveMinChanged(double value);
    void CurveMaxChanged(double value);
    void UpdateCurveUI();

private:
    Ui::MixerCurve* m_mixerUI;
    MixerCurveWidget* m_curve;
    QTableWidget* m_settings;
    MixerCurveType m_curveType;
    DoubleSpinDelegate* m_spinDelegate;

};

#endif // MIXERCURVE_H
