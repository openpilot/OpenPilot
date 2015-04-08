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
    setLayout(new QStackedLayout());
    layout()->addWidget(m_view);
}

SplitterOrView::SplitterOrView(SplitterOrView &splitterOrView, QWidget *parent) :
    QWidget(parent),
    m_uavGadgetManager(splitterOrView.m_uavGadgetManager),
    m_view(splitterOrView.m_view),
    m_splitter(splitterOrView.m_splitter)
{
    Q_ASSERT((m_view || m_splitter) && !(m_view && m_splitter));
    setLayout(new QStackedLayout());
    if (m_view) {
        layout()->addWidget(m_view);
    } else if (m_splitter) {
        layout()->addWidget(m_splitter);
    }
}

SplitterOrView::~SplitterOrView()
{}

void SplitterOrView::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) {
        return;
    }
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
            if (SplitterOrView * splitterOrView = qobject_cast<SplitterOrView *>(m_splitter->widget(i))) {
                if (SplitterOrView * result = splitterOrView->findFirstView()) {
                    return result;
                }
            }
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
    if (!uavGadget || hasGadget(uavGadget)) {
        return this;
    }
    if (m_splitter) {
        for (int i = 0; i < m_splitter->count(); ++i) {
            if (SplitterOrView * splitterOrView = qobject_cast<SplitterOrView *>(m_splitter->widget(i))) {
                if (SplitterOrView * result = splitterOrView->findView(uavGadget)) {
                    return result;
                }
            }
        }
    }
    return 0;
}

/* Contract: return SplitterOrView that has 'view', or 0 if not found.
 * Implications: must not return SplitterOrView that is splitter.
 */
