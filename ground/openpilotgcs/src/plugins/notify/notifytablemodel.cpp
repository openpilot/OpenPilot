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
#include <qdebug.h>

NotifyTableModel::NotifyTableModel(QList<NotificationItem*>* parentList, QObject* parent)
	: QAbstractTableModel(parent)
{
	_headerStrings << "Name" << "Repeats" << "Lifetime,sec" << "Enable";
	_list.reset(parentList);
}


bool NotifyTableModel::setData(const QModelIndex &index,
							   const QVariant &value, int role)
{
	if (index.isValid() && role == Qt::EditRole) {
		if(eREPEAT_VALUE == index.column())
			_list->at(index.row())->setRepeatFlag(value.toString());
		else {
			if(eEXPIRE_TIME == index.column())
				_list->at(index.row())->setExpireTimeout(value.toInt());
			else {
				if(eENABLE_NOTIFICATION == index.column())
					_list->at(index.row())->setEnableFlag(value.toBool());
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

	if (index.row() >= _list->size())
		return QVariant();

	if (role == Qt::DisplayRole || role == Qt::EditRole)
	{
		switch(index.column())
		{
		case eMESSAGE_NAME:
			return _list->at(index.row())->parseNotifyMessage();

		case eREPEAT_VALUE:
			return _list->at(index.row())->getRepeatFlag();

		case eEXPIRE_TIME:
			return _list->at(index.row())->getExpireTimeout();

		case eENABLE_NOTIFICATION:
			return _list->at(index.row())->getEnableFlag();

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

bool NotifyTableModel::insertRows(int position, int rows, const QModelIndex &index)
{
	Q_UNUSED(index);
	beginInsertRows(QModelIndex(), position, position+rows-1);
	endInsertRows();
	return true;
}

 bool NotifyTableModel::removeRows(int position, int rows, const QModelIndex &index)
 {
	 Q_UNUSED(index);
	 beginRemoveRows(QModelIndex(), position, position+rows-1);

	 for (int row=0; row < rows; ++row) {
		 _list->removeAt(position);
	 }

	 endRemoveRows();
	 return true;
 }

void NotifyTableModel::entryUpdated(int offset)
{
	QModelIndex idx = index(offset, 0);
	emit dataChanged(idx, idx);
}

void NotifyTableModel::entryAdded(int position)
{
	insertRows(position, 1,QModelIndex());
}
