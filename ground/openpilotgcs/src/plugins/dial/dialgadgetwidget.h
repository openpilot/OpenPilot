/**
 ******************************************************************************
 *
 * @file       dialgadgetwidget.h
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

#ifndef DIALGADGETWIDGET_H_
#define DIALGADGETWIDGET_H_

#include "dialgadgetconfiguration.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjectmanager.h"
#include "uavobject.h"
#include <QGraphicsView>
#include <QtSvg/QSvgRenderer>
#include <QtSvg/QGraphicsSvgItem>

#include <QFile>
#include <QTimer>

class DialGadgetWidget : public QGraphicsView
{
    Q_OBJECT

public:
    DialGadgetWidget(QWidget *parent = 0);
   ~DialGadgetWidget();
   void enableOpenGL(bool flag);
   void enableSmoothUpdates(bool flag) { beSmooth = flag; }
   void setDialFile(QString dfn, QString bg, QString fg, QString n1, QString n2, QString n3,
                    QString n1Move, QString n2Move, QString n3Move);
   void paint();
    // setNeedle1 and setNeedle2 use a timer to simulate
    // needle inertia
   void setNeedle1(double value);
   void setNeedle2(double value);
   void setNeedle3(double value);
   void setN1Min(double value) {n1MinValue = value;}
   void setN1Max(double value) {n1MaxValue = value;}
   void setN1Factor(double value) {n1Factor = value;}
   void setN2Min(double value) {n2MinValue = value;}
   void setN2Max(double value) {n2MaxValue = value;}
   void setN2Factor(double value) {n2Factor = value;}
   void setN3Min(double value) {n3MinValue = value;}
   void setN3Max(double value) {n3MaxValue = value;}
   void setN3Factor(double value) {n3Factor = value;}
   // Sets up needle/UAVObject connections:
   void connectNeedles(QString object1, QString field1,
                       QString object2, QString field2,
                       QString object3, QString field3);
   void setDialFont(QString fontProps);

public slots:
   void updateNeedle1(UAVObject *object1); // Called by the UAVObject
   void updateNeedle2(UAVObject *object2); // Called by the UAVObject
   void updateNeedle3(UAVObject *object3); // Called by the UAVObject

protected:
   void paintEvent(QPaintEvent *event);
   void resizeEvent(QResizeEvent *event);


private slots:
   void rotateNeedles();

private:
   QSvgRenderer *m_renderer;
   QGraphicsSvgItem *m_background;
   QGraphicsSvgItem *m_foreground;
   QGraphicsSvgItem *m_needle1;
   QGraphicsSvgItem *m_needle2;
   QGraphicsSvgItem *m_needle3;
   QGraphicsTextItem *m_text1;
   QGraphicsTextItem *m_text2;
   QGraphicsTextItem *m_text3;

   bool n3enabled;
   bool n2enabled; // Simple flag to skip rendering if the
   bool fgenabled; // layer does not exist.
   bool dialError ;

   // Settings concerning move of the dials
   bool rotateN1;
   bool rotateN2;
   bool rotateN3;
   bool horizN1;
   bool horizN2;
   bool horizN3;
   bool vertN1;
   bool vertN2;
   bool vertN3;

   double n1MinValue;
   double n1MaxValue;
   double n1Factor;
   double n2MinValue;
   double n2MaxValue;
   double n2Factor;
   double n3MinValue;
   double n3MaxValue;
   double n3Factor;

   // The Value and target variables
   // are expressed in degrees
   double needle1Target;
   double needle1Value;
   double needle2Target;
   double needle2Value;
   double needle3Target;
   double needle3Value;

   // Name of the fields to read when an update is received:
   UAVDataObject* obj1;
   UAVDataObject* obj2;
   UAVDataObject* obj3;
   QString field1;
   QString subfield1;
   bool haveSubField1;
   QString field2;
   QString subfield2;
   bool haveSubField2;
   QString field3;
   QString subfield3;
   bool haveSubField3;

   // Rotation timer
   QTimer dialTimer;

   bool beSmooth;
};
#endif /* DIALGADGETWIDGET_H_ */
