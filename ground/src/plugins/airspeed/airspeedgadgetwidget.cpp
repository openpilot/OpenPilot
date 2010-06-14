/**
 ******************************************************************************
 *
 * @file       airspeedgadgetwidget.cpp
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

#include "airspeedgadgetwidget.h"
#include <iostream>
#include <QDebug>
#include "math.h"

AirspeedGadgetWidget::AirspeedGadgetWidget(QWidget *parent) : QGraphicsView(parent)
{
    setMinimumSize(64,64);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setScene(new QGraphicsScene(this));
 
    m_renderer = new QSvgRenderer();
    m_background = new QGraphicsSvgItem();
    m_foreground = new QGraphicsSvgItem();
    m_needle1 = new QGraphicsSvgItem();
    m_needle2 = new QGraphicsSvgItem();
    paint();

    needle1Target = 0;
    needle2Target = 0;
    needle1Value = 0;
    needle2Value = 0;

    obj1 = NULL;
    obj2 = NULL;

    rotateN1 = horizN1 = vertN1 = false;
    rotateN2 = horizN2 = vertN2 = false;

    // This timer mechanism makes needles rotate smoothly
    connect(&dialTimer, SIGNAL(timeout()), this, SLOT(rotateNeedles()));
    dialTimer.start(20);

}

AirspeedGadgetWidget::~AirspeedGadgetWidget()
{
   // Do nothing
}

/*!
  \brief Connects the widget to the relevant UAVObjects
  */
void AirspeedGadgetWidget::connectNeedles(QString object1, QString nfield1, QString object2, QString nfield2 ) {
    if (obj1 != NULL)
        disconnect(obj1,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(updateNeedle1(UAVObject*)));
    if (obj2 != NULL)
        disconnect(obj2,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(updateNeedle2(UAVObject*)));
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();
    std::cout << "Connect needles - " << object1.toStdString() << "-"<< nfield1.toStdString() << "  -   " <<
            object2.toStdString() << "-" << nfield2.toStdString() << std::endl;

    // Check validity of arguments first, reject empty args and unknown fields.
    if (!(object1.isEmpty() || nfield1.isEmpty())) {
        obj1 = dynamic_cast<UAVDataObject*>( objManager->getObject(object1) );
        if (obj1 != NULL ) {
            std::cout << "Connected Object 1 (" << object1.toStdString() << ")." << std::endl;
            connect(obj1, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updateNeedle1(UAVObject*)));
            field1 = nfield1;
        } else {
            std::cout << "Error: Object is unknown (" << object1.toStdString() << ")." << std::endl;
        }
    }

    // And do the same for the second needle.
    if (!(object2.isEmpty() || nfield2.isEmpty())) {
        obj2 = dynamic_cast<UAVDataObject*>( objManager->getObject(object2) );
        if (obj2 != NULL ) {
            std::cout << "Connected Object 2 (" << object2.toStdString() << ")." << std::endl;
            connect(obj2, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updateNeedle2(UAVObject*)));
            field2 = nfield2;
        } else {
            std::cout << "Error: Object is unknown (" << object2.toStdString() << ")." << std::endl;
        }
    }
}

/*!
  \brief Called by the UAVObject which got updated
  */
void AirspeedGadgetWidget::updateNeedle1(UAVObject *object1) {
    // Double check that the field exists:
    UAVObjectField* field = object1->getField(field1);
    if (field) {
        setNeedle1(field->getDouble());
    } else {
        std::cout << "Wrong field, maybe an issue with object disconnection ?" << std::endl;
    }
    std::cout << "Update Needle 1 with value of field " << field1.toStdString() << std::endl;
}

/*!
  \brief Called by the UAVObject which got updated
  */
void AirspeedGadgetWidget::updateNeedle2(UAVObject *object2) {
    UAVObjectField* field = object2->getField(field2);
    if (field) {
        setNeedle2(field->getDouble());
    } else {
        std::cout << "Wrong field, maybe an issue with object disconnection ?" << std::endl;
    }
}

