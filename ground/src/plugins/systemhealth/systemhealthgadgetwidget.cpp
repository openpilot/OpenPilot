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
#include "extensionsystem/pluginmanager.h"
#include "uavobjects/uavobjectmanager.h"
#include "uavobjects/systemalarms.h"

#include <iostream>
#include <QtGui/QFileDialog>
#include <QDebug>

/*
 * Initialize the widget
 */
SystemHealthGadgetWidget::SystemHealthGadgetWidget(QWidget *parent) : QGraphicsView(parent)
{
    setMinimumSize(128,128);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    setScene(new QGraphicsScene(this));
 
    m_renderer = new QSvgRenderer();
    background = new QGraphicsSvgItem();
    foreground = new QGraphicsSvgItem();

    paint();

    // Now connect the widget to the SystemAlarms UAVObject
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    UAVObjectManager *objManager = pm->getObject<UAVObjectManager>();

    SystemAlarms* obj = dynamic_cast<SystemAlarms*>(objManager->getObject(QString("SystemAlarms")));
    connect(obj, SIGNAL(objectUpdated(UAVObject*)), this, SLOT(updateAlarms(UAVObject*)));

    // Test code for timer to move the index
    testValue=0;
    connect(&m_testTimer, SIGNAL(timeout()), this, SLOT(testRotate()));
    m_testTimer.start(1000);
}

void SystemHealthGadgetWidget::updateAlarms(UAVObject* systemAlarm)
{
    QString alarm = systemAlarm->getName();
    std::cout << "System Alarm: " << alarm.toStdString() << std::endl;
    std::cout << "Object Name: " << systemAlarm->objectName().toStdString() << std::endl;
    std::cout << "Object Elements: " << std::endl;
    foreach (UAVObjectField *field, systemAlarm->getFields()) {
        std::cout << "Field Name: " << field->getName().toStdString() << std::endl;
        for (uint i = 0; i < field->getNumElements(); ++i) {
            std::cout << "  Option: " << field->getElementNames()[i].toStdString() << std::endl;
            QString element = field->getElementNames()[i];
            if (m_renderer->elementExists(element)) {
                QMatrix blockMatrix = m_renderer->matrixForElement(element);
                qreal startX = blockMatrix.mapRect(m_renderer->boundsOnElement(element)).x();
                qreal startY = blockMatrix.mapRect(m_renderer->boundsOnElement(element)).y();
                std::cout << "     StartX: " << startX << std::endl;
                std::cout << "     StartY: " << startY << std::endl;
            } else {
                std::cout << "     Element not found in SVG" << std::endl;
            }
            std::cout << "  Option value: " << field->getValue(i).toString().toStdString() << std::endl;
        }


    }

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
