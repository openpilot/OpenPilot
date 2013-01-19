/**
 ******************************************************************************
 *
 * @file       pfdgadgetwidget.cpp
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

#include "pfdgadgetwidget.h"
#include <utils/stylehelper.h>
#include <utils/cachedsvgitem.h>
#include <iostream>
#include <QDebug>
#include <QPainter>
#include <QtOpenGL/QGLWidget>
#include <cmath>

PFDGadgetWidget::PFDGadgetWidget(QWidget *parent) : QGraphicsView(parent)
{

    setMinimumSize(64,64);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setScene(new QGraphicsScene(this));
//    setRenderHint(QPainter::SmoothPixmapTransform);

    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    //setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    //setRenderHints(QPainter::TextAntialiasing);
    //setRenderHints(QPainter::HighQualityAntialiasing);

    m_renderer = new QSvgRenderer();

    attitudeObj = NULL;
    headingObj = NULL;
    gcsBatteryObj = NULL;
    gpsObj = NULL;
    compassBandWidth = 0;
    pfdError = true;
    hqFonts = false;
    rollTarget = 0;
    rollValue = 0;
    pitchTarget = 0;
    pitchValue = 0;
    headingTarget = 0;
    headingValue = 0;
    groundspeedTarget = 0;
    groundspeedValue = 0;
    altitudeTarget = 0;
    altitudeValue = 0;

    // This timer mechanism makes needles rotate smoothly
    connect(&dialTimer, SIGNAL(timeout()), this, SLOT(moveNeedles()));
    dialTimer.start(30);

    connect(&skyDialTimer, SIGNAL(timeout()), this, SLOT(moveSky()));
    skyDialTimer.start(30);


}

PFDGadgetWidget::~PFDGadgetWidget()
{
    skyDialTimer.stop();
    dialTimer.stop();
}

void PFDGadgetWidget::setToolTipPrivate()
{
    static qint32 updateRate=0;
    UAVObject::Metadata mdata=attitudeObj->getMetadata();
    if(mdata.flightTelemetryUpdatePeriod!=updateRate)
        this->setToolTip("Current refresh rate:"+QString::number(mdata.flightTelemetryUpdatePeriod)+" miliseconds"+"\nIf you want to change it please edit the AttitudeActual metadata on the object browser.");
}

/*!
  \brief Enables/Disables OpenGL
  */
void PFDGadgetWidget::enableOpenGL(bool flag) {
    if (flag) {
        setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
    } else {
        setViewport(new QWidget);
    }
}

/*!
  \brief Connects the widget to the relevant UAVObjects

  Should only be called after the PFD artwork is loaded.
  We want: AttitudeActual, FlightBattery, Location.

  */
void PFDGadgetWidget::connectNeedles() {
    if (attitudeObj != NULL)
        disconnect(attitudeObj,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(updateAttitude(UAVObject*)));

    if (headingObj != NULL)
        disconnect(headingObj,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(updateHeading(UAVObject*)));

    if (gcsBatteryObj != NULL)
    	disconnect(gcsBatteryObj,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(updateBattery(UAVObject*)));

    if (gpsObj != NULL)
        disconnect(gpsObj,SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updateGPS(UAVObject*)));

    // Safeguard: if artwork did not load properly, don't go further
    if (pfdError)
    	return;
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

	airspeedObj = dynamic_cast<UAVDataObject*>(objManager->getObject("AirspeedActual"));
    if (airspeedObj != NULL ) {
		connect(airspeedObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updateAirspeed(UAVObject*)));
    } else {
		 qDebug() << "Error: Object is unknown (AirspeedActual).";
    }

    groundspeedObj = dynamic_cast<UAVDataObject*>(objManager->getObject("VelocityActual"));
    if (groundspeedObj != NULL ) {
        connect(groundspeedObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updateGroundspeed(UAVObject*)));
    } else {
         qDebug() << "Error: Object is unknown (VelocityActual).";
    }

    altitudeObj = dynamic_cast<UAVDataObject*>(objManager->getObject("PositionActual"));
    if (altitudeObj != NULL ) {
        connect(altitudeObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updateAltitude(UAVObject*)));
    } else {
         qDebug() << "Error: Object is unknown (PositionActual).";
    }

   attitudeObj = dynamic_cast<UAVDataObject*>(objManager->getObject("AttitudeActual"));
   if (attitudeObj != NULL ) {
       connect(attitudeObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updateAttitude(UAVObject*)));
   } else {
        qDebug() << "Error: Object is unknown (AttitudeActual).";
   }

   headingObj = dynamic_cast<UAVDataObject*>(objManager->getObject("PositionActual"));
   if (headingObj != NULL ) {
       connect(headingObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updateHeading(UAVObject*)));
   } else {
        qDebug() << "Error: Object is unknown (PositionActual).";
   }

   if (gcsGPSStats) {
       gpsObj = dynamic_cast<UAVDataObject*>(objManager->getObject("GPSPosition"));
       if (gpsObj != NULL) {
           connect(gpsObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updateGPS(UAVObject*)));
       } else {
           qDebug() << "Error: Object is unknown (GPSPosition).";
       }
   }

   if (gcsTelemetryArrow || gcsTelemetryStats ) {
       // Only register if the PFD wants link stats/status
      gcsTelemetryObj = dynamic_cast<UAVDataObject*>(objManager->getObject("GCSTelemetryStats"));
      if (gcsTelemetryObj != NULL ) {
          connect(gcsTelemetryObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updateLinkStatus(UAVObject*)));
      } else {
           qDebug() << "Error: Object is unknown (GCSTelemetryStats).";
      }
    }

   if (gcsBatteryStats) {  // Only register if the PFD wants battery display
       gcsBatteryObj = dynamic_cast<UAVDataObject*>(objManager->getObject("FlightBatteryState"));
       if (gcsBatteryObj != NULL ) {
           connect(gcsBatteryObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updateBattery(UAVObject*)));
       } else {
            qDebug() << "Error: Object is unknown (FlightBatteryState).";
       }
   }
}


