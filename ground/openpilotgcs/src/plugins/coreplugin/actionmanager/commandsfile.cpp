/**
 ******************************************************************************
 *
 * @file       commandsfile.cpp
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

#include "commandsfile.h"
#include "shortcutsettings.h"
#include "command_p.h"

#include <coreplugin/uniqueidmanager.h>

#include <QtCore/QFile>
#include <QtXml/QDomDocument>

using namespace Core;
using namespace Core::Internal;

/*!
    \class CommandsFile
    \brief The CommandsFile class provides a collection of import and export commands.
    \inheaderfile commandsfile.h
*/

/*!
    ...
*/
CommandsFile::CommandsFile(const QString &filename)
    : m_filename(filename)
{

}

/*!
    ...
*/
QMap<QString, QKeySequence> CommandsFile::importCommands() const
{
    QMap<QString, QKeySequence> result;

    QFile file(m_filename);
    if (!file.open(QIODevice::ReadOnly))
        return result;

    QDomDocument doc("KeyboardMappingScheme");
    if (!doc.setContent(&file))
        return result;

    QDomElement root = doc.documentElement();
    if (root.nodeName() != QLatin1String("mapping"))
        return result;

    QDomElement ks = root.firstChildElement();
    for (; !ks.isNull(); ks = ks.nextSiblingElement()) {
        if (ks.nodeName() == QLatin1String("shortcut")) {
            QString id = ks.attribute(QLatin1String("id"));
            QKeySequence shortcutkey;
            QDomElement keyelem = ks.firstChildElement("key");
            if (!keyelem.isNull())
                shortcutkey = QKeySequence(keyelem.attribute("value"));
            result.insert(id, shortcutkey);
        }
    }

    file.close();
    return result;
}

/*!
    ...
*/
bool CommandsFile::exportCommands(const QList<ShortcutItem *> &items)
{
    UniqueIDManager *idmanager = UniqueIDManager::instance();

    QFile file(m_filename);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    QDomDocument doc("KeyboardMappingScheme");
    QDomElement root = doc.createElement("mapping");
    doc.appendChild(root);

    foreach (const ShortcutItem *item, items) {
        QDomElement ctag = doc.createElement("shortcut");
        ctag.setAttribute(QLatin1String("id"), idmanager->stringForUniqueIdentifier(item->m_cmd->id()));
        root.appendChild(ctag);

        QDomElement ktag = doc.createElement("key");
        ktag.setAttribute(QLatin1String("value"), item->m_key.toString());
        ctag.appendChild(ktag);
    }

    file.write(doc.toByteArray());
    file.close();
    return true;
}
