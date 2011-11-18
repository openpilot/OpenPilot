/**
 ******************************************************************************
 *
 * @file       plotdata.cpp
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


#include "plotdata.h"
#include <math.h>
#include <QDebug>

PlotData::PlotData(QString p_uavObject, QString p_uavField)
{    
    uavObject = p_uavObject;

    if(p_uavField.contains("-"))
    {
        QStringList fieldSubfield = p_uavField.split("-", QString::SkipEmptyParts);
        uavField = fieldSubfield.at(0);
        uavSubField = fieldSubfield.at(1);
        haveSubField = true;
    }
    else
    {
        uavField =  p_uavField;
        haveSubField = false;
    }

    xData = new QVector<double>();
    yData = new QVector<double>();
    yDataHistory = new QVector<double>();

    curve = 0;
    scalePower = 0;
    interpolationSamples = 1;
    interpolationSum = 0.0f;
    yMinimum = 0;
    yMaximum = 0;

    m_xWindowSize = 0;
}

double PlotData::valueAsDouble(UAVObject* obj, UAVObjectField* field)
{
    Q_UNUSED(obj);
    QVariant value;

    if(haveSubField){
        int indexOfSubField = field->getElementNames().indexOf(QRegExp(uavSubField, Qt::CaseSensitive, QRegExp::FixedString));
        value = field->getValue(indexOfSubField);
    }else
        value = field->getValue();

    // qDebug() << "Data  (" << value.typeName() << ") " <<  value.toString();

    return value.toDouble();
}

PlotData::~PlotData()
{
    delete xData;
    delete yData;
    delete yDataHistory;
}


bool SequencialPlotData::append(UAVObject* obj)
{
    if (uavObject == obj->getName()) {

        //Get the field of interest
        UAVObjectField* field =  obj->getField(uavField);

        if (field) {

            //Shift data forward and put the new value at the front
            yDataHistory->append( valueAsDouble(obj, field) * pow(10, scalePower));
            interpolationSum += valueAsDouble(obj, field) * pow(10, scalePower);
            if(yDataHistory->size() > interpolationSamples) {
                interpolationSum -= yDataHistory->first();
                yDataHistory->pop_front();
            }
            yData->append(interpolationSum/yDataHistory->size());
            if (yData->size() > m_xWindowSize) {
                yData->pop_front();
            } else
                xData->insert(xData->size(), xData->size());

            //notify the gui of changes in the data
            //dataChanged();
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
        //qDebug() << "uavObject: " << uavObject << ", uavField: " << uavField;

        if (field) {
            //Put the new value at the front
            QDateTime NOW = QDateTime::currentDateTime();

            yDataHistory->append( valueAsDouble(obj, field) * pow(10, scalePower));
            interpolationSum += valueAsDouble(obj, field) * pow(10, scalePower);
            if(yDataHistory->size() > interpolationSamples) {
                interpolationSum -= yDataHistory->first();
                yDataHistory->pop_front();
            }

            double valueX = NOW.toTime_t() + NOW.time().msec() / 1000.0;
            double valueY = interpolationSum/yDataHistory->size();
            xData->append(valueX);
            yData->append(valueY);

            //qDebug() << "Data  " << uavObject << "." << field->getName() << " X,Y:" << valueX << "," <<  valueY;

            //Remove stale data
            removeStaleData();

            //notify the gui of chages in the data
            //dataChanged();
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

    //qDebug() << "removeStaleData ";
}

void ChronoPlotData::removeStaleDataTimeout()
{
    removeStaleData();
    //dataChanged();
    //qDebug() << "removeStaleDataTimeout";
}

bool UAVObjectPlotData::append(UAVObject* obj)
{
    Q_UNUSED(obj);
    return false;
}