/*!
  \brief Updates the GPS stats
  */
void PFDGadgetWidget::updateGPS(UAVObject *object1) {
    UAVObjectField* field = object1->getField(QString("Satellites"));
    UAVObjectField* field1 = object1->getField(QString("PDOP"));
    if (field && field1) {
        QString s = QString("GPS: ") + field->getValue().toString() + "\nPDP: "
                    + field1->getValue().toString();
        if (s != satString) {
            gcsGPSStats->setPlainText(s);
            satString = s;
        }
    }
}

/*!
 \brief Updates the link stats
 */
void PFDGadgetWidget::updateLinkStatus(UAVObject *object1) {
    // TODO: find a way to avoid updating the graphics if the value
    //       has not changed since the last update
    // Double check that the field exists:
    QString st = QString("Status");
    QString tdr = QString("TxDataRate");
    QString rdr = QString("RxDataRate");
    UAVObjectField* field = object1->getField(st);
    UAVObjectField* field2 = object1->getField(tdr);
    UAVObjectField* field3 = object1->getField(rdr);
    if (field && field2 && field3) {
    	QString s = field->getValue().toString();
        if (m_renderer->elementExists("gcstelemetry-" + s) && gcsTelemetryArrow) {
                gcsTelemetryArrow->setElementId("gcstelemetry-" + s);
            } else { // Safeguard
                gcsTelemetryArrow->setElementId("gcstelemetry-Disconnected");
            }
        double v1 = field2->getDouble();
        double v2 = field3->getDouble();
        s.sprintf("%.0f/%.0f",v1,v2);
        if (gcsTelemetryStats) gcsTelemetryStats->setPlainText(s);
    } else {
        qDebug() << "UpdateLinkStatus: Wrong field, maybe an issue with object disconnection ?";
    }
}

/*!
  \brief Reads the updated attitude and computes the new display position

  Resolution is 1 degree roll & 1/7.5 degree pitch.
  */
void PFDGadgetWidget::updateAttitude(UAVObject *object1) {
    setToolTipPrivate();
    UAVObjectField * rollField = object1->getField(QString("Roll"));
    UAVObjectField * yawField = object1->getField(QString("Yaw"));
    UAVObjectField * pitchField = object1->getField(QString("Pitch"));
    if(rollField && yawField && pitchField) {
        // These factors assume some things about the PFD SVG, namely:
        // - Roll, Pitch and Heading value in degrees
        // - Pitch lines are 300px high for a +20/-20 range, which means
        //   7.5 pixels per pitch degree.
        // TODO: loosen this constraint and only require a +/- 20 deg range,
        //       and compute the height from the SVG element.
        // Also: keep the integer value only, to avoid unnecessary redraws
        rollTarget = -floor(rollField->getDouble()*10)/10;
        if ((rollTarget - rollValue) > 180) {
            rollValue += 360;
        } else if (((rollTarget - rollValue) < -180)) {
            rollValue -= 360;
        }
        pitchTarget = floor(pitchField->getDouble()*7.5);

        // These factors assume some things about the PFD SVG, namely:
        // - Heading value in degrees
        // - "Scale" element is 540 degrees wide

        // Corvus Corax: "If you want a smooth transition between two angles, It is usually solved that by substracting
        // one from another, and if the result is >180 or <-180 I substract (respectively add) 360 degrees
        // to it. That way you always get the "shorter difference" to turn in."
        double fac = compassBandWidth/540;
        headingTarget = yawField->getDouble()*(-fac);
        if (headingTarget != headingTarget)
            headingTarget = headingValue; // NaN checking.
        if ((headingValue - headingTarget)/fac > 180) {
            headingTarget += 360*fac;
        } else if (((headingValue - headingTarget)/fac < -180)) {
            headingTarget -= 360*fac;
        }
        headingTarget = floor(headingTarget*10)/10; // Avoid stupid redraws

        if (!dialTimer.isActive())
            dialTimer.start(); // Rearm the dial Timer which might be stopped.

    } else {
        qDebug() << "Unable to get one of the fields for attitude update";
    }
}

/*!
  \brief Updates the compass reading and speed dial.

  This also updates speed & altitude according to PositionActual data.

    Note: the speed dial shows the ground speed at the moment, because
        there is no airspeed by default. Should become configurable in a future
        gadget release (TODO)
  */
void PFDGadgetWidget::updateHeading(UAVObject *object) {
    Q_UNUSED(object);
}

/*!
  \brief Called by updates to @PositionActual to compute groundspeed from velocity
  */
void PFDGadgetWidget::updateGroundspeed(UAVObject *object) {
    UAVObjectField* northField = object->getField("North");
    UAVObjectField* eastField = object->getField("East");
    if (northField && eastField) {
        double val = floor(sqrt(pow(northField->getDouble(),2) + pow(eastField->getDouble(),2))*10)/10;
        groundspeedTarget = 3.6*val*speedScaleHeight/30;

        if (!dialTimer.isActive())
            dialTimer.start(); // Rearm the dial Timer which might be stopped.

    } else {
        qDebug() << "UpdateHeading: Wrong field, maybe an issue with object disconnection ?";
    }
}


