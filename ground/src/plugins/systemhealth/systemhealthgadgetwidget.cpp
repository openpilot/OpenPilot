/**
 ******************************************************************************
 *
 * @file       systemhealthgadgetwidget.cpp
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup SystemHealthPlugin System Health Plugin
 * @{
 * @brief The System Health gadget plugin
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

}

void SystemHealthGadgetWidget::updateAlarms(UAVObject* systemAlarm)
{
    // This code does not know anything about alarms beforehand, and
    // I found to efficient way to locate items inside the scene by
    // name, so it's just as simple to reset the scene:
    // And add the one with the right name.
    QGraphicsScene *m_scene = scene();
    foreach ( QGraphicsItem* item ,background->childItems()){
        m_scene->removeItem(item);
        delete item; // removeItem does _not_ delete the item.
    }

    QString alarm = systemAlarm->getName();
    foreach (UAVObjectField *field, systemAlarm->getFields()) {
        for (uint i = 0; i < field->getNumElements(); ++i) {
            QString element = field->getElementNames()[i];
            QString value = field->getValue(i).toString();
            if (m_renderer->elementExists(element)) {
                QMatrix blockMatrix = m_renderer->matrixForElement(element);
                qreal startX = blockMatrix.mapRect(m_renderer->boundsOnElement(element)).x();
                qreal startY = blockMatrix.mapRect(m_renderer->boundsOnElement(element)).y();
                QString element2 = element + "-" + value;
                if (m_renderer->elementExists(element2)) {
                    QGraphicsSvgItem *ind = new QGraphicsSvgItem();
                    ind->setSharedRenderer(m_renderer);
                    ind->setElementId(element2);
                    ind->setParentItem(background);
                    QTransform matrix;
                    matrix.translate(startX,startY);
                    ind->setTransform(matrix,false);
                } else {
                    qDebug() << "Warning: element " << element2 << " not found in SVG.";
                }
            } else {
                qDebug() << "Warning: Element " << element << " not found in SVG.";
            }
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
            foreground->setZValue(99);
            fgenabled = true;
        }
         std::cout<<"Dial file loaded"<<std::endl;
         QGraphicsScene *l_scene = scene();
         l_scene->setSceneRect(background->boundingRect());
         fitInView(background, Qt::KeepAspectRatio );
     }
   }
   else
   { qDebug() <<"SystemHealthGadget: no file"; }
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
        qDebug() <<"SystemHealthGadget: System file not loaded, not rendering";
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
