/**
 ******************************************************************************
 *
 * @file       lineardialgadgetwidget.cpp
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   lineardial
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

#include "lineardialgadgetwidget.h"

#include <iostream>
#include <QtGui/QFileDialog>
#include <QDebug>

LineardialGadgetWidget::LineardialGadgetWidget(QWidget *parent) : QGraphicsView(parent)
{
    setMinimumSize(32,32);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setScene(new QGraphicsScene(this));
 
    m_renderer = new QSvgRenderer();
    background = new QGraphicsSvgItem();
    foreground = new QGraphicsSvgItem();
    green = new QGraphicsSvgItem();
    yellow = new QGraphicsSvgItem();
    red = new QGraphicsSvgItem();
    index = new QGraphicsSvgItem();
    fieldName = new QGraphicsTextItem("Field");
    fieldName->setDefaultTextColor(QColor("White"));
    fieldValue = new QGraphicsTextItem("0.00");
    fieldValue->setDefaultTextColor(QColor("White"));
    verticalDial = false;

    paint();

    obj1 = NULL;
    indexTarget = 0;
    indexValue = 0;

    // This timer mechanism makes the index rotate smoothly
    connect(&dialTimer, SIGNAL(timeout()), this, SLOT(moveIndex()));
    dialTimer.start(30);

#if 0
    // Test code for timer to move the index
    testSpeed=0;
    connect(&m_testTimer, SIGNAL(timeout()), this, SLOT(testRotate()));
    m_testTimer.start(1000);
#endif
}

LineardialGadgetWidget::~LineardialGadgetWidget()
{
   // Do nothing
}

/*!
  \brief Connects the widget to the relevant UAVObjects
  */
void LineardialGadgetWidget::connectInput(QString object1, QString nfield1) {

    if (obj1 != NULL)
        disconnect(obj1,SIGNAL(objectUpdated(UAVObject*)),this,SLOT(updateIndex(UAVObject*)));
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    std::cout << "Lineadial Connect needles - " << object1.toStdString() << "-"<< nfield1.toStdString() << std::endl;

    // Check validity of arguments first, reject empty args and unknown fields.
    if (!(object1.isEmpty() || nfield1.isEmpty())) {
        obj1 = dynamic_cast<UAVDataObject*>( objManager->getObject(object1) );
        if (obj1 != NULL ) {
            connect(obj1, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updateIndex(UAVObject*)));
            field1 = nfield1;
            fieldName->setPlainText(nfield1);
        } else {
            std::cout << "Error: Object is unknown (" << object1.toStdString() << ") this should not happen." << std::endl;
        }
    }
}

/*!
  \brief Called by the UAVObject which got updated
  */
void LineardialGadgetWidget::updateIndex(UAVObject *object1) {
    // Double check that the field exists:
    UAVObjectField* field = object1->getField(field1);
    if (field) {
        double v = field->getDouble();
        setIndex(v);
        QString s;
        s.sprintf("%.2f",v);
        fieldValue->setPlainText(s);
    } else {
        std::cout << "Wrong field, maybe an issue with object disconnection ?" << std::endl;
    }
}