/*!
  \brief Called by updates to @AirspeedActual
  */
void PFDGadgetWidget::updateAirspeed(UAVObject *object) {
	UAVObjectField* airspeedField = object->getField("CalibratedAirspeed");
    if (airspeedField) {
        airspeedTarget = airspeedField->getDouble();

        if (!dialTimer.isActive())
            dialTimer.start(); // Rearm the dial Timer which might be stopped.

    } else {
        qDebug() << "UpdateHeading: Wrong field, maybe an issue with object disconnection ?";
    }
}

/*!
  \brief Called by the @ref PositionActual updates to show altitude
  */
void PFDGadgetWidget::updateAltitude(UAVObject *object) {
    UAVObjectField* downField = object->getField("Down");
    if (downField) {
        altitudeTarget = -downField->getDouble();

        if (!dialTimer.isActive())
            dialTimer.start(); // Rearm the dial Timer which might be stopped.

    } else {
        qDebug() << "Unable to get field for altitude update.  Obj: " << object->getName();
    }
}


/*!
  \brief Called by the UAVObject which got updated
  */
void PFDGadgetWidget::updateBattery(UAVObject *object1) {
    // Double check that the field exists:
    QString voltage = QString("Voltage");
    QString current = QString("Current");
    QString energy = QString("ConsumedEnergy");
    UAVObjectField* field = object1->getField(voltage);
    UAVObjectField* field2 = object1->getField(current);
    UAVObjectField* field3 = object1->getField(energy);
    if (field && field2 && field3) {
    	QString s = QString();
    	double v0 = field->getDouble();
        double v1 = field2->getDouble();
        double v2 = field3->getDouble();
        s.sprintf("%.2fV\n%.2fA\n%.0fmAh",v0,v1,v2);
        if (s != batString) {
            gcsBatteryStats->setPlainText(s);
            batString = s;
        }
    } else {
        qDebug() << "UpdateBattery: Wrong field, maybe an issue with object disconnection ?";
    }

}


/*!
  \brief Sets up the PFD from the SVG master file.

  Initializes the display, and does all the one-time calculations.
  */
