/**
 ******************************************************************************
 *
 * @file       airspeedgadgetwidget.cpp
 * @author     David "Buzz" Carlson Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   dialplugin
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

AirspeedGadgetWidget::AirspeedGadgetWidget(QWidget *parent) : QGraphicsView(parent)
{
    // TODO: create a proper "needle" object instead of hardcoding all this
    // which is ugly (but easy).

    setMinimumSize(64,64);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setScene(new QGraphicsScene(this));
    setRenderHints(QPainter::Antialiasing);

    m_renderer = new QSvgRenderer();

    obj1 = NULL;
    obj2 = NULL;
    obj3 = NULL;

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
void AirspeedGadgetWidget::connectNeedles(QString object1, QString nfield1,
                                          QString object2, QString nfield2,
                                          QString object3, QString nfield3) {
    if (obj1 != NULL)
        disconnect(obj1,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(updateNeedle1(UAVObject*)));
    if (obj2 != NULL)
        disconnect(obj2,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(updateNeedle2(UAVObject*)));
    if (obj3 != NULL)
        disconnect(obj3,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(updateNeedle3(UAVObject*)));

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

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

    // And do the same for the third needle.
    if (!(object3.isEmpty() || nfield3.isEmpty())) {
        obj3 = dynamic_cast<UAVDataObject*>( objManager->getObject(object3) );
        if (obj3 != NULL ) {
            std::cout << "Connected Object 3 (" << object3.toStdString() << ")." << std::endl;
            connect(obj3, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updateNeedle3(UAVObject*)));
            field3 = nfield3;
        } else {
            std::cout << "Error: Object is unknown (" << object3.toStdString() << ")." << std::endl;
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

/*!
  \brief Called by the UAVObject which got updated
  */
void AirspeedGadgetWidget::updateNeedle3(UAVObject *object3) {
    UAVObjectField* field = object3->getField(field3);
    if (field) {
        setNeedle3(field->getDouble());
    } else {
        std::cout << "Wrong field, maybe an issue with object disconnection ?" << std::endl;
    }
}

/*
  Initializes the dial file, and does all the one-time calculations for
  display later.
  */
void AirspeedGadgetWidget::setDialFile(QString dfn, QString bg, QString fg, QString n1, QString n2,
                                       QString n3, QString n1Move, QString n2Move, QString n3Move)
{
   if (QFile::exists(dfn))
   {
      m_renderer->load(dfn);
      if(m_renderer->isValid())
      {
         fgenabled = false;
         n2enabled = false;
         n3enabled = false;
         QGraphicsScene *l_scene = scene();
         l_scene->clear(); // Deletes all items contained in the scene as well.
         m_background = new QGraphicsSvgItem();
         // All other items will be clipped to the shape of the background
         m_background->setFlags(QGraphicsItem::ItemClipsChildrenToShape|
                                QGraphicsItem::ItemClipsToShape);
         m_foreground = new QGraphicsSvgItem();
         m_needle1 = new QGraphicsSvgItem();
         m_needle2 = new QGraphicsSvgItem();
         m_needle3 = new QGraphicsSvgItem();
         m_needle1->setParentItem(m_background);
         m_needle2->setParentItem(m_background);
         m_needle3->setParentItem(m_background);
         m_foreground->setParentItem(m_background);

         // We assume the dial contains at least the background
         // and needle1
         m_background->setSharedRenderer(m_renderer);
         m_background->setElementId(bg);
         l_scene->addItem(m_background);

         m_needle1->setSharedRenderer(m_renderer);
         m_needle1->setElementId(n1);
         l_scene->addItem(m_needle1);

        // The dial gadget allows Needle1 and Needle2 to be
        // the same element, for combined movement. Needle3
        // is always independent.
        if (n1 == n2) {
            m_needle2 = m_needle1;
            n2enabled = true;
        } else {
         if (m_renderer->elementExists(n2)) {
             m_needle2->setSharedRenderer(m_renderer);
             m_needle2->setElementId(n2);
             l_scene->addItem(m_needle2);
             n2enabled = true;
            }
         }

        if (m_renderer->elementExists(n3)) {
            m_needle3->setSharedRenderer(m_renderer);
            m_needle3->setElementId(n3);
            l_scene->addItem(m_needle3);
            n3enabled = true;
           }

        if (m_renderer->elementExists(fg)) {
            m_foreground->setSharedRenderer(m_renderer);
            m_foreground->setElementId(fg);
            l_scene->addItem(m_foreground);
            fgenabled = true;
        }

        rotateN1 = false;
        horizN1 =  false;
        vertN1 = false;
        rotateN2 = false;
        horizN2 = false;
        vertN2 = false;
        rotateN3 = false;
        horizN3 = false;
        vertN3 = false;


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

        if (n3Move.contains("Rotate")) {
            rotateN3 = true;
        } else if (n3Move.contains("Horizontal")) {
            horizN3 = true;
        } else if (n3Move.contains("Vertical")) {
            vertN3 = true;
        }

        l_scene->setSceneRect(m_background->boundingRect());

        // Now Initialize the center for all transforms of the dial needles to the
        // center of the background:
        // - Move the center of the needle to the center of the background.
        QRectF rectB = m_background->boundingRect();
        QRectF rectN = m_needle1->boundingRect();
        m_needle1->setPos(rectB.width()/2-rectN.width()/2,rectB.height()/2-rectN.height()/2);
        // - Put the transform origin point of the needle at its center.
        m_needle1->setTransformOriginPoint(rectN.width()/2,rectN.height()/2);
        if ((n1 != n2) && n2enabled) {
            // Only do it for needle1 if it is not the same as n2
            rectN = m_needle2->boundingRect();
            m_needle2->setPos(rectB.width()/2-rectN.width()/2,rectB.height()/2-rectN.height()/2);
            m_needle2->setTransformOriginPoint(rectN.width()/2,rectN.height()/2);
        }
        if (n3enabled) {
            rectN = m_needle3->boundingRect();
            m_needle3->setPos(rectB.width()/2-rectN.width()/2,rectB.height()/2-rectN.height()/2);
            m_needle3->setTransformOriginPoint(rectN.width()/2,rectN.height()/2);
        }

        // Last: we just loaded the dial file which is by default valid for a "zero" value
        // of the needles, so we have to reset the needles too upon dial file loading, otherwise
        // we would end up with an offset when we change a dial file and the needle value
        // is not zero at that time.
        needle1Value = 0;
        needle2Value = 0;
        needle3Value = 0;
        if (!dialTimer.isActive())
            dialTimer.start();
     }
   }
   else
   { std::cout<<"no file: "<<std::endl; }
}

