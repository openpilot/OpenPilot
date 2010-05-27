/**
 ******************************************************************************
 *
 * @file       airspeedgadgetwidget.h
 * @author     David "Buzz" Carlson Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   airspeed 
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

#ifndef AIRSPEEDGADGETWIDGET_H_
#define AIRSPEEDGADGETWIDGET_H_

#include "airspeedgadgetconfiguration.h"
#include "uavobjects/uavobject.h"
#include <QGraphicsView>
#include <QtSvg/QSvgRenderer>
#include <QtSvg/QGraphicsSvgItem>

#include <QFile>
#include <QTimer>

class AirspeedGadgetWidget : public QGraphicsView
{
    Q_OBJECT

public:
    AirspeedGadgetWidget(QWidget *parent = 0);
   ~AirspeedGadgetWidget();
   void setDialFile(QString dfn, QString bg, QString fg, QString n1, QString n2);
   void paint();
    // setNeedle1 and setNeedle2 use a timer to simulate
    // needle inertia
   void setNeedle1(double value);
   void setNeedle2(double value);
   void setN1Min(double value) {n1MinValue = value;}
   void setN1Max(double value) {n1MaxValue = value;}
   void setN2Min(double value) {n1MinValue = value;}
   void setN2Max(double value) {n2MaxValue = value;}
   // Sets up needle/UAVObject connections:
   void connectNeedles(QString object1, QString field1,
                       QString object2, QString field2);

protected:
   void paintEvent(QPaintEvent *event);
   void resizeEvent(QResizeEvent *event);

private slots:
   // Test function
   void testRotate();
   void updateNeedle1(UAVObject *object1); // Called by the UAVObject
   void updateNeedle2(UAVObject *object2); // Called by the UAVObject
   void rotateNeedles();

private:
   QSvgRenderer *m_renderer;
   QGraphicsSvgItem *m_background;
   QGraphicsSvgItem *m_foreground;
   QGraphicsSvgItem *m_needle1;
   QGraphicsSvgItem *m_needle2;

   bool n2enabled; // Simple flag to skip rendering if the
   bool fgenabled; // layer does not exist.

   double n1MinValue;
   double n1MaxValue;
   double n2MinValue;
   double n2MaxValue;

   // The Value and target variables
   // are expressed in degrees
   double needle1Target;
   double needle1Value;
   double needle2Target;
   double needle2Value;

   // Name of the fields to read when an update is received:
   QString field1;
   QString field2;

   // Rotation timer
   QTimer dialTimer;

   // Test variables
#if 0
   int testSpeed;
   QTimer m_testTimer;
   // End test variables
#endif
};
#endif /* AIRSPEEDGADGETWIDGET_H_ */
