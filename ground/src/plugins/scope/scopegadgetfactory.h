/*
 * scopegadgetfactory.h
 *
 *  Created on: Mar 6, 2010
 *      Author: peter
 */

#ifndef SCOPEGADGETFACTORY_H_
#define SCOPEGADGETFACTORY_H_

#include <coreplugin/iuavgadgetfactory.h>

using namespace Core;
class IUAVGadget;
class IUAVGadgetFactory;

class ScopeGadgetFactory : public Core::IUAVGadgetFactory
{
    Q_OBJECT
public:
    ScopeGadgetFactory(QObject *parent = 0);
    ~ScopeGadgetFactory();

    Core::IUAVGadget *createUAVGadget(QWidget *parent);
};

#endif // SCOPEGADGETFACTORY_H_
