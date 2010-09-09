/**
 ******************************************************************************
 *
 * @file       lineardialgadgetconfiguration.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup LinearDialPlugin Linear Dial Plugin
 * @{
 * @brief Implements a gadget that displays linear gauges
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

#include "lineardialgadgetconfiguration.h"
#include "utils/pathutils.h"
#include <QtCore/QDataStream>

/**
 * Loads a saved configuration or defaults if non exist.
 *
 */
LineardialGadgetConfiguration::LineardialGadgetConfiguration(QString classId, const QByteArray &state, QObject *parent) :
    IUAVGadgetConfiguration(classId, parent),
    dialFile("Unknown"),
    sourceDataObject("Unknown"),
    sourceObjectField("Unknown"),
    minValue(0),
    maxValue(100),
    redMin(0),
    redMax(33),
    yellowMin(33),
    yellowMax(66),
    greenMin(66),
    greenMax(100),
    factor(1.00),
    decimalPlaces(0)
{
    //if a saved configuration exists load it
    if (state.count() > 0) {
        QDataStream stream(state);
        QString dFile;
        stream >> dFile;
        dialFile = Utils::PathUtils().InsertDataPath(dFile);
        stream >> sourceDataObject;
        stream >> sourceObjectField;
        stream >> minValue;
        stream >> maxValue;
        stream >> redMin;
        stream >> redMax;
        stream >> yellowMin;
        stream >> yellowMax;
        stream >> greenMin;
        stream >> greenMax;
        stream >> font;
        stream >> decimalPlaces;
        stream >> factor;
    }
}
/**
 * Clones a configuration.
 *
 */
IUAVGadgetConfiguration *LineardialGadgetConfiguration::clone()
{
    LineardialGadgetConfiguration *m = new LineardialGadgetConfiguration(this->classId());
    m->dialFile=dialFile;
    m->sourceDataObject = sourceDataObject;
    m->sourceObjectField = sourceObjectField;
    m->minValue = minValue;
    m->maxValue = maxValue;
    m->redMin = redMin;
    m->redMax = redMax;
    m->yellowMin = yellowMin;
    m->yellowMax = yellowMax;
    m->greenMin = greenMin;
    m->greenMax = greenMax;
    m->font = font;
    m->decimalPlaces = decimalPlaces;
    m->factor = factor;

    return m;
}
/**
 * Saves a configuration.
 *
 */
QByteArray LineardialGadgetConfiguration::saveState() const
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    QString dFile = Utils::PathUtils().RemoveDataPath(dialFile);
    stream << dFile;
    stream << sourceDataObject;
    stream << sourceObjectField;
    stream << minValue;
    stream << maxValue;
    stream << redMin;
    stream << redMax;
    stream << yellowMin;
    stream << yellowMax;
    stream << greenMin;
    stream << greenMax;
    stream << font;
    stream << decimalPlaces;
    stream << factor;

    return bytes;
}
