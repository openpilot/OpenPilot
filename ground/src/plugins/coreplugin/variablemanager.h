/**
 ******************************************************************************
 *
 * @file       variablemanager.h
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

#ifndef VARIABLEMANAGER_H
#define VARIABLEMANAGER_H

#include "core_global.h"

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE
class QFileInfo;
QT_END_NAMESPACE

namespace Core {

class CORE_EXPORT VariableManager : public QObject
{
    Q_OBJECT

public:
    VariableManager(QObject *parent);
    ~VariableManager();

    static VariableManager* instance() { return m_instance; }

    void insert(const QString &variable, const QString &value);
    void insertFileInfo(const QString &tag, const QFileInfo &file);
    void removeFileInfo(const QString &tag);
    QString value(const QString &variable) const;
    QString value(const QString &variable, const QString &defaultValue) const;
    bool remove(const QString &variable);
    QString resolve(const QString &stringWithVariables) const;


private:
    QMap<QString, QString> m_map;
    static VariableManager *m_instance;
};

} // namespace Core

#endif // VARIABLEMANAGER_H
