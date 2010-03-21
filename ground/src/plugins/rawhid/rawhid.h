/**
 ******************************************************************************
 *
 * @file       rawhid.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   rawhid_plugin
 * @{
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

#ifndef RAWHID_H
#define RAWHID_H

#include "rawhid_global.h"
#include <QIODevice>

#include "pjrc_rawhid.h"

/**
*   The actual IO device that will be used to communicate
*   with the board.
*/
class RAWHID_EXPORT RawHID : public QIODevice
{
public:
    RawHID();
    RawHID(const QString &deviceName);
    virtual ~RawHID();

    virtual bool open(OpenMode mode);
    virtual void close();
    virtual bool isSequential() const;

protected:
    virtual qint64 readData(char *data, qint64 maxSize);
    virtual qint64 writeData(const char *data, qint64 maxSize);

    QString serialNumber;
    int m_deviceNo;
    pjrc_rawhid dev;
};

#endif // RAWHID_H
