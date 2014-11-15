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

#include "scopegadgetwidget.h"
#include "utils/stylehelper.h"

#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include "coreplugin/icore.h"
#include "coreplugin/connectionmanager.h"
#include <coreplugin/icore.h>

#include "qwt/src/qwt_plot_curve.h"
#include "qwt/src/qwt_plot_grid.h"

#include <iostream>
#include <math.h>
#include <QDebug>
#include <QDir>
#include <QColor>
#include <QStringList>
#include <QWidget>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMutexLocker>
#include <QWheelEvent>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <QApplication>

#include <qwt/src/qwt_legend_label.h>
#include <qwt/src/qwt_plot_canvas.h>
#include <qwt/src/qwt_plot_layout.h>

ScopeGadgetWidget::ScopeGadgetWidget(QWidget *parent) : QwtPlot(parent),
    m_csvLoggingStarted(false), m_csvLoggingEnabled(false),
    m_csvLoggingHeaderSaved(false), m_csvLoggingDataSaved(false),
    m_csvLoggingNameSet(false), m_csvLoggingDataValid(false),
    m_csvLoggingDataUpdated(false), m_csvLoggingConnected(false),
    m_csvLoggingNewFileOnConnect(false),
    m_csvLoggingStartTime(QDateTime::currentDateTime()),
    m_csvLoggingPath("./csvlogging/"),
    m_plotLegend(NULL)
{
    setMouseTracking(true);

    QwtPlotCanvas *plotCanvas = dynamic_cast<QwtPlotCanvas *>(canvas());
    if (plotCanvas) {
        plotCanvas->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
        plotCanvas->setBorderRadius(8);
    }

    axisWidget(QwtPlot::yLeft)->setMargin(2);
    axisWidget(QwtPlot::xBottom)->setMargin(2);

    // Setup the timer that replots data
    replotTimer = new QTimer(this);
    connect(replotTimer, SIGNAL(timeout()), this, SLOT(replotNewData()));

    // Listen to telemetry connection/disconnection events, no point in
    // running the scopes if we are not connected and not replaying logs.
    // Also listen to disconnect actions from the user
    Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();
    connect(cm, SIGNAL(deviceAboutToDisconnect()), this, SLOT(stopPlotting()));
    connect(cm, SIGNAL(deviceConnected(QIODevice *)), this, SLOT(startPlotting()));

    // Listen to autopilot connection events
    connect(cm, SIGNAL(deviceAboutToDisconnect()), this, SLOT(csvLoggingDisconnect()));
    connect(cm, SIGNAL(deviceConnected(QIODevice *)), this, SLOT(csvLoggingConnect()));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(popUpMenu(const QPoint &)));
}

ScopeGadgetWidget::~ScopeGadgetWidget()
{
    if (replotTimer) {
        replotTimer->stop();
        delete replotTimer;
        replotTimer = NULL;
    }

    // Get the object to de-monitor
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    foreach(QString uavObjName, m_connectedUAVObjects) {
        UAVDataObject *obj = dynamic_cast<UAVDataObject *>(objManager->getObject(uavObjName));

        disconnect(obj, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(uavObjectReceived(UAVObject *)));
    }

    clearCurvePlots();
}

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
    // On double-click, toggle legend
    m_mutex.lock();
    if (legend()) {
        deleteLegend();
    } else {
        addLegend();
    }
    m_mutex.unlock();

    // On double-click, reset plot zoom
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
    // Change zoom on scroll wheel event
    QwtInterval yInterval = axisInterval(QwtPlot::yLeft);

    if (yInterval.minValue() != yInterval.maxValue()) { // Make sure that the two values are never the same. Sometimes axisInterval returns (0,0)
        // Determine what y value to zoom about. NOTE, this approach has a bug that the in that
        // the value returned by Qt includes the legend, whereas the value transformed by Qwt
        // does *not*. Thus, when zooming with a legend, there will always be a small bias error.
        // In practice, this seems not to be a UI problem.
        QPoint mouse_pos = e->pos(); // Get the mouse coordinate in the frame
        double zoomLine  = invTransform(QwtPlot::yLeft, mouse_pos.y()); // Transform the y mouse coordinate into a frame value.

        double zoomScale = 1.1; // THIS IS AN ARBITRARY CONSTANT, AND PERHAPS SHOULD BE IN A DEFINE INSTEAD OF BURIED HERE

        m_mutex.lock(); // DOES THIS mutex.lock NEED TO BE HERE? I DON'T KNOW, I JUST COPIED IT FROM THE ABOVE CODE
        // Set the scale
        if (e->delta() < 0) {
            setAxisScale(QwtPlot::yLeft,
                         (yInterval.minValue() - zoomLine) * zoomScale + zoomLine,
                         (yInterval.maxValue() - zoomLine) * zoomScale + zoomLine);
        } else {
            setAxisScale(QwtPlot::yLeft,
                         (yInterval.minValue() - zoomLine) / zoomScale + zoomLine,
                         (yInterval.maxValue() - zoomLine) / zoomScale + zoomLine);
        }
        m_mutex.unlock();
    }
    QwtPlot::wheelEvent(e);
}

