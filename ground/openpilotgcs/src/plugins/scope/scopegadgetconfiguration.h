/**
 ******************************************************************************
 *
 * @file       scopegadgetconfiguration.h
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

#ifndef SCOPEGADGETCONFIGURATION_H
#define SCOPEGADGETCONFIGURATION_H

#include "plotdata.h"
#include <coreplugin/iuavgadgetconfiguration.h>

#include <QVector>

using namespace Core;

struct PlotCurveConfiguration {
    QString uavObject;
    QString uavField;
    int     yScalePower; // This is the power to which each value must be raised
    QRgb    color;
    int     yMeanSamples;
    QString mathFunction;
    double  yMinimum;
    double  yMaximum;
    bool    drawAntialiased;
};

class ScopeGadgetConfiguration : public IUAVGadgetConfiguration {
    Q_OBJECT
public:
    explicit ScopeGadgetConfiguration(QString classId, QSettings *qSettings = 0, QObject *parent = 0);

    ~ScopeGadgetConfiguration();

    // configuration setter functions
    void setPlotType(int value)
    {
        m_plotType = value;
    }
    void setMathFunctionType(int value)
    {
        m_mathFunctionType = value;
    }
    void setDataSize(int value)
    {
        m_dataSize = value;
    }
    void setRefreashInterval(int value)
    {
        m_refreshInterval = value;
    }
    void addPlotCurveConfig(PlotCurveConfiguration *value)
    {
        m_plotCurveConfigs.append(value);
    }
    void replacePlotCurveConfig(QList<PlotCurveConfiguration *> m_plotCurveConfigs);

    // Configurations getter functions
    int plotType()
    {
        return m_plotType;
    }
    int mathFunctionType()
    {
        return m_mathFunctionType;
    }
    int dataSize()
    {
        return m_dataSize;
    }
    int refreshInterval()
    {
        return m_refreshInterval;
    }
    QList<PlotCurveConfiguration *> plotCurveConfigs()
    {
        return m_plotCurveConfigs;
    }

    void saveConfig(QSettings *settings) const; // THIS SEEMS TO BE UNUSED
    IUAVGadgetConfiguration *clone();

    bool getLoggingEnabled()
    {
        return m_loggingEnabled;
    }
    bool getLoggingNewFileOnConnect()
    {
        return m_loggingNewFileOnConnect;
    }
    QString getLoggingPath()
    {
        return m_loggingPath;
    }
    void setLoggingEnabled(bool value)
    {
        m_loggingEnabled = value;
    }
    void setLoggingNewFileOnConnect(bool value)
    {
        m_loggingNewFileOnConnect = value;
    }
    void setLoggingPath(QString value)
    {
        m_loggingPath = value;
    }

private:

    // Increment this if the stream format is not compatible with previous versions. This would cause existing configs to be discarded.
    static const uint m_configurationStreamVersion = 1000;
    // The type of the plot
    int m_plotType;
    // The size of the data buffer to render in the curve plot
    int m_dataSize;
    // The interval to replot the curve widget. The data buffer is refresh as the data comes in.
    int m_refreshInterval;
    // The type of math function to be used in the scope analysis
    int m_mathFunctionType;
    QList<PlotCurveConfiguration *> m_plotCurveConfigs;

    void clearPlotData();
    bool m_loggingEnabled;
    bool m_loggingNewFileOnConnect;
    QString m_loggingPath;
};

#endif // SCOPEGADGETCONFIGURATION_H
