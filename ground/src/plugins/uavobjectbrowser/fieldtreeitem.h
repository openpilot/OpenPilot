/**
 ******************************************************************************
 *
 * @file       fieldtreeitem.h
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

#ifndef FIELDTREEITEM_H
#define FIELDTREEITEM_H

#include "treeitem.h"
#include <QtCore/QStringList>
#include <QtGui/QWidget>
#include <QtGui/QSpinBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QComboBox>
#include <limits>

#define QINT8MIN std::numeric_limits<qint8>::min()
#define QINT8MAX std::numeric_limits<qint8>::max()
#define QUINTMIN std::numeric_limits<quint8>::min()
#define QUINT8MAX std::numeric_limits<quint8>::max()
#define QINT16MIN std::numeric_limits<qint16>::min()
#define QINT16MAX std::numeric_limits<qint16>::max()
#define QUINT16MAX std::numeric_limits<quint16>::max()
#define QINT32MIN std::numeric_limits<qint32>::min()
#define QINT32MAX std::numeric_limits<qint32>::max()
#define QUINT32MAX std::numeric_limits<qint32>::max()

class FieldTreeItem : public TreeItem
{
Q_OBJECT
public:
    FieldTreeItem(int index, const QList<QVariant> &data, TreeItem *parent = 0) :
            TreeItem(data, parent), m_index(index) { }
    FieldTreeItem(int index, const QVariant &data, TreeItem *parent = 0) :
            TreeItem(data, parent), m_index(index) { }
    bool isEditable() { return true; }
    virtual QWidget *createEditor(QWidget *parent) = 0;
    virtual QVariant getEditorValue(QWidget *editor) = 0;
    virtual void setEditorValue(QWidget *editor, QVariant value) = 0;
    virtual void apply() { }
protected:
    int m_index;
};

class EnumFieldTreeItem : public FieldTreeItem
{
Q_OBJECT
public:
    EnumFieldTreeItem(UAVObjectField *field, int index, const QList<QVariant> &data,
                      TreeItem *parent = 0) :
    FieldTreeItem(index, data, parent), m_enumOptions(field->getOptions()), m_field(field) { }
    EnumFieldTreeItem(UAVObjectField *field, int index, const QVariant &data,
                      TreeItem *parent = 0) :
    FieldTreeItem(index, data, parent), m_enumOptions(field->getOptions()), m_field(field) { }
    void setData(QVariant value, int column) {
        QStringList options = m_field->getOptions();
        QVariant tmpValue = m_field->getValue(m_index);
        int tmpValIndex = options.indexOf(tmpValue.toString());
        setChanged(tmpValIndex != value);
        TreeItem::setData(value, column);
    }
    QString enumOptions(int index) { return m_enumOptions.at(index); }
    void apply() {
        int value = data(dataColumn).toInt();
        QStringList options = m_field->getOptions();
        m_field->setValue(options[value], m_index);
        setChanged(false);
    }
    void update() {
        QStringList options = m_field->getOptions();
        QVariant value = m_field->getValue(m_index);
        int valIndex = options.indexOf(value.toString());
        if (data() != valIndex || changed()) {
            TreeItem::setData(valIndex);
            setHighlight(true);
        }
    }
    QWidget *createEditor(QWidget *parent) {
        QComboBox *editor = new QComboBox(parent);
        foreach (QString option, m_enumOptions)
            editor->addItem(option);
        return editor;
    }

    QVariant getEditorValue(QWidget *editor) {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        return comboBox->currentIndex();
    }

    void setEditorValue(QWidget *editor, QVariant value) {
        QComboBox *comboBox = static_cast<QComboBox*>(editor);
        comboBox->setCurrentIndex(value.toInt());
    }
private:
    QStringList m_enumOptions;
    UAVObjectField *m_field;
};

class IntFieldTreeItem : public FieldTreeItem
{
Q_OBJECT
public:
    IntFieldTreeItem(int index, const QList<QVariant> &data, int min, int max, TreeItem *parent = 0) :
            FieldTreeItem(index, data, parent), m_minValue(min), m_maxValue(max) { }
    IntFieldTreeItem(int index, const QVariant &data, int min, int max, TreeItem *parent = 0) :
            FieldTreeItem(index, data, parent), m_minValue(min), m_maxValue(max) { }
    QWidget *createEditor(QWidget *parent) {
        QSpinBox *editor = new QSpinBox(parent);
        editor->setMinimum(m_minValue);
        editor->setMaximum(m_maxValue);
        return editor;
    }

    QVariant getEditorValue(QWidget *editor) {
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->interpretText();
        return spinBox->value();
    }

    void setEditorValue(QWidget *editor, QVariant value) {
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->setValue(value.toInt());
    }
private:
    int m_minValue;
    int m_maxValue;
};

class Int8FieldTreeItem : public IntFieldTreeItem
{
Q_OBJECT
public:
    Int8FieldTreeItem(UAVObjectField *field, int index, const QList<QVariant> &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QINT8MIN, QINT8MAX, parent), m_field(field) { }
    Int8FieldTreeItem(UAVObjectField *field, int index, const QVariant &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QINT8MIN, QINT8MAX, parent), m_field(field) { }
    void setData(QVariant value, int column) {
        setChanged(m_field->getValue(m_index) != value);
        TreeItem::setData(value, column);
    }
    void apply() {
        m_field->setValue(data(dataColumn).toInt(), m_index);
        setChanged(false);
    }
    void update() {
        int value = m_field->getValue(m_index).toInt();
        if (data() != value || changed()) {
            TreeItem::setData(value);
            setHighlight(true);
        }
    }
private:
    UAVObjectField *m_field;
};

class Int16FieldTreeItem : public IntFieldTreeItem
{
Q_OBJECT
public:
    Int16FieldTreeItem(UAVObjectField *field, int index, const QList<QVariant> &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QINT16MIN, QINT16MAX, parent), m_field(field) { }
    Int16FieldTreeItem(UAVObjectField *field, int index, const QVariant &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QINT16MIN, QINT16MAX, parent), m_field(field) { }
    void setData(QVariant value, int column) {
        setChanged(m_field->getValue(m_index) != value);
        TreeItem::setData(value, column);
    }
    void apply() {
        m_field->setValue(data(dataColumn).toInt(), m_index);
        setChanged(false);
    }
    void update() {
        int value = m_field->getValue(m_index).toInt();
        if (data() != value || changed()) {
            TreeItem::setData(value);
            setHighlight(true);
        }
    }
private:
    UAVObjectField *m_field;
};

class Int32FieldTreeItem : public IntFieldTreeItem
{
Q_OBJECT
public:
    Int32FieldTreeItem(UAVObjectField *field, int index, const QList<QVariant> &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QINT32MIN, QINT32MAX, parent), m_field(field) { }
    Int32FieldTreeItem(UAVObjectField *field, int index, const QVariant &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QINT32MIN, QINT32MAX, parent), m_field(field) { }
    void setData(QVariant value, int column) {
        setChanged(m_field->getValue(m_index) != value);
        TreeItem::setData(value, column);
    }
    void apply() {
        m_field->setValue(data(dataColumn).toInt(), m_index);
        setChanged(false);
    }
    void update() {
        int value = m_field->getValue(m_index).toInt();
        if (data() != value || changed()) {
            TreeItem::setData(value);
            setHighlight(true);
        }
    }
private:
    UAVObjectField *m_field;
};

class UInt8FieldTreeItem : public IntFieldTreeItem
{
Q_OBJECT
public:
    UInt8FieldTreeItem(UAVObjectField *field, int index, const QList<QVariant> &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QUINTMIN, QUINT8MAX, parent), m_field(field) { }
    UInt8FieldTreeItem(UAVObjectField *field, int index, const QVariant &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QUINTMIN, QUINT8MAX, parent), m_field(field) { }
    void setData(QVariant value, int column) {
        setChanged(m_field->getValue(m_index) != value);
        TreeItem::setData(value, column);
    }
    void apply() {
        m_field->setValue(data(dataColumn).toInt(), m_index);
        setChanged(false);
    }
    void update() {
        int value = m_field->getValue(m_index).toInt();
        if (data() != value || changed()) {
            TreeItem::setData(value);
            setHighlight(true);
        }
    }
private:
    UAVObjectField *m_field;
};

class UInt16FieldTreeItem : public IntFieldTreeItem
{
Q_OBJECT
public:
    UInt16FieldTreeItem(UAVObjectField *field, int index, const QList<QVariant> &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QUINTMIN, QUINT16MAX, parent), m_field(field) { }
    UInt16FieldTreeItem(UAVObjectField *field, int index, const QVariant &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QUINTMIN, QUINT16MAX, parent), m_field(field) { }
    void setData(QVariant value, int column) {
        setChanged(m_field->getValue(m_index) != value);
        TreeItem::setData(value, column);
    }
    void apply() {
        m_field->setValue(data(dataColumn).toInt(), m_index);
        setChanged(false);
    }
    void update() {
        int value = m_field->getValue(m_index).toInt();
        if (data() != value || changed()) {
            TreeItem::setData(value);
            setHighlight(true);
        }
    }
private:
    UAVObjectField *m_field;
};

class UInt32FieldTreeItem : public IntFieldTreeItem
{
Q_OBJECT
public:
    UInt32FieldTreeItem(UAVObjectField *field, int index, const QList<QVariant> &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QUINTMIN, QUINT32MAX, parent), m_field(field) { }
    UInt32FieldTreeItem(UAVObjectField *field, int index, const QVariant &data, TreeItem *parent = 0) :
            IntFieldTreeItem(index, data, QUINTMIN, QUINT32MAX, parent), m_field(field) { }
    void setData(QVariant value, int column) {
        setChanged(m_field->getValue(m_index) != value);
        TreeItem::setData(value, column);
    }
    void apply() {
        m_field->setValue(data(dataColumn).toInt(), m_index);
        setChanged(false);
    }
    void update() {
        int value = m_field->getValue(m_index).toInt();
        if (data() != value || changed()) {
            TreeItem::setData(value);
            setHighlight(true);
        }
    }
private:
    UAVObjectField *m_field;
};


class FloatFieldTreeItem : public FieldTreeItem
{
Q_OBJECT
public:
    FloatFieldTreeItem(UAVObjectField *field, int index, const QList<QVariant> &data, TreeItem *parent = 0) :
            FieldTreeItem(index, data, parent), m_field(field) { }
    FloatFieldTreeItem(UAVObjectField *field, int index, const QVariant &data, TreeItem *parent = 0) :
            FieldTreeItem(index, data, parent), m_field(field) { }
    void setData(QVariant value, int column) {
        setChanged(m_field->getValue(m_index) != value);
        TreeItem::setData(value, column);
    }
    void apply() {
        m_field->setValue(data(dataColumn).toDouble(), m_index);
        setChanged(false);
    }
    void update() {
        double value = m_field->getValue(m_index).toDouble();
        if (data() != value || changed()) {
            TreeItem::setData(value);
            setHighlight(true);
        }
    }
    QWidget *createEditor(QWidget *parent) {
        QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
        editor->setDecimals(6);
        editor->setMinimum(-std::numeric_limits<float>::max());
        editor->setMaximum(std::numeric_limits<float>::max());
        return editor;
    }

    QVariant getEditorValue(QWidget *editor) {
        QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
        spinBox->interpretText();
        return spinBox->value();
    }

    void setEditorValue(QWidget *editor, QVariant value) {
        QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
        spinBox->setValue(value.toDouble());
    }
private:
    UAVObjectField *m_field;
};

#endif // FIELDTREEITEM_H
