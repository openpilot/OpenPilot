/**
 ******************************************************************************
 *
 * @file       dialgadgetconfiguration.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup DialPlugin Dial Plugin
 * @{
 * @brief Plots flight information rotary style dials 
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

#include "dialgadgetconfiguration.h"
#include "utils/pathutils.h"

/**
 * Loads a saved configuration or defaults if non exist.
 *
 */
DialGadgetConfiguration::DialGadgetConfiguration(QString classId, QSettings* qSettings, QObject *parent) :
    IUAVGadgetConfiguration(classId, parent),
    m_defaultDial("Unknown"),
	dialBackgroundID("background"),
    dialForegroundID("foreground"),
    dialNeedleID1("needle"),
    dialNeedleID2("needle2"),
    dialNeedleID3("needle3"),
    needle1MinValue(0),
    needle1MaxValue(100),
    needle2MinValue(0),
    needle2MaxValue(100),
    needle3MinValue(0),
    needle3MaxValue(100),
    needle1Factor(1),
    needle2Factor(1),
    needle3Factor(1),
    needle1Move("Rotate"),
    needle2Move("Rotate"),
	needle3Move("Rotate"),
	useOpenGLFlag(false),
	beSmooth(true)
{
    //if a saved configuration exists load it
    if(qSettings != 0) {
        QString dialFile = qSettings->value("dialFile").toString();

		m_defaultDial=Utils::PathUtils().InsertDataPath(dialFile);
		dialBackgroundID = qSettings->value("dialBackgroundID").toString();
        dialForegroundID = qSettings->value("dialForegroundID").toString();
        dialNeedleID1 = qSettings->value("dialNeedleID1").toString();
        dialNeedleID2 = qSettings->value("dialNeedleID2").toString();
        dialNeedleID3 = qSettings->value("dialNeedleID3").toString();
        needle1MinValue = qSettings->value("needle1MinValue").toDouble();
        needle1MaxValue = qSettings->value("needle1MaxValue").toDouble();
        needle2MinValue = qSettings->value("needle2MinValue").toDouble();
        needle2MaxValue = qSettings->value("needle2MaxValue").toDouble();
        needle3MinValue = qSettings->value("needle3MinValue").toDouble();
        needle3MaxValue = qSettings->value("needle3MaxValue").toDouble();
        needle1DataObject = qSettings->value("needle1DataObject").toString();
        needle1ObjectField = qSettings->value("needle1ObjectField").toString();
        needle2DataObject = qSettings->value("needle2DataObject").toString();
        needle2ObjectField = qSettings->value("needle2ObjectField").toString();
        needle3DataObject = qSettings->value("needle3DataObject").toString();
        needle3ObjectField = qSettings->value("needle3ObjectField").toString();
        needle1Factor = qSettings->value("needle1Factor").toDouble();
        needle2Factor = qSettings->value("needle2Factor").toDouble();
        needle3Factor = qSettings->value("needle3Factor").toDouble();
        needle1Move = qSettings->value("needle1Move").toString();
        needle2Move = qSettings->value("needle2Move").toString();
        needle3Move = qSettings->value("needle3Move").toString();
        font = qSettings->value("font").toString();
		useOpenGLFlag = qSettings->value("useOpenGLFlag").toBool();
		beSmooth = qSettings->value("beSmooth").toBool();
	}
}

/**
 * Clones a configuration.
 *
 */
IUAVGadgetConfiguration *DialGadgetConfiguration::clone()
{
    DialGadgetConfiguration *m = new DialGadgetConfiguration(this->classId());
	m->m_defaultDial=m_defaultDial;
    m->setDialBackgroundID(dialBackgroundID);
    m->setDialForegroundID(dialForegroundID);
    m->setDialNeedleID1(dialNeedleID1);
    m->setDialNeedleID2(dialNeedleID2);
    m->setDialNeedleID3(dialNeedleID3);
    m->setN1Min(needle1MinValue);
    m->setN1Max(needle1MaxValue);
    m->setN2Min(needle2MinValue);
    m->setN2Max(needle2MaxValue);
    m->setN3Min(needle3MinValue);
    m->setN3Max(needle3MaxValue);
    m->setN1DataObject(needle1DataObject);
    m->setN1ObjField(needle1ObjectField);
    m->setN2DataObject(needle2DataObject);
    m->setN2ObjField(needle2ObjectField);
    m->setN3DataObject(needle3DataObject);
    m->setN3ObjField(needle3ObjectField);
    m->setN1Factor(needle1Factor);
    m->setN2Factor(needle2Factor);
    m->setN3Factor(needle3Factor);
    m->setN1Move(needle1Move);
    m->setN2Move(needle2Move);
    m->setN3Move(needle3Move);
    m->setFont(font);
	m->useOpenGLFlag = useOpenGLFlag;
	m->beSmooth = beSmooth;

    return m;
}

/**
 * Saves a configuration.
 *
 */
void DialGadgetConfiguration::saveConfig(QSettings* settings) const {
    QString dialFile = Utils::PathUtils().RemoveDataPath(m_defaultDial);
    settings->setValue("dialFile", dialFile);

	settings->setValue("dialBackgroundID", dialBackgroundID);
    settings->setValue("dialForegroundID", dialForegroundID);

    settings->setValue("dialNeedleID1", dialNeedleID1);
    settings->setValue("dialNeedleID2", dialNeedleID2);
    settings->setValue("dialNeedleID3", dialNeedleID3);

    settings->setValue("needle1MinValue", needle1MinValue);
    settings->setValue("needle1MaxValue", needle1MaxValue);
    settings->setValue("needle2MinValue", needle2MinValue);
    settings->setValue("needle2MaxValue", needle2MaxValue);
    settings->setValue("needle3MinValue", needle3MinValue);
    settings->setValue("needle3MaxValue", needle3MaxValue);

    settings->setValue("needle1DataObject", needle1DataObject);
    settings->setValue("needle1ObjectField", needle1ObjectField);
    settings->setValue("needle2DataObject", needle2DataObject);
    settings->setValue("needle2ObjectField", needle2ObjectField);
    settings->setValue("needle3DataObject", needle3DataObject);
    settings->setValue("needle3ObjectField", needle3ObjectField);

    settings->setValue("needle1Factor", needle1Factor);
    settings->setValue("needle2Factor", needle2Factor);
    settings->setValue("needle3Factor", needle3Factor);

    settings->setValue("needle1Move", needle1Move);
    settings->setValue("needle2Move", needle2Move);
    settings->setValue("needle3Move", needle3Move);

    settings->setValue("font", font);

	settings->setValue("useOpenGLFlag", useOpenGLFlag);
	settings->setValue("beSmooth", beSmooth);
}
