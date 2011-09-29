/**
 ******************************************************************************
 *
 * @file       notifytablemodel.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   notifyplugin
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

#include "notifytablemodel.h"
#include "notifylogging.h"
#include <qdebug.h>
#include <QMimeData>

static int _dragStartRow = -1;
static int _dragNumRows = -1;
const char* mime_type_notify_table = "openpilot/notify_plugin_table";

NotifyTableModel::NotifyTableModel(QList<NotificationItem*>& parentList, QObject* parent)
	: QAbstractTableModel(parent)
	, _list(parentList)
{
	_headerStrings << "Name" << "Repeats" << "Lifetime,sec" << "Mute";
	connect(this, SIGNAL(dragRows(int, int)), this, SLOT(dropRows(int, int)));
}


bool NotifyTableModel::setData(const QModelIndex &index,
							   const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::DisplayRole) {
        if(eMESSAGE_NAME == index.column()) {
            emit dataChanged(index, index);
            return true;
        }
    }
    if (index.isValid() && role == Qt::EditRole) {
        if(eREPEAT_VALUE == index.column())
             _list.at(index.row())->setRetryString(value.toString());
        else {
            if(eEXPIRE_TIME == index.column())
                _list.at(index.row())->setLifetime(value.toInt());
            else {
                if(eENABLE_NOTIFICATION == index.column())
                    _list.at(index.row())->setMute(value.toBool());
            }
        }
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

QVariant NotifyTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        qWarning() << "NotifyTableModel::data - index.isValid()";
        return QVariant();
    }

    if (index.row() >= _list.size())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch(index.column())
        {
        case eMESSAGE_NAME:
            return _list.at(index.row())->parseNotifyMessage();

        case eREPEAT_VALUE:
            return _list.at(index.row())->retryString();

        case eEXPIRE_TIME:
            return _list.at(index.row())->lifetime();

        case eENABLE_NOTIFICATION:
            return _list.at(index.row())->mute();

        default:
            return QVariant();
        }
    }
    else
    {
        if (Qt::SizeHintRole == role){
            return  QVariant(10);
        }
    }
    return QVariant();
}

QVariant NotifyTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        return _headerStrings.at(section);
    else
        if(orientation == Qt::Vertical)
            return QString("%1").arg(section);

    return QVariant();
}

bool NotifyTableModel::insertRows(int position, int rows, const QModelIndex& index)
{
    Q_UNUSED(index);

    if((-1 == position) || (-1 == rows) )
        return false;

    beginInsertRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        _list.insert(position, new NotificationItem());
    }

    endInsertRows();
    return true;
}

bool NotifyTableModel::removeRows(int position, int rows, const QModelIndex& index)
{
    Q_UNUSED(index);

    if((-1 == position) || (-1 == rows) )
        return false;

    beginRemoveRows(QModelIndex(), position, position + rows - 1);

    for (int row = 0; row < rows; ++row) {
        _list.removeAt(position);
    }

    endRemoveRows();
    return true;
}

void NotifyTableModel::entryUpdated(int offset)
{
    QModelIndex idx = index(offset, 0);
    emit dataChanged(idx, idx);
}

void NotifyTableModel::entryAdded(NotificationItem* item)
{
    insertRows(rowCount(), 1, QModelIndex());
    NotificationItem* tmp = _list.at(rowCount() - 1);
    _list.replace(rowCount() - 1, item);
    delete tmp;
    entryUpdated(rowCount() - 1);
}


bool NotifyTableModel::dropMimeData( const QMimeData * data, Qt::DropAction action, int row,
                   int column, const QModelIndex& parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat(mime_type_notify_table))
        return false;

    int beginRow = -1;

    if (row != -1)
        beginRow = row;
    else {
        if (parent.isValid())
            beginRow = parent.row();
        else
            beginRow = rowCount(QModelIndex());
    }

    if(-1 == beginRow)
        return false;

    QByteArray encodedData = data->data(mime_type_notify_table);
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    int rows = beginRow;
    while(!stream.atEnd()) {
        qint32 ptr;
        stream >> ptr;
        NotificationItem* item = reinterpret_cast<NotificationItem*>(ptr);
        int dragged = _list.indexOf(item);
        if(-1 == dragged || rows >= _list.size() || dragged == rows) {
            qNotifyDebug() << "no such item";
            return false;
        }
        removeRows(rows, 1, QModelIndex());
        insertRows(rows, 1, QModelIndex());
        _list.replace(dragged, item);
        //        _list.swap(dragged, rows);
        ++rows;
    };

    QModelIndex idxTopLeft = index(beginRow, 0, QModelIndex());
    QModelIndex idxBotRight = index(beginRow, columnCount(QModelIndex()), QModelIndex());
    emit dataChanged(idxTopLeft, idxBotRight);
    //QStringList newItems;

    //removeRows(_dragStartRow, _dragNumRows, QModelIndex());
    //insertRows(beginRow, rows, QModelIndex());
//    int rows = beginRow;
//    while (!stream.atEnd()) {
//        _list.at(index.row())->deserialize(stream);
//        ++rows;
//    }

//    while(rows) {
//        int column = 0;
//        foreach (const QString& text, newItems) {
//            QModelIndex idx = index(beginRow, column, QModelIndex());
//            if(!column)
//                setData(const_cast<const QModelIndex&>(idx), text, Qt::DisplayRole);
//            else
//                setData(const_cast<const QModelIndex&>(idx), text, Qt::EditRole);
//            ++column;
//        }
//        ++beginRow;
//        --rows;
//    }
    return true;
}

QMimeData* NotifyTableModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    int rows = 0;
    foreach (const QModelIndex& index, indexes) {
        if(!index.column()) {
            qint32 item = reinterpret_cast<qint32>(_list.at(index.row()));
            stream << item;
        }
        ++rows;
    }

//    int numRows = 0;
//    foreach (const QModelIndex& index, indexes) {
//        if (index.isValid() && index.column()) {
//            _list.at(index.row())->serialize(stream);
////            if(!index.column()) {
////                numRows++;
////                QString name = data(index, Qt::DisplayRole).toString();
////                stream << name;
////            } else {
////                QString text = data(index, Qt::EditRole).toString();
////                stream << text;
////            }
//        }
//    }

    mimeData->setData(mime_type_notify_table, encodedData);

    //emit dragRows(indexes.at(0).row(), rows);
    dropRows(indexes.at(0).row(), rows);
    //_dragStartRow = indexes.at(0).row();
    //_dragNumRows = 1/*numRows*/;

    return mimeData;
}

void NotifyTableModel::dropRows(int position, int count) const
{
    for (int row = 0; row < count; ++row) {
        _list.removeAt(position);
    }
}
