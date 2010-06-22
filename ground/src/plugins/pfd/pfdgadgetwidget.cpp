/**
 ******************************************************************************
 *
 * @file       pfdgadgetwidget.cpp
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

#include "pfdgadgetwidget.h"
#include <iostream>
#include <QDebug>

PFDGadgetWidget::PFDGadgetWidget(QWidget *parent) : QGraphicsView(parent)
{
    // TODO: create a proper "needle" object instead of hardcoding all this
    // which is ugly (but easy).

    setMinimumSize(64,64);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setScene(new QGraphicsScene(this));
    setRenderHints(QPainter::Antialiasing);

    m_renderer = new QSvgRenderer();

    attitudeObj = NULL;
/*
    obj2 = NULL;
    obj3 = NULL;
*/
    // This timer mechanism makes needles rotate smoothly
    connect(&dialTimer, SIGNAL(timeout()), this, SLOT(rotateNeedles()));
    dialTimer.start(20);

}

PFDGadgetWidget::~PFDGadgetWidget()
{
   // Do nothing
}

/*!
  \brief Connects the widget to the relevant UAVObjects

  We want: AttitudeActual, FlightBattery, Location

  */
void PFDGadgetWidget::connectNeedles() {
    if (attitudeObj != NULL)
        disconnect(attitudeObj,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(updateAttitude(UAVObject*)));
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

   attitudeObj = dynamic_cast<UAVDataObject*>(objManager->getObject("AttitudeActual"));
   if (attitudeObj != NULL ) {
       connect(attitudeObj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updateAttitude(UAVObject*)));
   } else {
        std::cout << "Error: Object is unknown (AttitudeActual)." << std::endl;
   }

}

/*!
  \brief Called by the UAVObject which got updated
  */
void PFDGadgetWidget::updateAttitude(UAVObject *object1) {
    // Double check that the field exists:
    QString roll = QString("Roll");
    QString pitch = QString("Pitch");
    UAVObjectField* field = object1->getField(roll);
    UAVObjectField* field2 = object1->getField(pitch);
    if (field && field2) {
        // These factors assume some things about the PFD SVG, namely:
        // - Roll value in degrees
        // - Pitch lines are 300px high for a +20/-20 range, which means
        //   7.5 pixels per pitch degree.
        rollTarget = field->getDouble()*(-1);
        pitchTarget = field2->getDouble()*7.5;
        if (!dialTimer.isActive())
            dialTimer.start(); // Rearm the dial Timer which might be stopped.
    } else {
        std::cout << "UpdateAttitude: Wrong field, maybe an issue with object disconnection ?" << std::endl;
    }
}

/*!
  \brief Called by the UAVObject which got updated
  */
void PFDGadgetWidget::updateHeading(UAVObject *object2) {
}

/*!
  \brief Called by the UAVObject which got updated
  */
void PFDGadgetWidget::updateAirspeed(UAVObject *object3) {
}

/*!
  \brief Called by the UAVObject which got updated
  */
void PFDGadgetWidget::updateAltitude(UAVObject *object3) {
}


/*!
  \brief Called by the UAVObject which got updated
  */
void PFDGadgetWidget::updateBattery(UAVObject *object3) {
}


/*
  Initializes the dial file, and does all the one-time calculations for
  display later.
  */
