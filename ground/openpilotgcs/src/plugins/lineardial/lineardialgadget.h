/**
 ******************************************************************************
 *
 * @file       lineardialgadget.h
 * @author     Edouard Lafargue Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup LinearDialPlugin Linear Dial Plugin
 * @{
 * @brief Impliments a gadget that displays linear gauges 
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

#ifndef LINEARDIALGADGET_H_
#define LINEARDIALGADGET_H_

#include <coreplugin/iuavgadget.h>
#include "lineardialgadgetwidget.h"

class IUAVGadget;
class QWidget;
class QString;
class LineardialGadgetWidget;

using namespace Core;

class LineardialGadget : public Core::IUAVGadget
{
    Q_OBJECT
public:
    LineardialGadget(QString classId, LineardialGadgetWidget *widget, QWidget *parent = 0);
    ~LineardialGadget();

    QWidget *widget() { return m_widget; }
    void loadConfiguration(IUAVGadgetConfiguration* config);

private:
    LineardialGadgetWidget *m_widget;
};


#endif // LINEARDIALGADGET_H_
