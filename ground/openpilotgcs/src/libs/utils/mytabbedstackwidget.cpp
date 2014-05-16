/**
 ******************************************************************************
 *
 * @file       mytabbedstackwidget.cpp
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
#include "mytabbedstackwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QtCore/QDebug>
#include <QScrollBar>

MyTabbedStackWidget::MyTabbedStackWidget(QWidget *parent, bool isVertical, bool iconAbove)
    : QWidget(parent),
    m_vertical(isVertical),
    m_iconAbove(iconAbove)
{
    m_listWidget  = new QListWidget();
    m_listWidget->setObjectName("list");
    m_stackWidget = new QStackedWidget();

    QBoxLayout *toplevelLayout;
    if (m_vertical) {
        toplevelLayout = new QHBoxLayout();
        toplevelLayout->addWidget(m_listWidget);
        toplevelLayout->addWidget(m_stackWidget);
        m_listWidget->setFlow(QListView::TopToBottom);
        m_listWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        m_listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else {
        toplevelLayout = new QVBoxLayout();
        toplevelLayout->addWidget(m_stackWidget);
        toplevelLayout->addWidget(m_listWidget);
        m_listWidget->setFlow(QListView::LeftToRight);
        m_listWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        m_listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    if (m_iconAbove && m_vertical) {
        m_listWidget->setFixedWidth(LIST_VIEW_WIDTH); // this should be computed instead
        m_listWidget->setWrapping(false);
    }

    m_listWidget->setContentsMargins(0, 0, 0, 0);
    m_listWidget->setSpacing(0);
    m_listWidget->setViewMode(QListView::IconMode);
    m_listWidget->setMovement(QListView::Static);
    m_listWidget->setUniformItemSizes(true);
    m_listWidget->setStyleSheet("#list {border: 0px; margin-left: 9px; margin-top: 9px; background-color: transparent; }");

    m_stackWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    m_stackWidget->setContentsMargins(0, 0, 0, 0);

    toplevelLayout->setSpacing(0);
    toplevelLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(toplevelLayout);

    connect(m_listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(showWidget(int)), Qt::QueuedConnection);
}

void MyTabbedStackWidget::insertTab(const int index, QWidget *tab, const QIcon &icon, const QString &label)
{
    // create and insert item
    QListWidgetItem *item = new QListWidgetItem(icon, label);

    item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    item->setTextAlignment(Qt::AlignHCenter | Qt::AlignBottom);
    item->setToolTip(label);
    m_listWidget->insertItem(index, item);

    // setup and insert widget
    tab->setContentsMargins(0, 0, 0, 0);
    m_stackWidget->insertWidget(index, tab);
}

void MyTabbedStackWidget::replaceTab(int index, QWidget *tab)
{
    QWidget *wid = m_stackWidget->widget(index);

    // setup and insert new widget
    tab->setContentsMargins(0, 0, 0, 0);
    m_stackWidget->insertWidget(index, tab);
    // check special case when replacing currenlty selected tab
    if (index == currentIndex()) {
        // currently selected tab is being replaced so select the new tab before removing the old one
        m_stackWidget->setCurrentWidget(tab);
    }
    // remove and delete old widget
    m_stackWidget->removeWidget(wid);
    delete wid;
}

void MyTabbedStackWidget::removeTab(int index)
{
    QWidget *wid = m_stackWidget->widget(index);

    m_stackWidget->removeWidget(wid);
    delete wid;

    QListWidgetItem *item = m_listWidget->item(index);
    m_listWidget->removeItemWidget(item);
    delete item;
}

void MyTabbedStackWidget::setWidgetsEnabled(bool enabled)
{
    for (int i = 0; i < m_stackWidget->count(); i++) {
        m_stackWidget->widget(i)->setEnabled(enabled);
    }
}

int MyTabbedStackWidget::currentIndex() const
{
    return m_listWidget->currentRow();
}

void MyTabbedStackWidget::setCurrentIndex(int index)
{
    m_listWidget->setCurrentRow(index);
}

void MyTabbedStackWidget::showWidget(int index)
{
    if (m_stackWidget->currentIndex() == index) {
        return;
    }
    bool proceed = false;
    emit currentAboutToShow(index, &proceed);
    if (proceed) {
        m_stackWidget->setCurrentIndex(index);
        emit currentChanged(index);
    } else {
        m_listWidget->setCurrentRow(m_stackWidget->currentIndex(), QItemSelectionModel::ClearAndSelect);
    }
}

void MyTabbedStackWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    m_listWidget->setFixedWidth(m_listWidget->verticalScrollBar()->isVisible() ? LIST_VIEW_WIDTH + 20 : LIST_VIEW_WIDTH);
}

void MyTabbedStackWidget::insertCornerWidget(int index, QWidget *widget)
{
    Q_UNUSED(index);

    widget->hide();
}
