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

ScopeGadgetFactory::ScopeGadgetFactory(QObject *parent) :
        IUAVGadgetFactory(QString("ScopeGadget"),
                          tr("Scope Gadget"),
                          parent)
{
}

ScopeGadgetFactory::~ScopeGadgetFactory()
{

}

Core::IUAVGadget* ScopeGadgetFactory::createUAVGadget(QList<IUAVGadgetConfiguration*> *configurations, QWidget *parent) {
        ScopeGadgetWidget* gadgetWidget = new ScopeGadgetWidget(parent);
        return new ScopeGadget(QString("ScopeGadget"), configurations, gadgetWidget);
}
