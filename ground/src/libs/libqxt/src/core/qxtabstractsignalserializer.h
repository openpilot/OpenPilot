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

#ifndef QXTABSTRACTSIGNALSERIALIZER_H
#define QXTABSTRACTSIGNALSERIALIZER_H

#include <qxtglobal.h>
#include <QString>
#include <QVariant>
#include <QList>
#include <QPair>
#include <QByteArray>

/*!
 * \class QxtAbstractSignalSerializer
 * \inmodule QxtCore
 * \brief The QxtAbstractSignalSerializer class provides an interface for classes that convert signals to a binary form.
 *
 * QxtAbstractSignalSerializer is an abstract interface used by, for example, QxtRPCService to convert signals into a
 * serialized binary format suitable for storing or transmitting over an I/O device.
 *
 * Qxt provides a default implementation in the form of QxtDataStreamSignalSerializer, which is generally sufficient for
 * most applications. Implement other subclasses of QxtAbstractSignalSerializer to allow QxtRPCService and similar tools
 * to integrate with existing protocols.
 */

class QXT_CORE_EXPORT QxtAbstractSignalSerializer
{
public:
    /*!
     * The first half of QxtAbstractSignalSerializer::DeserializedData is the name of the deserialized signal. The
     * second half is an array of QVariants containing the parameters of the signal.
     */
    typedef QPair<QString, QList<QVariant> > DeserializedData;

    /*!
     * Destroys the QxtAbstractSignalSerializer.
     */
    virtual ~QxtAbstractSignalSerializer() {}

    /*!
     * Serializes a signal into a form suitable for sending to an I/O device.
     */
    virtual QByteArray serialize(const QString& fn, const QVariant& p1 = QVariant(), const QVariant& p2 = QVariant(),
                                 const QVariant& p3 = QVariant(), const QVariant& p4 = QVariant(),
                                 const QVariant& p5 = QVariant(), const QVariant& p6 = QVariant(),
                                 const QVariant& p7 = QVariant(), const QVariant& p8 = QVariant()) const = 0;

    /*!
     * Deserializes binary data into a signal name and a list of parameters. When implementing this function, be sure
     * to remove the processed portion of the data from the reference parameter.
     * Return QxtAbstractSignalSerializer::NoOp() if the deserialized data does not invoke a signal.
     * Return QxtAbstractSignalSerializer::ProtocolError() if the protocol has been violated and the connection should
     * be severed.
     */
    virtual DeserializedData deserialize(QByteArray& data) = 0;

    /*!
     * Indicates whether the data currently in the buffer can be deserialized.
     */
    virtual bool canDeserialize(const QByteArray& buffer) const = 0;

    /*!
     * Returns an object that indicates that the deserialized data does not invoke a signal.
     */
    static inline DeserializedData NoOp()
    {
        static DeserializedData rv = qMakePair(QString(), QList<QVariant>());
        return rv;
    }

    /*!
     * Returns an object that indicates that the deserialized data indicates a fatal protocol error.
     */
    static inline DeserializedData ProtocolError()
    {
        static DeserializedData rv = qMakePair(QString(), QList<QVariant>() << QVariant());
        return rv;
    }

    /*!
     * Checks to see if the provided object does not invoke a signal.
     */
    static inline bool isNoOp(const DeserializedData& value)
    {
        return value.first.isEmpty() && value.second.isEmpty();
    }

    /*!
     * Checks to see if the provided object indicates a fatal protocol error.
     */
    static inline bool isProtocolError(const DeserializedData& value)
    {
        return value.first.isEmpty() && !value.second.isEmpty();
    }
};

#endif
