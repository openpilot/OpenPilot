/**
 ******************************************************************************
 *
 * @file       uavgadgetview.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   coreplugin
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

#include "uavgadgetview.h"
#include "uavgadgetmanager.h"
#include "iuavgadgetfactory.h"
#include "iuavgadget.h"
#include "coreimpl.h"
#include "minisplitter.h"
#include <coreplugin/coreconstants.h>
#include <coreplugin/actionmanager/actionmanager.h>

#include <utils/qtcassert.h>
#include <utils/styledbar.h>

#include <QtCore/QDebug>

#include <QtGui/QApplication>
#include <QtGui/QComboBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QStyle>
#include <QtGui/QStyleOption>
#include <QtGui/QToolButton>
#include <QtGui/QMenu>
#include <QtGui/QClipboard>

#ifdef Q_WS_MAC
#include <qmacstyle_mac.h>
#endif

Q_DECLARE_METATYPE(Core::IUAVGadget *)

using namespace Core;
using namespace Core::Internal;

// ================UAVGadgetView====================

UAVGadgetView::UAVGadgetView(UAVGadgetManager *uavGadgetManager, IUAVGadget *uavGadget, QWidget *parent) :
        QWidget(parent),
        m_uavGadgetManager(uavGadgetManager),
        m_uavGadget(uavGadget),
        m_toolBar(new QWidget),
        m_defaultToolBar(new QWidget(this)),
        m_uavGadgetList(new QComboBox),
        m_closeButton(new QToolButton),
        m_defaultIndex(0)
{

    tl = new QVBoxLayout(this);
    tl->setSpacing(0);
    tl->setMargin(0);
    {
        m_uavGadgetList->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_uavGadgetList->setMinimumContentsLength(20);
        m_uavGadgetList->setMaxVisibleItems(40);
        m_uavGadgetList->setContextMenuPolicy(Qt::CustomContextMenu);
        UAVGadgetFactoryList fl = m_uavGadgetManager->uavGadgetFactories();
        int index = 0;
        foreach(Core::IUAVGadgetFactory *factory, fl)
        {
            m_uavGadgetList->addItem(factory->name(), factory->gadgetKind());
            if (factory->gadgetKind() == QString("EmptyGadget"))
                m_defaultIndex = index;
            ++index;
        }

        m_defaultToolBar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        m_activeToolBar = m_defaultToolBar;

        QHBoxLayout *toolBarLayout = new QHBoxLayout;
        toolBarLayout->setMargin(0);
        toolBarLayout->setSpacing(0);
        toolBarLayout->addWidget(m_defaultToolBar);
        m_toolBar->setLayout(toolBarLayout);
        m_toolBar->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

        m_closeButton->setAutoRaise(true);
        m_closeButton->setIcon(QIcon(":/core/images/closebutton.png"));

        QHBoxLayout *toplayout = new QHBoxLayout;
        toplayout->setSpacing(0);
        toplayout->setMargin(0);
        toplayout->addWidget(m_uavGadgetList);
        toplayout->addWidget(m_toolBar, 1); // Custom toolbar stretches
        toplayout->addWidget(m_closeButton);

        m_top = new Utils::StyledBar;
        m_top->setLayout(toplayout);
        tl->addWidget(m_top);

        connect(m_uavGadgetList, SIGNAL(activated(int)), this, SLOT(listSelectionActivated(int)));
        connect(m_closeButton, SIGNAL(clicked()), this, SLOT(closeView()), Qt::QueuedConnection);
    }
    if (m_uavGadget) {
        setUAVGadget(m_uavGadget);
    } else {
        listSelectionActivated(m_defaultIndex);
    }
}

UAVGadgetView::~UAVGadgetView()
{
}

bool UAVGadgetView::hasUAVGadget(IUAVGadget *uavGadget) const
{
    return (m_uavGadget == uavGadget);
}

void UAVGadgetView::showToolbar(bool show)
{
    m_top->setHidden(!show);
}

void UAVGadgetView::closeView()
{
    m_uavGadgetManager->closeView(this);
}

void UAVGadgetView::removeUAVGadget()
{
    if (!m_uavGadget)
        return;
    tl->removeWidget(m_uavGadget->widget());

    m_uavGadget->widget()->setParent(0);
    QWidget *toolBar = m_uavGadget->toolBar();
    if (toolBar != 0) {
        if (m_activeToolBar == toolBar) {
            m_activeToolBar = m_defaultToolBar;
            m_activeToolBar->setVisible(true);
        }
        m_toolBar->layout()->removeWidget(toolBar);
        toolBar->setVisible(false);
        toolBar->setParent(0);
    }
    m_uavGadget = 0;
}

IUAVGadget *UAVGadgetView::uavGadget() const
{
    return m_uavGadget;
}

void UAVGadgetView::setUAVGadget(IUAVGadget *uavGadget)
{
    if (!uavGadget) {
        return;
    }
    removeUAVGadget();
    m_uavGadget = uavGadget;
    tl->addWidget(m_uavGadget->widget());
    m_uavGadget->widget()->setParent(this);
    m_uavGadget->widget()->show();
    int index = indexOfGadgetKind(m_uavGadget->gadgetKind());
    Q_ASSERT(index >= 0);
    m_uavGadgetList->setCurrentIndex(index);

//    updateToolBar();
}

void UAVGadgetView::updateToolBar()
{
    if (!m_uavGadget)
        return;
    QWidget *toolBar = m_uavGadget->toolBar();
    if (!toolBar)
        toolBar = m_defaultToolBar;
    if (m_activeToolBar == toolBar)
        return;
    toolBar->setVisible(true);
    m_activeToolBar->setVisible(false);
    m_activeToolBar = toolBar;
}

void UAVGadgetView::listSelectionActivated(int index)
{
    if (index < 0 || m_uavGadgetManager->uavGadgetFactories().count() == 0)
        return;
    IUAVGadgetFactory *factory = m_uavGadgetManager->uavGadgetFactories().at(index);
    if (m_uavGadget && m_uavGadget->gadgetKind() == factory->gadgetKind())
        return;
    IUAVGadget *gadget = factory->createUAVGadget(this);
    setUAVGadget(gadget);
    m_uavGadgetManager->setCurrentUAVGadget(gadget);
}

int UAVGadgetView::indexOfGadgetKind(QString gadgetKind)
{
    return m_uavGadgetList->findData(gadgetKind);
}


SplitterOrView::SplitterOrView(UAVGadgetManager *uavGadgetManager, Core::IUAVGadget *uavGadget, bool isRoot) :
        m_uavGadgetManager(uavGadgetManager),
        m_isRoot(isRoot)
{
    m_view = new UAVGadgetView(m_uavGadgetManager, uavGadget);
    m_layout = new QStackedLayout(this);
    m_splitter = 0;
    m_layout->addWidget(m_view);
}

SplitterOrView::~SplitterOrView()
{
    delete m_view;
    m_view = 0;
    delete m_splitter;
    m_splitter = 0;
}

void SplitterOrView::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton)
        return;
    if (uavGadget()) {
        setFocus(Qt::MouseFocusReason);
        m_uavGadgetManager->setCurrentUAVGadget(this->uavGadget());
    }
}

void SplitterOrView::paintEvent(QPaintEvent *event)
{
    if (m_uavGadgetManager->currentSplitterOrView() != this)
        return;

    if (!m_view)
        return;

//    if (hasUAVGadget())
//        return;

    if (m_uavGadgetManager->toolbarsShown())
        return;

    // Discreet indication where an uavGadget would be if there is none
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    QColor shadeBrush(Qt::black);
    shadeBrush.setAlpha(25);
    painter.setBrush(shadeBrush);
    const int r = 3;
    painter.drawRoundedRect(rect().adjusted(r, r, -r, -r), r * 2, r * 2);

#if 0
    if (hasFocus()) {
#ifdef Q_WS_MAC
        // With QMacStyle, we have to draw our own focus rect, since I didn't find
        // a way to draw the nice mac focus rect _inside_ this widget
        if (qobject_cast<QMacStyle *>(style())) {
            painter.setPen(Qt::DotLine);
            painter.setBrush(Qt::NoBrush);
            painter.setOpacity(0.75);
            painter.drawRect(rect());
        } else {
#endif
            QStyleOptionFocusRect option;
            option.initFrom(this);
            option.backgroundColor = palette().color(QPalette::Background);

            // Some styles require a certain state flag in order to draw the focus rect
            option.state |= QStyle::State_KeyboardFocusChange;

            style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, &painter);
#ifdef Q_WS_MAC
        }
#endif
    }
#endif
}

/* Contract: return SplitterOrView that is not splitter, or 0 if not found.
 * Implications: must not return SplitterOrView that is splitter.
 */
