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
#include <QMutexLocker>
#include <QWheelEvent>

//using namespace Core;

// ******************************************************************

ScopeGadgetWidget::ScopeGadgetWidget(QWidget *parent) : QwtPlot(parent)
{
	setMouseTracking(true);
//	canvas()->setMouseTracking(true);

    //Setup the timer that replots data
    replotTimer = new QTimer(this);
    connect(replotTimer, SIGNAL(timeout()), this, SLOT(replotNewData()));

    // Listen to telemetry connection/disconnection events, no point in
    // running the scopes if we are not connected and not replaying logs.
    // Also listen to disconnect actions from the user
    Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();
    connect(cm, SIGNAL(deviceAboutToDisconnect()), this, SLOT(stopPlotting()));
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
    connect(cm, SIGNAL(deviceAboutToDisconnect()), this, SLOT(csvLoggingDisconnect()));
    connect(cm, SIGNAL(deviceConnected(QIODevice*)), this, SLOT(csvLoggingConnect()));
}

ScopeGadgetWidget::~ScopeGadgetWidget()
{
	if (replotTimer)
	{
		replotTimer->stop();

		delete replotTimer;
		replotTimer = NULL;
	}

	// Get the object to de-monitor
	ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
	UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
	foreach (QString uavObjName, m_connectedUAVObjects)
	{
		UAVDataObject *obj = dynamic_cast<UAVDataObject*>(objManager->getObject(uavObjName));
		disconnect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(uavObjectReceived(UAVObject*)));
	}

	clearCurvePlots();
}

// ******************************************************************

void ScopeGadgetWidget::mousePressEvent(QMouseEvent *e)
{
	QwtPlot::mousePressEvent(e);
}

void ScopeGadgetWidget::mouseReleaseEvent(QMouseEvent *e)
{
	QwtPlot::mouseReleaseEvent(e);
}

void ScopeGadgetWidget::mouseDoubleClickEvent(QMouseEvent *e)
{
    //On double-click, toggle legend
    mutex.lock();
    if (legend())
        deleteLegend();
    else
        addLegend();
    mutex.unlock();

    //On double-click, reset plot zoom
    setAxisAutoScale(QwtPlot::yLeft, true);

    update();

	QwtPlot::mouseDoubleClickEvent(e);
}

void ScopeGadgetWidget::mouseMoveEvent(QMouseEvent *e)
{
	QwtPlot::mouseMoveEvent(e);
}

void ScopeGadgetWidget::wheelEvent(QWheelEvent *e)
{
    //Change zoom on scroll wheel event
    QwtInterval yInterval=axisInterval(QwtPlot::yLeft);
    if (yInterval.minValue() != yInterval.maxValue()) //Make sure that the two values are never the same. Sometimes axisInterval returns (0,0)
    {
        //Determine what y value to zoom about. NOTE, this approach has a bug that the in that
        //the value returned by Qt includes the legend, whereas the value transformed by Qwt
        //does *not*. Thus, when zooming with a legend, there will always be a small bias error.
        //In practice, this seems not to be a UI problem.
        QPoint mouse_pos=e->pos(); //Get the mouse coordinate in the frame
        double zoomLine=invTransform(QwtPlot::yLeft, mouse_pos.y()); //Transform the y mouse coordinate into a frame value.

        double zoomScale=1.1; //THIS IS AN ARBITRARY CONSTANT, AND PERHAPS SHOULD BE IN A DEFINE INSTEAD OF BURIED HERE

        mutex.lock(); //DOES THIS mutex.lock NEED TO BE HERE? I DON'T KNOW, I JUST COPIED IT FROM THE ABOVE CODE
        // Set the scale
        if (e->delta()<0){
            setAxisScale(QwtPlot::yLeft,
                         (yInterval.minValue()-zoomLine)*zoomScale+zoomLine,
                         (yInterval.maxValue()-zoomLine)*zoomScale+zoomLine );
        }
        else{
            setAxisScale(QwtPlot::yLeft,
                         (yInterval.minValue()-zoomLine)/zoomScale+zoomLine,
                         (yInterval.maxValue()-zoomLine)/zoomScale+zoomLine );
        }
        mutex.unlock();
    }
    QwtPlot::wheelEvent(e);
}

/**
 * Starts/stops telemetry
 */
void ScopeGadgetWidget::startPlotting()
{
	if (!replotTimer)
		return;

	if (!replotTimer->isActive())
        replotTimer->start(m_refreshInterval);
}

void ScopeGadgetWidget::stopPlotting()
{
	if (!replotTimer)
		return;

	replotTimer->stop();
}

void ScopeGadgetWidget::deleteLegend()
{
	if (!legend())
		return;

	disconnect(this, SIGNAL(legendChecked(QwtPlotItem *, bool)), this, 0);

	insertLegend(NULL, QwtPlot::TopLegend);
//	insertLegend(NULL, QwtPlot::ExternalLegend);
}

