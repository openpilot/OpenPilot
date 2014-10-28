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

PlotData::PlotData(QString objectName, QString fieldName, QString elementName,
                   QwtPlotCurve *plotCurve, int scaleOrderFactor, int meanSamples,
                   QString mathFunction, double plotDataSize) :
    m_scalePower(scaleOrderFactor), m_meanSamples(meanSamples),
    m_mathFunction(mathFunction), m_plotDataSize(plotDataSize),
    m_objectName(objectName), m_fieldName(fieldName),
    m_elementName(elementName), m_plotCurve(plotCurve)

{    
    m_plotCurve->setSamples(m_xDataEntries, m_yDataEntries);

    m_meanSum         = 0.0f;
    m_correctionSum   = 0.0f;
    m_correctionCount = 0;
    m_yMin        = 0;
    m_yMax        = 0;
}

double PlotData::valueAsDouble(UAVObject *obj, UAVObjectField *field)
{
    Q_UNUSED(obj);
    QVariant value;

    if (!m_elementName.isEmpty()) {
        int indexOfSubField = field->getElementNames().indexOf(QRegExp(elementName(), Qt::CaseSensitive, QRegExp::FixedString));
        value = field->getValue(indexOfSubField);
    } else {
        value = field->getValue();
    }

    // qDebug() << "Data  (" << value.typeName() << ") " <<  value.toString();

    return value.toDouble();
}

PlotData::~PlotData()
{
    m_plotCurve->detach();
    delete m_plotCurve;
}

void PlotData::updatePlotCurveData()
{
    m_plotCurve->setSamples(m_xDataEntries, m_yDataEntries);
}

bool SequentialPlotData::append(UAVObject *obj)
{
    if (objectName() == obj->getName()) {
        // Get the field of interest
        UAVObjectField *field = obj->getField(fieldName());

        if (field) {
            double currentValue = valueAsDouble(obj, field) * pow(10, m_scalePower);

            // Perform scope math, if necessary
            if (m_mathFunction == "Boxcar average" || m_mathFunction == "Standard deviation") {
                // Put the new value at the front
                m_yDataHistory.append(currentValue);

                // calculate average value
                m_meanSum += currentValue;
                if (m_yDataHistory.size() > m_meanSamples) {
                    m_meanSum -= m_yDataHistory.first();
                    m_yDataHistory.pop_front();
                }

                // make sure to correct the sum every meanSamples steps to prevent it
                // from running away due to floating point rounding errors
                m_correctionSum += currentValue;
                if (++m_correctionCount >= m_meanSamples) {
                    m_meanSum = m_correctionSum;
                    m_correctionSum = 0.0f;
                    m_correctionCount = 0;
                }

                double boxcarAvg = m_meanSum / m_yDataHistory.size();

                if (m_mathFunction == "Standard deviation") {
                    // Calculate square of sample standard deviation, with Bessel's correction
                    double stdSum = 0;
                    for (int i = 0; i < m_yDataHistory.size(); i++) {
                        stdSum += pow(m_yDataHistory.at(i) - boxcarAvg, 2) / (m_meanSamples - 1);
                    }
                    m_yDataEntries.append(sqrt(stdSum));
                } else {
                    m_yDataEntries.append(boxcarAvg);
                }
            } else {
                m_yDataEntries.append(currentValue);
            }

            if (m_yDataEntries.size() > m_plotDataSize) { // If new data overflows the window, remove old data...
                m_yDataEntries.pop_front();
            } else { // ...otherwise, add a new y point at position xData
                m_xDataEntries.insert(m_xDataEntries.size(), m_xDataEntries.size());
            }

            return true;
        }
    }

    return false;
}

bool ChronoPlotData::append(UAVObject *obj)
{
    if (objectName() == obj->getName()) {
        // Get the field of interest
        UAVObjectField *field = obj->getField(fieldName());
        // qDebug() << "uavObject: " << uavObject << ", uavField: " << uavField;

        if (field) {
            QDateTime NOW = QDateTime::currentDateTime(); // THINK ABOUT REIMPLEMENTING THIS TO SHOW UAVO TIME, NOT SYSTEM TIME
            double currentValue = valueAsDouble(obj, field) * pow(10, m_scalePower);

            // Perform scope math, if necessary
            if (m_mathFunction == "Boxcar average" || m_mathFunction == "Standard deviation") {
                // Put the new value at the back
                m_yDataHistory.append(currentValue);

                // calculate average value
                m_meanSum += currentValue;
                if (m_yDataHistory.size() > m_meanSamples) {
                    m_meanSum -= m_yDataHistory.first();
                    m_yDataHistory.pop_front();
                }
                // make sure to correct the sum every meanSamples steps to prevent it
                // from running away due to floating point rounding errors
                m_correctionSum += currentValue;
                if (++m_correctionCount >= m_meanSamples) {
                    m_meanSum = m_correctionSum;
                    m_correctionSum = 0.0f;
                    m_correctionCount = 0;
                }

                double boxcarAvg = m_meanSum / m_yDataHistory.size();
                // qDebug()<<mathFunction;
                if (m_mathFunction == "Standard deviation") {
                    // Calculate square of sample standard deviation, with Bessel's correction
                    double stdSum = 0;
                    for (int i = 0; i < m_yDataHistory.size(); i++) {
                        stdSum += pow(m_yDataHistory.at(i) - boxcarAvg, 2) / (m_meanSamples - 1);
                    }
                    m_yDataEntries.append(sqrt(stdSum));
                } else {
                    m_yDataEntries.append(boxcarAvg);
                }
            } else {
                m_yDataEntries.append(currentValue);
            }

            double valueX = NOW.toTime_t() + NOW.time().msec() / 1000.0;
            m_xDataEntries.append(valueX);

            // qDebug() << "Data  " << uavObject << "." << field->getName() << " X,Y:" << valueX << "," <<  valueY;

            // Remove stale data
            removeStaleData();
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
        if (m_xDataEntries.size() == 0) {
            break;
        }

        newestValue = m_xDataEntries.last();
        oldestValue = m_xDataEntries.first();

        if (newestValue - oldestValue > m_plotDataSize) {
            m_yDataEntries.pop_front();
            m_xDataEntries.pop_front();
        } else {
            break;
        }
    }

    // qDebug() << "removeStaleData ";
}

void ChronoPlotData::removeStaleDataTimeout()
{
    removeStaleData();
    // qDebug() << "removeStaleDataTimeout";
}
