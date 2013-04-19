/**
 ******************************************************************************
 *
 * @file       fileiconprovider.h
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

#ifndef FILEICONPROVIDER_H
#define FILEICONPROVIDER_H

#include <coreplugin/core_global.h>

#include <QtCore/QFileInfo>
#include <QtCore/QPair>
#include <QtGui/QFileIconProvider>
#include <QtGui/QIcon>
#include <QtGui/QStyle>

namespace Core {

class MimeType;

class CORE_EXPORT FileIconProvider
{
public:
    ~FileIconProvider(); // used to clear the cache
    QIcon icon(const QFileInfo &fileInfo);

    QPixmap overlayIcon(QStyle::StandardPixmap baseIcon, const QIcon &overlayIcon, const QSize &size) const;
    void registerIconOverlayForSuffix(const QIcon &icon, const QString &suffix);
    void registerIconOverlayForMimeType(const QIcon &icon, const MimeType &mimeType);

    static FileIconProvider *instance();

private:
    QIcon iconForSuffix(const QString &suffix) const;

    // mapping of file ending to icon
    // TODO: Check if this is really faster than a QHash
    mutable QList<QPair<QString, QIcon> > m_cache;

    QFileIconProvider m_systemIconProvider;
    QIcon m_unknownFileIcon;

    // singleton pattern
    FileIconProvider();
    static FileIconProvider *m_instance;
};

} // namespace Core

#endif // FILEICONPROVIDER_H
