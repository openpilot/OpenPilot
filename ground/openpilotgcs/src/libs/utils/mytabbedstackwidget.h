/**
 ******************************************************************************
 *
 * @file       mytabbedstackwidget.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup
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

#ifndef MYTABBEDSTACKWIDGET_H
#define MYTABBEDSTACKWIDGET_H

#include "utils/mylistwidget.h"
#include <QtGui/QStackedWidget>

/*
 * MyTabbedStackWidget is a MyListWidget combined with a QStackedWidget,
 * similar in function to QTabWidget.
 */
class QTCREATOR_UTILS_EXPORT MyTabbedStackWidget : public QWidget
{
    Q_OBJECT

public:
    MyTabbedStackWidget(QWidget *parent = 0, bool isVertical = false, bool iconAbove = true);

    void insertTab(int index, QWidget *tab, const QIcon &icon, const QString &label);
    void removeTab(int index);
    void setIconSize(int size) { m_listWidget->setIconSize(QSize(size, size)); }

    int currentIndex() const;

    void insertCornerWidget(int index, QWidget *widget);
    int cornerWidgetCount() { return m_cornerWidgetCount; }
    QWidget * currentWidget(){return m_stackWidget->currentWidget();}
    QWidget * getWidget(int index) {return m_stackWidget->widget(index);}

signals:
    void currentAboutToShow(int index,bool * proceed);
    void currentChanged(int index);

public slots:
    void setCurrentIndex(int index);

private slots:
    void showWidget(int index);

private:
    MyListWidget *m_listWidget;
    QStackedWidget *m_stackWidget;
    QWidget *m_selectionWidget;
    bool m_vertical;
    bool m_iconAbove;
    int m_cornerWidgetCount;
};

#endif // MYTABBEDSTACKWIDGET_H
