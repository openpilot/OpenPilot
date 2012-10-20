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
    meanSamples = 1;
    meanSum = 0.0f;
//    mathFunction=0;
    correctionSum = 0.0f;
    correctionCount = 0;
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


bool SequentialPlotData::append(UAVObject* obj)
{
    if (uavObject == obj->getName()) {

        //Get the field of interest
        UAVObjectField* field =  obj->getField(uavField);

        if (field) {

            double currentValue = valueAsDouble(obj, field) * pow(10, scalePower);

            //Perform scope math, if necessary
            if (mathFunction  == "Boxcar average" || mathFunction  == "Standard deviation"){
                //Put the new value at the front
                yDataHistory->append( currentValue );

                // calculate average value
                meanSum += currentValue;
                if(yDataHistory->size() > meanSamples) {
                    meanSum -= yDataHistory->first();
                    yDataHistory->pop_front();
                }

                // make sure to correct the sum every meanSamples steps to prevent it
                // from running away due to floating point rounding errors
                correctionSum+=currentValue;
                if (++correctionCount >= meanSamples) {
                    meanSum = correctionSum;
                    correctionSum = 0.0f;
                    correctionCount = 0;
                }

                double boxcarAvg=meanSum/yDataHistory->size();

                if ( mathFunction  == "Standard deviation" ){
                    //Calculate square of sample standard deviation, with Bessel's correction
                    double stdSum=0;
                    for (int i=0; i < yDataHistory->size(); i++){
                        stdSum+= pow(yDataHistory->at(i)- boxcarAvg,2)/(meanSamples-1);
                    }
                    yData->append(sqrt(stdSum));
                }
                else  {
                    yData->append(boxcarAvg);
                }
            }
            else{
                yData->append( currentValue );
            }

            if (yData->size() > m_xWindowSize) { //If new data overflows the window, remove old data...
                yData->pop_front();
            } else //...otherwise, add a new y point at position xData
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
            QDateTime NOW = QDateTime::currentDateTime(); //THINK ABOUT REIMPLEMENTING THIS TO SHOW UAVO TIME, NOT SYSTEM TIME
            double currentValue = valueAsDouble(obj, field) * pow(10, scalePower);

            //Perform scope math, if necessary
            if (mathFunction  == "Boxcar average" || mathFunction  == "Standard deviation"){
                //Put the new value at the back
                yDataHistory->append( currentValue );

                // calculate average value
                meanSum += currentValue;
                if(yDataHistory->size() > meanSamples) {
                    meanSum -= yDataHistory->first();
                    yDataHistory->pop_front();
                }
                // make sure to correct the sum every meanSamples steps to prevent it
                // from running away due to floating point rounding errors
                correctionSum+=currentValue;
                if (++correctionCount >= meanSamples) {
                    meanSum = correctionSum;
                    correctionSum = 0.0f;
                    correctionCount = 0;
                }

                double boxcarAvg=meanSum/yDataHistory->size();
//qDebug()<<mathFunction;
                if ( mathFunction  == "Standard deviation" ){
                    //Calculate square of sample standard deviation, with Bessel's correction
                    double stdSum=0;
                    for (int i=0; i < yDataHistory->size(); i++){
                        stdSum+= pow(yDataHistory->at(i)- boxcarAvg,2)/(meanSamples-1);
                    }
                    yData->append(sqrt(stdSum));
                }
                else  {
                    yData->append(boxcarAvg);
                }
            }
            else{
                yData->append( currentValue );
            }

            double valueX = NOW.toTime_t() + NOW.time().msec() / 1000.0;
            xData->append(valueX);

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
