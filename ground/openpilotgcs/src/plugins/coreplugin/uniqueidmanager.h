/**
 ******************************************************************************
 *
 * @file       uniqueidmanager.h
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

#ifndef UNIQUEIDMANAGER_H
#define UNIQUEIDMANAGER_H

#include "core_global.h"

#include <QtCore/QString>
#include <QtCore/QHash>

namespace Core {

class CORE_EXPORT UniqueIDManager
{
public:
    UniqueIDManager();
    ~UniqueIDManager();

    static UniqueIDManager *instance() { return m_instance; }

    bool hasUniqueIdentifier(const QString &id) const;
    int uniqueIdentifier(const QString &id);
    QString stringForUniqueIdentifier(int uid);

private:
    QHash<QString, int> m_uniqueIdentifiers;
    static UniqueIDManager *m_instance;
};

} // namespace Core

#endif // UNIQUEIDMANAGER_H