SplitterOrView *SplitterOrView::findFirstView()
{
    if (m_splitter) {
        for (int i = 0; i < m_splitter->count(); ++i) {
            if (SplitterOrView *splitterOrView = qobject_cast<SplitterOrView*>(m_splitter->widget(i)))
                if (SplitterOrView *result = splitterOrView->findFirstView())
                    return result;
        }
        return 0;
    }
    return this;
}

/* Contract: return SplitterOrView that has 'uavGadget', or 0 if not found.
 * Implications: must not return SplitterOrView that is splitter.
 */
SplitterOrView *SplitterOrView::findView(Core::IUAVGadget *uavGadget)
{
    if (!uavGadget || hasUAVGadget(uavGadget))
        return this;
    if (m_splitter) {
        for (int i = 0; i < m_splitter->count(); ++i) {
            if (SplitterOrView *splitterOrView = qobject_cast<SplitterOrView*>(m_splitter->widget(i)))
                if (SplitterOrView *result = splitterOrView->findView(uavGadget))
                    return result;
        }
    }
    return 0;
}

/* Contract: return SplitterOrView that has 'view', or 0 if not found.
 * Implications: must not return SplitterOrView that is splitter.
 */
SplitterOrView *SplitterOrView::findView(UAVGadgetView *view)
{
    if (view == m_view)
        return this;
    if (m_splitter) {
        for (int i = 0; i < m_splitter->count(); ++i) {
            if (SplitterOrView *splitterOrView = qobject_cast<SplitterOrView*>(m_splitter->widget(i)))
                if (SplitterOrView *result = splitterOrView->findView(view))
                    return result;
        }
    }
    return 0;
}

