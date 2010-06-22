/**
 ******************************************************************************
 *
 * @file       airspeedgadget.cpp
 * @author     David "Buzz" Carlson Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   airspeed 
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

#include "pfdgadget.h"
#include "pfdgadgetwidget.h"
#include "pfdgadgetconfiguration.h"

PFDGadget::PFDGadget(QString classId, PFDGadgetWidget *widget, QWidget *parent) :
        IUAVGadget(classId, parent),
        m_widget(widget)
{
}

PFDGadget::~PFDGadget()
{
}

/*
  This is called when a configuration is loaded, and updates the plugin's settings.
  Careful: the plugin is already drawn before the loadConfiguration method is called the
  first time, so you have to be careful not to assume all the plugin values are initialized
  the first time you use them
 */
void PFDGadget::loadConfiguration(IUAVGadgetConfiguration* config)
{
    PFDGadgetConfiguration *m = qobject_cast<PFDGadgetConfiguration*>(config);
    m_widget->setDialFile(m->dialFile());

    m_widget->setN2Min(m->getN2Min());
    m_widget->setN2Max(m->getN2Max());
    m_widget->setN2Factor(m->getN2Factor());
    m_widget->setN3Min(m->getN3Min());
    m_widget->setN3Max(m->getN3Max());
    m_widget->setN3Factor(m->getN3Factor());
    m_widget->connectNeedles();
}