void LineardialGadgetWidget::setDialFile(QString dfn)
{
   if (QFile::exists(dfn))
   {
      m_renderer->load(dfn);
      if(m_renderer->isValid())
      {
         fgenabled = false;

         background->setSharedRenderer(m_renderer);
         background->setElementId("background");
         index->setSharedRenderer(m_renderer);
         index->setElementId("needle");
         green->setSharedRenderer(m_renderer);
         green->setElementId("green");
         yellow->setSharedRenderer(m_renderer);
         yellow->setElementId("yellow");
         red->setSharedRenderer(m_renderer);
         red->setElementId("red");

         if (m_renderer->elementExists("foreground")) {
            foreground->setSharedRenderer(m_renderer);
            foreground->setElementId("foreground");
            fgenabled = true;
        }
         //std::cout<<"Dial file loaded"<<std::endl;
         QGraphicsScene *l_scene = scene();

         QMatrix textMatrix = m_renderer->matrixForElement("field");
         startX = textMatrix.mapRect(m_renderer->boundsOnElement("field")).x();
         startY = textMatrix.mapRect(m_renderer->boundsOnElement("field")).y();
         QTransform matrix;
         matrix.translate(startX,startY);
         fieldName->setTransform(matrix,false);

         textMatrix = m_renderer->matrixForElement("value");
         startX = textMatrix.mapRect(m_renderer->boundsOnElement("value")).x();
         startY = textMatrix.mapRect(m_renderer->boundsOnElement("value")).y();
         matrix.reset();
         matrix.translate(startX,startY);
         fieldValue->setTransform(matrix,false);

         // In order to properly render the Green/Yellow/Red graphs, we need to find out
         // the starting location of the bargraph rendering area:
         textMatrix = m_renderer->matrixForElement("bargraph");
         startX = textMatrix.mapRect(m_renderer->boundsOnElement("bargraph")).x();
         startY = textMatrix.mapRect(m_renderer->boundsOnElement("bargraph")).y();
         //std::cout << "StartX: " << startX << std::endl;
         //std::cout << "StartY: " << startY << std::endl;
         bargraphSize = textMatrix.mapRect(m_renderer->boundsOnElement("bargraph")).width();
         // Detect if the bargraph is vertical or horizontal.
         qreal bargraphHeight = textMatrix.mapRect(m_renderer->boundsOnElement("bargraph")).height();
         if (bargraphHeight > bargraphSize) {
               verticalDial = true;
               bargraphSize = bargraphHeight;
           } else {
               verticalDial = false;
           }

           // Move the index to its base position:
         indexHeight = m_renderer->matrixForElement("needle").mapRect(m_renderer->boundsOnElement("needle")).height();
         indexWidth = m_renderer->matrixForElement("needle").mapRect(m_renderer->boundsOnElement("needle")).width();
         matrix.reset();
         matrix.translate(startX-indexWidth/2,startY-indexHeight/2);
         index->setTransform(matrix,false);

         // Now adjust the red/yellow/green zones:
         double range = maxValue-minValue;

         green->resetTransform();
         double greenScale = (greenMax-greenMin)/range;
         double greenStart = verticalDial ? (maxValue-greenMax)/range*green->boundingRect().height() :
                             (greenMin-minValue)/range*green->boundingRect().width();
         matrix.reset();
         if (verticalDial) {
             matrix.scale(1,greenScale);
             matrix.translate(startX,(greenStart+startY)/greenScale);
         } else {
             matrix.scale(greenScale,1);
             matrix.translate((greenStart+startX)/greenScale,startY);
         }
         green->setTransform(matrix,false);

         yellow->resetTransform();
         double yellowScale = (yellowMax-yellowMin)/range;
         double yellowStart = verticalDial ? (maxValue-yellowMax)/range*yellow->boundingRect().height() :
                              (yellowMin-minValue)/range*yellow->boundingRect().width();
         matrix.reset();
         if (verticalDial) {
             matrix.scale(1,yellowScale);
             matrix.translate(startX,(yellowStart+startY)/yellowScale);
         } else {
             matrix.scale(yellowScale,1);
             matrix.translate((yellowStart+startX)/yellowScale,startY);
         }
         yellow->setTransform(matrix,false);

         red->resetTransform();
         double redScale = (redMax-redMin)/range;
         double redStart = verticalDial ? (maxValue-redMax)/range*red->boundingRect().height() :
                           (redMin-minValue)/range*red->boundingRect().width();
         matrix.reset();
         if (verticalDial) {
             matrix.scale(1,redScale);
             matrix.translate(startX,(redStart+startY)/redScale);
         } else {
             matrix.scale(redScale,1);
             matrix.translate((redStart+startX)/redScale,startY);
         }
         red->setTransform(matrix,false);

         l_scene->setSceneRect(background->boundingRect());

     }
   }
   else
   { std::cout<<"no file: "<<std::endl; }
}

void LineardialGadgetWidget::paint()
{
    QGraphicsScene *l_scene = scene();
    l_scene->clear(); // Beware: clear also deletes all objects
                      // which are currently in the scene
    l_scene->addItem(background);
    // Order is important: red, then yellow then green
    // overlayed on top of each other
    l_scene->addItem(red);
    l_scene->addItem(yellow);
    l_scene->addItem(green);
    l_scene->addItem(index);
    l_scene->addItem(fieldName);
    l_scene->addItem(fieldValue);
    l_scene->addItem(foreground);
    update();
}

void LineardialGadgetWidget::paintEvent(QPaintEvent *event)
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
void LineardialGadgetWidget::resizeEvent(QResizeEvent *event)
{
    fitInView(background, Qt::KeepAspectRatio );
}

// Converts the value into an percentage:
// this enables smooth movement in moveIndex below
void LineardialGadgetWidget::setIndex(double value) {
    if (verticalDial) {
        indexTarget = 100*(maxValue-value)/(maxValue-minValue);
    } else {
        indexTarget = 100*(value-minValue)/(maxValue-minValue);
    }
}

// Take an input value and move the index accordingly
// Move is smooth, starts fast and slows down when
// approaching the target.
void LineardialGadgetWidget::moveIndex()
{
    if ((abs((indexValue-indexTarget)*10) > 3)) {
        indexValue += (indexTarget - indexValue)/5;
       index->resetTransform();
       qreal factor = indexValue*bargraphSize/100;
       QTransform matrix;
       if (verticalDial) {
           matrix.translate(startX-indexWidth/2,factor+startY-indexHeight/2);
       } else {
           matrix.translate(factor+startX-indexWidth/2,startY-indexHeight/2);
       }
       index->setTransform(matrix,false);
       update();
   }
}


// Test function for timer to rotate needles
void LineardialGadgetWidget::testRotate()
{
    testVal=0;
    if (testVal > maxValue) testVal=minValue;
   setIndex((double)testVal);
}
