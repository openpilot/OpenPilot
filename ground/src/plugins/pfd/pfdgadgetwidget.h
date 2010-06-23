/**
 ******************************************************************************
 *
 * @file       pfdgadgetwidget.h
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   pfd
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

#ifndef PFDGADGETWIDGET_H_
#define PFDGADGETWIDGET_H_

#include "pfdgadgetconfiguration.h"
#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/uavobject.h"
#include <QGraphicsView>
#include <QtSvg/QSvgRenderer>
#include <QtSvg/QGraphicsSvgItem>

#include <QFile>
#include <QTimer>

class PFDGadgetWidget : public QGraphicsView
{
    Q_OBJECT

public:
    PFDGadgetWidget(QWidget *parent = 0);
   ~PFDGadgetWidget();
   void setDialFile(QString dfn);
   void paint();
   void setN2Min(double value) {n2MinValue = value;}
   void setN2Max(double value) {n2MaxValue = value;}
   void setN2Factor(double value) {n2Factor = value;}
   void setN3Min(double value) {n3MinValue = value;}
   void setN3Max(double value) {n3MaxValue = value;}
   void setN3Factor(double value) {n3Factor = value;}
   // Sets up needle/UAVObject connections:
   void connectNeedles();

public slots:
   void updateAttitude(UAVObject *object1);
   void updateHeading(UAVObject *object1);
   void updateAirspeed(UAVObject *object1);
   void updateAltitude(UAVObject *object1);
   void updateBattery(UAVObject *object1);

protected:
   void paintEvent(QPaintEvent *event);
   void resizeEvent(QResizeEvent *event);


private slots:
   void rotateNeedles();

private:
   QSvgRenderer *m_renderer;

   // Background: background
   QGraphicsSvgItem *m_background;
   // Foreground: foreground (contains all fixed elements, including plane)
   QGraphicsSvgItem *m_foreground;
   // earth/sky : world
   QGraphicsSvgItem *m_world;
   // Roll scale: rollscale
   QGraphicsSvgItem *m_rollscale;
   // Compass dial:
   QGraphicsSvgItem *m_compass;
   // Compass band:
   QGraphicsSvgItem *m_compassband;
   // Home point:
   QGraphicsSvgItem *m_homewaypoint;
   // Next point:
   QGraphicsSvgItem *m_nextwaypoint;
   // Home point bearing:
   QGraphicsSvgItem *m_homepointbearing;
   // Next point bearing:
   QGraphicsSvgItem *m_nextpointbearing;

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
   double rollTarget;
   double rollValue;
   double pitchTarget;
   double pitchValue;
   double headingTarget;
   double headingValue;

   qreal compassBandWidth;

   // Name of the fields to read when an update is received:
   UAVDataObject* attitudeObj;
   UAVDataObject* headingObj;

   // Rotation timer
   QTimer dialTimer;

};
#endif /* PFDGADGETWIDGET_H_ */
