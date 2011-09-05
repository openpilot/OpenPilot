/**
 ******************************************************************************
 *
 * @file       mytabwidget.cpp
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
#include "mytabwidget.h"
#include <QtGui/QTabBar>

MyTabWidget::MyTabWidget(QWidget *parent)
    : QTabWidget(parent)
{
    QTabBar *tabBar = QTabWidget::tabBar();
    connect(tabBar, SIGNAL(tabMoved(int, int)), this, SLOT(myTabMoved(int,int)));
}

void MyTabWidget::moveTab(int from, int to)
{
    QTabBar *tabBar = QTabWidget::tabBar();
    tabBar->moveTab(from, to);
}


void MyTabWidget::myTabMoved(int from, int to)
{
    emit tabMoved(from, to);
}
