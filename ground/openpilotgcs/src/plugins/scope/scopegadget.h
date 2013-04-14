/**
 ******************************************************************************
 *
 * @file       scopegadget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ScopePlugin Scope Gadget Plugin
 * @{
 * @brief The scope Gadget, graphically plots the states of UAVObjects
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


#ifndef SCOPEGADGET_H_
#define SCOPEGADGET_H_

#include <coreplugin/iuavgadget.h>
#include "scopegadgetwidget.h"

class IUAVGadget;
//class QList<int>;
class QWidget;
class QString;
class ScopeGadgetWidget;

using namespace Core;

class ScopeGadget : public Core::IUAVGadget
{
    Q_OBJECT
public:
    ScopeGadget(QString classId, ScopeGadgetWidget *widget, QWidget *parent = 0);
    ~ScopeGadget();

    void loadConfiguration(IUAVGadgetConfiguration* config);

    QList<int> context() const {
        return m_context;
    }
    QWidget *widget() {
        return m_widget;
    }
    QString contextHelpId() const {
        return QString();
    }

private:
    ScopeGadgetWidget *m_widget;
    QList<int> m_context;

    bool configLoaded;
};


#endif // SCOPEGADGET_H_
