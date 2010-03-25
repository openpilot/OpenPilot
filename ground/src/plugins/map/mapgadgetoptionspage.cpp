/**
 ******************************************************************************
 *
 * @file       mapgadgetoptionspage.cpp
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

#include "mapgadgetoptionspage.h"
#include "mapgadgetconfiguration.h"
#include <QtGui/QLabel>

MapGadgetOptionsPage::MapGadgetOptionsPage(IUAVGadgetConfiguration *config, QObject *parent) :
    IOptionsPage(parent),
    m_config(qobject_cast<MapGadgetConfiguration*>(config))
{
}

QWidget *MapGadgetOptionsPage::createPage(QWidget *parent)
{
    QWidget *label = new QLabel("Settings will be here.", parent);
//    QLabel *label = new QLabel(QString(m_config->zoom()));
    label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    return label;
}

void MapGadgetOptionsPage::apply()
{

}

void MapGadgetOptionsPage::finish()
{

}

