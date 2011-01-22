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

#include <qxtdatastreamsignalserializer.h>
#include <QIODevice>
#include <QDataStream>
#include <QtGlobal>
#include <QtDebug>
#include <qendian.h>

QByteArray QxtDataStreamSignalSerializer::serialize(const QString& fn, const QVariant& p1, const QVariant& p2, const QVariant& p3,
        const QVariant& p4, const QVariant& p5, const QVariant& p6, const QVariant& p7, const QVariant& p8) const
{
    QByteArray rv;
    QDataStream str(&rv, QIODevice::WriteOnly);
    str << fn;
    unsigned char ct = 8;
    if (!p1.isValid()) ct = 0;
    else if (!p2.isValid()) ct = 1;
    else if (!p3.isValid()) ct = 2;
    else if (!p4.isValid()) ct = 3;
    else if (!p5.isValid()) ct = 4;
    else if (!p6.isValid()) ct = 5;
    else if (!p7.isValid()) ct = 6;
    else if (!p8.isValid()) ct = 7;
    str << ct;
    if (ct-- > 0) str << p1;
    if (ct-- > 0) str << p2;
    if (ct-- > 0) str << p3;
    if (ct-- > 0) str << p4;
    if (ct-- > 0) str << p5;
    if (ct-- > 0) str << p6;
    if (ct-- > 0) str << p7;
    if (ct-- > 0) str << p8;
    char sizeData[4];
    qToLittleEndian(quint32(rv.size()), (uchar*)sizeData);
    return QByteArray(sizeData, 4) + rv;
}

QxtAbstractSignalSerializer::DeserializedData QxtDataStreamSignalSerializer::deserialize(QByteArray& data)
{
    QByteArray cmd;
    quint32 len = qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(data.constData()));

    cmd = data.mid(4, len);
    data = data.mid(len + 4);
    if (cmd.length() == 0) return NoOp();

    QDataStream str(cmd);

    QString signal;
    unsigned char argCount;
    QList<QVariant> v;
    QVariant t;
    str >> signal >> argCount;

    if (str.status() == QDataStream::ReadCorruptData) return ProtocolError();

    for (int i = 0; i < argCount; i++)
    {
        str >> t;
        v << t;
    }
    return qMakePair(signal, v);
}

bool QxtDataStreamSignalSerializer::canDeserialize(const QByteArray& buffer) const
{
    int headerLen = qFromLittleEndian<quint32>(reinterpret_cast<const uchar*>(buffer.constData()));
    int bodyLen = quint32(buffer.length() - 4);
    return headerLen <= bodyLen;
}
