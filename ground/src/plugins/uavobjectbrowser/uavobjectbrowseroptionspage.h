/**
 ******************************************************************************
 *
 * @file       uavobjectbrowseroptionspage.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   uavobjectbrowser
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

#ifndef UAVOBJECTBROWSEROPTIONSPAGE_H
#define UAVOBJECTBROWSEROPTIONSPAGE_H

#include "coreplugin/dialogs/ioptionspage.h"

namespace Core {
class IUAVGadgetConfiguration;
}
class UAVObjectBrowserConfiguration;
class QSpinBox;
class QDoubleSpinBox;

using namespace Core;

class UAVObjectBrowserOptionsPage : public IOptionsPage
{
Q_OBJECT
public:
    explicit UAVObjectBrowserOptionsPage(UAVObjectBrowserConfiguration *config, QObject *parent = 0);

    QWidget *createPage(QWidget *parent);
    void apply();
    void finish();

signals:

public slots:
private:
    UAVObjectBrowserConfiguration *m_config;

};

#endif // UAVOBJECTBROWSEROPTIONSPAGE_H
