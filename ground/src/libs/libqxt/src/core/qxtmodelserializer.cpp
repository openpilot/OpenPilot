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
#include "qxtmodelserializer.h"
#include <QAbstractItemModel>
#include <QDataStream>

/*
    \class QxtModelSerializer
    \inmodule QxtCore
    \brief The QxtModelSerializer class provides serialization of QAbstractItemModel
 */

struct QxtModelItem
{
    QMap<int, QVariant> itemData;
    int rowCount;
    int columnCount;
};

inline QDataStream& operator<<(QDataStream& out, const QxtModelItem& item)
{
    out << item.itemData;
    out << item.rowCount;
    out << item.columnCount;
    return out;
}

inline QDataStream& operator>>(QDataStream& in, QxtModelItem& item)
{
    in >> item.itemData;
    in >> item.rowCount;
    in >> item.columnCount;
    return in;
}

class QxtModelSerializerPrivate : public QxtPrivate<QxtModelSerializer>
{
public:
    QxtModelSerializerPrivate() : model(0) { }

    void save(QDataStream& stream, const QModelIndex& index) const;
    bool restore(QDataStream& stream, const QModelIndex& index);
    void print(const QByteArray& data) const;

    QAbstractItemModel* model;
};

void QxtModelSerializerPrivate::save(QDataStream& stream, const QModelIndex& index) const
{
    QxtModelItem item;
    item.itemData = model->itemData(index);
    item.rowCount = model->rowCount(index);
    item.columnCount = model->columnCount(index);
    stream << item;
    for (int r = 0; r < item.rowCount; ++r)
        for (int c = 0; c < item.columnCount; ++c)
            save(stream, model->index(r, c, index));
}

bool QxtModelSerializerPrivate::restore(QDataStream& stream, const QModelIndex& index)
{
    QxtModelItem item;
    stream >> item;

    if (index.isValid())
        model->setItemData(index, item.itemData);
    if (item.rowCount > 0)
        model->insertRows(0, item.rowCount, index);
    if (item.columnCount > 0)
        model->insertColumns(0, item.columnCount, index);

    for (int r = 0; r < item.rowCount; ++r)
    {
        for (int c = 0; c < item.columnCount; ++c)
            restore(stream, model->index(r, c, index));
    }
    return stream.status() == QDataStream::Ok;
}

void QxtModelSerializerPrivate::print(const QByteArray& data) const
{
    QDataStream stream(data);
    while (!stream.atEnd())
    {
        QxtModelItem item;
        stream >> item;
    }
}

QxtModelSerializer::QxtModelSerializer(QAbstractItemModel* model)
{
    qxt_d().model = model;
}

QxtModelSerializer::~QxtModelSerializer()
{
}

QAbstractItemModel* QxtModelSerializer::model() const
{
    return qxt_d().model;
}

void QxtModelSerializer::setModel(QAbstractItemModel* model)
{
    qxt_d().model = model;
}

QByteArray QxtModelSerializer::saveModel(const QModelIndex& index) const
{
    if (!qxt_d().model)
    {
        qWarning("QxtModelSerializer::saveModel(): model == null");
        return QByteArray();
    }

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    qxt_d().save(stream, index);
    qxt_d().print(data);
    return data;
}

bool QxtModelSerializer::restoreModel(const QByteArray& data, const QModelIndex& index)
{
    if (!qxt_d().model)
    {
        qWarning("QxtModelSerializer::restoreModel(): model == null");
        return false;
    }

    QDataStream stream(data);
    qxt_d().print(data);
    return qxt_d().restore(stream, index);
}
