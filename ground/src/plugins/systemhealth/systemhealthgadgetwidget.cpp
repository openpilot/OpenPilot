/**
 ******************************************************************************
 *
 * @file       systemhealthgadgetwidget.cpp
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @brief      System Health widget, does the actual drawing
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   systemhealth
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

#include "systemhealthgadgetwidget.h"

#include <iostream>
#include <QtGui/QFileDialog>
#include <QDebug>

SystemHealthGadgetWidget::SystemHealthGadgetWidget(QWidget *parent) : QGraphicsView(parent)
{
    setMinimumSize(128,128);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setScene(new QGraphicsScene(this));
 
    m_renderer = new QSvgRenderer();
    background = new QGraphicsSvgItem();
    foreground = new QGraphicsSvgItem();

    paint();


    // Test code for timer to move the index
    testValue=0;
    connect(&m_testTimer, SIGNAL(timeout()), this, SLOT(testRotate()));
    m_testTimer.start(1000);
}

SystemHealthGadgetWidget::~SystemHealthGadgetWidget()
{
   // Do nothing
}


void SystemHealthGadgetWidget::setSystemFile(QString dfn)
{
   if (QFile::exists(dfn))
   {
      m_renderer->load(dfn);
      if(m_renderer->isValid())
      {
         fgenabled = false;

         background->setSharedRenderer(m_renderer);
         background->setElementId("background");

         if (m_renderer->elementExists("foreground")) {
            foreground->setSharedRenderer(m_renderer);
            foreground->setElementId("foreground");
            fgenabled = true;
        }
         std::cout<<"Dial file loaded"<<std::endl;
         QGraphicsScene *l_scene = scene();

         l_scene->setSceneRect(background->boundingRect());
         fitInView(background, Qt::KeepAspectRatio );
     }
   }
   else
   { std::cout<<"no file: "<<std::endl; }
}

void SystemHealthGadgetWidget::paint()
{

    QGraphicsScene *l_scene = scene();
    l_scene->clear();

    l_scene->addItem(background);
    l_scene->addItem(foreground);

    update();
}

void SystemHealthGadgetWidget::paintEvent(QPaintEvent *event)
{
    // Skip painting until the dial file is loaded
    if (! m_renderer->isValid()) {
        std::cout<<"System file not loaded, not rendering"<<std::endl;
        return;
    }
   QGraphicsView::paintEvent(event);
}

// This event enables the dial to be dynamically resized
// whenever the gadget is resized, taking advantage of the vector
// nature of SVG dials.
void SystemHealthGadgetWidget::resizeEvent(QResizeEvent *event)
{
    fitInView(background, Qt::KeepAspectRatio );
}


// Test function for timer to rotate needles
void SystemHealthGadgetWidget::test()
{
    testValue=0;

}
