/**
 ******************************************************************************
 *
 * @file       scopegadgetwidget.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Scope Gadget Widget
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


#include "uavobjects/uavobjectmanager.h"
#include "extensionsystem/pluginmanager.h"
#include "scopegadgetwidget.h"

#include "qwt/src/qwt_plot_curve.h"
#include "qwt/src/qwt_legend.h"

#include <iostream>
#include <math.h>
#include <QDebug>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>


TestDataGen* ScopeGadgetWidget::testDataGen;

ScopeGadgetWidget::ScopeGadgetWidget(QWidget *parent) : QwtPlot(parent)
{
//    if(testDataGen == 0)
//        testDataGen = new TestDataGen();
}

void ScopeGadgetWidget::preparePlot(PlotType plotType)
{
    m_plotType = plotType;

    clearCurvePlots();

    setMinimumSize(64, 64);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    // Show a title
    setTitle("Scope");

    // Show a legend at the bottom
    if (legend() == 0) {
        QwtLegend *legend = new QwtLegend();
        legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
        insertLegend(legend, QwtPlot::BottomLegend);
    }
}

void ScopeGadgetWidget::setupSequencialPlot()
{
    preparePlot(SequencialPlot);

    setAxisTitle(QwtPlot::xBottom, "Index");
    setAxisScaleDraw(QwtPlot::xBottom, new QwtScaleDraw());
    setAxisScale(QwtPlot::xBottom, 0, m_xWindowSize);
    setAxisLabelRotation(QwtPlot::xBottom, 0.0);
    setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);
}

void ScopeGadgetWidget::setupChronoPlot()
{
    preparePlot(ChronoPlot);

    setAxisTitle(QwtPlot::xBottom, "Time [h:m:s]");
    setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw());
    uint NOW = QDateTime::currentDateTime().toTime_t();
    setAxisScale(QwtPlot::xBottom, NOW - m_xWindowSize, NOW);
    setAxisLabelRotation(QwtPlot::xBottom, -50.0);
    setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);

    /*
     In situations, when there is a label at the most right position of the
     scale, additional space is needed to display the overlapping part
     of the label would be taken by reducing the width of scale and canvas.
     To avoid this "jumping canvas" effect, we add a permanent margin.
     We don't need to do the same for the left border, because there
     is enough space for the overlapping label below the left scale.
     */

    QwtScaleWidget *scaleWidget = axisWidget(QwtPlot::xBottom);
    const int fmh = QFontMetrics(scaleWidget->font()).height();
    scaleWidget->setMinBorderDist(0, fmh / 2);
}

void ScopeGadgetWidget::addCurvePlot(QString uavObject, QString uavField, int scaleOrderFactor, QPen pen)
{
    PlotData* plotData;

    if (m_plotType == SequencialPlot)
        plotData = new SequencialPlotData(uavObject, uavField);
    else if (m_plotType == ChronoPlot)
        plotData = new ChronoPlotData(uavObject, uavField, m_refreshInterval);
    //else if (m_plotType == UAVObjectPlot)
    //    plotData = new UAVObjectPlotData(uavObject, uavField);

    plotData->m_xWindowSize = m_xWindowSize;
    plotData->scalePower = scaleOrderFactor;

    //If the y-bounds are supplied, set them
    if (plotData->yMinimum != plotData->yMaximum)
        setAxisScale(QwtPlot::yLeft, plotData->yMinimum, plotData->yMaximum);

    //Create the curve
    QString curveName = (plotData->uavObject) + "." + (plotData->uavField);
    QwtPlotCurve* plotCurve = new QwtPlotCurve(curveName);
    plotCurve->setPen(pen);
    plotCurve->setData(*plotData->xData, *plotData->yData);
    plotCurve->attach(this);
    plotData->curve = plotCurve;

    //Keep the curve details for later
    m_curvesData.insert(curveName, plotData);

    //Get the object to monitor
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject((plotData->uavObject)));

    //Link to the signal of new data only if this UAVObject has not been to connected yet
    if (!m_connectedUAVObjects.contains(obj->getName())) {
        m_connectedUAVObjects.append(obj->getName());
        connect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(uavObjectReceived(UAVObject*)));
    }

    connect(plotData, SIGNAL(dataChanged()), this, SLOT(replotNewData()));
}

