/**
 ******************************************************************************
 *
 * @file       mytabwidget.h
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
#ifndef MYTABWIDGET_H
#define MYTABWIDGET_H

#include "utils_global.h"

#include <QtGui/QTabWidget>

/*
 * MyTabWidget is a plain QTabWidget with the addition of the signal
 * tabMoved(int, int) which QTabBar has but for some reason is
 * not made available from QTabWidget.
 */
class QTCREATOR_UTILS_EXPORT MyTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    MyTabWidget(QWidget *parent = 0);
    void moveTab(int from, int to);

private slots:
    void myTabMoved(int from, int to);

signals:
    void tabMoved(int from, int to);
};

#endif // MYTABWIDGET_H
