/**
 ******************************************************************************
 *
 * @file       scopegadgetwidget.cpp
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


#include <QDir>
#include "scopegadgetwidget.h"
#include "utils/stylehelper.h"

#include "uavtalk/telemetrymanager.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include "coreplugin/icore.h"
#include "coreplugin/connectionmanager.h"

#include "qwt/src/qwt_plot_curve.h"
#include "qwt/src/qwt_legend.h"
#include "qwt/src/qwt_legend_item.h"
#include "qwt/src/qwt_plot_grid.h"

#include <iostream>
#include <math.h>
#include <QDebug>
#include <QColor>
#include <QStringList>
#include <QtGui/QWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QPushButton>

using namespace Core;

TestDataGen* ScopeGadgetWidget::testDataGen;

ScopeGadgetWidget::ScopeGadgetWidget(QWidget *parent) : QwtPlot(parent)
{
    //if(testDataGen == 0)
    //    testDataGen = new TestDataGen();

    //Setup the timer that replots data
    replotTimer = new QTimer(this);
    connect(replotTimer, SIGNAL(timeout()), this, SLOT(replotNewData()));

    // Listen to telemetry connection/disconnection events, no point
    // running the scopes if we are not connected and not replaying logs
    // Also listen to disconnect actions from the user
    Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();
    connect(cm, SIGNAL(deviceDisconnected()), this, SLOT(stopPlotting()));
    connect(cm, SIGNAL(deviceConnected(QIODevice*)), this, SLOT(startPlotting()));


    m_csvLoggingStarted=0;
    m_csvLoggingEnabled=0;
    m_csvLoggingHeaderSaved=0;
    m_csvLoggingDataSaved=0;
    m_csvLoggingDataUpdated=0;
    m_csvLoggingNameSet=0;
    m_csvLoggingConnected=0;
    m_csvLoggingNewFileOnConnect=0;
    m_csvLoggingPath = QString("./csvlogging/");
    m_csvLoggingStartTime = QDateTime::currentDateTime();

    //Listen to autopilot connection events
    connect(cm, SIGNAL(deviceDisconnected()), this, SLOT(csvLoggingDisconnect()));
    connect(cm, SIGNAL(deviceConnected(QIODevice*)), this, SLOT(csvLoggingConnect()));

}

/**
 * Starts/stops telemetry
 */
void ScopeGadgetWidget::startPlotting()
{
    if(!replotTimer->isActive())
        replotTimer->start(m_refreshInterval);
}

void ScopeGadgetWidget::stopPlotting()
{
    replotTimer->stop();
}

void ScopeGadgetWidget::preparePlot(PlotType plotType)
{
    m_plotType = plotType;

    clearCurvePlots();

    setMinimumSize(64, 64);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    // Show a title
//	setTitle("Scope");

	setMargin(1);

//	QPalette pal = palette();
//	QPalette::ColorRole cr = backgroundRole();
//	pal.setColor(cr, QColor(128, 128, 128));				// background colour
//	pal.setColor(QPalette::Text, QColor(255, 255, 255));	// text colour
//	setPalette(pal);

//    setCanvasBackground(Utils::StyleHelper::baseColor());
	setCanvasBackground(QColor(64, 64, 64));

    //Add grid lines
    QwtPlotGrid *grid = new QwtPlotGrid;
	grid->setMajPen(QPen(Qt::gray, 0, Qt::DashLine));
	grid->setMinPen(QPen(Qt::lightGray, 0, Qt::DotLine));
	grid->setPen(QPen(Qt::darkGray, 1, Qt::DotLine));
	grid->attach(this);

	// Show a legend at the top
	if (!legend())
	{
		QwtLegend *legend = new QwtLegend();
		legend->setItemMode(QwtLegend::CheckableItem);
		legend->setFrameStyle(QFrame::Box | QFrame::Sunken);

		QPalette::ColorRole br = legend->backgroundRole();
		QPalette pal = legend->palette();
//		pal.setColor(QPalette::Window, QColor(64, 64, 64));		// background colour
		pal.setColor(br, QColor(128, 128, 128));				// background colour
		pal.setColor(QPalette::Text, QColor(255, 255, 255));	// text colour
		legend->setPalette(pal);

		insertLegend(legend, QwtPlot::TopLegend);
	}
	// Show a legend at the bottom
//	if (legend() == 0) {
//		QwtLegend *legend = new QwtLegend();
//		legend->setItemMode(QwtLegend::CheckableItem);
//		legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
//		insertLegend(legend, QwtPlot::BottomLegend);
//	}

    connect(this, SIGNAL(legendChecked(QwtPlotItem *, bool)),this, SLOT(showCurve(QwtPlotItem *, bool)));

    // Only start the timer if we are already connected
    Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();
    if (cm->getCurrentConnection()) {
        if(!replotTimer->isActive())
            replotTimer->start(m_refreshInterval);
        else
        {
            replotTimer->setInterval(m_refreshInterval);
        }
    }

}

