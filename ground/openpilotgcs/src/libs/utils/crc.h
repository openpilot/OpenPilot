/**
 ******************************************************************************
 *
 * @file       crc.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2013.
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

#ifndef CRC_H
#define CRC_H

#include "utils_global.h"

namespace Utils {

class QTCREATOR_UTILS_EXPORT Crc {

public:
    /**
     * Update the crc value with new data.
     *
     * \param crc      The current crc value.
     * \param data     data to update the crc with.
     * \return         The updated crc value.
     */
    static quint8 updateCRC(quint8 crc, const quint8 data);

    /**
     * Update the crc value with new data.
     *
     * \param crc      The current crc value.
     * \param data     Pointer to a buffer of \a data_len bytes.
     * \param length   Number of bytes in the \a data buffer.
     * \return         The updated crc value.
     */
    static quint8 updateCRC(quint8 crc, const quint8 *data, qint32 length);

};

} // namespace Utils

#endif // CRC_H
