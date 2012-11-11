/**
 ******************************************************************************
 *
 * @file       pfdgadgetwidget.h
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup OPMapPlugin Primary Flight Display Plugin
 * @{
 * @brief The Primary Flight Display Gadget 
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
#include "uavobjectmanager.h"
#include "uavobject.h"
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
   // Sets up needle/UAVObject connections:
   void connectNeedles();
   void enableOpenGL(bool flag);
   void setHqFonts(bool flag) { hqFonts = flag; }
   void enableSmoothUpdates(bool flag) { beSmooth = flag; }


public slots:
   void updateAttitude(UAVObject *object1);
   void updateHeading(UAVObject *object1);
   void updateGPS(UAVObject *object1);
   void updateGroundspeed(UAVObject *object1);
   void updateAirspeed(UAVObject *object1);
   void updateAltitude(UAVObject *object1);
   void updateBattery(UAVObject *object1);
   void updateLinkStatus(UAVObject *object1);

protected:
   void paintEvent(QPaintEvent *event);
   void resizeEvent(QResizeEvent *event);


private slots:
   void moveNeedles();
   void moveVerticalScales();
   void moveSky();
   void setToolTipPrivate();
private:
   QSvgRenderer *m_renderer;

   // Background: background
   QGraphicsSvgItem *m_background;
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
   // Speed scale:
   QGraphicsItemGroup *m_speedscale;
   // Speed indicator text:
   QGraphicsTextItem *m_speedtext;
   // Vertical altitude scale:
   QGraphicsItemGroup *m_altitudescale;
   // Altitude indicator text:
   QGraphicsTextItem *m_altitudetext;
   // GCS link status Arrow
   QGraphicsSvgItem *gcsTelemetryArrow;
   QGraphicsTextItem *gcsTelemetryStats;
   QGraphicsTextItem *gcsBatteryStats;
   QGraphicsTextItem *gcsGPSStats;

   // The Value and target variables
   // are expressed in degrees
   double rollTarget;
   double rollValue;
   double pitchTarget;
   double pitchValue;
   double headingTarget;
   double headingValue;
   double groundspeedTarget;
   double groundspeedValue;
   double airspeedTarget;
   double airspeedValue;
   double altitudeTarget;
   double altitudeValue;

   qreal compassBandWidth;
   qreal speedScaleHeight;
   qreal altitudeScaleHeight;

   // Name of the fields to read when an update is received:
   UAVDataObject* airspeedObj;
   UAVDataObject* altitudeObj;
   UAVDataObject* attitudeObj;
   UAVDataObject* groundspeedObj;
   UAVDataObject* headingObj;
   UAVDataObject* gpsObj;
   UAVDataObject* gcsTelemetryObj;
   UAVDataObject* gcsBatteryObj;

   // Rotation timer
   QTimer dialTimer;
   QTimer skyDialTimer;

   QString satString;
   QString batString;

   // Flag to check for pfd Error
   bool pfdError;
   // Flag to enable better rendering of fonts in OpenGL
   bool hqFonts;
   bool beSmooth;

};
#endif /* PFDGADGETWIDGET_H_ */