void PFDGadgetWidget::setDialFile(QString dfn)
{
   QGraphicsScene *l_scene = scene();
   setBackgroundBrush(QBrush(Utils::StyleHelper::baseColor()));
   if (QFile::exists(dfn) && m_renderer->load(dfn) && m_renderer->isValid())
   {
/* The PFD element IDs are fixed, not like with the analog dial.
     - Background: background
     - Foreground: foreground (contains all fixed elements, including plane)
     - earth/sky : world
     - Roll scale: rollscale
     - compass frame: compass (part of the foreground)
     - compass band : compass-band
     - Home point: homewaypoint
     - Next point: nextwaypoint
     - Home point bearing: homewaypoint-bearing
     - Next point bearing: nextwaypoint-bearing
     - Speed rectangle (left side): speed-bg
     - Speed scale: speed-scale.
     - Black speed window: speed-window.
     - Altitude rectangle (right site): altitude-bg.
     - Altitude scale: altitude-scale.
     - Black altitude window: altitude-window.
     - GCS Telemetry status arrow: gcstelemetry-XXXX
     - Telemetry link rate: linkrate
     - GPS status text: gps-txt
     - Battery stats: battery-txt
 */
         l_scene->clear(); // Deletes all items contained in the scene as well.
         m_background = new CachedSvgItem();
         // All other items will be clipped to the shape of the background
         m_background->setFlags(QGraphicsItem::ItemClipsChildrenToShape|
                                QGraphicsItem::ItemClipsToShape);
         m_background->setSharedRenderer(m_renderer);
         m_background->setElementId("background");
         l_scene->addItem(m_background);

         m_world = new CachedSvgItem();
         m_world->setParentItem(m_background);
         m_world->setSharedRenderer(m_renderer);
         m_world->setElementId("world");
         l_scene->addItem(m_world);

         // red Roll scale: rollscale
         m_rollscale = new CachedSvgItem();
         m_rollscale->setSharedRenderer(m_renderer);
         m_rollscale->setElementId("rollscale");
         l_scene->addItem(m_rollscale);

         // Home point:
         m_homewaypoint = new CachedSvgItem();
         // Next point:
         m_nextwaypoint = new CachedSvgItem();
         // Home point bearing:
         m_homepointbearing = new CachedSvgItem();
         // Next point bearing:
         m_nextpointbearing = new CachedSvgItem();

         QGraphicsSvgItem *m_foreground = new CachedSvgItem();
         m_foreground->setParentItem(m_background);
         m_foreground->setSharedRenderer(m_renderer);
         m_foreground->setElementId("foreground");
         l_scene->addItem(m_foreground);

         ////////////////////
         // Compass
         ////////////////////
         // Get the default location of the Compass:
         QMatrix compassMatrix = m_renderer->matrixForElement("compass");
         qreal startX = compassMatrix.mapRect(m_renderer->boundsOnElement("compass")).x();
         qreal startY = compassMatrix.mapRect(m_renderer->boundsOnElement("compass")).y();
         // Then once we have the initial location, we can put it
         // into a QGraphicsSvgItem which we will display at the same
         // place: we do this so that the heading scale can be clipped to
         // the compass dial region.
         m_compass = new CachedSvgItem();
         m_compass->setSharedRenderer(m_renderer);
         m_compass->setElementId("compass");
         m_compass->setFlags(QGraphicsItem::ItemClipsChildrenToShape|
                             QGraphicsItem::ItemClipsToShape);
         l_scene->addItem(m_compass);
         QTransform matrix;
         matrix.translate(startX,startY);
         m_compass->setTransform(matrix,false);

         // Now place the compass scale inside:
         m_compassband = new CachedSvgItem();
         m_compassband->setSharedRenderer(m_renderer);
         m_compassband->setElementId("compass-band");
         m_compassband->setParentItem(m_compass);
         l_scene->addItem(m_compassband);
         matrix.reset();
         // Note: the compass band has to be a path, which means all text elements have to be
         // converted, ortherwise boundsOnElement does not compute the height correctly
         // if the highest element is a text element. This is a Qt Bug as far as I can tell.

         // compass-scale is the while bottom line inside the band: using the band's width
         // includes half the width of the letters, which causes errors:
         compassBandWidth = m_renderer->boundsOnElement("compass-scale").width();

         ////////////////////
         // Speed
         ////////////////////
         // Speedometer on the left hand:
         //
         compassMatrix = m_renderer->matrixForElement("speed-bg");
         startX = compassMatrix.mapRect(m_renderer->boundsOnElement("speed-bg")).x();
         startY = compassMatrix.mapRect(m_renderer->boundsOnElement("speed-bg")).y();
         QGraphicsSvgItem *verticalbg = new CachedSvgItem();
         verticalbg->setSharedRenderer(m_renderer);
         verticalbg->setElementId("speed-bg");
         verticalbg->setFlags(QGraphicsItem::ItemClipsChildrenToShape|
                             QGraphicsItem::ItemClipsToShape);
         l_scene->addItem(verticalbg);
         matrix.reset();
         matrix.translate(startX,startY);
         verticalbg->setTransform(matrix,false);

         // Note: speed-scale should contain exactly 6 major ticks
         // for 30km/h
         m_speedscale = new QGraphicsItemGroup();
         m_speedscale->setParentItem(verticalbg);

         QGraphicsSvgItem *speedscalelines = new CachedSvgItem();
         speedscalelines->setSharedRenderer(m_renderer);
         speedscalelines->setElementId("speed-scale");
         speedScaleHeight = m_renderer->matrixForElement("speed-scale").mapRect(
                        m_renderer->boundsOnElement("speed-scale")).height();
         startX = compassMatrix.mapRect(m_renderer->boundsOnElement("speed-bg")).width();
         startX -= m_renderer->matrixForElement("speed-scale").mapRect(
                        m_renderer->boundsOnElement("speed-scale")).width();
         matrix.reset();
         matrix.translate(startX,0);
         speedscalelines->setTransform(matrix,false);
         // Quick way to reposition the item before putting it in the group:
         speedscalelines->setParentItem(verticalbg);
         m_speedscale->addToGroup(speedscalelines); // (reparents the item)

         // Add the scale text elements:
         QGraphicsTextItem *speed0 = new QGraphicsTextItem("0");
         speed0->setDefaultTextColor(QColor("White"));
         speed0->setFont(QFont("Arial",(int) speedScaleHeight/30));
         if (hqFonts) speed0->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
         matrix.reset();
         matrix.translate(compassMatrix.mapRect(m_renderer->boundsOnElement("speed-bg")).width()/10,-speedScaleHeight/30);
         speed0->setTransform(matrix,false);
         speed0->setParentItem(verticalbg);
         m_speedscale->addToGroup(speed0);
         for (int i=0; i<6;i++) {
             speed0 = new QGraphicsTextItem("");
             speed0->setDefaultTextColor(QColor("White"));
             speed0->setFont(QFont("Arial",(int) speedScaleHeight/30));
             speed0->setPlainText(QString().setNum(i*5+1));
             if (hqFonts) speed0->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
             matrix.translate(0,speedScaleHeight/6);
             speed0->setTransform(matrix,false);
             speed0->setParentItem(verticalbg);
             m_speedscale->addToGroup(speed0);    
         }
         // Now vertically center the speed scale on the speed background
         QRectF rectB = verticalbg->boundingRect();
         QRectF rectN = speedscalelines->boundingRect();
         m_speedscale->setPos(0,rectB.height()/2-rectN.height()/2-rectN.height()/6);

         // Isolate the speed window and put it above the speed scale
         compassMatrix = m_renderer->matrixForElement("speed-window");
         startX = compassMatrix.mapRect(m_renderer->boundsOnElement("speed-window")).x();
         startY = compassMatrix.mapRect(m_renderer->boundsOnElement("speed-window")).y();
         qreal speedWindowHeight = compassMatrix.mapRect(m_renderer->boundsOnElement("speed-window")).height();
         QGraphicsSvgItem *speedwindow = new CachedSvgItem();
         speedwindow->setSharedRenderer(m_renderer);
         speedwindow->setElementId("speed-window");
         speedwindow->setFlags(QGraphicsItem::ItemClipsChildrenToShape|
                             QGraphicsItem::ItemClipsToShape);
         l_scene->addItem(speedwindow);
         matrix.reset();
         matrix.translate(startX,startY);
         speedwindow->setTransform(matrix,false);

         // Last create a Text Item at the location of the speed window
         // and save it for future updates:
         m_speedtext = new QGraphicsTextItem("0000");
         m_speedtext->setDefaultTextColor(QColor("White"));
         m_speedtext->setFont(QFont("Arial",(int) speedWindowHeight/2));
         if (hqFonts)  m_speedtext->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
         m_speedtext->setParentItem(speedwindow);

         ////////////////////
         // Altitude
         ////////////////////
         // Right hand, very similar to speed
         compassMatrix = m_renderer->matrixForElement("altitude-bg");
         startX = compassMatrix.mapRect(m_renderer->boundsOnElement("altitude-bg")).x();
         startY = compassMatrix.mapRect(m_renderer->boundsOnElement("altitude-bg")).y();
         verticalbg = new CachedSvgItem();
         verticalbg->setSharedRenderer(m_renderer);
         verticalbg->setElementId("altitude-bg");
         verticalbg->setFlags(QGraphicsItem::ItemClipsChildrenToShape|
                             QGraphicsItem::ItemClipsToShape);
         l_scene->addItem(verticalbg);
         matrix.reset();
         matrix.translate(startX,startY);
         verticalbg->setTransform(matrix,false);

         // Note: altitude-scale should contain exactly 6 major ticks
         // for 30 meters
         m_altitudescale = new QGraphicsItemGroup();
         m_altitudescale->setParentItem(verticalbg);

         QGraphicsSvgItem *altitudescalelines = new CachedSvgItem();
         altitudescalelines->setSharedRenderer(m_renderer);
         altitudescalelines->setElementId("altitude-scale");
         altitudeScaleHeight = m_renderer->matrixForElement("altitude-scale").mapRect(
                        m_renderer->boundsOnElement("altitude-scale")).height();
         // Quick way to reposition the item before putting it in the group:
         altitudescalelines->setParentItem(verticalbg);
         m_altitudescale->addToGroup(altitudescalelines); // (reparents the item)

         // Add the scale text elements:
         speed0 = new QGraphicsTextItem("XXXX");
         speed0->setDefaultTextColor(QColor("White"));
         speed0->setFont(QFont("Arial",(int) altitudeScaleHeight/30));
         if (hqFonts) speed0->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
         matrix.reset();
         matrix.translate(m_renderer->matrixForElement("altitude-scale").mapRect(m_renderer->boundsOnElement("altitude-scale")).width()
                          + m_renderer->matrixForElement("altitude-bg").mapRect(m_renderer->boundsOnElement("altitude-bg")).width()/10,-altitudeScaleHeight/30);
         speed0->setTransform(matrix,false);
         speed0->setParentItem(verticalbg);
         m_altitudescale->addToGroup(speed0);
         for (int i=0; i<6;i++) {
             speed0 = new QGraphicsTextItem("XXXX");
             speed0->setDefaultTextColor(QColor("White"));
             speed0->setFont(QFont("Arial",(int) altitudeScaleHeight/30));
             speed0->setPlainText(QString().setNum(i*5+1));
             if (hqFonts) speed0->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
             matrix.translate(0,altitudeScaleHeight/6);
             speed0->setTransform(matrix,false);
             speed0->setParentItem(verticalbg);
             m_altitudescale->addToGroup(speed0);
         }
         // Now vertically center the speed scale on the speed background
         rectB = verticalbg->boundingRect();
         rectN = altitudescalelines->boundingRect();
         m_altitudescale->setPos(0,rectB.height()/2-rectN.height()/2-rectN.height()/6);

         // Isolate the Altitude window and put it above the altitude scale
         compassMatrix = m_renderer->matrixForElement("altitude-window");
         startX = compassMatrix.mapRect(m_renderer->boundsOnElement("altitude-window")).x();
         startY = compassMatrix.mapRect(m_renderer->boundsOnElement("altitude-window")).y();
         qreal altitudeWindowHeight = compassMatrix.mapRect(m_renderer->boundsOnElement("altitude-window")).height();
         QGraphicsSvgItem *altitudewindow = new CachedSvgItem();
         altitudewindow->setSharedRenderer(m_renderer);
         altitudewindow->setElementId("altitude-window");
         altitudewindow->setFlags(QGraphicsItem::ItemClipsChildrenToShape|
                             QGraphicsItem::ItemClipsToShape);
         l_scene->addItem(altitudewindow);
         matrix.reset();
         matrix.translate(startX,startY);
         altitudewindow->setTransform(matrix,false);

         // Last create a Text Item at the location of the speed window
         // and save it for future updates:
         m_altitudetext = new QGraphicsTextItem("0000");
         m_altitudetext->setDefaultTextColor(QColor("White"));
         m_altitudetext->setFont(QFont("Arial",(int) altitudeWindowHeight/2));
         if (hqFonts)  m_altitudetext->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
         m_altitudetext->setParentItem(altitudewindow);
         startX = compassMatrix.mapRect(m_renderer->boundsOnElement("altitude-window")).width()/10;
         matrix.reset();
         matrix.translate(startX,0);
         m_altitudetext->setTransform(matrix,false);

         ////////////////
         // GCS Telemetry Indicator
         ////////////////
         if (m_renderer->elementExists("gcstelemetry-Disconnected")) {
             compassMatrix = m_renderer->matrixForElement("gcstelemetry-Disconnected");
             startX = compassMatrix.mapRect(m_renderer->boundsOnElement("gcstelemetry-Disconnected")).x();
             startY = compassMatrix.mapRect(m_renderer->boundsOnElement("gcstelemetry-Disconnected")).y();
             gcsTelemetryArrow = new CachedSvgItem();
             gcsTelemetryArrow->setSharedRenderer(m_renderer);
             gcsTelemetryArrow->setElementId("gcstelemetry-Disconnected");
             l_scene->addItem(gcsTelemetryArrow);
             matrix.reset();
             matrix.translate(startX,startY);
             gcsTelemetryArrow->setTransform(matrix,false);
         } else {
             gcsTelemetryArrow = NULL;
         }

         if (m_renderer->elementExists("linkrate")) {
             compassMatrix = m_renderer->matrixForElement("linkrate");
             startX = compassMatrix.mapRect(m_renderer->boundsOnElement("linkrate")).x();
             startY = compassMatrix.mapRect(m_renderer->boundsOnElement("linkrate")).y();
             qreal linkRateHeight = compassMatrix.mapRect(m_renderer->boundsOnElement("linkrate")).height();
             gcsTelemetryStats = new QGraphicsTextItem();
             gcsTelemetryStats->setDefaultTextColor(QColor("White"));
             gcsTelemetryStats->setFont(QFont("Arial",(int) linkRateHeight));
             if (hqFonts)  gcsTelemetryStats->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
             l_scene->addItem(gcsTelemetryStats);
             matrix.reset();
             matrix.translate(startX,startY-linkRateHeight/2);
             gcsTelemetryStats->setTransform(matrix,false);
         } else {
             gcsTelemetryStats = NULL;
         }


         ////////////////
         // GCS Battery Indicator
         ////////////////
         /* (to be used the day I add a green/yellow/red indicator)
         compassMatrix = m_renderer->matrixForElement("gcstelemetry-Disconnected");
         startX = compassMatrix.mapRect(m_renderer->boundsOnElement("gcstelemetry-Disconnected")).x();
         startY = compassMatrix.mapRect(m_renderer->boundsOnElement("gcstelemetry-Disconnected")).y();
         gcsTelemetryArrow = new CachedSvgItem();
         gcsTelemetryArrow->setSharedRenderer(m_renderer);
         gcsTelemetryArrow->setElementId("gcstelemetry-Disconnected");
         l_scene->addItem(gcsTelemetryArrow);
         matrix.reset();
         matrix.translate(startX,startY);
         gcsTelemetryArrow->setTransform(matrix,false);
         */

         if (m_renderer->elementExists("battery-txt")) {
             compassMatrix = m_renderer->matrixForElement("battery-txt");
             startX = compassMatrix.mapRect(m_renderer->boundsOnElement("battery-txt")).x();
             startY = compassMatrix.mapRect(m_renderer->boundsOnElement("battery-txt")).y();
             qreal batStatHeight = compassMatrix.mapRect(m_renderer->boundsOnElement("battery-txt")).height();
             gcsBatteryStats = new QGraphicsTextItem("Battery");
             gcsBatteryStats->setDefaultTextColor(QColor("White"));
             gcsBatteryStats->setFont(QFont("Arial",(int) batStatHeight));
             if (hqFonts) gcsBatteryStats->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
             l_scene->addItem(gcsBatteryStats);
             matrix.reset();
             matrix.translate(startX,startY-batStatHeight/2);
             gcsBatteryStats->setTransform(matrix,false);
         } else {
             gcsBatteryStats = NULL;
         }

         ////////////////
         // GCS GPS Indicator
         ////////////////
         /* (to be used the day I add a green/yellow/red indicator)
         compassMatrix = m_renderer->matrixForElement("gcstelemetry-Disconnected");
         startX = compassMatrix.mapRect(m_renderer->boundsOnElement("gcstelemetry-Disconnected")).x();
         startY = compassMatrix.mapRect(m_renderer->boundsOnElement("gcstelemetry-Disconnected")).y();
         gcsTelemetryArrow = new CachedSvgItem();
         gcsTelemetryArrow->setSharedRenderer(m_renderer);
         gcsTelemetryArrow->setElementId("gcstelemetry-Disconnected");
         l_scene->addItem(gcsTelemetryArrow);
         matrix.reset();
         matrix.translate(startX,startY);
         gcsTelemetryArrow->setTransform(matrix,false);
         */

         if (m_renderer->elementExists("gps-txt")) {
             compassMatrix = m_renderer->matrixForElement("gps-txt");
             startX = compassMatrix.mapRect(m_renderer->boundsOnElement("gps-txt")).x();
             startY = compassMatrix.mapRect(m_renderer->boundsOnElement("gps-txt")).y();
             qreal gpsStatHeight = compassMatrix.mapRect(m_renderer->boundsOnElement("gps-txt")).height();
             gcsGPSStats = new QGraphicsTextItem("GPS");
             gcsGPSStats->setDefaultTextColor(QColor("White"));
             gcsGPSStats->setFont(QFont("Arial",(int) gpsStatHeight));
             if (hqFonts) gcsGPSStats->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
             l_scene->addItem(gcsGPSStats);
             matrix.reset();
             matrix.translate(startX,startY-gpsStatHeight/2);
             gcsGPSStats->setTransform(matrix,false);
         } else {
             gcsGPSStats = NULL;
         }

        l_scene->setSceneRect(m_background->boundingRect());

        /////////////////
        // Item placement
        /////////////////

        // Now Initialize the center for all transforms of the relevant elements to the
        // center of the background:

        // 1) Move the center of the needle to the center of the background.
        rectB = m_background->boundingRect();
        rectN = m_world->boundingRect();
        m_world->setPos(rectB.width()/2-rectN.width()/2,rectB.height()/2-rectN.height()/2);
        // 2) Put the transform origin point of the needle at its center.
        m_world->setTransformOriginPoint(rectN.width()/2,rectN.height()/2);

        rectN = m_rollscale->boundingRect();
        m_rollscale->setPos(rectB.width()/2-rectN.width()/2,rectB.height()/2-rectN.height()/2);
        m_rollscale->setTransformOriginPoint(rectN.width()/2,rectN.height()/2);

        // Also to the same init for the compass:
        rectB = m_compass->boundingRect();
        rectN = m_compassband->boundingRect();
        m_compassband->setPos(rectB.width()/2-rectN.width()/2,rectB.height()/2-rectN.height()/2);
        m_compassband->setTransformOriginPoint(rectN.width()/2,rectN.height()/2);

        // Last: we just loaded the dial file which is by default valid for a "zero" value
        // of the needles, so we have to reset the needles too upon dial file loading, otherwise
        // we would end up with an offset when we change a dial file and the needle value
        // is not zero at that time.
        rollValue = 0;
        pitchValue = 0;
        headingValue = 0;
        groundspeedValue = 0;
        altitudeValue = 0;
        pfdError = false;
        if (!dialTimer.isActive())
            dialTimer.start(); // Rearm the dial Timer which might be stopped.
   }
   else
   { qDebug()<<"Error on PFD artwork file.";
       m_renderer->load(QString(":/pfd/images/pfd-default.svg"));
       l_scene->clear(); // This also deletes all items contained in the scene.
       m_background = new CachedSvgItem();
       m_background->setSharedRenderer(m_renderer);
       l_scene->addItem(m_background);
       pfdError = true;
   }
}

