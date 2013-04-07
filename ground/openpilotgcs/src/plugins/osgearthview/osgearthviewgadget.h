/********************************************************************************
*
* @file       osgearthviewgadget.h
* @author     The OpenPilot Team Copyright (C) 2012.
* @addtogroup GCSPlugins GCS Plugins
* @{
* @addtogroup OsgEarthview Plugin
* @{
* @brief Osg Earth view of UAV
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

#ifndef OSGEARTHVIEWGADGET_H_
#define OSGEARTHVIEWGADGET_H_

#include <coreplugin/iuavgadget.h>
#include "osgearthviewwidget.h"

class IUAVGadget;
class QWidget;
class QString;
class OsgEarthviewWidget;

using namespace Core;

class OsgEarthviewGadget : public Core::IUAVGadget
{
    Q_OBJECT
public:
    OsgEarthviewGadget(QString classId, OsgEarthviewWidget *widget, QWidget *parent = 0);
    ~OsgEarthviewGadget();

    QWidget *widget() { return m_widget; }
    void loadConfiguration(IUAVGadgetConfiguration* config);

private:
    OsgEarthviewWidget *m_widget;
};


#endif // OSGEARTHVIEWGADGET_H_
