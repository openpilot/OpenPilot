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
#include <uavdataobject.h>

/*!
   \brief Defines the different type of plots.
 */
enum PlotType {SequentialPlot, ChronoPlot};

/*!
   \brief Base class that keeps the data for each curve in the plot.
 */
class PlotData : public QObject {
    Q_OBJECT

public:
    PlotData(UAVObject *object, UAVObjectField *field, int element,
             QwtPlotCurve *plotCurve, int scaleOrderFactor, int meanSamples,
             QString mathFunction, double plotDataSize);
    ~PlotData();

    UAVObject *object() const { return m_object; }
    UAVObjectField *field() const { return m_field; }
    int element() const { return m_element; }
    QString elementName() const { return m_elementName; }

    bool isVisible() const { return m_plotCurve->isVisible(); }

    double yMin() const { return m_yMin;}
    double yMax() const { return m_yMax;}

    virtual bool append(UAVObject *obj) = 0;
    virtual PlotType plotType() const    = 0;
    virtual void removeStaleData() = 0;

    void updatePlotCurveData();

    bool hasData() { return !m_xDataEntries.isEmpty(); }
    double lastData() { return m_yDataEntries.last(); }

protected:
    double valueAsDouble(UAVObject *obj, UAVObjectField *field);

    int m_scalePower; // This is the power to which each value must be raised
    int m_meanSamples;
    double m_meanSum;
    QString m_mathFunction;
    double m_correctionSum;
    int m_correctionCount;
    double m_plotDataSize;

    QVector<double> m_xDataEntries;
    QVector<double> m_yDataEntries;
    QVector<double> m_yDataHistory;

    UAVObject *m_object;
    UAVObjectField *m_field;
    int m_element;
    QString m_elementName;

    virtual void calcMathFunction(double currentValue);

private:
    double m_yMin;
    double m_yMax;
    QwtPlotCurve *m_plotCurve;
};

/*!
   \brief The sequential plot have a fixed size buffer of data. All the curves in one plot
   have the same size buffer.
 */
class SequentialPlotData : public PlotData {
    Q_OBJECT
public:
    SequentialPlotData(UAVObject *object, UAVObjectField *field, int element,
                       QwtPlotCurve *plotCurve, int scaleFactor, int meanSamples,
                       QString mathFunction, double plotDataSize)
        : PlotData(object, field, element, plotCurve, scaleFactor, meanSamples, mathFunction, plotDataSize) {}
    ~SequentialPlotData() {}

    /*!
       \brief Append new data to the plot
     */
    bool append(UAVObject *obj);

    /*!
       \brief The type of plot
     */
    PlotType plotType() const
    {
        return SequentialPlot;
    }

    /*!
       \brief Removes the old data from the buffer
     */
    void removeStaleData() {}
};

/*!
   \brief The chrono plot have a variable sized buffer of data, where the data is for a specified time period.
 */
class ChronoPlotData : public PlotData {
    Q_OBJECT
public:
    ChronoPlotData(UAVObject *object, UAVObjectField *field, int element,
                   QwtPlotCurve *plotCurve, int scaleFactor, int meanSamples,
                   QString mathFunction, double plotDataSize)
        : PlotData(object, field, element, plotCurve, scaleFactor, meanSamples, mathFunction, plotDataSize)
    {
    }
    ~ChronoPlotData() {}

    bool append(UAVObject *obj);

    PlotType plotType() const
    {
        return ChronoPlot;
    }

    void removeStaleData();

private slots:
    void removeStaleDataTimeout();
};

#endif // PLOTDATA_H
