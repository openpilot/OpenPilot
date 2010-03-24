/*
 * mapgadgetfactory.cpp
 *
 *  Created on: Mar 11, 2010
 *      Author: peter
 */
#include "mapgadgetfactory.h"
#include "mapgadgetwidget.h"
#include "mapgadget.h"
#include "mapgadgetconfiguration.h"
#include "mapgadgetoptionspage.h"
#include <coreplugin/iuavgadget.h>

MapGadgetFactory::MapGadgetFactory(QObject *parent) :
        IUAVGadgetFactory(QString("MapGadget"),
                          tr("Map Gadget"),
                          parent)
{
}

MapGadgetFactory::~MapGadgetFactory()
{

}

Core::IUAVGadget* MapGadgetFactory::createUAVGadget(QList<IUAVGadgetConfiguration*> *configurations, QWidget *parent) {
	MapGadgetWidget* gadgetWidget = new MapGadgetWidget(parent);
        return new MapGadget(QString("MapGadget"), configurations, gadgetWidget);
}

IUAVGadgetConfiguration *MapGadgetFactory::createUAVGadgetConfiguration(bool locked,
                                                      const QString configName,
                                                      const QByteArray &state)
{
    return new MapGadgetConfiguration(locked, QString("MapGadget"), configName, state);
}

UAVGadgetOptionsPage *MapGadgetFactory::createUAVGadgetOptionsPage(IUAVGadgetConfiguration *config)
{
    return new MapGadgetOptionsPage(config);
}

