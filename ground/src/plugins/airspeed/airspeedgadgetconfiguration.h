/**
 ******************************************************************************
 *
 * @file       airspeedgadgetconfiguration.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Airspeed Plugin Gadget configuration
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   Airspeed
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

#ifndef AIRSPEEDGADGETCONFIGURATION_H
#define AIRSPEEDGADGETCONFIGURATION_H

#include <coreplugin/iuavgadgetconfiguration.h>

using namespace Core;

/* Despite its name, this is actually a generic analog dial
   supporting up to two rotating "needle" indicators.
  */
class AirspeedGadgetConfiguration : public IUAVGadgetConfiguration
{
Q_OBJECT
public:
    explicit AirspeedGadgetConfiguration(QString classId, const QByteArray &state = 0, QObject *parent = 0);

    //set dial configuration functions
    void setDialFile(QString dialFile){m_defaultDial=dialFile;}
    void setDialBackgroundID(QString elementID) { dialBackgroundID = elementID;}
    void setDialForegroundID(QString elementID) { dialForegroundID = elementID;}
    void setDialNeedleID1(QString elementID) { dialNeedleID1 = elementID;}
    void setDialNeedleID2(QString elementID) { dialNeedleID2 = elementID;}
    void setN1Min(double val) { needle1MinValue = val;}
    void setN2Min(double val) { needle2MinValue = val;}
    void setN1Max(double val) { needle1MaxValue = val;}
    void setN2Max(double val) { needle2MaxValue = val;}
    void setN1Factor(double val) { needle1Factor = val;}
    void setN2Factor(double val) { needle2Factor = val;}
    void setN1DataObject(QString text) {needle1DataObject = text; }
    void setN2DataObject(QString text){ needle2DataObject = text; }
    void setN1ObjField(QString text) { needle1ObjectField = text; }
    void setN2ObjField(QString text) { needle2ObjectField = text; }
    void setN1Move( QString move) { needle1Move = move; }
    void setN2Move( QString move) { needle2Move = move; }

    //get dial configuration functions
    QString dialFile() {return m_defaultDial;}
    QString dialBackground() {return dialBackgroundID;}
    QString dialForeground() {return dialForegroundID;}
    QString dialNeedle1() {return dialNeedleID1;}
    QString dialNeedle2() {return dialNeedleID2;}
    double getN1Min() { return needle1MinValue;}
    double getN2Min() { return needle2MinValue;}
    double getN1Max() { return needle1MaxValue;}
    double getN2Max() { return needle2MaxValue;}
    double getN1Factor() { return needle1Factor;}
    double getN2Factor() { return needle2Factor;}
    QString getN1DataObject() { return needle1DataObject; }
    QString getN2DataObject() { return needle2DataObject; }
    QString getN1ObjField() { return needle1ObjectField; }
    QString getN2ObjField() { return needle2ObjectField; }
    QString getN1Move() { return needle1Move; }
    QString getN2Move() { return needle2Move; }

    QByteArray saveState() const;
    IUAVGadgetConfiguration *clone();

private:
    QString m_defaultDial; // The name of the dial's SVG source file
    QString dialBackgroundID; // SVG elementID of the background
    QString dialForegroundID; // ... of the foreground
    QString dialNeedleID1;     // ... and the first needle
    QString dialNeedleID2;     // ... and the second

    // Note: MinValue not used at the moment!
    double needle1MinValue; // Value corresponding to a 0 degree angle;
    double needle1MaxValue; // Value corresponding to a 360 degree angle;
    double needle2MinValue;
    double needle2MaxValue;
    double needle1Factor;
    double needle2Factor;

    QString needle1DataObject;
    QString needle1ObjectField;
    QString needle2DataObject;
    QString needle2ObjectField;

    // How the two dials move:
    QString needle1Move;
    QString needle2Move;
};

#endif // AIRSPEEDGADGETCONFIGURATION_H
