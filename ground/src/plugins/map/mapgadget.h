/**
 ******************************************************************************
 *
 * @file       mapgadget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   map
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

#ifndef MAPGADGET_H_
#define MAPGADGET_H_

#include <coreplugin/iuavgadget.h>
#include "mapgadgetwidget.h"

class IUAVGadget;
//class QList<int>;
class QWidget;
class QString;
class MapGadgetWidget;

using namespace Core;

class MapGadget : public Core::IUAVGadget
{
    Q_OBJECT
public:
    MapGadget(QString classId, MapGadgetWidget *widget, QWidget *parent = 0);
    ~MapGadget();

    QWidget *widget() { return m_widget; }
    void loadConfiguration(IUAVGadgetConfiguration* config);

private:
    MapGadgetWidget *m_widget;
};


#endif // MAPGADGET_H_