void AirspeedGadgetWidget::setDialFile(QString dfn, QString bg, QString fg, QString n1, QString n2,
                                       QString n1Move, QString n2Move)
{
   if (QFile::exists(dfn))
   {
      m_renderer->load(dfn);
      if(m_renderer->isValid())
      {
         fgenabled = false;
         n2enabled = false;
         QGraphicsScene *l_scene = scene();

         m_background->setSharedRenderer(m_renderer);
         m_background->setElementId(bg);
         m_needle1->setSharedRenderer(m_renderer);
         m_needle1->setElementId(n1);

         if (m_renderer->elementExists(fg)) {
            m_foreground->setSharedRenderer(m_renderer);
            m_foreground->setElementId(fg);
            if (!l_scene->items().contains(m_foreground))
                l_scene->addItem(m_foreground);
            fgenabled = true;
        } else {
            if (l_scene->items().contains(m_foreground))
                l_scene->removeItem(m_foreground);
            fgenabled = false;
        }

        if (n1 == n2) {
            m_needle2 = m_needle1;
            n2enabled = true;
        } else {
         if (m_renderer->elementExists(n2)) {
             m_needle2->setSharedRenderer(m_renderer);
             m_needle2->setElementId(n2);
             if (!l_scene->items().contains(m_needle2))
                l_scene->addItem(m_needle2);
             n2enabled = true;
         } else {
             if (l_scene->items().contains(m_needle2))
                l_scene->removeItem(m_needle2);
             n2enabled = false;
         }
     }

         // Now setup the rotation/translation settings:
         // this is UGLY UGLY UGLY, sorry...
         if (n1Move.contains("Rotate")) {
             rotateN1 = true;
         } else if (n1Move.contains("Horizontal")) {
             horizN1 = true;
         } else if (n1Move.contains("Vertical")) {
             vertN1 = true;
         }

         if (n2Move.contains("Rotate")) {
             rotateN2 = true;
         } else if (n2Move.contains("Horizontal")) {
             horizN2 = true;
         } else if (n2Move.contains("Vertical")) {
             vertN2 = true;
         }

         // std::cout<<"Dial file loaded ("<< dfn.toStdString() << ")" << std::endl;
         l_scene->setSceneRect(m_background->boundingRect());

         // Initialize the center for all transforms of the dials to the
         // center of the background:
         QRectF rect1 = m_background->boundingRect();
         QPointF tr1 = m_background->mapToScene(rect1.width()/2,rect1.height()/2);
         QPointF tr = m_needle1->mapFromScene(tr1);
         m_needle1->setTransformOriginPoint(tr.x(),tr.y());
         tr = m_needle2->mapFromScene(tr1);
         m_needle2->setTransformOriginPoint(tr.x(),tr.y());
     }
   }
   else
   { std::cout<<"no file: "<<std::endl; }
}

void AirspeedGadgetWidget::paint()
{
    QGraphicsScene *l_scene = scene();
    l_scene->clear(); // Careful: clear() deletes the objects
    l_scene->addItem(m_background);
    l_scene->addItem(m_needle1);
    l_scene->addItem(m_needle2);
    l_scene->addItem(m_foreground);
    l_scene->setSceneRect(m_background->boundingRect());
    update();
}

void AirspeedGadgetWidget::paintEvent(QPaintEvent *event)
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
void AirspeedGadgetWidget::resizeEvent(QResizeEvent *event)
{
    fitInView(m_background, Qt::KeepAspectRatio );
}

// Converts the value into an angle:
// this enables smooth rotation in rotateNeedles below
void AirspeedGadgetWidget::setNeedle1(double value) {
    if (rotateN1) {
        needle1Target = 360*value*n1Factor/(n1MaxValue-n1MinValue);
    }
    if (horizN1) {
        needle1Target = value*n1Factor/(n1MaxValue-n1MinValue);
    }
    if (vertN1) {
        needle1Target = value*n1Factor/(n1MaxValue-n1MinValue);
    }
}

void AirspeedGadgetWidget::setNeedle2(double value) {
    if (rotateN2) {
        needle2Target = 360*value*n2Factor/(n2MaxValue-n2MinValue);
    }
    if (horizN2) {
        needle2Target = value*n2Factor/(n2MaxValue-n2MinValue);
    }
    if (vertN2) {
        needle2Target = value*n2Factor/(n2MaxValue-n2MinValue);
    }
}

// Take an input value and rotate the dial accordingly
// Rotation is smooth, starts fast and slows down when
// approaching the target.
// We aim for a 0.5 degree precision.
//
// Note: this code is valid even if needle1 and needle2 point
// to the same element.
void AirspeedGadgetWidget::rotateNeedles()
{
    if ((abs((needle2Value-needle2Target)*10) > 5) && n2enabled) {
        double needle2Diff;
        needle2Diff =(needle2Target - needle2Value)/5;
        if (rotateN2) {
            m_needle2->setRotation(m_needle2->rotation()+needle2Diff);
        } else {
            QPointF opd = QPointF(0,0);
           if (horizN2)  {
               opd = QPointF(needle2Diff,0);
           }
           if (vertN2) {
               opd = QPointF(0,needle2Diff);
           }
           m_needle2->setTransform(QTransform::fromTranslate(opd.x(),opd.y()), true);
           // Since we have moved the needle, we need to move
           // the transform origin point the opposite way
           // so that it keeps rotating from the same point.
           // (this is only useful if needle1 and needle2 are the
           // same object, for combined movement.
           QPointF oop = m_needle2->transformOriginPoint();
           m_needle2->setTransformOriginPoint(oop.x()-opd.x(),oop.y()-opd.y());
        }
        needle2Value += needle2Diff;
    }

    if ((abs((needle1Value-needle1Target)*10) > 5)) {
        double needle1Diff;
        needle1Diff = (needle1Target - needle1Value)/5;
        if (rotateN1) {
           m_needle1->setRotation(m_needle1->rotation()+needle1Diff);
       } else {
           QPointF opd = QPointF(0,0);
           if (horizN1) {
               opd = QPointF(needle1Diff,0);
           }
           if (vertN1) {
               opd = QPointF(0,needle1Diff);
           }
           m_needle1->setTransform(QTransform::fromTranslate(opd.x(),opd.y()), true);
           QPointF oop = m_needle1->transformOriginPoint();
           m_needle1->setTransformOriginPoint((oop.x()-opd.x()),(oop.y()-opd.y()));
       }
       needle1Value += needle1Diff;
    }

   update();
}

