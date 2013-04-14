/**
 ******************************************************************************
 *
 * @file       modelviewgadget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ModelViewPlugin ModelView Plugin
 * @{
 * @brief A gadget that displays a 3D representation of the UAV 
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

#ifndef MODELVIEWGADGET_H_
#define MODELVIEWGADGET_H_

#include <coreplugin/iuavgadget.h>
#include "modelviewgadgetwidget.h"

class IUAVGadget;
class QWidget;
class QString;
class ModelViewGadgetWidget;

using namespace Core;

class ModelViewGadget : public Core::IUAVGadget
{
    Q_OBJECT
public:
    ModelViewGadget(QString classId, ModelViewGadgetWidget *widget, QWidget *parent = 0);
    ~ModelViewGadget();

    QWidget *widget() { return m_widget; }
    void loadConfiguration(IUAVGadgetConfiguration* config);

private:
    ModelViewGadgetWidget *m_widget;
};


#endif // MODELVIEWGADGET_H_
