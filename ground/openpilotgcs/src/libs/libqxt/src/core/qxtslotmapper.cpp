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
#include "qxtslotmapper.h"
#include "qxtmetaobject.h"
#include <QMetaMethod>
#include <QPointer>
#include <QVariant>

/*!
    \class QxtSlotMapper
    \inmodule QxtCore
    \brief The QxtSlotMapper class maps signals with a parameter to certain slots.

    QxtSlotMapper is the counterpart of QSignalMapper. Where QSignalMapper maps
    a parameterless signal into one that has a parameter based on the sender, 
    QxtSlotMapper maps a signal with a parameter to a specified slot and 
    receiver based on that parameter.

    The parameter comparison is based on QVariant::operator==(). Thus, any
    built-in Qt type supported by QVariant and QMetaType is supported.

    \sa QVariant, QMetaType::Type

    \image qxtslotmapper.png

    Example usage:
    \code
    QxtSlotMapper mapper;
    mapper.connect(sender, SIGNAL(signal1(int)));
    mapper.connect(sender, SIGNAL(signal2(QString)));

    mapper.addMapping(3, receiver1, SLOT(slot1(int)));
    mapper.addMapping(QString("abc"), receiver2, SLOT(slot2(QString)));
    \endcode

    \sa QSignalMapper
*/

struct QxtSlotMapInfo
{
    QVariant parameter;
    QPointer<QObject> receiver;
    const char* member;
};

class QxtSlotMapperPrivate : public QxtPrivate<QxtSlotMapper>
{
public:
    static int checkSignal(const QObject* sender, const char* signal);

    QList<QxtSlotMapInfo> mappings;
};

int QxtSlotMapperPrivate::checkSignal(const QObject* sender, const char* signal)
{
    if (!sender || !signal)
    {
        qWarning("QxtSlotMapper: Cannot connect %s::%s",
            sender ? sender->metaObject()->className() : "(null)",
            (signal && *signal) ? signal+1 : "(null)");
        return false;
    }

    const QByteArray signalName = QMetaObject::normalizedSignature(signal+1);
    int signalId = sender->metaObject()->indexOfSignal(signalName);
    if (signalId < 0)
    {
        qWarning("QxtSlotMapper: No such signal %s::%s",
            sender->metaObject()->className(), signal+1);
        return false;
    }
    return signalId;
}

/*!
    Constructs a new QxtSlotMapper with \a parent.
*/
QxtSlotMapper::QxtSlotMapper(QObject* parent)
    : QObject(parent)
{
}

/*!
    Destructs the slot mapper.
*/
QxtSlotMapper::~QxtSlotMapper()
{
}

/*!
    Adds mapping from connected signals with \a parameter to \a receiver's \a member.
*/
void QxtSlotMapper::addMapping(const QVariant& parameter, QObject* receiver, const char* member)
{
    QxtSlotMapInfo mapping = { parameter, receiver, member };
    qxt_d().mappings.append(mapping);
}

/*!
    Removes mapping from connected signals with \a parameter to \a receiver's \a member.

    If \a member is \c 0, any mapping with \a parameter to \a receiver is removed.
    If \a receiver is \c 0, any mapping with \a parameter is removed.
*/
void QxtSlotMapper::removeMapping(const QVariant& parameter, QObject* receiver, const char* member)
{
    QMutableListIterator<QxtSlotMapInfo> it(qxt_d().mappings);
    while (it.hasNext())
    {
        const QxtSlotMapInfo& info = it.next();
        if (info.parameter == parameter)
        {
            if ((!receiver || receiver == info.receiver) &&
                (!member || QxtMetaObject::methodName(member) == QxtMetaObject::methodName(info.member)))
            {
                it.remove();
            }
        }
    }
}

/*!
    Connects to \a sender's \a signal. Returns \c true if the connection
    succeeds and \c false otherwise.
*/
bool QxtSlotMapper::connect(const QObject* sender, const char* signal)
{
    QByteArray signalName = QMetaObject::normalizedSignature(signal);
    int signalId = QxtSlotMapperPrivate::checkSignal(sender, signal);
    if (signalId < 0)
        return false;

    int index = signalName.indexOf('(');
    if (index != -1)
    {
        QByteArray typeName = signalName.mid(index + 1);
        int index = typeName.indexOf(')');
        if (index != -1)
            typeName.truncate(index);
        typeName = QMetaObject::normalizedType(typeName);

        int type = QMetaType::type(typeName);
        int methods = metaObject()->methodCount();
        return QMetaObject::connect(sender, signalId, this, methods + type);
    }
    return false;
}

/*!
    \internal
 */
int QxtSlotMapper::qt_metacall(QMetaObject::Call call, int id, void** arguments)
{
    id = QObject::qt_metacall(call, id, arguments);
    if (id < 0 || call != QMetaObject::InvokeMetaMethod)
        return id;

    QVariant param(id, arguments[1]);
    foreach (const QxtSlotMapInfo& info, qxt_d().mappings)
    {
        if (info.receiver && info.parameter == param)
        {
            const QMetaObject* metaObject = info.receiver->metaObject();
            int index = metaObject->indexOfMethod(QxtMetaObject::methodSignature(info.member));
            if (index != -1)
            {
                QMetaMethod method = metaObject->method(index);
                switch (method.parameterTypes().count())
                {
                    case 0:
                        QMetaObject::invokeMethod(info.receiver, 
                                                QxtMetaObject::methodName(info.member));
                        break;
                    case 1:
                        QMetaObject::invokeMethod(info.receiver, 
                                                QxtMetaObject::methodName(info.member),
                                                QGenericArgument(QMetaType::typeName(param.type()), arguments[1]));
                        break;
                    default:
                        qWarning("QxtSlotMapper does not support slots with more than one parameter");
                        break;
                }
            }
        }
    }
    return -1;
}