void ScopeGadgetWidget::removeCurvePlot(QString uavObject, QString uavField)
{
    QString curveName = uavObject + "." + uavField;

    PlotData* plotData = m_curvesData.take(curveName);
    m_curvesData.remove(curveName);
    plotData->curve->detach();

    delete plotData->curve;
    delete plotData;
}

void ScopeGadgetWidget::uavObjectReceived(UAVObject* obj)
{
    foreach(PlotData* plotData, m_curvesData.values()) {
        plotData->append(obj);
        plotData->curve->setData(*plotData->xData, *plotData->yData);
    }
}

void ScopeGadgetWidget::replotNewData()
{
    if (m_plotType == ChronoPlot) {
        uint NOW = QDateTime::currentDateTime().toTime_t();
        setAxisScale(QwtPlot::xBottom, NOW - m_xWindowSize, NOW);
    }
    replot();
}


void ScopeGadgetWidget::setupExamplePlot()
{
    preparePlot(SequencialPlot);

    // Show the axes

    setAxisTitle(xBottom, "x");
    setAxisTitle(yLeft, "y");

    // Calculate the data, 500 points each
    const int points = 500;
    double x[ points ];
    double sn[ points ];
    double cs[ points ];
    double sg[ points ];

    for (int i = 0; i < points; i++) {
        x[i] = (3.0 * 3.14 / double(points)) * double(i);
        sn[i] = 2.0 * sin(x[i]);
        cs[i] = 3.0 * cos(x[i]);
        sg[i] = (sn[i] > 0) ? 1 : ((sn[i] < 0) ? -1 : 0);
    }

    // add curves
    QwtPlotCurve *curve1 = new QwtPlotCurve("Curve 1");
    curve1->setPen(QPen(Qt::blue));
    QwtPlotCurve *curve2 = new QwtPlotCurve("Curve 2");
    curve2->setPen(QPen(Qt::red));
    QwtPlotCurve *curve3 = new QwtPlotCurve("Curve 3");
    curve3->setPen(QPen(Qt::green));

    // copy the data into the curves
    curve1->setData(x, sn, points);
    curve2->setData(x, cs, points);
    curve3->setData(x, sg, points);



    curve1->attach(this);
    curve2->attach(this);
    curve3->attach(this);



    // finally, refresh the plot
    replot();
}


ScopeGadgetWidget::~ScopeGadgetWidget()
{
    //Get the object to de-monitor
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    foreach(QString uavObjName, m_connectedUAVObjects) {
        UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject(uavObjName));
        disconnect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(uavObjectReceived(UAVObject*)));
    }

    clearCurvePlots();
}

void ScopeGadgetWidget::clearCurvePlots()
{
    foreach(PlotData* plotData, m_curvesData.values()) {
        plotData->curve->detach();

        delete plotData->curve;
        delete plotData;
    }

    m_curvesData.clear();
}

TestDataGen::TestDataGen()
{
    // Get required UAVObjects
    ExtensionSystem::PluginManager* pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager* objManager = pm->getObject<UAVObjectManager>();

    altActual = AltitudeActual::GetInstance(objManager);
    gps = PositionActual::GetInstance(objManager);

    //Setup timer
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(genTestData()));
    timer->start(5000);
}

void TestDataGen::genTestData()
{
    // Update AltitudeActual object
    AltitudeActual::DataFields altActualData;
    altActualData.Altitude = 500 * sin(0.1 * testTime) + 200 * cos(0.4 * testTime) + 800;
    altActualData.Temperature = 30 * sin(0.05 * testTime);
    altActualData.Pressure = 100;
    altActual->setData(altActualData);


    // Update gps objects
    PositionActual::DataFields gpsData;
    gpsData.Altitude = 0;
    gpsData.Heading = 0;
    gpsData.Groundspeed = 0;
    gpsData.Latitude = 0;
    gpsData.Longitude = 0;
    gpsData.Satellites = 10;
    gps->setData(gpsData);

    testTime++;
}

TestDataGen::~TestDataGen()
{
    if (timer)
        timer->stop();

    delete timer;
}