/* Contract: return SplitterOrView that is splitter that has as child SplitterOrView containing 'uavGadget',
 * or 0 if not found.
 * Implications: must return SplitterOrView that is splitter.
 */
SplitterOrView *SplitterOrView::findSplitter(Core::IUAVGadget *uavGadget)
{
    if (m_splitter) {
        for (int i = 0; i < m_splitter->count(); ++i) {
            if (SplitterOrView *splitterOrView = qobject_cast<SplitterOrView*>(m_splitter->widget(i))) {
                if (splitterOrView->hasUAVGadget(uavGadget))
                    return this;
                if (SplitterOrView *result = splitterOrView->findSplitter(uavGadget))
                    return result;
            }
        }
    }
    return 0;
}

/* Contract: return SplitterOrView that is splitter that has as child SplitterOrView 'child',
 * or 0 if not found.
 * Implications: must return SplitterOrView that is splitter.
 */
SplitterOrView *SplitterOrView::findSplitter(SplitterOrView *child)
{
    if (m_splitter) {
        for (int i = 0; i < m_splitter->count(); ++i) {
            if (SplitterOrView *splitterOrView = qobject_cast<SplitterOrView*>(m_splitter->widget(i))) {
                if (splitterOrView == child)
                    return this;
                if (SplitterOrView *result = splitterOrView->findSplitter(child))
                    return result;
            }
        }
    }
    return 0;
}

/* Contract: return SplitterOrView that follows SplitterOrView 'view' in tree structure,
 * or 0 if not found.
 * Implications: must not return SplitterOrView that is splitter.
 */
SplitterOrView *SplitterOrView::findNextView(SplitterOrView *view)
{
    bool found = false;
    return findNextView_helper(view, &found);
}

SplitterOrView *SplitterOrView::findNextView_helper(SplitterOrView *view, bool *found)
{
    if (*found && m_view) {
        return this;
    }

    if (this == view) {
        *found = true;
        return 0;
    }

    if (m_splitter) {
        for (int i = 0; i < m_splitter->count(); ++i) {
            if (SplitterOrView *splitterOrView = qobject_cast<SplitterOrView*>(m_splitter->widget(i))) {
                if (SplitterOrView *result = splitterOrView->findNextView_helper(view, found))
                    return result;
            }
        }
    }
    return 0;
}

QSize SplitterOrView::minimumSizeHint() const
{
    if (m_splitter)
        return m_splitter->minimumSizeHint();
    return QSize(64, 64);
}

QSplitter *SplitterOrView::takeSplitter()
{
    QSplitter *oldSplitter = m_splitter;
    if (m_splitter)
        m_layout->removeWidget(m_splitter);
    m_splitter = 0;
    return oldSplitter;
}

UAVGadgetView *SplitterOrView::takeView()
{
    UAVGadgetView *oldView = m_view;
    if (m_view)
        m_layout->removeWidget(m_view);
    m_view = 0;
    return oldView;
}

