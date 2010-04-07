/**
 ******************************************************************************
 *
 * @file       uavobjectfieldint32.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   uavobjects_plugin
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
#ifndef UAVOBJECTFIELDINT32_H
#define UAVOBJECTFIELDINT32_H

#include "uavobjects_global.h"
#include "uavobjectfield.h"
#include <QtEndian>


class UAVOBJECTS_EXPORT UAVObjectFieldInt32: public UAVObjectField
{
    Q_OBJECT

public:
    UAVObjectFieldInt32(const QString& name, const QString& units, quint32 numElements);
    void initializeValues();
    qint32 pack(quint8* dataOut);
    qint32 unpack(const quint8* dataIn);
    double getDouble(quint32 index = 0);
    void setDouble(double value, quint32 index = 0);
    quint32 getNumBytesElement();
    qint32 getValue(quint32 index = 0);
    void setValue(qint32 value, quint32 index = 0);

private:
    quint32 numBytesPerElement;
};

#endif // UAVOBJECTFIELDINT32_H
