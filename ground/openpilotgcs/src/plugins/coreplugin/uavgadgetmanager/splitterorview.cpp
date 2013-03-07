/**
 ******************************************************************************
 *
 * @file       splitterorview.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @addtogroup GCSPlugins GCS Plugins
 * @{
 * @addtogroup CorePlugin Core Plugin
 * @{
 * @brief The Core GCS plugin
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

#include "splitterorview.h"
#include "uavgadgetview.h"
#include "uavgadgetmanager.h"
#include "iuavgadget.h"
#include "minisplitter.h"

#include <QtCore/QDebug>


#ifdef Q_WS_MAC
#include <qmacstyle_mac.h>
#endif

using namespace Core;
using namespace Core::Internal;

SplitterOrView::SplitterOrView(Core::UAVGadgetManager *uavGadgetManager, Core::IUAVGadget *uavGadget) :
        m_uavGadgetManager(uavGadgetManager),
        m_splitter(0)
{
    m_view = new UAVGadgetView(m_uavGadgetManager, uavGadget, this);
    m_layout = new QStackedLayout(this);
    m_layout->addWidget(m_view);
}

SplitterOrView::~SplitterOrView()
{
    if(m_view) {
        delete m_view;
        m_view = 0;
    }
    if(m_splitter) {
        delete m_splitter;
        m_splitter = 0;
    }
}

void SplitterOrView::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton)
        return;
    if (gadget()) {
        setFocus(Qt::MouseFocusReason);
        m_uavGadgetManager->setCurrentGadget(this->gadget());
    }
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
    if (!uavGadget || hasGadget(uavGadget))
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
                if (splitterOrView->hasGadget(uavGadget))
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

QList<IUAVGadget*> SplitterOrView::gadgets()
{
    QList<IUAVGadget*> g;
    if (hasGadget())
        g.append(gadget());
    if (m_splitter) {
        for (int i = 0; i < m_splitter->count(); ++i) {
            if (SplitterOrView *splitterOrView = qobject_cast<SplitterOrView*>(m_splitter->widget(i))) {
                QList<IUAVGadget*> result = splitterOrView->gadgets();
                g.append(result);
            }
        }
    }
    return g;
}

void SplitterOrView::split(Qt::Orientation orientation)
{
    Q_ASSERT(m_view);
    Q_ASSERT(!m_splitter);
    m_splitter = new MiniSplitter(this);
    m_splitter->setOrientation(orientation);
    connect(m_splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(onSplitterMoved(int,int)));
    m_layout->addWidget(m_splitter);
    Core::IUAVGadget *ourGadget = m_view->gadget();

    if (ourGadget) {
        // Give our gadget to the new left or top SplitterOrView.
        m_view->removeGadget();
        m_splitter->addWidget(new SplitterOrView(m_uavGadgetManager, ourGadget));
        m_splitter->addWidget(new SplitterOrView(m_uavGadgetManager));
    } else {
        m_splitter->addWidget(new SplitterOrView(m_uavGadgetManager));
        m_splitter->addWidget(new SplitterOrView(m_uavGadgetManager));
    }

    m_layout->setCurrentWidget(m_splitter);

    if (m_view) {
        m_uavGadgetManager->emptyView(m_view);
        delete m_view;
        m_view = 0;
    }
}

void SplitterOrView::onSplitterMoved( int pos, int index ) {
    Q_UNUSED(pos);
    Q_UNUSED(index);
    // Update when the splitter is actually moved.
    m_sizes = m_splitter->sizes();
}

void SplitterOrView::unsplitAll(IUAVGadget *currentGadget)
{
    Q_ASSERT(m_splitter);
    Q_ASSERT(!m_view);
    m_splitter->hide();
    m_layout->removeWidget(m_splitter); // workaround Qt bug
    unsplitAll_helper();
    delete m_splitter;
    m_splitter = 0;

    m_view = new UAVGadgetView(m_uavGadgetManager, currentGadget, this);
    m_layout->addWidget(m_view);
}

void SplitterOrView::unsplitAll_helper()
{
    if (m_view) {
        m_uavGadgetManager->emptyView(m_view);
    }
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
            if (IUAVGadget *e = childView->gadget()) {
                childView->removeGadget();
                m_view->setGadget(e);
            }
            m_uavGadgetManager->emptyView(childView);
        } else {
            m_view = childSplitterOrView->takeView();
            m_layout->addWidget(m_view);
        }
        m_layout->setCurrentWidget(m_view);
    }
    delete oldSplitter;
    m_uavGadgetManager->setCurrentGadget(findFirstView()->gadget());
}

void SplitterOrView::saveState(QSettings* qSettings) const {
    if (m_splitter) {
        qSettings->setValue("type", "splitter");
        qSettings->setValue("splitterOrientation", (qint32)m_splitter->orientation());
        QList<QVariant> sizesQVariant;
        foreach (int value, m_sizes) {
            sizesQVariant.append(value);
        }
        qSettings->setValue("splitterSizes", sizesQVariant);
        qSettings->beginGroup("side0");
        static_cast<SplitterOrView*>(m_splitter->widget(0))->saveState(qSettings);
        qSettings->endGroup();
        qSettings->beginGroup("side1");
        static_cast<SplitterOrView*>(m_splitter->widget(1))->saveState(qSettings);
        qSettings->endGroup();
    } else if (gadget()) {
        m_view->saveState(qSettings);
    }
}

void SplitterOrView::restoreState(QSettings* qSettings)
{
    QString mode = qSettings->value("type").toString();
    if (mode == "splitter") {
        qint32 orientation = qSettings->value("splitterOrientation").toInt();
        QList<QVariant> sizesQVariant = qSettings->value("splitterSizes").toList();
        m_sizes.clear();
        foreach (QVariant value, sizesQVariant) {
            m_sizes.append(value.toInt());
        }
        split((Qt::Orientation)orientation);
        m_splitter->setSizes(m_sizes);
        qSettings->beginGroup("side0");
        static_cast<SplitterOrView*>(m_splitter->widget(0))->restoreState(qSettings);
        qSettings->endGroup();
        qSettings->beginGroup("side1");
        static_cast<SplitterOrView*>(m_splitter->widget(1))->restoreState(qSettings);
        qSettings->endGroup();
    } else if (mode == "uavGadget") {
        m_view->restoreState(qSettings);
    }
}
