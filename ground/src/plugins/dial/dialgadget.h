/**
 ******************************************************************************
 *
 * @file       dialgadget.h
 * @author     Edouard Lafargue and David Carlson Copyright (C) 2010.
 * @see        The GNU Public License (GPL) Version 3
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup DialPlugin Dial Plugin
 * @{
 * @brief Plots flight information rotary style dials 
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

#ifndef DIALGADGET_H_
#define DIALGADGET_H_

#include <coreplugin/iuavgadget.h>
#include "dialgadgetwidget.h"

class IUAVGadget;
class QWidget;
class QString;
class DialGadgetWidget;

using namespace Core;

class DialGadget : public Core::IUAVGadget
{
    Q_OBJECT
public:
    DialGadget(QString classId, DialGadgetWidget *widget, QWidget *parent = 0);
    ~DialGadget();

    QWidget *widget() { return m_widget; }
    void loadConfiguration(IUAVGadgetConfiguration* config);

private:
    DialGadgetWidget *m_widget;
};


#endif // DIALGADGET_H_
