/**
 ******************************************************************************
 *
 * @file       uavobjectfieldenum.h
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
#ifndef UAVOBJECTFIELDENUM_H
#define UAVOBJECTFIELDENUM_H

#include "uavobjects_global.h"
#include "uavobjectfield.h"
#include <QStringList>

// Note: This class could be implemented as a template but due to limitations of the Qt
// plugins it not possible.

class UAVOBJECTS_EXPORT UAVObjectFieldEnum: public UAVObjectField
{
    Q_OBJECT

public:
    UAVObjectFieldEnum(const QString& name, const QString& units, quint32 numElements, QStringList& options);
    QStringList getOptions();
    QString getSelected(quint32 arrayIndex);
    void setSelected(QString& val, quint32 arrayIndex);
    quint8 getSelectedIndex(quint32 arrayIndex);
    void setSelectedIndex(quint8 index, quint32 arrayIndex);
    void initializeValues();
    qint32 pack(quint8* dataOut);
    qint32 unpack(const quint8* dataIn);
    double getDouble(quint32 index = 0);
    void setDouble(double value, quint32 index = 0);
    quint32 getNumBytesElement();

private:
    quint32 numBytesPerElement;
    QStringList options;

    quint8 getValue(quint32 index = 0);
    void setValue(quint8 value, quint32 index = 0);
};

#endif // UAVOBJECTFIELDENUM_H
