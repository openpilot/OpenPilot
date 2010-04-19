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
    m_desired = new QGraphicsSvgItem();
    m_actual = new QGraphicsSvgItem();
    setDialFile(QFileDialog::getOpenFileName(qobject_cast<QWidget*>(this),
        tr("Airspeed Dial"), "../artwork/", tr("SVG File (*.svg)")) );
    //setDialFile(QString("/usr/src/openpilot/artwork/Dials/extracts/speed-complete.svg"));
    paint();

    // Test code for timer to rotate the needle
    testSpeed=0;
    connect(&m_testTimer, SIGNAL(timeout()), this, SLOT(testRotate()));
    m_testTimer.start(60);
}

AirspeedGadgetWidget::~AirspeedGadgetWidget()
{
   // Do nothing
}

void AirspeedGadgetWidget::setDialFile(QString dfn)
{
   if (QFile::exists(dfn))
   {
      m_renderer->load(dfn);
      if(m_renderer->isValid())
      {
         m_background->setSharedRenderer(m_renderer);
         m_background->setElementId(QString("background"));

         m_foreground->setSharedRenderer(m_renderer);
         m_foreground->setElementId(QString("foreground"));

         m_desired->setSharedRenderer(m_renderer);
         m_desired->setElementId(QString("desired"));

         m_actual->setSharedRenderer(m_renderer);
         m_actual->setElementId(QString("needle"));
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
    l_scene->addItem(m_foreground);
    l_scene->addItem(m_desired);
    l_scene->addItem(m_actual);

    l_scene->setSceneRect(m_background->boundingRect());
    update();
}

void AirspeedGadgetWidget::paintEvent(QPaintEvent *event)
{
   QGraphicsView::paintEvent(event);
}

// Take a unitless speed value and sets the actual needle accordingly
// scale fixed at 0-90 for now
void AirspeedGadgetWidget::setActual(int speed)
{
   m_actual->resetTransform();
   QRectF rect = m_actual->boundingRect();
   m_actual->translate(rect.width()/2,rect.height()/2);
   m_actual->rotate(speed*3);
   m_actual->translate(-rect.width()/2,-rect.height()/2);
   update();
}

// Take a unitless speed value and sets the desired needle accordingly
// scale fixed at 0-90 for now
void AirspeedGadgetWidget::setDesired(int speed)
{
   m_desired->resetTransform();
   QRectF rect = m_desired->boundingRect();
   m_desired->translate(rect.width()/2,rect.height()/2);
   m_desired->rotate(speed*3);
   m_desired->translate(-rect.width()/2,-rect.height()/2);
   update();
}

// Test function for timer to rotate needles
void AirspeedGadgetWidget::testRotate()
{
   if(testSpeed==90) testSpeed=0;
   setActual(testSpeed++);
}
