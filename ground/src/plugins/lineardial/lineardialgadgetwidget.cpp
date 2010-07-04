/**
 ******************************************************************************
 *
 * @file       lineardialgadgetwidget.cpp
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   lineardialplugin
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
    setRenderHints(QPainter::Antialiasing);
    m_renderer = new QSvgRenderer();
    verticalDial = false;

    paint();

    obj1 = NULL;
    fieldName = NULL;
    fieldValue = NULL;
    indexTarget = 0;
    indexValue = 0;
    places = 0;
    factor = 1;

    // This timer mechanism makes the index rotate smoothly
    connect(&dialTimer, SIGNAL(timeout()), this, SLOT(moveIndex()));
    dialTimer.start(30);

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
            if (fieldName)
                fieldName->setPlainText(nfield1);
            updateIndex(obj1);

        } else {
            std::cout << "Error: Object is unknown (" << object1.toStdString() << ") this should not happen." << std::endl;
        }
    }
}

/*!
  \brief Called by the UAVObject which got updated

  Updates the numeric value and/or the icon if the dial wants this.
  */
void LineardialGadgetWidget::updateIndex(UAVObject *object1) {
    // Double check that the field exists:
    UAVObjectField* field = object1->getField(field1);
    if (field) {
        QString s;
        if (field->isNumeric()) {
            double v = field->getDouble()*factor;
            setIndex(v);
            s.sprintf("%.*f",places,v);
        }
        if (field->isText()) {
            s = field->getValue().toString();
            if (fieldSymbol) {
                // If we defined a symbol, we will look for a matching
                // SVG element to display:
                if (m_renderer->elementExists("symbol-" + s)) {
                    fieldSymbol->setElementId("symbol-" + s);
                } else {
                    fieldSymbol->setElementId("symbol");
                }
            }
        }
        if (fieldValue)
            fieldValue->setPlainText(s);

        if (index && !dialTimer.isActive())
            dialTimer.start();
    } else {
        std::cout << "Wrong field, maybe an issue with object disconnection ?" << std::endl;
    }
}

/*!
  \brief Setup dial using its master SVG template.

  Should only be called after the min/max ranges have been set.

  */
