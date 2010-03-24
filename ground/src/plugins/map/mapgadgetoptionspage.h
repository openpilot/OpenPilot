/**
 ******************************************************************************
 *
 * @file       mapgadgetoptionspage.h
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

#ifndef MAPGADGETOPTIONSPAGE_H
#define MAPGADGETOPTIONSPAGE_H

#include "coreplugin/uavgadgetoptionspage.h"

class MapGadgetConfiguration;

using namespace Core;

class MapGadgetOptionsPage : public UAVGadgetOptionsPage
{
Q_OBJECT
public:
    explicit MapGadgetOptionsPage(IUAVGadgetConfiguration *config, QObject *parent = 0);
    QWidget *widget();
    void apply();
    void finish();

signals:

public slots:
private:
    MapGadgetConfiguration *m_config;


};

#endif // MAPGADGETOPTIONSPAGE_H
