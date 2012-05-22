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

struct PlotCurveConfiguration
{
    QString uavObject;
    QString uavField;
    int yScalePower; //This is the power to which each value must be raised
    QRgb color;
    int yMeanSamples;
    QString mathFunction;
    double yMinimum;
    double yMaximum;
};

class ScopeGadgetConfiguration : public IUAVGadgetConfiguration
{
    Q_OBJECT
public:
    explicit ScopeGadgetConfiguration(QString classId, QSettings* qSettings = 0, QObject *parent = 0);

    ~ScopeGadgetConfiguration();

    //configuration setter functions
    void setPlotType(int value){m_plotType = value;}
    void setMathFunctionType(int value){m_mathFunctionType = value;}
    void setDataSize(int value){m_dataSize = value;}
    void setRefreashInterval(int value){m_refreshInterval = value;}
    void addPlotCurveConfig(PlotCurveConfiguration* value){m_PlotCurveConfigs.append(value);}
    void replacePlotCurveConfig(QList<PlotCurveConfiguration*> m_PlotCurveConfigs);


    //configurations getter functions
    int plotType(){return m_plotType;}
    int mathFunctionType(){return m_mathFunctionType;}
    int dataSize(){return m_dataSize;}
    int refreshInterval(){return m_refreshInterval;}
    QList<PlotCurveConfiguration*> plotCurveConfigs(){return m_PlotCurveConfigs;}

    void saveConfig(QSettings* settings) const; //THIS SEEMS TO BE UNUSED
    IUAVGadgetConfiguration *clone();

    bool getLoggingEnabled(){return m_LoggingEnabled;};
    bool getLoggingNewFileOnConnect(){return m_LoggingNewFileOnConnect;};
    QString getLoggingPath(){return m_LoggingPath;};
    void setLoggingEnabled(bool value){m_LoggingEnabled=value;};
    void setLoggingNewFileOnConnect(bool value){m_LoggingNewFileOnConnect=value;};
    void setLoggingPath(QString value){m_LoggingPath=value;};

private:

    static const uint m_configurationStreamVersion = 1000;//Increment this if the stream format is not compatible with previous versions. This would cause existing configs to be discarded.
    int m_plotType; //The type of the plot
    int m_dataSize; //The size of the data buffer to render in the curve plot
    int m_refreshInterval; //The interval to replot the curve widget. The data buffer is refresh as the data comes in.
    int m_mathFunctionType; //The type of math function to be used in the scope analysis
    QList<PlotCurveConfiguration*> m_PlotCurveConfigs;

    void clearPlotData();
    bool m_LoggingEnabled;
    bool m_LoggingNewFileOnConnect;
    QString m_LoggingPath;

};

#endif // SCOPEGADGETCONFIGURATION_H
