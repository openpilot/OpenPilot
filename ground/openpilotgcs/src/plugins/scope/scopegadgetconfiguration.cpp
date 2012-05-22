/**
 ******************************************************************************
 *
 * @file       scopegadgetconfiguration.cpp
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

#include "scopegadgetconfiguration.h"

ScopeGadgetConfiguration::ScopeGadgetConfiguration(QString classId, QSettings* qSettings, QObject *parent) :
        IUAVGadgetConfiguration(classId, parent),
        m_plotType((int)ChronoPlot),
        m_dataSize(60),
        m_refreshInterval(1000),
        m_mathFunctionType(0)
{
    uint currentStreamVersion = 0;
    int plotCurveCount = 0;


    //if a saved configuration exists load it
    if(qSettings != 0) {
        currentStreamVersion = qSettings->value("configurationStreamVersion").toUInt();

        if(currentStreamVersion != m_configurationStreamVersion)
            return;

        m_plotType = qSettings->value("plotType").toInt();
        m_dataSize = qSettings->value("dataSize").toInt();
        m_refreshInterval = qSettings->value("refreshInterval").toInt();
        plotCurveCount = qSettings->value("plotCurveCount").toInt();

        for(int plotDatasLoadIndex = 0; plotDatasLoadIndex < plotCurveCount; plotDatasLoadIndex++)
        {
            QString uavObject;
            QString uavField;
            QRgb color;

            qSettings->beginGroup(QString("plotCurve") + QString().number(plotDatasLoadIndex));

            PlotCurveConfiguration* plotCurveConf = new PlotCurveConfiguration();
            uavObject = qSettings->value("uavObject").toString();
            plotCurveConf->uavObject = uavObject;
            uavField = qSettings->value("uavField").toString();
            plotCurveConf->uavField = uavField;
            color = qSettings->value("color").value<QRgb>();
            plotCurveConf->color = color;
            plotCurveConf->yScalePower = qSettings->value("yScalePower").toInt();
            plotCurveConf->mathFunction = qSettings->value("mathFunction").toString();
            plotCurveConf->yMeanSamples = qSettings->value("yMeanSamples").toInt();

            if (!plotCurveConf->yMeanSamples) plotCurveConf->yMeanSamples = 1; // fallback for backward compatibility with earlier versions //IS THIS STILL NECESSARY?

            plotCurveConf->yMinimum = qSettings->value("yMinimum").toDouble();
            plotCurveConf->yMaximum = qSettings->value("yMaximum").toDouble();

            m_PlotCurveConfigs.append(plotCurveConf);

            qSettings->endGroup();
        }

        m_LoggingEnabled = qSettings->value("LoggingEnabled").toBool();
        m_LoggingNewFileOnConnect = qSettings->value("LoggingNewFileOnConnect").toBool();
        m_LoggingPath = qSettings->value("LoggingPath").toString();

    }
}

void ScopeGadgetConfiguration::clearPlotData()
{
    PlotCurveConfiguration* poltCurveConfig;

    while(m_PlotCurveConfigs.size() > 0)
    {
        poltCurveConfig = m_PlotCurveConfigs.first();
        m_PlotCurveConfigs.pop_front();

        delete poltCurveConfig;
    }
}

/**
 * Clones a configuration.
 *
 */
IUAVGadgetConfiguration *ScopeGadgetConfiguration::clone()
{
    int plotCurveCount = 0;
    int plotDatasLoadIndex = 0;

    ScopeGadgetConfiguration *m = new ScopeGadgetConfiguration(this->classId());
    m->setPlotType( m_plotType);
    m->setDataSize( m_dataSize);
    m->setMathFunctionType( m_mathFunctionType);
    m->setRefreashInterval( m_refreshInterval);

    plotCurveCount = m_PlotCurveConfigs.size();

    for(plotDatasLoadIndex = 0; plotDatasLoadIndex < plotCurveCount; plotDatasLoadIndex++)
    {
        PlotCurveConfiguration* currentPlotCurveConf = m_PlotCurveConfigs.at(plotDatasLoadIndex);

        PlotCurveConfiguration* newPlotCurveConf = new PlotCurveConfiguration();
        newPlotCurveConf->uavObject = currentPlotCurveConf->uavObject;
        newPlotCurveConf->uavField = currentPlotCurveConf->uavField;
        newPlotCurveConf->color = currentPlotCurveConf->color;
        newPlotCurveConf->yScalePower = currentPlotCurveConf->yScalePower;
        newPlotCurveConf->yMeanSamples = currentPlotCurveConf->yMeanSamples;
        newPlotCurveConf->mathFunction = currentPlotCurveConf->mathFunction;

        newPlotCurveConf->yMinimum = currentPlotCurveConf->yMinimum;
        newPlotCurveConf->yMaximum = currentPlotCurveConf->yMaximum;

        m->addPlotCurveConfig(newPlotCurveConf);
    }

    m->setLoggingEnabled(m_LoggingEnabled);
    m->setLoggingNewFileOnConnect(m_LoggingNewFileOnConnect);
    m->setLoggingPath(m_LoggingPath);



    return m;
}


/**
 * Saves a configuration. //REDEFINES saveConfig CHILD BEHAVIOR?
 *
 */
void ScopeGadgetConfiguration::saveConfig(QSettings* qSettings) const {

    int plotCurveCount = m_PlotCurveConfigs.size();
    int plotDatasLoadIndex = 0;

    qSettings->setValue("configurationStreamVersion", m_configurationStreamVersion);
    qSettings->setValue("plotType", m_plotType);
    qSettings->setValue("dataSize", m_dataSize);
    qSettings->setValue("refreshInterval", m_refreshInterval);
    qSettings->setValue("plotCurveCount", plotCurveCount);

    for(plotDatasLoadIndex = 0; plotDatasLoadIndex < plotCurveCount; plotDatasLoadIndex++)
    {
        qSettings->beginGroup(QString("plotCurve") + QString().number(plotDatasLoadIndex));

        PlotCurveConfiguration* plotCurveConf = m_PlotCurveConfigs.at(plotDatasLoadIndex);
        qSettings->setValue("uavObject",  plotCurveConf->uavObject);
        qSettings->setValue("uavField",  plotCurveConf->uavField);
        qSettings->setValue("color",  plotCurveConf->color);
        qSettings->setValue("mathFunction",  plotCurveConf->mathFunction);
        qSettings->setValue("yScalePower",  plotCurveConf->yScalePower);
        qSettings->setValue("yMeanSamples",  plotCurveConf->yMeanSamples);
        qSettings->setValue("yMinimum",  plotCurveConf->yMinimum);
        qSettings->setValue("yMaximum",  plotCurveConf->yMaximum);

        qSettings->endGroup();
    }

    qSettings->setValue("LoggingEnabled",  m_LoggingEnabled);
    qSettings->setValue("LoggingNewFileOnConnect",  m_LoggingNewFileOnConnect);
    qSettings->setValue("LoggingPath",  m_LoggingPath);


}

void ScopeGadgetConfiguration::replacePlotCurveConfig(QList<PlotCurveConfiguration*> newPlotCurveConfigs)
{
    clearPlotData();

    m_PlotCurveConfigs.append(newPlotCurveConfigs);
}

ScopeGadgetConfiguration::~ScopeGadgetConfiguration()
{
    clearPlotData();
}