void ScopeGadgetWidget::showEvent(QShowEvent *e)
{
    replotNewData();
    QwtPlot::showEvent(e);
}

/**
 * Starts/stops telemetry
 */
void ScopeGadgetWidget::startPlotting()
{
    if (replotTimer && !replotTimer->isActive()) {
        foreach(PlotData * plot, m_curvesData.values()) {
            if (plot->wantsInitialData()) {
                plot->append(NULL);
            }
        }
        replotTimer->start(m_refreshInterval);
    }
}

void ScopeGadgetWidget::stopPlotting()
{
    if (replotTimer) {
        replotTimer->stop();
    }
}

void ScopeGadgetWidget::deleteLegend()
{
    if (m_plotLegend) {
        insertLegend(NULL, QwtPlot::TopLegend);
        m_plotLegend = NULL;
    }
}

void ScopeGadgetWidget::addLegend()
{
    if (legend()) {
        return;
    }

    // Show a legend at the top
    m_plotLegend = new QwtLegend(this);
    m_plotLegend->setDefaultItemMode(QwtLegendData::Checkable);
    m_plotLegend->setFrameStyle(QFrame::NoFrame);
    m_plotLegend->setToolTip(tr("Click legend to show/hide scope trace.\nDouble click legend or plot to show/hide legend."));

    // set colors
    QPalette pal = m_plotLegend->palette();
    pal.setColor(m_plotLegend->backgroundRole(), QColor(100, 100, 100));
    pal.setColor(QPalette::Text, QColor(0, 0, 0));
    m_plotLegend->setPalette(pal);

    insertLegend(m_plotLegend, QwtPlot::TopLegend);

    // Update the checked/unchecked state of the legend items
    // -> this is necessary when hiding a legend where some plots are
    // not visible, and then un-hiding it.
    foreach(QwtPlotItem * plotItem, itemList()) {
        QWidget *legendWidget = m_plotLegend->legendWidget(QwtPlot::itemToInfo(plotItem));

        if (legendWidget && legendWidget->inherits("QwtLegendLabel")) {
            ((QwtLegendLabel *)legendWidget)->setChecked(!plotItem->isVisible());
        }
    }

    connect(m_plotLegend, SIGNAL(checked(QVariant, bool, int)), this, SLOT(showCurve(QVariant, bool, int)));
}

void ScopeGadgetWidget::preparePlot(PlotType plotType)
{
    m_plotType = plotType;

    clearCurvePlots();

    setMinimumSize(64, 64);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    setCanvasBackground(QColor(64, 64, 64));

    // Add grid lines
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->setPen(Qt::darkGray, 1, Qt::DotLine);
    grid->attach(this);

    // Only start the timer if we are already connected
    Core::ConnectionManager *cm = Core::ICore::instance()->connectionManager();
    if (cm->isConnected() && replotTimer) {
        if (!replotTimer->isActive()) {
            replotTimer->start(m_refreshInterval);
        } else {
            replotTimer->setInterval(m_refreshInterval);
        }
    }
}

void ScopeGadgetWidget::showCurve(QVariant itemInfo, bool visible, int index)
{
    Q_UNUSED(index);
    QwtPlotItem *item = infoToItem(itemInfo);
    item->setVisible(!visible);
    emit visibilityChanged(item);
    if (m_plotLegend) {
        QWidget *legendItem = legend()->find((WId)item);
        if (legendItem && legendItem->inherits("QwtLegendLabel")) {
            ((QwtLegendLabel *)legendItem)->setChecked(visible);
        }
    }

    m_mutex.lock();
    replot();
    m_mutex.unlock();
}

