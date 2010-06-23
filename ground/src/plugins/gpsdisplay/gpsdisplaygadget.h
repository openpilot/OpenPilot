/**
 ******************************************************************************
 *
 * @file       gpsdisplaygadget.h
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @brief      GPS gadget to display GPS Status header
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   gpsdisplayplugin
 * @{
 *
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

#ifndef GPSDISPLAYGADGET_H_
#define GPSDISPLAYGADGET_H_

#include <coreplugin/iuavgadget.h>
#include "gpsdisplaywidget.h"

class IUAVGadget;
class QWidget;
class QString;
class GpsDisplayWidget;

using namespace Core;

class GpsDisplayGadget : public Core::IUAVGadget
{
    Q_OBJECT
public:
    GpsDisplayGadget(QString classId, GpsDisplayWidget *widget, QWidget *parent = 0);
    ~GpsDisplayGadget();

    QWidget *widget() { return m_widget; }
    void loadConfiguration(IUAVGadgetConfiguration* config);

private:
    GpsDisplayWidget *m_widget;
};


#endif // GPSDISPLAYGADGET_H_
