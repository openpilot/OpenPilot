/**
 ******************************************************************************
 *
 * @file       scopegadgetfactory.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ScopePlugin Scope Gadget Plugin
 * @{
 * @brief The scope Gadget, graphically plots the states of UAVObjects
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

#ifndef SCOPEGADGETFACTORY_H_
#define SCOPEGADGETFACTORY_H_

#include "scope_global.h"
#include <coreplugin/iuavgadgetfactory.h>

namespace Core
{
class IUAVGadget;
class IUAVGadgetFactory;
}

using namespace Core;

class SCOPE_EXPORT ScopeGadgetFactory : public IUAVGadgetFactory
{
    Q_OBJECT
public:
    ScopeGadgetFactory(QObject *parent = 0);
    ~ScopeGadgetFactory();

    Core::IUAVGadget *createGadget(QWidget *parent);
    IUAVGadgetConfiguration *createConfiguration(QSettings* qSettings);
    IOptionsPage *createOptionsPage(IUAVGadgetConfiguration *config);

public slots:
    void stopPlotting();
    void startPlotting();

signals:
    void onStopPlotting();
    void onStartPlotting();

};

#endif // SCOPEGADGETFACTORY_H_
