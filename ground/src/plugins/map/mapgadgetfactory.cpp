/*
 * mapgadgetfactory.cpp
 *
 *  Created on: Mar 11, 2010
 *      Author: peter
 */
#include "mapgadgetfactory.h"
#include "mapgadgetwidget.h"
#include "mapgadget.h"
#include <coreplugin/iuavgadget.h>

MapGadgetFactory::MapGadgetFactory(QObject *parent) : IUAVGadgetFactory(parent)
{
}

MapGadgetFactory::~MapGadgetFactory()
{

}

Core::IUAVGadget* MapGadgetFactory::createUAVGadget(QWidget *parent) {
	MapGadgetWidget* gadgetWidget = new MapGadgetWidget(parent);
	return new MapGadget(gadgetWidget);
}

QString MapGadgetFactory::name() {
	return QString("MapGadget");
}
