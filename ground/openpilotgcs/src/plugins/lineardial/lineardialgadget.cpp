/**
 ******************************************************************************
 *
 * @file       lineardialgadget.cpp
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

#include "lineardialgadget.h"
#include "lineardialgadgetwidget.h"
#include "lineardialgadgetconfiguration.h"

LineardialGadget::LineardialGadget(QString classId, LineardialGadgetWidget *widget, QWidget *parent) :
        IUAVGadget(classId, parent),
        m_widget(widget)
{
}

LineardialGadget::~LineardialGadget()
{
    delete m_widget;
}

/*
  This is called when a configuration is loaded, and updates the plugin's settings.
  Careful: the plugin is already drawn before the loadConfiguration method is called the
  first time, so you have to be careful not to assume all the plugin values are initialized
  the first time you use them
 */
void LineardialGadget::loadConfiguration(IUAVGadgetConfiguration* config)
{
    LineardialGadgetConfiguration *m = qobject_cast<LineardialGadgetConfiguration*>(config);
    m_widget->setFactor(m->getFactor());
    m_widget->setDecimalPlaces(m->getDecimalPlaces());
    m_widget->setRange(m->getMin(),m->getMax());
    m_widget->setGreenRange(m->getGreenMin(), m->getGreenMax());
    m_widget->setYellowRange(m->getYellowMin(), m->getYellowMax());
    m_widget->setRedRange(m->getRedMin(), m->getRedMax());
    m_widget->setDialFile(m->getDialFile()); // Triggers widget repaint
    m_widget->setDialFont(m->getFont());
    m_widget->connectInput(m->getSourceDataObject(), m->getSourceObjectField());
	m_widget->enableOpenGL(m->useOpenGL());
}
