/**
 ******************************************************************************
 *
 * @file       baseview.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup CorePlugin Core Plugin
 * @{
 * @brief The Core GCS plugin
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

#ifndef BASEVIEW_H
#define BASEVIEW_H

#include "core_global.h"
#include "iview.h"
#include <QtCore/QPointer>

namespace Core {

class CORE_EXPORT BaseView : public IView
{
    Q_OBJECT

public:
    BaseView(QObject *parent = 0);
    ~BaseView();

    QList<int> context() const;
    QWidget *widget();
    const char *uniqueViewName() const;
    IView::ViewPosition defaultPosition() const;

    void setUniqueViewName(const char *name);
    QWidget *setWidget(QWidget *widget);
    void setContext(const QList<int> &context);
    void setDefaultPosition(IView::ViewPosition position);

private:
    const char *m_viewName;
    QPointer<QWidget> m_widget;
    QList<int> m_context;
    IView::ViewPosition m_defaultPosition;
};

} // namespace Core

#endif // BASEVIEW_H
