/**
 ******************************************************************************
 *
 * @file       generator_common.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      common functions for generating uavobjects code
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

#ifndef UAVOBJECTGENERATORCOMMON_H
#define UAVOBJECTGENERATORCOMMON_H

#include "../uavobjectparser.h"
#include "generator_io.h"

// These special chars (regexp) will be removed from C/java identifiers
#define ENUM_SPECIAL_CHARS "[\\.\\-\\s\\+/\\(\\)]"

void replaceCommonTags(QString& out, ObjectInfo* info);
void replaceCommonTags(QString& out);
QString boolTo01String(bool value);
QString boolToTRUEFALSEString(bool value);

#endif
