/**
 ******************************************************************************
 *
 * @file       pathutils.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      Utilities to find the location of openpilot GCS files:
 *             - Plugins Share directory path
 *
 * @see        The GNU Public License (GPL) Version 3
 *
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

#include "pathutils.h"
#include <stdint.h>
#include <QDebug>


namespace Utils {

    PathUtils::PathUtils()
    {

    }

    /**
      Returns the base path of the share directory
      */
QString PathUtils::GetDataPath()
{
        // Figure out root:  Up one from 'bin'
        QDir rootDir = QApplication::applicationDirPath();
        rootDir.cdUp();
        const QString rootDirPath = rootDir.canonicalPath();
        QString pluginPath = rootDirPath;
        pluginPath += QLatin1Char('/');
        pluginPath += QLatin1String(GCS_DATA_BASENAME);
        pluginPath += QLatin1Char('/');
      return pluginPath;
}

/**
  Cuts the standard data path from the 'path' argument. If path does not start
with the standard data path, then do nothing.
  */
QString PathUtils::RemoveDataPath(QString path)
{
    if (path.startsWith(GetDataPath())) {
        int i = path.length()- GetDataPath().length();
        return QString("%%DATAPATH%%") + path.right(i);
    } else
        return path;
}

/**
  Inserts the data path (only if the path starts with %%DATAPATH%%)
  */
QString PathUtils::InsertDataPath(QString path)
{
    if (path.startsWith(QString("%%DATAPATH%%")))
    {
        QString newPath = GetDataPath();
        newPath += path.right(path.length()-12);
        return newPath;
    }
    return path;
}

}
