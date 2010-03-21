/*
 * emptygadgetfactory.h
 *
 *  Created on: Mar 6, 2010
 *      Author: peter
 */

#ifndef EMPTYGADGETFACTORY_H_
#define EMPTYGADGETFACTORY_H_

#include <coreplugin/iuavgadgetfactory.h>

using namespace Core;
class IUAVGadget;
class IUAVGadgetFactory;

class EmptyGadgetFactory : public Core::IUAVGadgetFactory
{
    Q_OBJECT
public:
    EmptyGadgetFactory(QObject *parent = 0);
    ~EmptyGadgetFactory();

    Core::IUAVGadget *createUAVGadget(QWidget *parent);
};

#endif // EMPTYGADGETFACTORY_H_