void ScopeGadgetWidget::showCurve(QwtPlotItem *item, bool on)
{
    item->setVisible(!on);
    QWidget *w = legend()->find(item);
    if ( w && w->inherits("QwtLegendItem") )
        ((QwtLegendItem *)w)->setChecked(on);

    replot();
}

void ScopeGadgetWidget::setupSequencialPlot()
{
    preparePlot(SequencialPlot);

//	QwtText title("Index");
////	title.setFont(QFont("Helvetica", 20));
//	title.font().setPointSize(title.font().pointSize() / 2);
//	setAxisTitle(QwtPlot::xBottom, title);
////    setAxisTitle(QwtPlot::xBottom, "Index");

    setAxisScaleDraw(QwtPlot::xBottom, new QwtScaleDraw());
    setAxisScale(QwtPlot::xBottom, 0, m_xWindowSize);
    setAxisLabelRotation(QwtPlot::xBottom, 0.0);
    setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);
}

void ScopeGadgetWidget::setupChronoPlot()
{
    preparePlot(ChronoPlot);

//	QwtText title("Time [h:m:s]");
////	title.setFont(QFont("Helvetica", 20));
//	title.font().setPointSize(title.font().pointSize() / 2);
//	setAxisTitle(QwtPlot::xBottom, title);
////	setAxisTitle(QwtPlot::xBottom, "Time [h:m:s]");

    setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw());
    uint NOW = QDateTime::currentDateTime().toTime_t();
    setAxisScale(QwtPlot::xBottom, NOW - m_xWindowSize / 1000, NOW);
    setAxisLabelRotation(QwtPlot::xBottom, -15.0);
    setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);

	QwtScaleWidget *scaleWidget = axisWidget(QwtPlot::xBottom);
//	QwtScaleDraw *scaleDraw = axisScaleDraw();

	// set the axis colours .. can;t seem to change the background colour :(
//	QPalette pal = scaleWidget->palette();
//	QPalette::ColorRole cr = scaleWidget->backgroundRole();
//	pal.setColor(cr, QColor(128, 128, 128));				// background colour
//	cr = scaleWidget->foregroundRole();
//	pal.setColor(cr, QColor(255, 255, 255));				// tick colour
//	pal.setColor(QPalette::Text, QColor(255, 255, 255));	// text colour
//	scaleWidget->setPalette(pal);

    /*
     In situations, when there is a label at the most right position of the
     scale, additional space is needed to display the overlapping part
     of the label would be taken by reducing the width of scale and canvas.
     To avoid this "jumping canvas" effect, we add a permanent margin.
     We don't need to do the same for the left border, because there
     is enough space for the overlapping label below the left scale.
     */

    const int fmh = QFontMetrics(scaleWidget->font()).height();
	scaleWidget->setMinBorderDist(0, fmh / 2);
}

void ScopeGadgetWidget::addCurvePlot(QString uavObject, QString uavFieldSubField, int scaleOrderFactor, QPen pen)
{
    PlotData* plotData;

    if (m_plotType == SequencialPlot)
        plotData = new SequencialPlotData(uavObject, uavFieldSubField);
    else if (m_plotType == ChronoPlot)
        plotData = new ChronoPlotData(uavObject, uavFieldSubField);
    //else if (m_plotType == UAVObjectPlot)
    //    plotData = new UAVObjectPlotData(uavObject, uavField);

    plotData->m_xWindowSize = m_xWindowSize;
    plotData->scalePower = scaleOrderFactor;

    //If the y-bounds are supplied, set them
    if (plotData->yMinimum != plotData->yMaximum)
		setAxisScale(QwtPlot::yLeft, plotData->yMinimum, plotData->yMaximum);

    //Create the curve    
    QString curveName = (plotData->uavObject) + "." + (plotData->uavField);
    if(plotData->haveSubField)
        curveName = curveName.append("." + plotData->uavSubField);

    //Get the uav object
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject((plotData->uavObject)));

    UAVObjectField* field = obj->getField(plotData->uavField);
    QString units = field->getUnits();

    if(units == 0)
        units = QString();

    QString curveNameScaled;
    if(scaleOrderFactor == 0)
        curveNameScaled = curveName + "(" + units + ")";
    else
        curveNameScaled = curveName + "(x10^" + QString::number(scaleOrderFactor) + " " + units + ")";

    QwtPlotCurve* plotCurve = new QwtPlotCurve(curveNameScaled);
    plotCurve->setPen(pen);
    plotCurve->setData(*plotData->xData, *plotData->yData);
    plotCurve->attach(this);
    plotData->curve = plotCurve;

    //Keep the curve details for later
    m_curvesData.insert(curveNameScaled, plotData);

    //Link to the signal of new data only if this UAVObject has not been to connected yet
    if (!m_connectedUAVObjects.contains(obj->getName())) {
        m_connectedUAVObjects.append(obj->getName());
        connect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(uavObjectReceived(UAVObject*)));
    }

    replot();
}

