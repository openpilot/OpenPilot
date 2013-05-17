/**
 ******************************************************************************
 * @file       version_info.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
 * @addtogroup VersionInfo
 * @{
 * @brief      GCS version info class header.
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

#ifndef VERSION_INFO_H
#define VERSION_INFO_H

#include <QString>

class VersionInfo {
public:
    static QString revision();
    static QString year();
    static QString origin();
    static QString uavoHash();
private:
    VersionInfo();
};

#endif // VERSION_INFO_H
