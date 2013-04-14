/**
 ******************************************************************************
 *
 * @file       fileiconprovider.cpp
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

#include "fileiconprovider.h"
#include "mimedatabase.h"

#include <QtGui/QApplication>
#include <QtGui/QStyle>
#include <QtGui/QPainter>

using namespace Core;

/*!
  \class FileIconProvider

  Provides icons based on file suffixes.

  The class is a singleton: It's instance can be accessed via the static instance() method.
  Plugins can register custom icons via registerIconSuffix(), and retrieve icons via the icon()
  method.
  */

FileIconProvider *FileIconProvider::m_instance = 0;

FileIconProvider::FileIconProvider()
    : m_unknownFileIcon(qApp->style()->standardIcon(QStyle::SP_FileIcon))
{
}

FileIconProvider::~FileIconProvider()
{
    m_instance = 0;
}

/*!
  Returns the icon associated with the file suffix in fileInfo. If there is none,
  the default icon of the operating system is returned.
  */
QIcon FileIconProvider::icon(const QFileInfo &fileInfo)
{
    const QString suffix = fileInfo.suffix();
    QIcon icon = iconForSuffix(suffix);

    if (icon.isNull()) {
        // Get icon from OS and store it in the cache

        // Disabled since for now we'll make sure that all icons fit with our
        // own custom icons by returning an empty one if we don't know it.
#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
        // This is incorrect if the OS does not always return the same icon for the
        // same suffix (Mac OS X), but should speed up the retrieval a lot ...
        icon = m_systemIconProvider.icon(fileInfo);
        if (!suffix.isEmpty())
            registerIconOverlayForSuffix(icon, suffix);
#else
        if (fileInfo.isDir()) {
            icon = m_systemIconProvider.icon(fileInfo);
        } else {
            icon = m_unknownFileIcon;
        }
#endif
    }

    return icon;
}

/*!
  Creates a pixmap with baseicon at size and overlays overlayIcon over it.
  */
QPixmap FileIconProvider::overlayIcon(QStyle::StandardPixmap baseIcon, const QIcon &overlayIcon, const QSize &size) const
{
    QPixmap iconPixmap = qApp->style()->standardIcon(baseIcon).pixmap(size);
    QPainter painter(&iconPixmap);
    painter.drawPixmap(0, 0, overlayIcon.pixmap(size));
    painter.end();
    return iconPixmap;
}

/*!
  Registers an icon for a given suffix, overlaying the system file icon.
  */
void FileIconProvider::registerIconOverlayForSuffix(const QIcon &icon, const QString &suffix)
{
    QPixmap fileIconPixmap = overlayIcon(QStyle::SP_FileIcon, icon, QSize(16, 16));
    // delete old icon, if it exists
    QList<QPair<QString,QIcon> >::iterator iter = m_cache.begin();
    for (; iter != m_cache.end(); ++iter) {
        if ((*iter).first == suffix) {
            iter = m_cache.erase(iter);
            break;
        }
    }

    QPair<QString,QIcon> newEntry(suffix, fileIconPixmap);
    m_cache.append(newEntry);
}

/*!
  Registers an icon for all the suffixes of a given mime type, overlaying the system file icon.
  */
void FileIconProvider::registerIconOverlayForMimeType(const QIcon &icon, const MimeType &mimeType)
{
    foreach (const QString &suffix, mimeType.suffixes())
        registerIconOverlayForSuffix(icon, suffix);
}

/*!
  Returns an icon for the given suffix, or an empty one if none registered.
  */
QIcon FileIconProvider::iconForSuffix(const QString &suffix) const
{
    QIcon icon;
#if defined(Q_WS_WIN) || defined(Q_WS_MAC) // On Windows and Mac we use the file system icons
    Q_UNUSED(suffix)
#else
    if (suffix.isEmpty())
        return icon;

    QList<QPair<QString,QIcon> >::const_iterator iter = m_cache.constBegin();
    for (; iter != m_cache.constEnd(); ++iter) {
        if ((*iter).first == suffix) {
            icon = (*iter).second;
            break;
        }
    }
#endif
    return icon;
}

/*!
  Returns the sole instance of FileIconProvider.
  */
FileIconProvider *FileIconProvider::instance()
{
    if (!m_instance)
        m_instance = new FileIconProvider;
    return m_instance;
}
