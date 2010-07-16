/**
 ******************************************************************************
 *
 * @file       variablemanager.cpp
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

#include "variablemanager.h"

#include <QtCore/QFileInfo>

using namespace Core;

VariableManager *VariableManager::m_instance = 0;

VariableManager::VariableManager(QObject *parent) : QObject(parent)
{
    m_instance = this;
}

VariableManager::~VariableManager()
{
    m_instance = 0;
}

void VariableManager::insert(const QString &variable, const QString &value)
{
    m_map.insert(variable, value);
}

void VariableManager::insertFileInfo(const QString &tag, const QFileInfo &file)
{
    insert(tag, file.filePath());
    insert(tag  + QLatin1String(":absoluteFilePath"), file.absoluteFilePath());
    insert(tag + QLatin1String(":absolutePath"), file.absolutePath());
    insert(tag + QLatin1String(":baseName"), file.baseName());
    insert(tag + QLatin1String(":canonicalPath"), file.canonicalPath());
    insert(tag + QLatin1String(":canonicalFilePath"), file.canonicalFilePath());
    insert(tag + QLatin1String(":completeBaseName"), file.completeBaseName());
    insert(tag + QLatin1String(":completeSuffix"), file.completeSuffix());
    insert(tag + QLatin1String(":fileName"), file.fileName());
    insert(tag + QLatin1String(":filePath"), file.filePath());
    insert(tag + QLatin1String(":path"), file.path());
    insert(tag + QLatin1String(":suffix"), file.suffix());
}

void VariableManager::removeFileInfo(const QString &tag)
{
    if (remove(tag)) {
        remove(tag + QLatin1String(":absoluteFilePath"));
        remove(tag + QLatin1String(":absolutePath"));
        remove(tag + QLatin1String(":baseName"));
        remove(tag + QLatin1String(":canonicalPath"));
        remove(tag + QLatin1String(":canonicalFilePath"));
        remove(tag + QLatin1String(":completeBaseName"));
        remove(tag + QLatin1String(":completeSuffix"));
        remove(tag + QLatin1String(":fileName"));
        remove(tag + QLatin1String(":filePath"));
        remove(tag + QLatin1String(":path"));
        remove(tag + QLatin1String(":suffix"));
    }
}

QString VariableManager::value(const QString &variable) const
{
    return m_map.value(variable);
}

QString VariableManager::value(const QString &variable, const QString &defaultValue) const
{
    return m_map.value(variable, defaultValue);
}

bool VariableManager::remove(const QString &variable)
{
    return m_map.remove(variable) > 0;
}

QString VariableManager::resolve(const QString &stringWithVariables) const
{
    QString result = stringWithVariables;
    QMapIterator<QString, QString> i(m_map);
    while (i.hasNext()) {
        i.next();
        QString key = QLatin1String("${");
        key += i.key();
        key += QLatin1Char('}');
        result.replace(key, i.value());
    }
    return result;
}
