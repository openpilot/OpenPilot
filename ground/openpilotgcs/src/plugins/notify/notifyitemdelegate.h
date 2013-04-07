/**
 ******************************************************************************
 *
 * @file       notifyitemdelegate.h
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

#ifndef NOTIFYITEMDELEGATE_H
#define NOTIFYITEMDELEGATE_H

#include <QItemDelegate>
#include <QComboBox>


class NotifyItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    NotifyItemDelegate(QObject *parent = 0);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &,
                                              const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                                      const QModelIndex &index) const;
    QSize  sizeHint ( const QStyleOptionViewItem  & option, const QModelIndex & index ) const;

private slots:
    void selectRow(const QString & text);
    void commitAndCloseEditor();

private:
    QObject* _parent;
};

#endif // NOTIFYITEMDELEGATE_H