void ScopeGadgetWidget::addLegend()
{
	if (legend())
		return;

	// Show a legend at the top
	QwtLegend *legend = new QwtLegend();
	legend->setItemMode(QwtLegend::CheckableItem);
	legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
	legend->setToolTip(tr("Click legend to show/hide scope trace"));

	QPalette pal = legend->palette();
	pal.setColor(legend->backgroundRole(), QColor(100, 100, 100));	// background colour
//	pal.setColor(legend->backgroundRole(), Qt::transparent);		// background colour
//	pal.setColor(QPalette::Text, QColor(255, 255, 255));			// text colour
	pal.setColor(QPalette::Text, QColor(0, 0, 0));			// text colour
	legend->setPalette(pal);

	insertLegend(legend, QwtPlot::TopLegend);
//	insertLegend(legend, QwtPlot::ExternalLegend);

//	// Show a legend at the bottom
//	QwtLegend *legend = new QwtLegend();
//	legend->setItemMode(QwtLegend::CheckableItem);
//	legend->setFrameStyle(QFrame::Box | QFrame::Sunken);
//	insertLegend(legend, QwtPlot::BottomLegend);

    // Update the checked/unchecked state of the legend items
    // -> this is necessary when hiding a legend where some plots are
    //    not visible, and the un-hiding it.
    foreach (QwtPlotItem *item, this->itemList()) {
        bool on = item->isVisible();
        QWidget *w = legend->find(item);
        if ( w && w->inherits("QwtLegendItem") )
            ((QwtLegendItem *)w)->setChecked(!on);
    }

    connect(this, SIGNAL(legendChecked(QwtPlotItem *, bool)), this, SLOT(showCurve(QwtPlotItem *, bool)));
}

void ScopeGadgetWidget::preparePlot(PlotType plotType)
{
    m_plotType = plotType;

    clearCurvePlots();

    setMinimumSize(64, 64);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

//	setMargin(1);

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

    // Add the legend
    addLegend();

    // Only start the timer if we are already connected
    Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();
    if (cm->getCurrentConnection() && replotTimer)
    {
        if (!replotTimer->isActive())
            replotTimer->start(m_refreshInterval);
        else
            replotTimer->setInterval(m_refreshInterval);
    }
}

void ScopeGadgetWidget::showCurve(QwtPlotItem *item, bool on)
{
    item->setVisible(!on);
    QWidget *w = legend()->find(item);
    if ( w && w->inherits("QwtLegendItem") )
        ((QwtLegendItem *)w)->setChecked(on);

	mutex.lock();
		replot();
	mutex.unlock();
}

void ScopeGadgetWidget::setupSequentialPlot()
{
    preparePlot(SequentialPlot);

//	QwtText title("Index");
////	title.setFont(QFont("Helvetica", 20));
//	title.font().setPointSize(title.font().pointSize() / 2);
//	setAxisTitle(QwtPlot::xBottom, title);
////    setAxisTitle(QwtPlot::xBottom, "Index");

    setAxisScaleDraw(QwtPlot::xBottom, new QwtScaleDraw());
    setAxisScale(QwtPlot::xBottom, 0, m_xWindowSize);
    setAxisLabelRotation(QwtPlot::xBottom, 0.0);
    setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);

	QwtScaleWidget *scaleWidget = axisWidget(QwtPlot::xBottom);

	// reduce the gap between the scope canvas and the axis scale
	scaleWidget->setMargin(0);

	// reduce the axis font size
	QFont fnt(axisFont(QwtPlot::xBottom));
	fnt.setPointSize(7);
	setAxisFont(QwtPlot::xBottom, fnt);	// x-axis
	setAxisFont(QwtPlot::yLeft, fnt);	// y-axis
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
//	setAxisLabelRotation(QwtPlot::xBottom, -15.0);
	setAxisLabelRotation(QwtPlot::xBottom, 0.0);
	setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);
//	setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignCenter | Qt::AlignBottom);

	QwtScaleWidget *scaleWidget = axisWidget(QwtPlot::xBottom);
//	QwtScaleDraw *scaleDraw = axisScaleDraw();

	// reduce the gap between the scope canvas and the axis scale
	scaleWidget->setMargin(0);

	// reduce the axis font size
	QFont fnt(axisFont(QwtPlot::xBottom));
	fnt.setPointSize(7);
	setAxisFont(QwtPlot::xBottom, fnt);	// x-axis
	setAxisFont(QwtPlot::yLeft, fnt);	// y-axis

	// set the axis colours .. can't seem to change the background colour :(
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

//	const int fmh = QFontMetrics(scaleWidget->font()).height();
//	scaleWidget->setMinBorderDist(0, fmh / 2);

//	const int fmw = QFontMetrics(scaleWidget->font()).width(" 00:00:00 ");
//	const int fmw = QFontMetrics(scaleWidget->font()).width(" ");
//	scaleWidget->setMinBorderDist(0, fmw);
}

