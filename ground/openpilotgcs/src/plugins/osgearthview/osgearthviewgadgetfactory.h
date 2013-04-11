/**
 ******************************************************************************

 * @file       osgearthviewgadgetfactory.h
 * @author     The OpenPilot Team Copyright (C) 2012.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup OsgEarthview Plugin
 * @{
 * @brief The Primary Flight Display Gadget 
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef OSGEARTHVIEWGADGETFACTORY_H_
#define OSGEARTHVIEWGADGETFACTORY_H_

#include <coreplugin/iuavgadgetfactory.h>

namespace Core {
class IUAVGadget;
class IUAVGadgetFactory;
}

using namespace Core;

class OsgEarthviewGadgetFactory : public IUAVGadgetFactory
{
    Q_OBJECT
public:
    OsgEarthviewGadgetFactory(QObject *parent = 0);
    ~OsgEarthviewGadgetFactory();

    Core::IUAVGadget *createGadget(QWidget *parent);
    IUAVGadgetConfiguration *createConfiguration(QSettings* qSettings);
    IOptionsPage *createOptionsPage(IUAVGadgetConfiguration *config);
};

#endif // OSGEARTHVIEWGADGETFACTORY_H_