void PFDGadgetWidget::setDialFile(QString dfn)
{
   if (QFile::exists(dfn))
   {
      m_renderer->load(dfn);
      if(m_renderer->isValid())
      {
          /* The PFD element IDs are fixed, not like with the analog dial.
     - Background: background
     - Foreground: foreground (contains all fixed elements, including plane)
     - earth/sky : world
     - red pointer: needle3
     - compass frame: compass (part of the foreground)
     - compass band : compass-band
     - Home point: homewaypoint
     - Next point: nextwaypoint
     - Home point bearing: homewaypoint-bearing
     - Next point bearing: nextwaypoint-bearing
          */
         QGraphicsScene *l_scene = scene();
         l_scene->clear(); // Deletes all items contained in the scene as well.
         m_background = new QGraphicsSvgItem();
         // All other items will be clipped to the shape of the background
         m_background->setFlags(QGraphicsItem::ItemClipsChildrenToShape|
                                QGraphicsItem::ItemClipsToShape);
         m_background->setSharedRenderer(m_renderer);
         m_background->setElementId("background");
         l_scene->addItem(m_background);

         m_world = new QGraphicsSvgItem();
         m_world->setParentItem(m_background);
         m_world->setSharedRenderer(m_renderer);
         m_world->setElementId("world");
         l_scene->addItem(m_world);

         // red pointer: redpointer
         m_redpointer = new QGraphicsSvgItem();
         // Compass band:
         // Get the location of the Compass:
         QMatrix compassMatrix = m_renderer->matrixForElement("compass");
         // Then once we have the initial location, we can put it
         // into a QGraphicsSvgItem which we will display at the same
         // place:
         m_compassband = new QGraphicsSvgItem();
         // Home point:
         m_homewaypoint = new QGraphicsSvgItem();
         // Next point:
         m_nextwaypoint = new QGraphicsSvgItem();
         // Home point bearing:
         m_homepointbearing = new QGraphicsSvgItem();
         // Next point bearing:
         m_nextpointbearing = new QGraphicsSvgItem();

         m_foreground = new QGraphicsSvgItem();
         m_foreground->setParentItem(m_background);
         m_foreground->setSharedRenderer(m_renderer);
         m_foreground->setElementId("foreground");
         l_scene->addItem(m_foreground);

        l_scene->setSceneRect(m_background->boundingRect());

        // Now Initialize the center for all transforms of the dial needles to the
        // center of the background:
        // - Move the center of the needle to the center of the background.
        QRectF rectB = m_background->boundingRect();
        QRectF rectN = m_world->boundingRect();
        m_world->setPos(rectB.width()/2-rectN.width()/2,rectB.height()/2-rectN.height()/2);
        // - Put the transform origin point of the needle at its center.
        m_world->setTransformOriginPoint(rectN.width()/2,rectN.height()/2);


        // Last: we just loaded the dial file which is by default valid for a "zero" value
        // of the needles, so we have to reset the needles too upon dial file loading, otherwise
        // we would end up with an offset when we change a dial file and the needle value
        // is not zero at that time.
        rollValue = 0;
        pitchValue = 0;
        if (!dialTimer.isActive())
            dialTimer.start(); // Rearm the dial Timer which might be stopped.
     }
   }
   else
   { std::cout<<"no file: "<<std::endl; }
}

void PFDGadgetWidget::paint()
{
    update();
}

void PFDGadgetWidget::paintEvent(QPaintEvent *event)
{
    // Skip painting until the dial file is loaded
    if (! m_renderer->isValid()) {
        std::cout<<"Dial file not loaded, not rendering"<<std::endl;
        return;
    }
   QGraphicsView::paintEvent(event);
}

// This event enables the dial to be dynamically resized
// whenever the gadget is resized, taking advantage of the vector
// nature of SVG dials.
void PFDGadgetWidget::resizeEvent(QResizeEvent *event)
{
    fitInView(m_background, Qt::KeepAspectRatio );
}


// Take an input value and rotate the dial accordingly
// Rotation is smooth, starts fast and slows down when
// approaching the target.
// We aim for a 0.5 degree precision.
//
// Note: this code is valid even if needle1 and needle2 point
// to the same element.
void PFDGadgetWidget::rotateNeedles()
{
    int dialCount = 2;
    if ((abs((rollValue-rollTarget)*10) > 5)) {
        double rollDiff;
        rollDiff =(rollTarget - rollValue)/5;
        m_world->setRotation(m_world->rotation()+rollDiff);
        rollValue += rollDiff;
    } else {
        rollValue = rollTarget;
        dialCount--;
    }

    if ((abs((pitchValue-pitchTarget)*10) > 5)) {
        double pitchDiff;
        pitchDiff = (pitchTarget - pitchValue)/5;
        QPointF opd = QPointF(0,pitchDiff);
        m_world->setTransform(QTransform::fromTranslate(opd.x(),opd.y()), true);
        QPointF oop = m_world->transformOriginPoint();
        m_world->setTransformOriginPoint((oop.x()-opd.x()),(oop.y()-opd.y()));
        pitchValue += pitchDiff;
    } else {
        pitchValue = pitchTarget;
        dialCount--;
    }

   update();
   if (!dialCount)
       dialTimer.stop();
}
