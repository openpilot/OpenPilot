/**
 ******************************************************************************
 *
 * @file       plotdata.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Implementation of plotdata.
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


#include "plotdata.h"
#include <math.h>

PlotData::PlotData(QString p_uavObject, QString p_uavField)
{
    uavObject = p_uavObject;
    uavField =  p_uavField;

    xData = new QVector<double>();
    yData = new QVector<double>();

    curve = 0;
    scalePower = 0;
    yMinimum = 0;
    yMaximum = 0;

    m_xWindowSize = 0;
}

PlotData::~PlotData()
{
    delete xData;
    delete yData;
}


bool SequencialPlotData::append(UAVObject* obj)
{
    if (uavObject == obj->getName()) {
        //Get the field of interest
        UAVObjectField* field =  obj->getField(uavField);

        if (field) {
            //Shift data forward and put the new value at the front
            yData->append(field->getDouble() * pow(10, scalePower));
            if (yData->size() > m_xWindowSize) {
                yData->pop_front();
            } else
                xData->insert(xData->size(), xData->size());

            //notify the gui of changes in the data
            dataChanged();
            return true;
        }
    }

    return false;
}

bool ChronoPlotData::append(UAVObject* obj)
{
    if (uavObject == obj->getName()) {
        //Get the field of interest
        UAVObjectField* field =  obj->getField(uavField);

        if (field) {
            //Put the new value at the front
            QDateTime NOW = QDateTime::currentDateTime();

            double newestValue = NOW.toTime_t() + NOW.time().msec() / 1000.0;
            xData->append(newestValue);
            yData->append(field->getDouble() * pow(10, scalePower));

            //Remove stale data
            removeStaleData();

            //notify the gui of chages in the data
            dataChanged();
            return true;
        }
    }

    return false;
}

void ChronoPlotData::removeStaleData()
{
    double newestValue;
    double oldestValue;

    while (1) {
        if (xData->size() == 0)
            break;

        newestValue = xData->last();
        oldestValue = xData->first();

        if (newestValue - oldestValue > m_xWindowSize) {
            yData->pop_front();
            xData->pop_front();
        } else
            break;
    }
}

void ChronoPlotData::removeStaleDataTimeout()
{
    removeStaleData();
    dataChanged();
}

bool UAVObjectPlotData::append(UAVObject* obj)
{
    return false;
}
