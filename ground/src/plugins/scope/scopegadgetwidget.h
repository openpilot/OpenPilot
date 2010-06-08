/**
 ******************************************************************************
 *
 * @file       scopegadgetwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Scope Plugin Gadget Widget
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   Scope
 * @{
 *
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

#ifndef SCOPEGADGETWIDGET_H_
#define SCOPEGADGETWIDGET_H_

#include "plotdata.h"
#include "uavobjects/uavobject.h"
#include "uavobjects/altitudeactual.h"
#include "uavobjects/gpsobject.h"


#include "qwt/src/qwt.h"
#include "qwt/src/qwt_plot.h"
#include "qwt/src/qwt_plot_curve.h"
#include "qwt/src/qwt_scale_draw.h"
#include "qwt/src/qwt_scale_widget.h"

#include <QTimer>
#include <QTime>
#include <QVector>

class TimeScaleDraw : public QwtScaleDraw
{
public:
    TimeScaleDraw() {
        baseTime = QDateTime::currentDateTime().toTime_t();
    }
    virtual QwtText label(double v) const {
        QDateTime upTime = QDateTime::fromTime_t((uint)v);
        return upTime.toLocalTime().toString("hh:mm:ss");
    }
private:
    double baseTime;
};

class TestDataGen : QObject
{
    Q_OBJECT

public:

    TestDataGen();
    ~TestDataGen();

private:
    AltitudeActual* altActual;
    GpsObject* gps;

    QTimer *timer;
    double testTime;

private slots:
    void genTestData();
};


class ScopeGadgetWidget : public QwtPlot
{
    Q_OBJECT

public:
    ScopeGadgetWidget(QWidget *parent = 0);
    ~ScopeGadgetWidget();


private slots:
    void uavObjectReceived(UAVObject*);
    void replotNewData();

private:

    void preparePlot(PlotType plotType);
    void setupExamplePlot();
    void setupSequencialPlot();
    void setupChronoPlot();
    void setupUAVObjectPlot();

    void addCurvePlot(QString uavObject, QString uavField, int scaleOrderFactor = 0, QPen pen = QPen(Qt::black));
    void removeCurvePlot(QString uavObject, QString uavField);
    void clearCurvePlots();

    PlotType m_plotType;

    double m_xWindowSize;
    QVector<QString> m_connectedUAVObjects;
    QMap<QString, PlotData*> m_curvesData;

    static TestDataGen* testDataGen;
};


#endif /* SCOPEGADGETWIDGET_H_ */
