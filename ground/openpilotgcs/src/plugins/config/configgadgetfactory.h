/**
 ******************************************************************************
 *
 * @file       configgadgetfactory.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief The Configuration Gadget used to update settings in the firmware
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
#ifndef CONFIGGADGETFACTORY_H
#define CONFIGGADGETFACTORY_H

#include <coreplugin/iuavgadgetfactory.h>
#include "configgadgetwidget.h"
#include "config_global.h"


namespace Core {
class IUAVGadget;
class IUAVGadgetFactory;
}

using namespace Core;

class CONFIG_EXPORT ConfigGadgetFactory:  public Core::IUAVGadgetFactory
{
   Q_OBJECT
public:

    ConfigGadgetFactory(QObject *parent = 0);
    ~ConfigGadgetFactory();

    IUAVGadget *createGadget(QWidget *parent);
    IUAVGadgetConfiguration *createConfiguration(QSettings* qSettings);
    IOptionsPage *createOptionsPage(IUAVGadgetConfiguration *config);

public slots:
    void startInputWizard();

private:
    ConfigGadgetWidget* gadgetWidget;
};

#endif // CONFIGGADGETFACTORY_H