//void ScopeGadgetWidget::removeCurvePlot(QString uavObject, QString uavField)
//{
//    QString curveName = uavObject + "." + uavField;
//
//    PlotData* plotData = m_curvesData.take(curveName);
//    m_curvesData.remove(curveName);
//    plotData->curve->detach();
//
//    delete plotData->curve;
//    delete plotData;
//
//    replot();
//}

void ScopeGadgetWidget::uavObjectReceived(UAVObject* obj)
{
    foreach(PlotData* plotData, m_curvesData.values()) {
        if (plotData->append(obj)) m_csvLoggingDataUpdated=1;
    }
}

void ScopeGadgetWidget::replotNewData()
{
    foreach(PlotData* plotData, m_curvesData.values()) {
        plotData->removeStaleData();
        plotData->curve->setData(*plotData->xData, *plotData->yData);
    }

    QDateTime NOW = QDateTime::currentDateTime();
    double toTime = NOW.toTime_t();
    toTime += NOW.time().msec() / 1000.0;
    if (m_plotType == ChronoPlot) {
        setAxisScale(QwtPlot::xBottom, toTime - m_xWindowSize, toTime);
    }
     //qDebug() << "replotNewData from " << NOW.addSecs(- m_xWindowSize) << " to " << NOW;
    csvLoggingInsertData();
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
    if (replotTimer)
        replotTimer->stop();
    delete replotTimer;
    replotTimer = 0;

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

    baroAltitude = BaroAltitude::GetInstance(objManager);
    gps = PositionActual::GetInstance(objManager);
    attRaw = AttitudeRaw::GetInstance(objManager);
    manCtrlCmd = ManualControlCommand::GetInstance(objManager);

    //Setup timer
    periodMs = 20;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(genTestData()));
    timer->start(periodMs);

    debugCounter = 0;
    testTime = 0;
}

void TestDataGen::genTestData()
{
    // Update BaroAltitude object
    BaroAltitude::DataFields baroAltitudeData;
    baroAltitudeData.Altitude = 500 * sin(1 * testTime) + 200 * cos(4 * testTime) + 800;
    baroAltitudeData.Temperature = 30 * sin(0.5 * testTime);
    baroAltitudeData.Pressure = baroAltitudeData.Altitude * 0.01 + baroAltitudeData.Temperature;
    baroAltitude->setData(baroAltitudeData);

    // Update Attitude Raw data
    AttitudeRaw::DataFields attData;
//    attData.accels[0] = 4 *  sin(2 * testTime) + 1 * cos(6 * testTime) + 4;
//    attData.accels[1] = 3 * sin(2.3 * testTime) + 1.5 * cos(3.3 * testTime) + 2;
//    attData.accels[2] = 4 * sin(5.3 * testTime) + 1.5 * cos(1.3 * testTime) + 1;
    attData.accels[0] = 1;
    attData.accels[1] = 4;
    attData.accels[2] = 9;
    attRaw->setData(attData);


    ManualControlCommand::DataFields manCtlData;
    manCtlData.Channel[0] = 400 * cos(2 * testTime) + 100 * sin(6 * testTime) + 400;
    manCtlData.Channel[1] = 350 * cos(2.3 * testTime) + 150 * sin(3.3 * testTime) + 200;
    manCtlData.Channel[2] = 450 * cos(5.3 * testTime) + 150 * sin(1.3 * testTime) + 150;
    manCtrlCmd->setData(manCtlData);

    testTime += (periodMs / 1000.0);

//    debugCounter++;
//    if (debugCounter % (100/periodMs) == 0 )
//        qDebug() << "Test Time = " << testTime;
}

TestDataGen::~TestDataGen()
{
    if (timer)
        timer->stop();

    delete timer;
}