void PFDGadgetWidget::paint()
{
   // update();
}

void PFDGadgetWidget::paintEvent(QPaintEvent *event)
{
    // Skip painting until the dial file is loaded
    if (! m_renderer->isValid()) {
        qDebug() << "Dial file not loaded, not rendering";
        return;
    }
   QGraphicsView::paintEvent(event);
}

// This event enables the dial to be dynamically resized
// whenever the gadget is resized, taking advantage of the vector
// nature of SVG dials.
void PFDGadgetWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    fitInView(m_background, Qt::KeepAspectRatio );
}


/*!
  \brief Update method for the vertical scales
  */
void PFDGadgetWidget::moveVerticalScales() {

}

void PFDGadgetWidget::moveSky() {
    int dialCount = 2; // Gets decreased by one for each element
                       // which has finished moving
//    qDebug() << "MoveSky";
    /// TODO: optimize!!!
    if (pfdError) {
        //skyDialTimer.stop();
        return;
    }

    // In some instances, it can happen that the rollValue & target are
    // invalid inside the UAVObjects, and become a "nan" value, which freezes
    // the PFD and the whole GCS: for this reason, we check this here.
    // The strange check below works, it is a workaround because "isnan(double)"
    // is not supported on every compiler.
    if (rollTarget != rollTarget || pitchTarget != pitchTarget)
        return;
    //////
    // Roll
    //////
    if (rollValue != rollTarget) {
        double rollDiff;
        if ((abs((rollValue-rollTarget)*10) > 5) && beSmooth ) {
            rollDiff =(rollTarget - rollValue)/2;
        } else {
            rollDiff = rollTarget - rollValue;
            dialCount--;
        }
        m_world->setRotation(m_world->rotation()+rollDiff);
        m_rollscale->setRotation(m_rollscale->rotation()+rollDiff);
        rollValue += rollDiff;
    } else {
        dialCount--;
    }

    //////
    // Pitch
    //////
    if (pitchValue != pitchTarget) {
        double pitchDiff;
        if ((abs((pitchValue-pitchTarget)*10) > 5) && beSmooth ) {
  //      if (0) {
            pitchDiff = (pitchTarget - pitchValue)/2;
        } else {
            pitchDiff = pitchTarget - pitchValue;
            dialCount--;
        }
        QPointF opd = QPointF(0,pitchDiff);
        m_world->setTransform(QTransform::fromTranslate(opd.x(),opd.y()), true);
        QPointF oop = m_world->transformOriginPoint();
        m_world->setTransformOriginPoint((oop.x()-opd.x()),(oop.y()-opd.y()));
        pitchValue += pitchDiff;
    } else {
        dialCount--;
    }

    if (dialCount)
        scene()->update(sceneRect());
  //  if (!dialCount)
  //      skyDialTimer.stop();

}


