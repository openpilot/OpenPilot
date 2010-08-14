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
#include <QtCore/QDataStream>


ScopeGadgetConfiguration::ScopeGadgetConfiguration(QString classId, const QByteArray &state, QObject *parent) :
        IUAVGadgetConfiguration(classId, parent),
        m_plotType((int)ChronoPlot),
        m_dataSize(60),
        m_refreshInterval(1000)
{
    uint currentStreamVersion = 0;
    int plotCurveCount = 0;

    if (state.count() > 0) {

        QDataStream stream(state);
        stream >> currentStreamVersion;

        if(currentStreamVersion != m_configurationStreamVersion)
            return;

        stream >> m_plotType;
        stream >> m_dataSize;
        stream >> m_refreshInterval;
        stream >> plotCurveCount;

        while(plotCurveCount-- > 0)
        {
            QString uavObject;
            QString uavField;
            QRgb color;

            PlotCurveConfiguration* plotCurveConf = new PlotCurveConfiguration();
            stream >> uavObject;
            plotCurveConf->uavObject = uavObject;
            stream >> uavField;
            plotCurveConf->uavField = uavField;
            stream >> color;
            plotCurveConf->color = color;
            stream >> plotCurveConf->yScalePower;
            stream >> plotCurveConf->yMinimum;
            stream >> plotCurveConf->yMaximum;

            m_PlotCurveConfigs.append(plotCurveConf);
        }

        //The value is converted to milliseconds, so if it is < 100, it is still seconds
        if(m_refreshInterval < 100)
            m_refreshInterval *= 1000;
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
    m->setPlotType(m_plotType);
    m->setDataSize( m_dataSize);
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
        newPlotCurveConf->yMinimum = currentPlotCurveConf->yMinimum;
        newPlotCurveConf->yMaximum = currentPlotCurveConf->yMaximum;

        m->addPlotCurveConfig(newPlotCurveConf);
    }

    return m;
}

/**
 * Saves a configuration.
 *
 */
QByteArray ScopeGadgetConfiguration::saveState() const
{
    int plotCurveCount = m_PlotCurveConfigs.size();
    int plotDatasLoadIndex = 0;
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream << m_configurationStreamVersion;
    stream << m_plotType;
    stream << m_dataSize;
    stream << m_refreshInterval;
    stream << plotCurveCount;

    for(plotDatasLoadIndex = 0; plotDatasLoadIndex < plotCurveCount; plotDatasLoadIndex++)
    {
        PlotCurveConfiguration* plotCurveConf = m_PlotCurveConfigs.at(plotDatasLoadIndex);

        stream << plotCurveConf->uavObject;
        stream << plotCurveConf->uavField;
        stream << plotCurveConf->color;
        stream << plotCurveConf->yScalePower;
        stream << plotCurveConf->yMinimum;
        stream << plotCurveConf->yMaximum;
    }

    return bytes;
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