/*
int csvLoggingEnable;
int csvLoggingHeaderSaved;
int csvLoggingDataSaved;
QString csvLoggingPath;
QFile csvLoggingFile;
*/
int ScopeGadgetWidget::csvLoggingStart()
{
    if (!m_csvLoggingStarted)
    if (m_csvLoggingEnabled)
    if ((!m_csvLoggingNewFileOnConnect)||(m_csvLoggingNewFileOnConnect && m_csvLoggingConnected))
    {
        QDateTime NOW = QDateTime::currentDateTime();
        m_csvLoggingStartTime = NOW;
        m_csvLoggingHeaderSaved=0;
        m_csvLoggingDataSaved=0;
        QDir PathCheck(m_csvLoggingPath);
        if (!PathCheck.exists())
        {
            PathCheck.mkpath("./");
        }


        if (m_csvLoggingNameSet)
        {
            m_csvLoggingFile.setFileName(QString("%1/%2_%3_%4.csv").arg(m_csvLoggingPath).arg(m_csvLoggingName).arg(NOW.toString("yyyy-MM-dd")).arg(NOW.toString("hh-mm-ss")));
        }
        else
        {
            m_csvLoggingFile.setFileName(QString("%1/Log_%2_%3.csv").arg(m_csvLoggingPath).arg(NOW.toString("yyyy-MM-dd")).arg(NOW.toString("hh-mm-ss")));
        }
        QDir FileCheck(m_csvLoggingFile.fileName());
        if (FileCheck.exists())
        {
            m_csvLoggingFile.setFileName("");
        }
        else
        {
            m_csvLoggingStarted=1;
            csvLoggingInsertHeader();
        }

    }

    return 0;
}

int ScopeGadgetWidget::csvLoggingStop()
{
    m_csvLoggingStarted=0;

    return 0;
}

int ScopeGadgetWidget::csvLoggingInsertHeader()
{
    if (!m_csvLoggingStarted) return -1;
    if (m_csvLoggingHeaderSaved) return -2;
    if (m_csvLoggingDataSaved) return -3;

    m_csvLoggingHeaderSaved=1;
    if(m_csvLoggingFile.open(QIODevice::WriteOnly | QIODevice::Append)== FALSE)
    {
        qDebug() << "Unable to open " << m_csvLoggingFile.fileName() << " for csv logging Header";
    }
    else
    {
        QTextStream ts( &m_csvLoggingFile );
        ts << "date" << ", " << "Time"<< ", " << "Sec since start"<< ", " << "Connected" << ", " << "Data changed";

        foreach(PlotData* plotData2, m_curvesData.values())
        {
            ts  << ", ";
            ts  << plotData2->uavObject;
            ts  << "." << plotData2->uavField;
            if (plotData2->haveSubField) ts  << "." << plotData2->uavSubField;
        }
        ts << endl;
        m_csvLoggingFile.close();
    }
    return 0;
}

int ScopeGadgetWidget::csvLoggingInsertData()
{
    if (!m_csvLoggingStarted) return -1;
    m_csvLoggingDataSaved=1;
    m_csvLoggingDataValid=0;
    QDateTime NOW = QDateTime::currentDateTime();
    QString tempString;

    if(m_csvLoggingFile.open(QIODevice::WriteOnly | QIODevice::Append)== FALSE)
    {
        qDebug() << "Unable to open " << m_csvLoggingFile.fileName() << " for csv logging Data";
    }
    else
    {
        QTextStream ss( &tempString );
        ss << NOW.toString("yyyy-MM-dd") << ", " << NOW.toString("hh:mm:ss.z") << ", " ;

#if QT_VERSION >= 0x040700
        ss <<(NOW.toMSecsSinceEpoch() - m_csvLoggingStartTime.toMSecsSinceEpoch())/1000.00;
#else
        ss <<(NOW.toTime_t() - m_csvLoggingStartTime.toTime_t());
#endif
        ss << ", " << m_csvLoggingConnected << ", " << m_csvLoggingDataUpdated;
        m_csvLoggingDataUpdated=0;

        foreach(PlotData* plotData2, m_curvesData.values())
        {
            ss  << ", ";
            if (plotData2->xData->isEmpty ())
            {
            }
            else
            {
                ss  << QString().sprintf("%3.6g",plotData2->yData->last()/pow(10,plotData2->scalePower));
                m_csvLoggingDataValid=1;
            }
        }
        ss << endl;
        if (m_csvLoggingDataValid)
        {
            QTextStream ts( &m_csvLoggingFile );
            ts << tempString;
        }
        m_csvLoggingFile.close();
    }


    return 0;
}
void ScopeGadgetWidget::csvLoggingSetName(QString newName)
{
    m_csvLoggingName = newName;
    m_csvLoggingNameSet=1;
}



void ScopeGadgetWidget::csvLoggingConnect()
{
    m_csvLoggingConnected=1;
    if (m_csvLoggingNewFileOnConnect)csvLoggingStart();
    return;
}
void ScopeGadgetWidget::csvLoggingDisconnect()
{
    m_csvLoggingHeaderSaved=0;
    m_csvLoggingConnected=0;
    if (m_csvLoggingNewFileOnConnect)csvLoggingStop();
    return;
}

