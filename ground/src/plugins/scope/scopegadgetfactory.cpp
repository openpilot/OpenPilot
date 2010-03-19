/*
 * scopegadgetfactory.cpp
 *
 *  Created on: Mar 11, 2010
 *      Author: peter
 */
#include "scopegadgetfactory.h"
#include "scopegadgetwidget.h"
#include "scopegadget.h"
#include <coreplugin/iuavgadget.h>

ScopeGadgetFactory::ScopeGadgetFactory(QObject *parent) : IUAVGadgetFactory(parent)
{
}

ScopeGadgetFactory::~ScopeGadgetFactory()
{

}

Core::IUAVGadget* ScopeGadgetFactory::createUAVGadget(QWidget *parent) {
        ScopeGadgetWidget* gadgetWidget = new ScopeGadgetWidget(parent);
        return new ScopeGadget(gadgetWidget);
}

QString ScopeGadgetFactory::name() {
        return QString("ScopeGadget");
}
