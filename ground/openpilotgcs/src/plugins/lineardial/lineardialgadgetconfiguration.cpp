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

/**
 * Loads a saved configuration or defaults if non exist.
 *
 */
LineardialGadgetConfiguration::LineardialGadgetConfiguration(QString classId, QSettings* qSettings, QObject *parent) :
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
	decimalPlaces(0),
	useOpenGLFlag(false)
{
    //if a saved configuration exists load it
    if(qSettings != 0) {
        QString dFile = qSettings->value("dFile").toString();
        dialFile = Utils::PathUtils().InsertDataPath(dFile);
        sourceDataObject = qSettings->value("sourceDataObject").toString();
        sourceObjectField = qSettings->value("sourceObjectField").toString();
        minValue = qSettings->value("minValue").toDouble();
        maxValue = qSettings->value("maxValue").toDouble();
        redMin = qSettings->value("redMin").toDouble();
        redMax = qSettings->value("redMax").toDouble();
        yellowMin = qSettings->value("yellowMin").toDouble();
        yellowMax = qSettings->value("yellowMax").toDouble();
        greenMin = qSettings->value("greenMin").toDouble();
        greenMax = qSettings->value("greenMax").toDouble();
        font = qSettings->value("font").toString();
        decimalPlaces = qSettings->value("decimalPlaces").toInt();
        factor = qSettings->value("factor").toDouble();
		useOpenGLFlag = qSettings->value("useOpenGLFlag").toBool();
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
	m->useOpenGLFlag = useOpenGLFlag;

    return m;
}

/**
 * Saves a configuration.
 *
 */
void LineardialGadgetConfiguration::saveConfig(QSettings* qSettings) const {
   QString dFile = Utils::PathUtils().RemoveDataPath(dialFile);
   qSettings->setValue("dFile", dFile);
   qSettings->setValue("sourceDataObject", sourceDataObject);
   qSettings->setValue("sourceObjectField", sourceObjectField);
   qSettings->setValue("minValue", minValue);
   qSettings->setValue("maxValue", maxValue);
   qSettings->setValue("redMin", redMin);
   qSettings->setValue("redMax", redMax);
   qSettings->setValue("yellowMin", yellowMin);
   qSettings->setValue("yellowMax", yellowMax);
   qSettings->setValue("greenMin", greenMin);
   qSettings->setValue("greenMax", greenMax);
   qSettings->setValue("font", font);
   qSettings->setValue("decimalPlaces", decimalPlaces);
   qSettings->setValue("factor", factor);
	qSettings->setValue("useOpenGLFlag", useOpenGLFlag);
}
