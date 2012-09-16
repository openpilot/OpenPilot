/**
 ******************************************************************************
 *
 * @file       uavobjectbrowser.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup UAVObjectBrowserPlugin UAVObject Browser Plugin
 * @{
 * @brief The UAVObject Browser gadget plugin
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

#ifndef UAVOBJECTBROWSER_H_
#define UAVOBJECTBROWSER_H_

#include <coreplugin/iuavgadget.h>
#include "uavobjectbrowserwidget.h"
#include "uavobjectbrowserconfiguration.h"

class IUAVGadget;
class QWidget;
class QString;
class UAVObjectBrowserWidget;

using namespace Core;

class UAVObjectBrowser : public Core::IUAVGadget
{
    Q_OBJECT
public:
    UAVObjectBrowser(QString classId, UAVObjectBrowserWidget *widget, QWidget *parent = 0);
    ~UAVObjectBrowser();

    QWidget *widget() { return m_widget; }
    void loadConfiguration(IUAVGadgetConfiguration* config);
private slots:
    void viewOptionsChangedSlot(bool categorized,bool scientific,bool metadata);
private:
    UAVObjectBrowserWidget *m_widget;
    UAVObjectBrowserConfiguration *m_config;
};


#endif // UAVOBJECTBROWSER_H_
