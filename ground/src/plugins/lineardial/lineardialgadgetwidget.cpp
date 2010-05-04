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
    setMinimumSize(64,64);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setScene(new QGraphicsScene(this));
 
    m_renderer = new QSvgRenderer();
    background = new QGraphicsSvgItem();
    foreground = new QGraphicsSvgItem();
    bargraph = new QGraphicsSvgItem();
    green = new QGraphicsSvgItem();
    yellow = new QGraphicsSvgItem();
    red = new QGraphicsSvgItem();
    index = new QGraphicsSvgItem();
    paint();

    indexTarget = 0;
    indexValue = 0;

    // This timer mechanism makes needles rotate smoothly
    connect(&dialTimer, SIGNAL(timeout()), this, SLOT(moveIndex()));
    dialTimer.start(30);

    // Test code for timer to rotate the needle
    testSpeed=0;
    connect(&m_testTimer, SIGNAL(timeout()), this, SLOT(testRotate()));
    m_testTimer.start(4000);
}

LineardialGadgetWidget::~LineardialGadgetWidget()
{
   // Do nothing
}

void LineardialGadgetWidget::connectInput(QString object1, QString field1) {

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
         // TODO: transform the green, yellow & red zones
         // according to their min/max.
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

         std::cout<<"Dial file loaded"<<std::endl;
         QGraphicsScene *l_scene = scene();

         maxValue = 100;
         minValue = 0;
         redMin = 0;
         redMax = 100;
         yellowMin = 30;
         yellowMax = 90;
         greenMin = 50;
         greenMax = 70;
         // Now adjust the red/yellow/green zones:
         double range = maxValue-minValue;
         double greenScale = (greenMax-greenMin)/range;
         double greenStart = greenMin/range*green->boundingRect().width();
         std::cout << "Width:" << green->boundingRect().width();
         QTransform matrix;
         matrix.translate(greenStart,0);
         matrix.scale(greenScale,1);
         green->setTransform(matrix,false);

         double yellowScale = (yellowMax-yellowMin)/range;
         double yellowStart = yellowMin/range*yellow->boundingRect().width();
         matrix.reset();
         matrix.translate(yellowStart,0);
         matrix.scale(yellowScale,1);
         yellow->setTransform(matrix,false);

         double redScale = (redMax-redMin)/range;
         double redStart = redMin/range*red->boundingRect().width();
         matrix.reset();
         matrix.translate(redStart,0);
         matrix.scale(redScale,1);
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
    l_scene->clear();

    l_scene->addItem(background);
    // Order is important: red, then yellow then green
    // overlayed on top of each other
    l_scene->addItem(red);
    l_scene->addItem(yellow);
    l_scene->addItem(green);
    l_scene->addItem(index);
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
    indexTarget = 100*(value-minValue)/maxValue;
}

// Take an input value and move the index accordingly
// Move is smooth, starts fast and slows down when
// approaching the target.
void LineardialGadgetWidget::moveIndex()
{
    if ((abs((indexValue-indexTarget)*10) > 3)) {
        indexValue += (indexTarget - indexValue)/10;
       index->resetTransform();
       double factor = index->boundingRect().width()/100;
       index->translate(indexValue*factor,0);
    }
   update();
}


// Test function for timer to rotate needles
void LineardialGadgetWidget::testRotate()
{
    int testVal = rand() % (int)maxValue;
   setIndex((double)testVal);

}
