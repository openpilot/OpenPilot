/**
 ******************************************************************************
 *
 * @file       uniqueidmanager.cpp
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

#include "uniqueidmanager.h"
#include "coreconstants.h"

using namespace Core;

UniqueIDManager* UniqueIDManager::m_instance = 0;

UniqueIDManager::UniqueIDManager()
{
    m_instance = this;
    m_uniqueIdentifiers.insert(Constants::C_GLOBAL, Constants::C_GLOBAL_ID);
}

UniqueIDManager::~UniqueIDManager()
{
    m_instance = 0;
}

bool UniqueIDManager::hasUniqueIdentifier(const QString &id) const
{
    return m_uniqueIdentifiers.contains(id);
}

int UniqueIDManager::uniqueIdentifier(const QString &id)
{
    if (hasUniqueIdentifier(id))
        return m_uniqueIdentifiers.value(id);

    int uid = m_uniqueIdentifiers.count() + 1;
    m_uniqueIdentifiers.insert(id, uid);
    return uid;
}

QString UniqueIDManager::stringForUniqueIdentifier(int uid)
{
    return m_uniqueIdentifiers.key(uid);
}