// Take an input value and move the elements accordingly.
// Movement is smooth, starts fast and slows down when
// approaching the target.
//
void PFDGadgetWidget::moveNeedles()
{
    int dialCount = 3; // Gets decreased by one for each element
                       // which has finished moving

//    qDebug() << "MoveNeedles";
    /// TODO: optimize!!!

    if (pfdError) {
    	dialTimer.stop();
    	return;
    }

    //////
    // Heading
    //
    // If you want a smooth transition between two angles, It is usually solved that by substracting
    // one from another, and if the result is >180 or <-180 I substract (respectively add) 360 degrees
    // to it. That way you always get the "shorter difference" to turn in.
    //////
    if (headingValue != headingTarget) {
        double headingDiff;
        if ((abs((headingValue-headingTarget)*10) > 5) && beSmooth ) {
            headingDiff = (headingTarget - headingValue)/5;
        } else {
            headingDiff = headingTarget-headingValue;
            dialCount--;
        }
        double threshold = 180*compassBandWidth/540;
        // Note: rendering can jump oh so very slightly when crossing the 180 degree
        // boundary, should not impact actual useability of the display.
        if ((headingValue+headingDiff)>=threshold) {
            // We went over 180°: activate a -360 degree offset
            headingDiff -= 2*threshold;
            headingTarget -= 2*threshold;
        } else if ((headingValue+headingDiff)<-threshold) {
            // We went under -180°: remove the -360 degree offset
            headingDiff += 2*threshold;
            headingTarget += 2*threshold;
        }
        QPointF opd = QPointF(headingDiff,0);
        m_compassband->setTransform(QTransform::fromTranslate(opd.x(),opd.y()), true);
        headingValue += headingDiff;
    }  else {
        dialCount--;
    }

    //////
    // Airspeed
    //////
    if (airspeedValue != airspeedTarget) {
        if ((abs(airspeedValue-airspeedTarget) > speedScaleHeight/100) && beSmooth ) {
            airspeedValue += (airspeedTarget-airspeedValue)/2;
        } else {
            airspeedValue = airspeedTarget;
            dialCount--;
        }

        float airspeed_kph=airspeedValue*3.6;
        float airspeed_kph_scale = airspeed_kph*speedScaleHeight/30;

        qreal x = m_speedscale->transform().dx();
        //opd = QPointF(x,fmod(airspeed_kph,speedScaleHeight/6));
        // fmod does rounding errors!! the formula below works better:
        QPointF opd = QPointF(x, airspeed_kph_scale-floor(airspeed_kph_scale/speedScaleHeight*6)*speedScaleHeight/6);
        m_speedscale->setTransform(QTransform::fromTranslate(opd.x(),opd.y()), false);

        double speedText = airspeed_kph;
        QString s = QString().sprintf("%.0f",speedText);
        m_speedtext->setPlainText(s);

        // Now update the text elements inside the scale:
        // (Qt documentation states that the list has the same order
        // as the item add order in the QGraphicsItemGroup)
        QList <QGraphicsItem *> textList = m_speedscale->childItems();
        qreal val = 5*floor(airspeed_kph_scale/speedScaleHeight*6)+20;
        foreach( QGraphicsItem * item, textList) {
            if (QGraphicsTextItem *text = qgraphicsitem_cast<QGraphicsTextItem *>(item)) {
                QString s = (val<0) ? QString() : QString().sprintf("%.0f",val);
                if (text->toPlainText() == s)
                    break; // This should happen at element one if is has not changed, indicating
                           // that it's not necessary to do the rest of the list
                text->setPlainText(s);
                val -= 5;
            }
        }
    } else {
        dialCount--;
    }

    //////
    // Groundspeed
    //////
    if (groundspeedValue != groundspeedTarget) {
        groundspeedValue = groundspeedTarget;
		qreal x = m_speedscale->transform().dx();
		//opd = QPointF(x,fmod(groundspeedValue,speedScaleHeight/6));
		// fmod does rounding errors!! the formula below works better:
		QPointF opd = QPointF(x,groundspeedValue-floor(groundspeedValue/speedScaleHeight*6)*speedScaleHeight/6);
		m_speedscale->setTransform(QTransform::fromTranslate(opd.x(),opd.y()), false);

		double speedText = groundspeedValue/speedScaleHeight*30;
		QString s = QString().sprintf("%.0f",speedText);
		m_speedtext->setPlainText(s);

		// Now update the text elements inside the scale:
		// (Qt documentation states that the list has the same order
		// as the item add order in the QGraphicsItemGroup)
		QList <QGraphicsItem *> textList = m_speedscale->childItems();
		qreal val = 5*floor(groundspeedValue/speedScaleHeight*6)+20;
		foreach( QGraphicsItem * item, textList) {
			if (QGraphicsTextItem *text = qgraphicsitem_cast<QGraphicsTextItem *>(item)) {
				QString s = (val<0) ? QString() : QString().sprintf("%.0f",val);
				if (text->toPlainText() == s)
					break; // This should happen at element one if is has not changed, indicating
						   // that it's not necessary to do the rest of the list
				text->setPlainText(s);
				val -= 5;
			}
		}
    }

    //////
    // Altitude
    //////
    if (altitudeValue != altitudeTarget) {
        if ((abs(altitudeValue-altitudeTarget) > altitudeScaleHeight/100) && beSmooth ) {
            altitudeValue += (altitudeTarget-altitudeValue)/2;
        } else {
            altitudeValue = altitudeTarget;
            dialCount--;
        }

        // The altitude scale represents 30 meters
        float altitudeValue_scale = floor(altitudeValue*10)/10*altitudeScaleHeight/30;

        qreal x = m_altitudescale->transform().dx();
        //opd = QPointF(x,fmod(altitudeValue,altitudeScaleHeight/6));
        // fmod does rounding errors!! the formula below works better:
        QPointF opd = QPointF(x,altitudeValue_scale-floor(altitudeValue_scale/altitudeScaleHeight*6)*altitudeScaleHeight/6);
        m_altitudescale->setTransform(QTransform::fromTranslate(opd.x(),opd.y()), false);

        double altitudeText = altitudeValue;
        QString s = QString().sprintf("%.0f",altitudeText);
        m_altitudetext->setPlainText(s);

        // Now update the text elements inside the scale:
        // (Qt documentation states that the list has the same order
        // as the item add order in the QGraphicsItemGroup)
        QList <QGraphicsItem *> textList = m_altitudescale->childItems();
        qreal val = 5*floor(altitudeValue_scale/altitudeScaleHeight*6)+20;
        foreach( QGraphicsItem * item, textList) {
            if (QGraphicsTextItem *text = qgraphicsitem_cast<QGraphicsTextItem *>(item)) {
                QString s = (val<0) ? QString() : QString().sprintf("%.0f",val);
                if (text->toPlainText() == s)
                    break; // This should happen at element one if is has not changed, indicating
                           // that it's not necessary to do the rest of the list
                text->setPlainText(s);
                val -= 5;
            }
        }
    } else {
        dialCount--;
    }

   if (!dialCount)
       dialTimer.stop();
   else
       scene()->update(sceneRect());
}

/**
  @}
  @}
  */