void ScopeGadgetWidget::setupSequentialPlot()
{
    preparePlot(SequentialPlot);

    setAxisScaleDraw(QwtPlot::xBottom, new QwtScaleDraw());
    setAxisScale(QwtPlot::xBottom, 0, m_plotDataSize);
    setAxisLabelRotation(QwtPlot::xBottom, 0.0);
    setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);

    // reduce the axis font size
    QFont fnt(axisFont(QwtPlot::xBottom));
    fnt.setPointSize(7);
    setAxisFont(QwtPlot::xBottom, fnt); // x-axis
    setAxisFont(QwtPlot::yLeft, fnt); // y-axis
}

void ScopeGadgetWidget::setupChronoPlot()
{
    preparePlot(ChronoPlot);

    setAxisScaleDraw(QwtPlot::xBottom, new TimeScaleDraw());
    uint NOW = QDateTime::currentDateTime().toTime_t();
    setAxisScale(QwtPlot::xBottom, NOW - m_plotDataSize / 1000, NOW);
    setAxisLabelRotation(QwtPlot::xBottom, 0.0);
    setAxisLabelAlignment(QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom);

    // reduce the axis font size
    QFont fnt(axisFont(QwtPlot::xBottom));
    fnt.setPointSize(7);
    setAxisFont(QwtPlot::xBottom, fnt); // x-axis
    setAxisFont(QwtPlot::yLeft, fnt); // y-axis
}

void ScopeGadgetWidget::addCurvePlot(QString objectName, QString fieldPlusSubField, int scaleFactor,
                                     int meanSamples, QString mathFunction, QPen pen, bool antialiased)
{
    QString fieldName = fieldPlusSubField;
    QString elementName;
    int element = 0;

    if (fieldPlusSubField.contains("-")) {
        QStringList fieldSubfield = fieldName.split("-", QString::SkipEmptyParts);
        fieldName   = fieldSubfield.at(0);
        elementName = fieldSubfield.at(1);
    }

    // Get the uav object
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    UAVDataObject *object = dynamic_cast<UAVDataObject *>(objManager->getObject(objectName));
    if (!object) {
        qDebug() << "Object" << objectName << "is missing";
        return;
    }

    UAVObjectField *field = object->getField(fieldName);
    if (!field) {
        qDebug() << "In scope gadget, in fields loaded from GCS config file, field" <<
            fieldName << "of object" << objectName << "is missing";
        return;
    }

    if (!elementName.isEmpty()) {
        element = field->getElementNames().indexOf(QRegExp(elementName, Qt::CaseSensitive, QRegExp::FixedString));
        if (element < 0) {
            qDebug() << "In scope gadget, in fields loaded from GCS config file, field" <<
                fieldName << "of object" << objectName << "element name" << elementName << "is missing";
            return;
        }
    }

    PlotData *plotData;

    if (m_plotType == SequentialPlot) {
        plotData = new SequentialPlotData(object, field, element, scaleFactor,
                                          meanSamples, mathFunction, m_plotDataSize,
                                          pen, antialiased);
    } else if (m_plotType == ChronoPlot) {
        plotData = new ChronoPlotData(object, field, element, scaleFactor,
                                      meanSamples, mathFunction, m_plotDataSize,
                                      pen, antialiased);
    }
    connect(this, SIGNAL(visibilityChanged(QwtPlotItem *)), plotData, SLOT(visibilityChanged(QwtPlotItem *)));
    plotData->attach(this);

    // Keep the curve details for later
    m_curvesData.insert(plotData->plotName(), plotData);

    // Link to the new signal data only if this UAVObject has not been connected yet
    if (!m_connectedUAVObjects.contains(object->getName())) {
        m_connectedUAVObjects.append(object->getName());
        connect(object, SIGNAL(objectUpdated(UAVObject *)), this, SLOT(uavObjectReceived(UAVObject *)));
    }

    m_mutex.lock();
    replot();
    m_mutex.unlock();
}

