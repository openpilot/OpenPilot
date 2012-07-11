/**
 ******************************************************************************
 *
 * @file       plotdata.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ScopePlugin Scope Gadget Plugin
 * @{
 * @brief The scope Gadget, graphically plots the states of UAVObjects
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

#include "uavobject.h"

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
    SequentialPlot,
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
    QString uavSubField;
    bool haveSubField;
    int scalePower; //This is the power to which each value must be raised
    int meanSamples;
    double meanSum;
    QString mathFunction;
    double correctionSum;
    int correctionCount;
    double yMinimum;
    double yMaximum;
    double m_xWindowSize;
    QwtPlotCurve* curve;
    QVector<double>* xData;
    QVector<double>* yData;
    QVector<double>* yDataHistory;

    virtual bool append(UAVObject* obj) = 0;
    virtual PlotType plotType() = 0;
    virtual void removeStaleData() = 0;

    void updatePlotCurveData();

protected:
    double valueAsDouble(UAVObject* obj, UAVObjectField* field);

signals:
    void dataChanged();
};

/*!
  \brief The sequential plot have a fixed size buffer of data. All the curves in one plot
  have the same size buffer.
  */
class SequentialPlotData : public PlotData
{
    Q_OBJECT
public:
    SequentialPlotData(QString uavObject, QString uavField)
            : PlotData(uavObject, uavField) {}
    ~SequentialPlotData() {}

    /*!
      \brief Append new data to the plot
      */
    bool append(UAVObject* obj);

    /*!
      \brief The type of plot
      */
    virtual PlotType plotType() {
        return SequentialPlot;
    }

    /*!
      \brief Removes the old data from the buffer
      */
    virtual void removeStaleData(){}
};

/*!
  \brief The chrono plot have a variable sized buffer of data, where the data is for a specified time period.
  */
class ChronoPlotData : public PlotData
{
    Q_OBJECT
public:
    ChronoPlotData(QString uavObject, QString uavField)
            : PlotData(uavObject, uavField) {
        scalePower = 1;
    }
    ~ChronoPlotData() {
    }

    bool append(UAVObject* obj);

    virtual PlotType plotType() {
        return ChronoPlot;
    }

    virtual void removeStaleData();

private:

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

    virtual void removeStaleData(){}
};

#endif // PLOTDATA_H
