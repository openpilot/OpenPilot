/**
 ******************************************************************************
 *
 * @file       plotdata.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Object that manages the data for a curve.
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

#ifndef PLOTDATA_H
#define PLOTDATA_H

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

enum PlotType {
    SequencialPlot,
    ChronoPlot,
    UAVObjectPlot,

    NPlotTypes
};


class PlotData : public QObject
{
    Q_OBJECT

public:
    PlotData(QString* uavObject, QString* uavField);
    ~PlotData();

    QString* uavObject;
    QString* uavField;
    QwtPlotCurve* curve;
    QVector<double>* xData;
    QVector<double>* yData;
    int scalePower; //This is the power to wich each value must be raised
    double yMinimum;
    double yMaximum;

    double m_xWindowSize;

    virtual bool append(UAVObject* obj) = 0;
    virtual PlotType plotType() = 0;

signals:
    void dataChanged();
};

class SequencialPlotData : public PlotData
{
    Q_OBJECT
public:
    SequencialPlotData(QString* uavObject, QString* uavField)
            : PlotData(uavObject, uavField) {}
    ~SequencialPlotData() {}

    bool append(UAVObject* obj);

    virtual PlotType plotType() {
        return SequencialPlot;
    }
};

class ChronoPlotData : public PlotData
{
    Q_OBJECT
public:
    ChronoPlotData(QString* uavObject, QString* uavField)
            : PlotData(uavObject, uavField) {
        scalePower = 1;
        //Setup timer
        timer = new QTimer();
        connect(timer, SIGNAL(timeout()), this, SLOT(removeStaleDataTimeout()));
        timer->start(100);
    }
    ~ChronoPlotData() {
        delete timer;
    }

    bool append(UAVObject* obj);

    virtual PlotType plotType() {
        return ChronoPlot;
    }
private:
    //QDateTime m_epoch;
    void removeStaleData();

    QTimer *timer;

private slots:
    void removeStaleDataTimeout();
};

class UAVObjectPlotData : public PlotData
{
    Q_OBJECT
public:
    UAVObjectPlotData(QString* uavObject, QString* uavField)
            : PlotData(uavObject, uavField) {}
    ~UAVObjectPlotData() {}

    bool append(UAVObject* obj);

    virtual PlotType plotType() {
        return UAVObjectPlot;
    }
};

#endif // PLOTDATA_H
