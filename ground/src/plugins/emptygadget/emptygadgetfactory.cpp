/*
 * emptygadgetfactory.cpp
 *
 *  Created on: Mar 11, 2010
 *      Author: peter
 */
#include "emptygadgetfactory.h"
#include "emptygadgetwidget.h"
#include "emptygadget.h"
#include <coreplugin/iuavgadget.h>

EmptyGadgetFactory::EmptyGadgetFactory(QObject *parent) : IUAVGadgetFactory(parent)
{
}

EmptyGadgetFactory::~EmptyGadgetFactory()
{

}

Core::IUAVGadget* EmptyGadgetFactory::createUAVGadget(QWidget *parent) {
        EmptyGadgetWidget* gadgetWidget = new EmptyGadgetWidget(parent);
        return new EmptyGadget(gadgetWidget);
}

QString EmptyGadgetFactory::name() {
        return QString("EmptyGadget");
}
