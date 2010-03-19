/*
 * mapgadget.cpp
 *
 *  Created on: Mar 11, 2010
 *      Author: peter
 */
#include "mapgadget.h"
#include "mapgadgetwidget.h"
#include <QtGui/QToolBar>

MapGadget::MapGadget(MapGadgetWidget *widget) :
        IUAVGadget(widget),
        m_widget(widget),
        m_toolbar(new QToolBar())
{
}

MapGadget::~MapGadget()
{

}
