/**
 ******************************************************************************
 *
 * @file       browseritemdelegate.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   uavobjectbrowser
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

#include "browseritemdelegate.h"
#include "treeitem.h"
#include <QtGui/QSpinBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QComboBox>

BrowserItemDelegate::BrowserItemDelegate(QObject *parent) :
        QStyledItemDelegate(parent)
{
}

QWidget *BrowserItemDelegate::createEditor(QWidget *parent,
                                           const QStyleOptionViewItem & option ,
                                           const QModelIndex & index ) const
{
    FieldTreeItem *item = static_cast<FieldTreeItem*>(index.internalPointer());
    if (item->isIntType()) {
        IntFieldTreeItem *intItem = static_cast<IntFieldTreeItem*>(item);
        QSpinBox *editor = new QSpinBox(parent);
        editor->setMinimum(intItem->MIN);
        editor->setMaximum(intItem->MAX);
        return editor;
    } else if (item->isEnum()) {
        EnumFieldTreeItem *enumItem = static_cast<EnumFieldTreeItem*>(item);
        QComboBox *editor = new QComboBox(parent);
        foreach (QString option, enumItem->enumOptions)
            editor->addItem(option);
        return editor;
    }
    return QStyledItemDelegate::createEditor(parent, option, index);
}


void BrowserItemDelegate::setEditorData(QWidget *editor,
                                        const QModelIndex &index) const
{
    FieldTreeItem *item = static_cast<FieldTreeItem*>(index.internalPointer());
    if (item->isIntType()) {
        int value = index.model()->data(index, Qt::EditRole).toInt();
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->setValue(value);
    } else if (item->isEnum()) {
        int value = index.model()->data(index, Qt::EditRole).toInt();
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        comboBox->setCurrentIndex(value);
    } else {
        QStyledItemDelegate::setEditorData(editor, index);
    }
}

void BrowserItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                       const QModelIndex &index) const
{
    FieldTreeItem *item = static_cast<FieldTreeItem*>(index.internalPointer());
    if (item->isIntType()) {
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->interpretText();
        int value = spinBox->value();
        model->setData(index, value, Qt::EditRole);
    } else if (item->isEnum()) {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        int value = comboBox->currentIndex();
        model->setData(index, value, Qt::EditRole);
    } else {
        QStyledItemDelegate::setModelData(editor, model, index);
    }
}

void BrowserItemDelegate::updateEditorGeometry(QWidget *editor,
                                               const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

QSize BrowserItemDelegate::sizeHint(const QStyleOptionViewItem & option, const QModelIndex &index) const
{
       return QSpinBox().sizeHint();
}
