/**
 ******************************************************************************
 *
 * @file       doublespindelegate.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup ConfigPlugin Config Plugin
 * @{
 * @brief A double spinbox delegate
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
#ifndef DOUBLESPINDELEGATE_H
#define DOUBLESPINDELEGATE_H

#include <QDoubleSpinBox>
#include <QItemDelegate>

namespace Ui {
class DoubleSpinDelegate;
}

class DoubleSpinDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    DoubleSpinDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setMin(double min) { m_min = min; }
    void setMax(double max) { m_max = max; }
    void setRange(double min, double max) { m_min = min; m_max = max; }
    void setStep(double step) { m_step = step; }
    void setDecimals(int decimals) { m_decimals = decimals; }

private:
    double  m_min;
    double  m_max;
    double  m_step;
    int     m_decimals;

signals:
    void ValueChanged();

private slots:
    void valueChanged();
};

#endif // DOUBLESPINDELEGATE_H