void ScopeGadgetWidget::addCurvePlot(QString uavObject, QString uavFieldSubField, int scaleOrderFactor, int meanSamples, QString mathFunction, QPen pen)
{
    PlotData* plotData;

    if (m_plotType == SequentialPlot)
        plotData = new SequentialPlotData(uavObject, uavFieldSubField);
    else if (m_plotType == ChronoPlot)
        plotData = new ChronoPlotData(uavObject, uavFieldSubField);
    //else if (m_plotType == UAVObjectPlot)
    //    plotData = new UAVObjectPlotData(uavObject, uavField);

    plotData->m_xWindowSize = m_xWindowSize;
    plotData->scalePower = scaleOrderFactor;
    plotData->meanSamples = meanSamples;
    plotData->mathFunction = mathFunction;

    //If the y-bounds are supplied, set them
    if (plotData->yMinimum != plotData->yMaximum)
	{
		setAxisScale(QwtPlot::yLeft, plotData->yMinimum, plotData->yMaximum);
	}

    //Create the curve    
    QString curveName = (plotData->uavObject) + "." + (plotData->uavField);
    if(plotData->haveSubField)
        curveName = curveName.append("." + plotData->uavSubField);

    //Get the uav object
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject* obj = dynamic_cast<UAVDataObject*>(objManager->getObject((plotData->uavObject)));
    if(!obj) {
        qDebug() << "Object " << plotData->uavObject << " is missing";
        return;
    }
    UAVObjectField* field = obj->getField(plotData->uavField);
    if(!field) {
        qDebug() << "Field " << plotData->uavField << " of object " << plotData->uavObject << " is missing";
        return;
    }
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
    plotCurve->setSamples(*plotData->xData, *plotData->yData);
    plotCurve->attach(this);
    plotData->curve = plotCurve;

    //Keep the curve details for later
    m_curvesData.insert(curveNameScaled, plotData);

    //Link to the new signal data only if this UAVObject has not been connected yet
    if (!m_connectedUAVObjects.contains(obj->getName())) {
        m_connectedUAVObjects.append(obj->getName());
        connect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(uavObjectReceived(UAVObject*)));
    }

	mutex.lock();
		replot();
	mutex.unlock();
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
//	mutex.lock();
//	    replot();
//	mutex.unlock();
//}

void ScopeGadgetWidget::uavObjectReceived(UAVObject* obj)
{
    foreach(PlotData* plotData, m_curvesData.values()) {
        if (plotData->append(obj)) m_csvLoggingDataUpdated=1;
    }
    csvLoggingAddData();
}

void ScopeGadgetWidget::replotNewData()
{
	QMutexLocker locker(&mutex);

	foreach(PlotData* plotData, m_curvesData.values())
	{
        plotData->removeStaleData();
        plotData->curve->setSamples(*plotData->xData, *plotData->yData);
    }

    QDateTime NOW = QDateTime::currentDateTime();
    double toTime = NOW.toTime_t();
    toTime += NOW.time().msec() / 1000.0;
	if (m_plotType == ChronoPlot)
        setAxisScale(QwtPlot::xBottom, toTime - m_xWindowSize, toTime);

//	qDebug() << "replotNewData from " << NOW.addSecs(- m_xWindowSize) << " to " << NOW;

    csvLoggingInsertData();

	replot();
}

/*
void ScopeGadgetWidget::setupExamplePlot()
{
    preparePlot(SequentialPlot);

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
    curve1->setSamples(x, sn, points);
    curve2->setSamples(x, cs, points);
    curve3->setSamples(x, sg, points);
    curve1->attach(this);
    curve2->attach(this);
    curve3->attach(this);

    // finally, refresh the plot
	mutex.lock();
		replot();
	mutex.unlock();
}
*/

void ScopeGadgetWidget::clearCurvePlots()
{
    foreach(PlotData* plotData, m_curvesData.values()) {
        plotData->curve->detach();

        delete plotData->curve;
        delete plotData;
    }

    m_curvesData.clear();
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
        m_csvLoggingBuffer.clear();
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

int ScopeGadgetWidget::csvLoggingAddData()
{
    if (!m_csvLoggingStarted) return -1;
    m_csvLoggingDataValid=0;
    QDateTime NOW = QDateTime::currentDateTime();
    QString tempString;

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
            ss  << ", ";
            if (plotData2->xData->isEmpty ())
            {
            }
            else
            {
                ss  << QString().sprintf("%3.10g",plotData2->yData->last());
                m_csvLoggingDataValid=1;
            }
        }
        else
        {
            ss  << QString().sprintf("%3.10g",plotData2->yData->last());
            m_csvLoggingDataValid=1;
        }
    }
    ss << endl;
    if (m_csvLoggingDataValid)
    {
        QTextStream ts( &m_csvLoggingBuffer );
        ts << tempString;
    }

    return 0;
}

int ScopeGadgetWidget::csvLoggingInsertData()
{
    if (!m_csvLoggingStarted) return -1;
    m_csvLoggingDataSaved=1;

    if(m_csvLoggingFile.open(QIODevice::WriteOnly | QIODevice::Append)== FALSE)
    {
        qDebug() << "Unable to open " << m_csvLoggingFile.fileName() << " for csv logging Data";
    }
    else
    {
        QTextStream ts( &m_csvLoggingFile );
        ts << m_csvLoggingBuffer;
        m_csvLoggingFile.close();
    }
    m_csvLoggingBuffer.clear();

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
