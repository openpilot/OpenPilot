/**
 ******************************************************************************
 *
 * @file       basemode.h
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

#ifndef BASEMODE_H
#define BASEMODE_H

#include "core_global.h"
#include "imode.h"

#include <QtCore/QObject>
#include <QtGui/QWidget>
#include <QtGui/QIcon>

namespace Core {

class CORE_EXPORT BaseMode
  : public IMode
{
    Q_OBJECT

public:
    BaseMode(QObject *parent = 0);
    ~BaseMode();

    // IMode
    QString name() const { return m_name; }
    QIcon icon() const { return m_icon; }
    int priority() const { return m_priority; }
    QWidget *widget() { return m_widget; }
    const char *uniqueModeName() const { return m_uniqueModeName; }
    QList<int> context() const { return m_context; }

    void setName(const QString &name) { m_name = name; }
    void setIcon(const QIcon &icon) { m_icon = icon; }
    void setPriority(int priority) { m_priority = priority; }
    void setWidget(QWidget *widget) { m_widget = widget; }
    void setUniqueModeName(const char *uniqueModeName) { m_uniqueModeName = uniqueModeName; }
    void setContext(const QList<int> &context) { m_context = context; }

private:
    QString m_name;
    QIcon m_icon;
    int m_priority;
    QWidget *m_widget;
    const char * m_uniqueModeName;
    QList<int> m_context;
};

} // namespace Core

#endif // BASEMODE_H