void AirspeedGadgetWidget::paint()
{
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
    if (!dialTimer.isActive())
        dialTimer.start();
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
    if (!dialTimer.isActive())
        dialTimer.start();
}

void AirspeedGadgetWidget::setNeedle3(double value) {
    if (rotateN3) {
        needle3Target = 360*value*n3Factor/(n3MaxValue-n3MinValue);
    }
    if (horizN3) {
        needle3Target = value*n3Factor/(n3MaxValue-n3MinValue);
    }
    if (vertN3) {
        needle3Target = value*n3Factor/(n3MaxValue-n3MinValue);
    }
    if (!dialTimer.isActive())
        dialTimer.start();
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
    int dialRun = 3;
    if (n2enabled) {
        if (abs((needle2Value-needle2Target)*10) > 5) {
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
               // same object, for combined movement such as attitude indicator).
               QPointF oop = m_needle2->transformOriginPoint();
               m_needle2->setTransformOriginPoint(oop.x()-opd.x(),oop.y()-opd.y());
            }
            needle2Value += needle2Diff;
        } else {
            needle2Value = needle2Target;
            dialRun--;
        }
    } else {
        dialRun--;
    }

    // We assume that needle1 always exists!
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
    } else { // value is close enough to target, value to exact target
             // to cancel out any potential drift
        needle1Value = needle1Target;
        dialRun--;
    }

   if (n3enabled) {
       if ((abs((needle3Value-needle3Target)*10) > 5)) {
           double needle3Diff;
           needle3Diff = (needle3Target - needle3Value)/5;
           if (rotateN3) {
                m_needle3->setRotation(m_needle3->rotation()+needle3Diff);
           } else {
                QPointF opd = QPointF(0,0);
                if (horizN3) {
                    opd = QPointF(needle3Diff,0);
                }
           if (vertN3) {
               opd = QPointF(0,needle3Diff);
           }
           m_needle3->setTransform(QTransform::fromTranslate(opd.x(),opd.y()), true);
           QPointF oop = m_needle3->transformOriginPoint();
           m_needle3->setTransformOriginPoint((oop.x()-opd.x()),(oop.y()-opd.y()));
           }
       needle3Value += needle3Diff;
       } else {
           needle3Value = needle3Target;
           dialRun--;
       }
    } else {
        dialRun--;
    }
    update();
    // Now check: if dialRun is now zero, we should
    // just stop the timer since all needles have finished moving
    if (!dialRun) dialTimer.stop();
}
