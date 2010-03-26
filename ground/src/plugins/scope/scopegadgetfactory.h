/*
 * scopegadgetfactory.h
 *
 *  Created on: Mar 6, 2010
 *      Author: peter
 */

#ifndef SCOPEGADGETFACTORY_H_
#define SCOPEGADGETFACTORY_H_

#include <coreplugin/iuavgadgetfactory.h>

namespace Core {
class IUAVGadget;
class IUAVGadgetFactory;
}

using namespace Core;

class ScopeGadgetFactory : public IUAVGadgetFactory
{
    Q_OBJECT
public:
    ScopeGadgetFactory(QObject *parent = 0);
    ~ScopeGadgetFactory();

    IUAVGadget *createGadget(QWidget *parent);
};

#endif // SCOPEGADGETFACTORY_H_
