/**
 ******************************************************************************
 *
 * @file       pfdgadget.h
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup OPMapPlugin Primary Flight Display Plugin
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

#ifndef PFDGADGET_H_
#define PFDGADGET_H_

#include <coreplugin/iuavgadget.h>
#include "pfdgadgetwidget.h"

class IUAVGadget;
class QWidget;
class QString;
class PFDGadgetWidget;

using namespace Core;

class PFDGadget : public Core::IUAVGadget
{
    Q_OBJECT
public:
    PFDGadget(QString classId, PFDGadgetWidget *widget, QWidget *parent = 0);
    ~PFDGadget();

    QWidget *widget() { return m_widget; }
    void loadConfiguration(IUAVGadgetConfiguration* config);

private:
    PFDGadgetWidget *m_widget;
};


#endif // PFDGADGET_H_
