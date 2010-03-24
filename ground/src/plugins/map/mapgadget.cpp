/*
 * mapgadget.cpp
 *
 *  Created on: Mar 11, 2010
 *      Author: peter
 */
#include "mapgadget.h"
#include "mapgadgetwidget.h"

MapGadget::MapGadget(QString classId, QList<IUAVGadgetConfiguration*> *configurations, MapGadgetWidget *widget) :
        IUAVGadget(classId, configurations, widget),
        m_widget(widget)
{
}

MapGadget::~MapGadget()
{

}
