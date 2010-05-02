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
#include <QtGui/QFileDialog>
#include <QDebug>

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

    connect(&dialTimer, SIGNAL(timeout()), this, SLOT(rotateNeedles()));
    dialTimer.start(30);

    // Test code for timer to rotate the needle
    testSpeed=0;
    connect(&m_testTimer, SIGNAL(timeout()), this, SLOT(testRotate()));
    m_testTimer.start(4000);
}

AirspeedGadgetWidget::~AirspeedGadgetWidget()
{
   // Do nothing
}

void AirspeedGadgetWidget::setDialFile(QString dfn, QString bg, QString fg, QString n1, QString n2)
{
   if (QFile::exists(dfn))
   {
      m_renderer->load(dfn);
      if(m_renderer->isValid())
      {

         m_background->setSharedRenderer(m_renderer);
         m_background->setElementId(bg);

         m_foreground->setSharedRenderer(m_renderer);
         m_foreground->setElementId(fg);

         m_needle1->setSharedRenderer(m_renderer);
         m_needle1->setElementId(n1);

         m_needle2->setSharedRenderer(m_renderer);
         m_needle2->setElementId(n2);


         std::cout<<"Dial file loaded"<<std::endl;
         QGraphicsScene *l_scene = scene();
         l_scene->setSceneRect(m_background->boundingRect());

     }
   }
   else
   { std::cout<<"no file: "<<std::endl; }
}

void AirspeedGadgetWidget::paint()
{

    QGraphicsScene *l_scene = scene();
    l_scene->clear();

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
        std::cout<<"Dial file not loaded"<<std::endl;
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

void AirspeedGadgetWidget::setNeedle1(double value) {
    needle1Target = value;
}

void AirspeedGadgetWidget::setNeedle2(double value) {
    needle2Target = value;
}


// Take an input value and rotate the dial accordingly
void AirspeedGadgetWidget::rotateNeedles()
{
    if (needle2Value != needle2Target) {
        (needle2Value > needle2Target) ? needle2Value-- : needle2Value++;
        m_needle2->resetTransform();
        QRectF rect = m_needle2->boundingRect();
        m_needle2->translate(rect.width()/2,rect.height()/2);
        m_needle2->rotate(360*needle2Value/n2MaxValue);
        m_needle2->translate(-rect.width()/2,-rect.height()/2);
    }

    if (needle1Value != needle1Target) {
      (needle1Value > needle1Target) ? needle1Value-- : needle1Value++;
       m_needle1->resetTransform();
       QRectF rect = m_needle1->boundingRect();
       m_needle1->translate(rect.width()/2,rect.height()/2);
       m_needle1->rotate(360*needle1Value/n1MaxValue);
       m_needle1->translate(-rect.width()/2,-rect.height()/2);
    }

   update();
}


// Test function for timer to rotate needles
void AirspeedGadgetWidget::testRotate()
{
    int testVal1 = rand() % (int)n1MaxValue;
    int testVal2 = rand() % (int)n2MaxValue;
   setNeedle1((double)testVal1);
   setNeedle2((double)testVal2);

}
