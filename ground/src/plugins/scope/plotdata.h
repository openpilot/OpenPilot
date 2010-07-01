/**
 ******************************************************************************
 *
 * @file       plotdata.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Object that manages the data for a curve.
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   scopeplugin
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
#include "uavobjects/positionactual.h"


#include "qwt/src/qwt.h"
#include "qwt/src/qwt_plot.h"
#include "qwt/src/qwt_plot_curve.h"
#include "qwt/src/qwt_scale_draw.h"
#include "qwt/src/qwt_scale_widget.h"

#include <QTimer>
#include <QTime>
#include <QVector>

/*!
\brief Defines the different type of plots.
  */
enum PlotType {
    SequencialPlot,
    ChronoPlot,
    UAVObjectPlot,

    NPlotTypes
};

/*!
  \brief Base class that keeps the data for each curve in the plot.
  */
class PlotData : public QObject
{
    Q_OBJECT

public:
    PlotData(QString uavObject, QString uavField);
    ~PlotData();

    QString uavObject;
    QString uavField;
    int scalePower; //This is the power to which each value must be raised
    double yMinimum;
    double yMaximum;
    double m_xWindowSize;
    QwtPlotCurve* curve;
    QVector<double>* xData;
    QVector<double>* yData;

    virtual bool append(UAVObject* obj) = 0;
    virtual PlotType plotType() = 0;

signals:
    void dataChanged();
};

/*!
  \brief The sequencial plot have a fixed size buffer of data. All the curves in one plot
  have the same size buffer.
  */
class SequencialPlotData : public PlotData
{
    Q_OBJECT
public:
    SequencialPlotData(QString uavObject, QString uavField)
            : PlotData(uavObject, uavField) {}
    ~SequencialPlotData() {}

    /*!
      \brief Append new data to the plot
      */
    bool append(UAVObject* obj);

    /*!
      \brief The type of plot
      */
    virtual PlotType plotType() {
        return SequencialPlot;
    }
};

/*!
  \brief The chrono plot have a variable sized buffer of data, where the data is for a specified time period.
  */
class ChronoPlotData : public PlotData
{
    Q_OBJECT
public:
    ChronoPlotData(QString uavObject, QString uavField, double refreshInterval)
            : PlotData(uavObject, uavField) {
        scalePower = 1;
        //Setup timer that removes stale data
        timer = new QTimer();
        connect(timer, SIGNAL(timeout()), this, SLOT(removeStaleDataTimeout()));
        timer->start(refreshInterval * 1000);
    }
    ~ChronoPlotData() {
        delete timer;
    }

    bool append(UAVObject* obj);

    virtual PlotType plotType() {
        return ChronoPlot;
    }
private:
    void removeStaleData();

    QTimer *timer;

private slots:
    void removeStaleDataTimeout();
};

/*!
  \brief UAVObject plot use a fixed size buffer of data, where the horizontal axis values come from
  a UAVObject field.
  */
class UAVObjectPlotData : public PlotData
{
    Q_OBJECT
public:
    UAVObjectPlotData(QString uavObject, QString uavField)
            : PlotData(uavObject, uavField) {}
    ~UAVObjectPlotData() {}

    bool append(UAVObject* obj);

    virtual PlotType plotType() {
        return UAVObjectPlot;
    }
};

#endif // PLOTDATA_H