void ScopeGadgetWidget::uavObjectReceived(UAVObject *obj)
{
    foreach(PlotData * plotData, m_curvesData.values()) {
        if (plotData->append(obj)) {
            m_csvLoggingDataUpdated = 1;
        }
    }
    csvLoggingAddData();
}

void ScopeGadgetWidget::replotNewData()
{
    if (!isVisible()) {
        return;
    }

    QMutexLocker locker(&m_mutex);
    foreach(PlotData * plotData, m_curvesData.values()) {
        plotData->removeStaleData();
        plotData->updatePlotData();
    }

    QDateTime NOW = QDateTime::currentDateTime();
    double toTime = NOW.toTime_t();
    toTime += NOW.time().msec() / 1000.0;
    if (m_plotType == ChronoPlot) {
        setAxisScale(QwtPlot::xBottom, toTime - m_plotDataSize, toTime);
    }

    csvLoggingInsertData();

    replot();
}

void ScopeGadgetWidget::clearCurvePlots()
{
    foreach(PlotData * plotData, m_curvesData.values()) {
        delete plotData;
    }

    m_curvesData.clear();
}

void ScopeGadgetWidget::saveState(QSettings *qSettings)
{
    // plot state
    int i = 1;

    foreach(PlotData * plotData, m_curvesData.values()) {
        bool plotVisible = plotData->isVisible();

        if (!plotVisible) {
            qSettings->setValue(QString("plot%1").arg(i), plotVisible);
        }
        i++;
    }
    // legend state
    qSettings->setValue("legendVisible", legend() != NULL);
}

void ScopeGadgetWidget::restoreState(QSettings *qSettings)
{
    // plot state
    int i = 1;

    foreach(PlotData * plotData, m_curvesData.values()) {
        plotData->setVisible(qSettings->value(QString("plot%1").arg(i), true).toBool());
        i++;
    }
    // legend state
    bool legendVisible = qSettings->value("legendVisible", true).toBool();
    if (legendVisible) {
        addLegend();
    } else {
        deleteLegend();
    }
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
    if (!m_csvLoggingStarted) {
        if (m_csvLoggingEnabled) {
            if ((!m_csvLoggingNewFileOnConnect) || (m_csvLoggingNewFileOnConnect && m_csvLoggingConnected)) {
                QDateTime NOW = QDateTime::currentDateTime();
                m_csvLoggingStartTime   = NOW;
                m_csvLoggingHeaderSaved = 0;
                m_csvLoggingDataSaved   = 0;
                m_csvLoggingBuffer.clear();
                QDir PathCheck(m_csvLoggingPath);
                if (!PathCheck.exists()) {
                    PathCheck.mkpath("./");
                }

                if (m_csvLoggingNameSet) {
                    m_csvLoggingFile.setFileName(QString("%1/%2_%3_%4.csv").arg(m_csvLoggingPath).arg(m_csvLoggingName).arg(NOW.toString("yyyy-MM-dd")).arg(NOW.toString("hh-mm-ss")));
                } else {
                    m_csvLoggingFile.setFileName(QString("%1/Log_%2_%3.csv").arg(m_csvLoggingPath).arg(NOW.toString("yyyy-MM-dd")).arg(NOW.toString("hh-mm-ss")));
                }
                QDir FileCheck(m_csvLoggingFile.fileName());
                if (FileCheck.exists()) {
                    m_csvLoggingFile.setFileName("");
                } else {
                    m_csvLoggingStarted = 1;
                    csvLoggingInsertHeader();
                }
            }
        }
    }

    return 0;
}

int ScopeGadgetWidget::csvLoggingStop()
{
    m_csvLoggingStarted = 0;

    return 0;
}

int ScopeGadgetWidget::csvLoggingInsertHeader()
{
    if (!m_csvLoggingStarted) {
        return -1;
    }
    if (m_csvLoggingHeaderSaved) {
        return -2;
    }
    if (m_csvLoggingDataSaved) {
        return -3;
    }

    m_csvLoggingHeaderSaved = 1;
    if (m_csvLoggingFile.open(QIODevice::WriteOnly | QIODevice::Append) == false) {
        qDebug() << "Unable to open " << m_csvLoggingFile.fileName() << " for csv logging Header";
    } else {
        QTextStream ts(&m_csvLoggingFile);
        ts << "date" << ", " << "Time" << ", " << "Sec since start" << ", " << "Connected" << ", " << "Data changed";

        foreach(PlotData * plotData2, m_curvesData.values()) {
            ts << ", ";
            ts << plotData2->objectName();
            ts << "." << plotData2->field()->getName();
            if (!plotData2->elementName().isEmpty()) {
                ts << "." << plotData2->elementName();
            }
        }
        ts << endl;
        m_csvLoggingFile.close();
    }
    return 0;
}

