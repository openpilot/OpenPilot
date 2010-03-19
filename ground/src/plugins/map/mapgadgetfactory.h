/*
 * mapgadgetfactory.h
 *
 *  Created on: Mar 6, 2010
 *      Author: peter
 */

#ifndef MAPGADGETFACTORY_H_
#define MAPGADGETFACTORY_H_

#include <coreplugin/iuavgadgetfactory.h>

using namespace Core;
class IUAVGadget;
class IUAVGadgetFactory;

class MapGadgetFactory : public Core::IUAVGadgetFactory
{
    Q_OBJECT
public:
    MapGadgetFactory(QObject *parent = 0);
    ~MapGadgetFactory();

    Core::IUAVGadget *createUAVGadget(QWidget *parent);
    QString name();
};

#endif // MAPGADGETFACTORY_H_
