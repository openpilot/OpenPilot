/**
 ******************************************************************************
 *
 * @file       notifyitemdelegate.cpp
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

#include "notifyitemdelegate.h"
#include <QtGui>

//RepeatCounterDelegate::RepeatCounterDelegate(QObject *parent)
//		 : QItemDelegate(parent) {}

// QWidget* RepeatCounterDelegate::createEditor(QWidget *parent,
//										   const QStyleOptionViewItem &,
//										   const QModelIndex &index) const
// {
//	 if (index.row() == 2) {
//		 QSpinBox *editor = new QSpinBox(parent);
//		 editor->setMinimum(0);
//		 editor->setMaximum(100);
//		 return editor;
//	 }

//	 QLabel *editor = new QLabel(parent);

//	 connect(editor, SIGNAL(editingFinished()),
//		 this, SLOT(commitAndCloseEditor()));
//	 return editor;
// }

// void RepeatCounterDelegate::commitAndCloseEditor()
// {
//	 QLabel* editor = qobject_cast<QLabel*>(sender());
//	 emit commitData(editor);
//	 emit closeEditor(editor);
// }

// void RepeatCounterDelegate::setEditorData(QWidget *editor,
//	 const QModelIndex &index) const
// {
//	 QLabel* edit = qobject_cast<QLabel*>(editor);
//	 if (edit) {
//		 edit->setText(index.model()->data(index, Qt::EditRole).toString());
//	 } else {
//		 QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
//		 if (spinBox)
//		 {
//			 int value = index.model()->data(index, Qt::EditRole).toInt();
//			 spinBox->setValue(value);

//			 //repeatEditor->setCurrentIndex(repeatEditor->findText(index.model()->data(index, Qt::EditRole).toString()));
////			 repeatEditor->setDate(QDate::fromString(
////			 index.model()->data(index, Qt::EditRole).toString(),
////			 "d/M/yyyy"));
//		 }
//	 }
// }

// void RepeatCounterDelegate::setModelData(QWidget *editor,
//	 QAbstractItemModel *model, const QModelIndex &index) const
// {
//	 QLabel* edit = qobject_cast<QLabel*>(editor);
//	 if (edit) {
//		 model->setData(index, edit->text());
//	 } else {
//		 QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
//		 if (spinBox) {
//			 spinBox->interpretText();
//			 int value = spinBox->value();
//			 model->setData(index, value, Qt::EditRole);
//		 }
//	 }
// }

//////////////////////////////////////////////////////////////////////////

 NotifyItemDelegate::NotifyItemDelegate(QStringList items,QObject *parent)
		 : QItemDelegate(parent),
		 m_parent(parent),
		 m_items(items) {

		 }

 QWidget *NotifyItemDelegate::createEditor(QWidget *parent,
										   const QStyleOptionViewItem &,
										   const QModelIndex &index) const
 {
	 if (index.column() == 1) {
		 QComboBox* editor = new QComboBox(parent);
		 editor->clear();
		 editor->addItems(m_items);
		 //repeatEditor->setCurrentIndex(0);
		 //repeatEditor->setItemDelegate(new RepeatCounterDelegate());

		 //connect(repeatEditor,SIGNAL(activated (const QString& )),this,SLOT(selectRow(const QString& )));
		 //QTableWidgetItem* item = qobject_cast<QTableWidgetItem *>(parent);
		 //((QTableWidgetItem*)parent)->setSelected(true);
		 connect(editor, SIGNAL(editingFinished()),
				 this, SLOT(commitAndCloseEditor()));
		  return editor;
	 }
	 QLineEdit *editor = new QLineEdit(parent);



	 connect(editor, SIGNAL(editingFinished()),
		 this, SLOT(commitAndCloseEditor()));
	 return editor;
 }

 void NotifyItemDelegate::commitAndCloseEditor()
 {
	 QLineEdit *editor = qobject_cast<QLineEdit *>(sender());
	 if (editor)
	 {
		 emit commitData(editor);
		 emit closeEditor(editor);
	 }
	 else
	 {
		 QComboBox* editor = qobject_cast<QComboBox*>(sender());
		  if (editor)
		 {
			  emit commitData(editor);
			  emit closeEditor(editor);
		  }
	  }
 }

 void NotifyItemDelegate::setEditorData(QWidget *editor,
	 const QModelIndex &index) const
 {
	 QLineEdit *edit = qobject_cast<QLineEdit*>(editor);
	 if (edit) {
		 edit->setText(index.model()->data(index, Qt::EditRole).toString());
	 } else {
		 QComboBox * repeatEditor = qobject_cast<QComboBox *>(editor);
		 if (repeatEditor)
			 repeatEditor->setCurrentIndex(repeatEditor->findText(index.model()->data(index, Qt::EditRole).toString()));
//		 QTableWidgetItem* item = ((QTableWidget*)m_parent)->item(index.row(),1);
//		 item->setSelected(true);
		 }
 }

 void NotifyItemDelegate::setModelData(QWidget *editor,
	 QAbstractItemModel *model, const QModelIndex &index) const
 {
	 QLineEdit *edit = qobject_cast<QLineEdit *>(editor);
	 if (edit) {
		 model->setData(index, edit->text());
	 } else {
		 QComboBox * repeatEditor = qobject_cast<QComboBox *>(editor);
		 if (repeatEditor) {
			 model->setData(index, repeatEditor->currentText());
//			 emit commitData(repeatEditor);
//			 emit closeEditor(repeatEditor);
		 }
	 }
 }

void NotifyItemDelegate::selectRow(const QString & text)
{
	//QList<QTableWidgetItem *> list = ((QTableWidget*)(sender()->parent()))->findItems(text,Qt::MatchExactly);
	QComboBox* combo = qobject_cast<QComboBox*>(sender());
	QTableWidget* table = new QTableWidget;
	table = (QTableWidget*)(combo->parent());
	qDebug()<<table->columnCount();
	qDebug()<<table->rowCount();
	qDebug()<<table->currentRow();
	//table->setCurrentIndex(1);
	//table->findItems(text,Qt::MatchExactly);
	//item->model()->index()
	//item->setSelected(true);
}

// bool NotifyItemDelegate::editorEvent(QEvent * event, QAbstractItemModel * model,
//					 const QStyleOptionViewItem & option, const QModelIndex & index )
// {
//	 if(event->type()==QEvent::EnabledChange)
//	 {
//		 QTableWidgetItem* item = ((QTableWidget*)parent())->item(index.row(),index.column());
//		 item->setSelected(true);
//	 }
//	 return false;
// }