int ScopeGadgetWidget::csvLoggingAddData()
{
    if (!m_csvLoggingStarted) {
        return -1;
    }
    m_csvLoggingDataValid = false;
    QDateTime NOW = QDateTime::currentDateTime();
    QString tempString;

    QTextStream ss(&tempString);
    ss << NOW.toString("yyyy-MM-dd") << ", " << NOW.toString("hh:mm:ss.z") << ", ";

#if QT_VERSION >= 0x040700
    ss << (NOW.toMSecsSinceEpoch() - m_csvLoggingStartTime.toMSecsSinceEpoch()) / 1000.00;
#else
    ss << (NOW.toTime_t() - m_csvLoggingStartTime.toTime_t());
#endif
    ss << ", " << m_csvLoggingConnected << ", " << m_csvLoggingDataUpdated;
    m_csvLoggingDataUpdated = false;

    foreach(PlotData * plotData2, m_curvesData.values()) {
        ss << ", ";
        if (plotData2->hasData()) {
            ss << plotData2->lastDataAsString();
            m_csvLoggingDataValid = true;
        }
    }
    ss << endl;
    if (m_csvLoggingDataValid) {
        QTextStream ts(&m_csvLoggingBuffer);
        ts << tempString;
    }

    return 0;
}

int ScopeGadgetWidget::csvLoggingInsertData()
{
    if (!m_csvLoggingStarted) {
        return -1;
    }
    m_csvLoggingDataSaved = 1;

    if (m_csvLoggingFile.open(QIODevice::WriteOnly | QIODevice::Append) == false) {
        qDebug() << "Unable to open " << m_csvLoggingFile.fileName() << " for csv logging Data";
    } else {
        QTextStream ts(&m_csvLoggingFile);
        ts << m_csvLoggingBuffer;
        m_csvLoggingFile.close();
    }
    m_csvLoggingBuffer.clear();

    return 0;
}

void ScopeGadgetWidget::csvLoggingSetName(QString newName)
{
    m_csvLoggingName    = newName;
    m_csvLoggingNameSet = 1;
}

void ScopeGadgetWidget::csvLoggingConnect()
{
    m_csvLoggingConnected = 1;
    if (m_csvLoggingNewFileOnConnect) {
        csvLoggingStart();
    }
}

void ScopeGadgetWidget::csvLoggingDisconnect()
{
    m_csvLoggingHeaderSaved = 0;
    m_csvLoggingConnected   = 0;
    if (m_csvLoggingNewFileOnConnect) {
        csvLoggingStop();
    }
}

void ScopeGadgetWidget::popUpMenu(const QPoint &mousePosition)
{
    Q_UNUSED(mousePosition);

    QMenu menu;
    QAction *action = menu.addAction(tr("Clear"));
    connect(action, &QAction::triggered, this, &ScopeGadgetWidget::clearPlot);
    action = menu.addAction(tr("Copy to Clipboard"));
    connect(action, &QAction::triggered, this, &ScopeGadgetWidget::copyToClipboardAsImage);
    menu.addSeparator();
    action = menu.addAction(tr("Options..."));
    connect(action, &QAction::triggered, this, &ScopeGadgetWidget::showOptionDialog);
    menu.exec(QCursor::pos());
}

void ScopeGadgetWidget::clearPlot()
{
    m_mutex.lock();
    foreach(PlotData * plot, m_curvesData.values()) {
        plot->clear();
    }
    m_mutex.unlock();
    replot();
}

void ScopeGadgetWidget::copyToClipboardAsImage()
{
    QPixmap pixmap = QWidget::grab();

    if (pixmap.isNull()) {
        qDebug("Failed to capture the plot");
        return;
    }
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setPixmap(pixmap);
}

void ScopeGadgetWidget::showOptionDialog()
{
    Core::ICore::instance()->showOptionsDialog("ScopeGadget", objectName());
}
