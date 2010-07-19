/**
 ******************************************************************************
 *
 * @file       notifytablemodel.h
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

#ifndef NOTIFYTABLEMODEL_H
#define NOTIFYTABLEMODEL_H


#include <QAbstractTableModel>
#include <QList>
#include "NotifyPluginConfiguration.h"

class NotifyTableModel : public QAbstractTableModel
{
	Q_OBJECT
  public:
	NotifyTableModel(QList<NotifyPluginConfiguration*> *parentList, const QStringList& parentHeaderList, QObject *parent = 0)
		 : QAbstractTableModel(parent),
		  _list(parentList),
		  headerStrings(parentHeaderList)
	{ }

	int rowCount(const QModelIndex &parent = QModelIndex()) const
	{
		return _list->count();
	}

	int columnCount(const QModelIndex &/*parent*/) const
	{
		return 3;
	}

	Qt::ItemFlags flags(const QModelIndex &index) const
	{
		if (!index.isValid())
			return Qt::ItemIsEnabled;

		return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
	}

	bool setData(const QModelIndex &index, const QVariant &value, int role);
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	bool insertRows(int position, int rows, const QModelIndex &index);
	bool removeRows(int position, int rows, const QModelIndex &index);

private slots:
	void entryUpdated(int offset);
	void entryAdded(int position);
private:
	QList<NotifyPluginConfiguration*> *_list;
	QStringList headerStrings;
};


#endif // NOTIFYTABLEMODEL_H
