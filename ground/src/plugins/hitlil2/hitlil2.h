/**
 ******************************************************************************
 *
 * @file       hitlil2.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   hitlil2plugin
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

#ifndef HITLIL2_H
#define HITLIL2_H

#include <coreplugin/iuavgadget.h>
#include "hitlil2widget.h"

class IUAVGadget;
class QWidget;
class QString;
class HITLIL2Widget;

using namespace Core;

class HITLIL2 : public Core::IUAVGadget
{
    Q_OBJECT
public:
    HITLIL2(QString classId, HITLIL2Widget *widget, QWidget *parent = 0);
    ~HITLIL2();

    QWidget *widget() { return m_widget; }
    void loadConfiguration(IUAVGadgetConfiguration* config);

private:
    HITLIL2Widget *m_widget;
};


#endif // HITLIL2_H
