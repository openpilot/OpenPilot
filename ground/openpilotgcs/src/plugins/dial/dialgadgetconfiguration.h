/**
 ******************************************************************************
 *
 * @file       dialgadgetconfiguration.h
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

#ifndef DIALGADGETCONFIGURATION_H
#define DIALGADGETCONFIGURATION_H

#include <coreplugin/iuavgadgetconfiguration.h>

using namespace Core;

/* Despite its name, this is actually a generic analog dial
   supporting up to two rotating "needle" indicators.
  */
class DialGadgetConfiguration : public IUAVGadgetConfiguration
{
Q_OBJECT
public:
    explicit DialGadgetConfiguration(QString classId, QSettings* qSettings = 0, QObject *parent = 0);


    //set dial configuration functions
    void setDialFile(QString dialFile){m_defaultDial=dialFile;}
    void setDialBackgroundID(QString elementID) { dialBackgroundID = elementID;}
    void setDialForegroundID(QString elementID) { dialForegroundID = elementID;}
    void setDialNeedleID1(QString elementID) { dialNeedleID1 = elementID;}
    void setDialNeedleID2(QString elementID) { dialNeedleID2 = elementID;}
    void setDialNeedleID3(QString elementID) { dialNeedleID3 = elementID;}
    void setN1Min(double val) { needle1MinValue = val;}
    void setN2Min(double val) { needle2MinValue = val;}
    void setN3Min(double val) { needle3MinValue = val;}
    void setN1Max(double val) { needle1MaxValue = val;}
    void setN2Max(double val) { needle2MaxValue = val;}
    void setN3Max(double val) { needle3MaxValue = val;}
    void setN1Factor(double val) { needle1Factor = val;}
    void setN2Factor(double val) { needle2Factor = val;}
    void setN3Factor(double val) { needle3Factor = val;}
    void setN1DataObject(QString text) {needle1DataObject = text; }
    void setN2DataObject(QString text){ needle2DataObject = text; }
    void setN3DataObject(QString text){ needle3DataObject = text; }
    void setN1ObjField(QString text) { needle1ObjectField = text; }
    void setN2ObjField(QString text) { needle2ObjectField = text; }
    void setN3ObjField(QString text) { needle3ObjectField = text; }
    void setN1Move( QString move) { needle1Move = move; }
    void setN2Move( QString move) { needle2Move = move; }
    void setN3Move( QString move) { needle3Move = move; }
    void setFont(QString text) { font = text; }
	void setUseOpenGL(bool flag) { useOpenGLFlag = flag; }
	void setBeSmooth(bool flag) { beSmooth = flag;}


    //get dial configuration functions
    QString dialFile() {return m_defaultDial;}
    QString dialBackground() {return dialBackgroundID;}
    QString dialForeground() {return dialForegroundID;}
    QString dialNeedle1() {return dialNeedleID1;}
    QString dialNeedle2() {return dialNeedleID2;}
    QString dialNeedle3() {return dialNeedleID3;}
    double getN1Min() { return needle1MinValue;}
    double getN2Min() { return needle2MinValue;}
    double getN3Min() { return needle3MinValue;}
    double getN1Max() { return needle1MaxValue;}
    double getN2Max() { return needle2MaxValue;}
    double getN3Max() { return needle3MaxValue;}
    double getN1Factor() { return needle1Factor;}
    double getN2Factor() { return needle2Factor;}
    double getN3Factor() { return needle3Factor;}
    QString getN1DataObject() { return needle1DataObject; }
    QString getN2DataObject() { return needle2DataObject; }
    QString getN3DataObject() { return needle3DataObject; }
    QString getN1ObjField() { return needle1ObjectField; }
    QString getN2ObjField() { return needle2ObjectField; }
    QString getN3ObjField() { return needle3ObjectField; }
    QString getN1Move() { return needle1Move; }
    QString getN2Move() { return needle2Move; }
    QString getN3Move() { return needle3Move; }
    QString getFont() { return font;}
	bool useOpenGL() { return useOpenGLFlag; }
	bool getBeSmooth() { return beSmooth; }

    void saveConfig(QSettings* settings) const;
    IUAVGadgetConfiguration *clone();

private:
    QString m_defaultDial; // The name of the dial's SVG source file
    QString dialBackgroundID; // SVG elementID of the background
    QString dialForegroundID; // ... of the foreground
    QString dialNeedleID1;     // ... and the first needle
    QString dialNeedleID2;     // ... and the second
    QString dialNeedleID3;     // ... and the third

    // Note: MinValue not used at the moment!
    double needle1MinValue; // Value corresponding to a 0 degree angle;
    double needle1MaxValue; // Value corresponding to a 360 degree angle;
    double needle2MinValue;
    double needle2MaxValue;
    double needle3MinValue;
    double needle3MaxValue;
    double needle1Factor;
    double needle2Factor;
    double needle3Factor;

    // The font used for the dial
    QString font;

    QString needle1DataObject;
    QString needle1ObjectField;
    QString needle2DataObject;
    QString needle2ObjectField;
    QString needle3DataObject;
    QString needle3ObjectField;

    // How the two dials move:
    QString needle1Move;
    QString needle2Move;
    QString needle3Move;

	bool useOpenGLFlag;
	bool beSmooth;
};

#endif // DIALGADGETCONFIGURATION_H
