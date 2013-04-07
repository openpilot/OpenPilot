/**
 ******************************************************************************
 *
 * @file       icontext.h
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

#ifndef ICONTEXT_H
#define ICONTEXT_H

#include <coreplugin/core_global.h>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

namespace Core {

class CORE_EXPORT IContext : public QObject
{
    Q_OBJECT
public:
    IContext(QObject *parent = 0) : QObject(parent) {}
    virtual ~IContext() {}

    virtual QList<int> context() const = 0;
    virtual QWidget *widget() = 0;
    virtual QString contextHelpId() const { return QString(); }
};

class BaseContext : public Core::IContext
{
public:
    BaseContext(QWidget *widget, const QList<int> &context, QObject *parent = 0)
        : Core::IContext(parent),
        m_widget(widget),
        m_context(context)
    {
    }

    QList<int> context() const { return m_context; }

    QWidget *widget() { return m_widget; }

private:
    QWidget *m_widget;
    QList<int> m_context;
};

} // namespace Core

#endif //ICONTEXT_H
