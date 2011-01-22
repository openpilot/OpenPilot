/****************************************************************************
 **
 ** Copyright (C) Qxt Foundation. Some rights reserved.
 **
 ** This file is part of the QxtCore module of the Qxt library.
 **
 ** This library is free software; you can redistribute it and/or modify it
 ** under the terms of the Common Public License, version 1.0, as published
 ** by IBM, and/or under the terms of the GNU Lesser General Public License,
 ** version 2.1, as published by the Free Software Foundation.
 **
 ** This file is provided "AS IS", without WARRANTIES OR CONDITIONS OF ANY
 ** KIND, EITHER EXPRESS OR IMPLIED INCLUDING, WITHOUT LIMITATION, ANY
 ** WARRANTIES OR CONDITIONS OF TITLE, NON-INFRINGEMENT, MERCHANTABILITY OR
 ** FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** You should have received a copy of the CPL and the LGPL along with this
 ** file. See the LICENSE file and the cpl1.0.txt/lgpl-2.1.txt files
 ** included with the source distribution for more information.
 ** If you did not receive a copy of the licenses, contact the Qxt Foundation.
 **
 ** <http://libqxt.org>  <foundation@libqxt.org>
 **
 ****************************************************************************/

#ifndef QXTDATASTREAMSIGNALSERIALIZER_H
#define QXTDATASTREAMSIGNALSERIALIZER_H

#include <qxtglobal.h>
#include <qxtabstractsignalserializer.h>

class QXT_CORE_EXPORT QxtDataStreamSignalSerializer : public QxtAbstractSignalSerializer
{
public:
    /*!
     * Serializes a signal into a form suitable for sending to an I/O device.
     */
    virtual QByteArray serialize(const QString& fn, const QVariant& p1 = QVariant(), const QVariant& p2 = QVariant(), const QVariant& p3 = QVariant(),
                                 const QVariant& p4 = QVariant(), const QVariant& p5 = QVariant(), const QVariant& p6 = QVariant(),
                                 const QVariant& p7 = QVariant(), const QVariant& p8 = QVariant()) const;

    /*!
     * Deserializes binary data into a signal name and a list of parameters.
     */
    virtual DeserializedData deserialize(QByteArray& data);

    /*!
     * Indicates whether the data currently in the buffer can be deserialized.
     */
    virtual bool canDeserialize(const QByteArray& buffer) const;
};

#endif
