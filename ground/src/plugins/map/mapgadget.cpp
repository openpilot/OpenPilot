/*
 * mapgadget.cpp
 *
 *  Created on: Mar 11, 2010
 *      Author: peter
 */
#include "mapgadget.h"
#include "mapgadgetwidget.h"
#include "mapgadgetconfiguration.h"

MapGadget::MapGadget(QString classId, QList<IUAVGadgetConfiguration*> *configurations, MapGadgetWidget *widget) :
        IUAVGadget(classId, configurations, widget),
        m_widget(widget)
{
}

MapGadget::~MapGadget()
{

}

void MapGadget::loadConfiguration(IUAVGadgetConfiguration* config)
{
    MapGadgetConfiguration *m = qobject_cast<MapGadgetConfiguration*>(config);
    m_widget->setZoom(m->zoom());
    m_widget->setPosition(QPointF(m->longitude(), m->latitude()));
    int index = m_toolbar->findText(config->name());
    m_toolbar->setCurrentIndex(index);
}