void SplitterOrView::split(Qt::Orientation orientation)
{
    Q_ASSERT(m_view && (m_splitter == 0));
    m_splitter = new MiniSplitter(this);
    m_splitter->setOrientation(orientation);
    m_layout->addWidget(m_splitter);
    Core::IUAVGadget *e = m_view->uavGadget();

    SplitterOrView *view = 0;
    SplitterOrView *otherView = 0;
    if (e) {
        m_view->removeUAVGadget();
        m_splitter->addWidget((view = new SplitterOrView(m_uavGadgetManager, e)));
        m_splitter->addWidget((otherView = new SplitterOrView(m_uavGadgetManager)));
    } else {
        m_splitter->addWidget((otherView = new SplitterOrView(m_uavGadgetManager)));
        m_splitter->addWidget((view = new SplitterOrView(m_uavGadgetManager)));
    }

    m_layout->setCurrentWidget(m_splitter);

    if (m_view && !m_isRoot) {
        m_uavGadgetManager->emptyView(m_view);
        delete m_view;
        m_view = 0;
    }
}

void SplitterOrView::unsplitAll()
{
    m_splitter->hide();
    m_layout->removeWidget(m_splitter); // workaround Qt bug
    unsplitAll_helper();
    delete m_splitter;
    m_splitter = 0;
}

void SplitterOrView::unsplitAll_helper()
{
    if (!m_isRoot && m_view)
        m_uavGadgetManager->emptyView(m_view);
    if (m_splitter) {
        for (int i = 0; i < m_splitter->count(); ++i) {
            if (SplitterOrView *splitterOrView = qobject_cast<SplitterOrView*>(m_splitter->widget(i))) {
                splitterOrView->unsplitAll_helper();
            }
        }
    }
}

void SplitterOrView::unsplit()
{
    if (!m_splitter)
        return;
    Q_ASSERT(m_splitter->count() == 1);
    SplitterOrView *childSplitterOrView = qobject_cast<SplitterOrView*>(m_splitter->widget(0));
    QSplitter *oldSplitter = m_splitter;
    m_splitter = 0;

    if (childSplitterOrView->isSplitter()) {
        Q_ASSERT(childSplitterOrView->view() == 0);
        m_splitter = childSplitterOrView->takeSplitter();
        m_layout->addWidget(m_splitter);
        m_layout->setCurrentWidget(m_splitter);
    } else {
        UAVGadgetView *childView = childSplitterOrView->view();
        Q_ASSERT(childView);
        if (m_view) {
            if (IUAVGadget *e = childView->uavGadget()) {
                childView->removeUAVGadget();
                m_view->setUAVGadget(e);
            }
            m_uavGadgetManager->emptyView(childView);
        } else {
            m_view = childSplitterOrView->takeView();
            m_layout->addWidget(m_view);
        }
        m_layout->setCurrentWidget(m_view);
    }
    delete oldSplitter;
    m_uavGadgetManager->setCurrentUAVGadget(findFirstView()->uavGadget());
}


QByteArray SplitterOrView::saveState() const
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);

    if (m_splitter) {
        stream << QByteArray("splitter")
                << (qint32)m_splitter->orientation()
                << m_splitter->saveState()
                << static_cast<SplitterOrView*>(m_splitter->widget(0))->saveState()
                << static_cast<SplitterOrView*>(m_splitter->widget(1))->saveState();
    } else {
        if (uavGadget())
            stream << QByteArray("uavGadget") << uavGadget()->gadgetKind() << uavGadget()->saveState();
    }
    return bytes;
}

void SplitterOrView::restoreState(const QByteArray &state)
{
    QDataStream stream(state);
    QByteArray mode;
    stream >> mode;
    if (mode == "splitter") {
        qint32 orientation;
        QByteArray splitter, first, second;
        stream >> orientation >> splitter >> first >> second;
        split((Qt::Orientation)orientation);
        m_splitter->restoreState(splitter);
        static_cast<SplitterOrView*>(m_splitter->widget(0))->restoreState(first);
        static_cast<SplitterOrView*>(m_splitter->widget(1))->restoreState(second);
    } else if (mode == "uavGadget") {
        QString gadgetKind;
        QByteArray uavGadgetState;
        stream >> gadgetKind >> uavGadgetState;
        m_view = new UAVGadgetView(m_uavGadgetManager, 0);
        int index = m_view->indexOfGadgetKind(gadgetKind);
        m_view->listSelectionActivated(index);
        m_layout->addWidget(m_view);
        m_layout->setCurrentWidget(m_view);
        uavGadget()->restoreState(uavGadgetState);
    }
}