SplitterOrView *SplitterOrView::findView(UAVGadgetView *view)
{
    if (view == m_view) {
        return this;
    }
    if (m_splitter) {
        for (int i = 0; i < m_splitter->count(); ++i) {
            if (SplitterOrView * splitterOrView = qobject_cast<SplitterOrView *>(m_splitter->widget(i))) {
                if (SplitterOrView * result = splitterOrView->findView(view)) {
                    return result;
                }
            }
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
            if (SplitterOrView * splitterOrView = qobject_cast<SplitterOrView *>(m_splitter->widget(i))) {
                if (splitterOrView->hasGadget(uavGadget)) {
                    return this;
                }
                if (SplitterOrView * result = splitterOrView->findSplitter(uavGadget)) {
                    return result;
                }
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
            if (SplitterOrView * splitterOrView = qobject_cast<SplitterOrView *>(m_splitter->widget(i))) {
                if (splitterOrView == child) {
                    return this;
                }
                if (SplitterOrView * result = splitterOrView->findSplitter(child)) {
                    return result;
                }
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
            if (SplitterOrView * splitterOrView = qobject_cast<SplitterOrView *>(m_splitter->widget(i))) {
                if (SplitterOrView * result = splitterOrView->findNextView_helper(view, found)) {
                    return result;
                }
            }
        }
    }
    return 0;
}

QSize SplitterOrView::minimumSizeHint() const
{
    if (m_splitter) {
        return m_splitter->minimumSizeHint();
    }
    return QSize(64, 64);
}

QSplitter *SplitterOrView::takeSplitter()
{
    QSplitter *oldSplitter = m_splitter;

    if (m_splitter) {
        layout()->removeWidget(m_splitter);
    }
    m_splitter = 0;
    return oldSplitter;
}

UAVGadgetView *SplitterOrView::takeView()
{
    UAVGadgetView *oldView = m_view;

    if (m_view) {
        layout()->removeWidget(m_splitter);
    }
    m_view = 0;
    return oldView;
}

QList<IUAVGadget *> SplitterOrView::gadgets()
{
    QList<IUAVGadget *> g;
    if (hasGadget()) {
        g.append(gadget());
    }
    if (m_splitter) {
        for (int i = 0; i < m_splitter->count(); ++i) {
            if (SplitterOrView * splitterOrView = qobject_cast<SplitterOrView *>(m_splitter->widget(i))) {
                QList<IUAVGadget *> result = splitterOrView->gadgets();
                g.append(result);
            }
        }
    }
    return g;
}

void SplitterOrView::onSplitterMoved(int pos, int index)
{
    Q_UNUSED(pos);
    Q_UNUSED(index);
    // Update when the splitter is actually moved.
    m_sizes = m_splitter->sizes();
}

void SplitterOrView::split(Qt::Orientation orientation)
{
    Q_ASSERT(m_view);
    Q_ASSERT(!m_splitter);

    MiniSplitter *splitter = new MiniSplitter(this);
    splitter->setOrientation(orientation);
    layout()->addWidget(splitter);

    // [OP-1586] make sure that the view never becomes parent less otherwise a rendering bug happens
    // in osgearth QML views (not all kind of scenes are affected but those containing terrain are)
    // Making the view parent less will destroy the OpenGL context used by the QQuickFramebufferObject used OSGViewport
    // A new OpenGL context will be created but for some reason, osgearth does not switch to it gracefully.
    // Enabling the stats overlay (by pressing the 's' key in the view) will restore proper rendering (?).
    // Note : avoiding to make the view parent less is a workaround... the real cause of the rendering bug needs to be
    // understood and fixed (the same workaround is also need in unsplit and unsplitAll)
    // Important : the changes also apparently make splitting and un-splitting more reactive and less jumpy!

    // Give our view to the new left or top SplitterOrView.
    splitter->addWidget(new SplitterOrView(*this, splitter));
    splitter->addWidget(new SplitterOrView(m_uavGadgetManager));

    m_view     = 0;
    m_splitter = splitter;

    connect(m_splitter, SIGNAL(splitterMoved(int, int)), this, SLOT(onSplitterMoved(int, int)));
}

void SplitterOrView::unsplit(IUAVGadget *gadget)
{
    if (!m_splitter) {
        return;
    }
    SplitterOrView *view = findView(gadget);
    if (!view || view == this) {
        return;
    }

    // find the other gadgets
    // TODO handle case where m_splitter->count() > 2
    SplitterOrView *splitterOrView = NULL;
    for (int i = 0; i < m_splitter->count(); ++i) {
        splitterOrView = qobject_cast<SplitterOrView *>(m_splitter->widget(i));
        if (splitterOrView && (splitterOrView != view)) {
            break;
        }
    }
    if (splitterOrView) {
        if (splitterOrView->isView()) {
            layout()->addWidget(splitterOrView->m_view);
        } else {
            layout()->addWidget(splitterOrView->m_splitter);
        }
        layout()->removeWidget(m_splitter);

        m_uavGadgetManager->emptyView(view->m_view);
        delete view;
        delete m_splitter;

        m_view     = splitterOrView->m_view;
        m_splitter = splitterOrView->m_splitter;
    }
}

void SplitterOrView::unsplitAll(Core::IUAVGadget *gadget)
{
    Q_ASSERT(m_splitter);
    Q_ASSERT(!m_view);

    SplitterOrView *splitterOrView = findView(gadget);
    if (!splitterOrView || splitterOrView == this) {
        return;
    }

    // first re-parent the gadget (see split for an explanation)
    m_view = splitterOrView->m_view;
    layout()->addWidget(m_view);
    layout()->removeWidget(m_splitter);
    // make sure the old m_view is not emptied...
    splitterOrView->m_view = NULL;

    // cleanup
    unsplitAll_helper(m_uavGadgetManager, m_splitter);

    delete m_splitter;
    m_splitter = 0;
}

void SplitterOrView::unsplitAll_helper(UAVGadgetManager *uavGadgetManager, QSplitter *splitter)
{
    for (int i = 0; i < splitter->count(); ++i) {
        if (SplitterOrView * splitterOrView = qobject_cast<SplitterOrView *>(splitter->widget(i))) {
            if (splitterOrView->m_view) {
                uavGadgetManager->emptyView(splitterOrView->m_view);
            }
            if (splitterOrView->m_splitter) {
                unsplitAll_helper(uavGadgetManager, splitterOrView->m_splitter);
            }
            delete splitterOrView;
        }
    }
}

void SplitterOrView::saveState(QSettings *qSettings) const
{
    if (m_splitter) {
        qSettings->setValue("type", "splitter");
        qSettings->setValue("splitterOrientation", (qint32)m_splitter->orientation());
        QList<QVariant> sizesQVariant;
        foreach(int value, m_sizes) {
            sizesQVariant.append(value);
        }
        qSettings->setValue("splitterSizes", sizesQVariant);
        qSettings->beginGroup("side0");
        static_cast<SplitterOrView *>(m_splitter->widget(0))->saveState(qSettings);
        qSettings->endGroup();
        qSettings->beginGroup("side1");
        static_cast<SplitterOrView *>(m_splitter->widget(1))->saveState(qSettings);
        qSettings->endGroup();
    } else if (gadget()) {
        m_view->saveState(qSettings);
    }
}

void SplitterOrView::restoreState(QSettings *qSettings)
{
    QString mode = qSettings->value("type").toString();

    if (mode == "splitter") {
        qint32 orientation = qSettings->value("splitterOrientation").toInt();
        QList<QVariant> sizesQVariant = qSettings->value("splitterSizes").toList();
        m_sizes.clear();
        foreach(QVariant value, sizesQVariant) {
            m_sizes.append(value.toInt());
        }
        split((Qt::Orientation)orientation);
        m_splitter->setSizes(m_sizes);
        qSettings->beginGroup("side0");
        static_cast<SplitterOrView *>(m_splitter->widget(0))->restoreState(qSettings);
        qSettings->endGroup();
        qSettings->beginGroup("side1");
        static_cast<SplitterOrView *>(m_splitter->widget(1))->restoreState(qSettings);
        qSettings->endGroup();
    } else if (mode == "uavGadget") {
        m_view->restoreState(qSettings);
    }
}