void LineardialGadgetWidget::setDialFile(QString dfn)
{
   if (QFile::exists(dfn))
   {
      QGraphicsScene *l_scene = scene();
      m_renderer->load(dfn);
      if(m_renderer->isValid())
      {
          l_scene->clear(); // Beware: clear also deletes all objects
                            // which are currently in the scene
          background = new QGraphicsSvgItem();
          background->setSharedRenderer(m_renderer);
          background->setElementId("background");
          background->setFlags(QGraphicsItem::ItemClipsChildrenToShape|
                                 QGraphicsItem::ItemClipsToShape);
          l_scene->addItem(background);

          // The red/yellow/green zones are optional, we just
          // test on the presence of "red"
          if (m_renderer->elementExists("red")) {
              // Order is important: red, then yellow then green
              // overlayed on top of each other
              red = new QGraphicsSvgItem();
              red->setSharedRenderer(m_renderer);
              red->setElementId("red");
              red->setParentItem(background);
              yellow = new QGraphicsSvgItem();
              yellow->setSharedRenderer(m_renderer);
              yellow->setElementId("yellow");
              yellow->setParentItem(background);
              green = new QGraphicsSvgItem();
              green->setSharedRenderer(m_renderer);
              green->setElementId("green");
              green->setParentItem(background);
              // In order to properly render the Green/Yellow/Red graphs, we need to find out
              // the starting location of the bargraph rendering area:
              QMatrix textMatrix = m_renderer->matrixForElement("bargraph");
              qreal bgX = textMatrix.mapRect(m_renderer->boundsOnElement("bargraph")).x();
              qreal bgY = textMatrix.mapRect(m_renderer->boundsOnElement("bargraph")).y();
              bargraphSize = textMatrix.mapRect(m_renderer->boundsOnElement("bargraph")).width();
              // Detect if the bargraph is vertical or horizontal.
              qreal bargraphHeight = textMatrix.mapRect(m_renderer->boundsOnElement("bargraph")).height();
              if (bargraphHeight > bargraphSize) {
                    verticalDial = true;
                    bargraphSize = bargraphHeight;
              } else {
                    verticalDial = false;
              }
              // Now adjust the red/yellow/green zones:
              double range = maxValue-minValue;

              green->resetTransform();
              double greenScale = (greenMax-greenMin)/range;
              double greenStart = verticalDial ? (maxValue-greenMax)/range*green->boundingRect().height() :
                                  (greenMin-minValue)/range*green->boundingRect().width();
              QTransform matrix;
              matrix.reset();
              if (verticalDial) {
                  matrix.scale(1,greenScale);
                  matrix.translate(bgX,(greenStart+bgY)/greenScale);
              } else {
                  matrix.scale(greenScale,1);
                  matrix.translate((greenStart+bgX)/greenScale,bgY);
              }
              green->setTransform(matrix,false);

              yellow->resetTransform();
              double yellowScale = (yellowMax-yellowMin)/range;
              double yellowStart = verticalDial ? (maxValue-yellowMax)/range*yellow->boundingRect().height() :
                                   (yellowMin-minValue)/range*yellow->boundingRect().width();
              matrix.reset();
              if (verticalDial) {
                  matrix.scale(1,yellowScale);
                  matrix.translate(bgX,(yellowStart+bgY)/yellowScale);
              } else {
                  matrix.scale(yellowScale,1);
                  matrix.translate((yellowStart+bgX)/yellowScale,bgY);
              }
              yellow->setTransform(matrix,false);

              red->resetTransform();
              double redScale = (redMax-redMin)/range;
              double redStart = verticalDial ? (maxValue-redMax)/range*red->boundingRect().height() :
                                (redMin-minValue)/range*red->boundingRect().width();
              matrix.reset();
              if (verticalDial) {
                  matrix.scale(1,redScale);
                  matrix.translate(bgX,(redStart+bgY)/redScale);
              } else {
                  matrix.scale(redScale,1);
                  matrix.translate((redStart+bgX)/redScale,bgY);
              }
              red->setTransform(matrix,false);

          } else {
            red = NULL;
            yellow = NULL;
            green = NULL;
          }

          // Check whether the dial wants to display a moving index:
          if (m_renderer->elementExists("needle")) {
              QMatrix textMatrix = m_renderer->matrixForElement("needle");
              QRectF nRect = textMatrix.mapRect(m_renderer->boundsOnElement("needle"));
              startX = nRect.x();
              startY = nRect.y();
              //indexWidth = nRect.width();
              //indexHeight = nRect.height();
              QTransform matrix;
              matrix.translate(startX,startY);
              index = new QGraphicsSvgItem();
              index->setSharedRenderer(m_renderer);
              index->setElementId("needle");
              index->setTransform(matrix,false);
              index->setParentItem(background);
          } else {
              index = NULL;
          }

          // Check whether the dial wants display its field name:
          if (m_renderer->elementExists("field")) {
              QMatrix textMatrix = m_renderer->matrixForElement("field");
              qreal startX = textMatrix.mapRect(m_renderer->boundsOnElement("field")).x();
              qreal startY = textMatrix.mapRect(m_renderer->boundsOnElement("field")).y();
              QTransform matrix;
              matrix.translate(startX,startY);
              fieldName = new QGraphicsTextItem("0.00");
              fieldName->setDefaultTextColor(QColor("White"));
              fieldName->setTransform(matrix,false);
              fieldName->setParentItem(background);
          } else {
              fieldName = NULL;
          }

          // Check whether the dial wants display the numeric value:
          if (m_renderer->elementExists("value")) {
              QMatrix textMatrix = m_renderer->matrixForElement("value");
              qreal startX = textMatrix.mapRect(m_renderer->boundsOnElement("value")).x();
              qreal startY = textMatrix.mapRect(m_renderer->boundsOnElement("value")).y();
              QTransform matrix;
              matrix.translate(startX,startY);
              fieldValue = new QGraphicsTextItem("00");
              fieldValue->setDefaultTextColor(QColor("White"));
              fieldValue->setTransform(matrix,false);
              fieldValue->setParentItem(background);
          } else {
              fieldValue = NULL;
          }

          // Check whether the dial wants to display the value as a
          // symbol (only works for text values):
          if (m_renderer->elementExists("symbol")) {
              QMatrix textMatrix = m_renderer->matrixForElement("symbol");
              qreal startX = textMatrix.mapRect(m_renderer->boundsOnElement("symbol")).x();
              qreal startY = textMatrix.mapRect(m_renderer->boundsOnElement("symbol")).y();
              QTransform matrix;
              matrix.translate(startX,startY);
              fieldSymbol = new QGraphicsSvgItem();
              fieldSymbol->setElementId("symbol");
              fieldSymbol->setSharedRenderer(m_renderer);
              fieldSymbol->setTransform(matrix,false);
              fieldSymbol->setParentItem(background);
          } else {
              fieldSymbol = NULL;
          }

         if (m_renderer->elementExists("foreground")) {
            foreground = new QGraphicsSvgItem();
            foreground->setSharedRenderer(m_renderer);
            foreground->setElementId("foreground");
            //l_scene->addItem(foreground);
            foreground->setParentItem(background);
            fgenabled = true;
        } else {
            fgenabled = false;
        }

         l_scene->setSceneRect(background->boundingRect());

         // Reset the current index value:
         indexValue = 0;
         if (!dialTimer.isActive() && index)
             dialTimer.start();
     }
   }
   else
   { std::cout<<"no file: "<<std::endl; }
}


void LineardialGadgetWidget::setDialFont(QString fontProps)
{
    QFont font = QFont("Arial",12);
    font.fromString(fontProps);
    if (fieldName)
        fieldName->setFont(font);
    if (fieldValue)
        fieldValue->setFont(font);
}


void LineardialGadgetWidget::paint()
{
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
    if (!index) { // Safeguard
        dialTimer.stop();
        return;
    }
    if ((abs((indexValue-indexTarget)*10) > 3)) {
        indexValue += (indexTarget - indexValue)/5;
    } else {
        indexValue = indexTarget;
        dialTimer.stop();
    }
    QTransform matrix;
    index->resetTransform();
    qreal trans = indexValue*bargraphSize/100;
    if (verticalDial) {
        //matrix.translate(startX-indexWidth/2,trans+startY-indexHeight/2);
        matrix.translate(startX,trans+startY);
    } else {
        //matrix.translate(trans+startX-indexWidth/2,startY-indexHeight/2);
        matrix.translate(trans+startX,startY);
    }
    index->setTransform(matrix,false);
    
    update();
}
