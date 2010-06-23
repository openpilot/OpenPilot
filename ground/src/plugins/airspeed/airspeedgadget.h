/**
 ******************************************************************************
 *
 * @file       airspeedgadget.h
 * @author     David "Buzz" Carlson Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   dialplugin
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

#ifndef AIRSPEEDGADGET_H_
#define AIRSPEEDGADGET_H_

#include <coreplugin/iuavgadget.h>
#include "airspeedgadgetwidget.h"

class IUAVGadget;
class QWidget;
class QString;
class AirspeedGadgetWidget;

using namespace Core;

class AirspeedGadget : public Core::IUAVGadget
{
    Q_OBJECT
public:
    AirspeedGadget(QString classId, AirspeedGadgetWidget *widget, QWidget *parent = 0);
    ~AirspeedGadget();

    QWidget *widget() { return m_widget; }
    void loadConfiguration(IUAVGadgetConfiguration* config);

private:
    AirspeedGadgetWidget *m_widget;
};


#endif // AIRSPEEDGADGET_H_
