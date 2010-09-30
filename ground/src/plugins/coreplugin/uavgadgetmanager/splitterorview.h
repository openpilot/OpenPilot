/**
 ******************************************************************************
 *
 * @file       splitterorview.h
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

#ifndef SPLITTERORVIEW_H
#define SPLITTERORVIEW_H

#include <QWidget>
#include <QtGui/QMouseEvent>
#include "uavgadgetmanager.h"
#include "uavgadgetview.h"

namespace Core {

namespace Internal {

class SplitterOrView  : public QWidget
{
    Q_OBJECT
public:
    SplitterOrView(UAVGadgetManager *uavGadgetManager, Core::IUAVGadget *uavGadget = 0);
    ~SplitterOrView();

    void split(Qt::Orientation orientation);
    void unsplit();

    inline bool isView() const { return m_view != 0; }

    inline bool isSplitter() const { return m_splitter != 0; }
    inline Core::IUAVGadget *gadget() const { return m_view ? m_view->gadget() : 0; }
    inline bool hasGadget(Core::IUAVGadget *uavGadget) const { return m_view && m_view->hasGadget(uavGadget); }
    inline bool hasGadget() const { return m_view && (m_view->gadget() != 0); }
    inline UAVGadgetView *view() const { return m_view; }
    inline QSplitter *splitter() const { return m_splitter; }
    QList<Core::IUAVGadget*> gadgets();
    QSplitter *takeSplitter();
    UAVGadgetView *takeView();

    void saveState(QSettings*) const;
    void restoreState(QSettings*);

    SplitterOrView *findView(Core::IUAVGadget *uavGadget);
    SplitterOrView *findView(UAVGadgetView *view);
    SplitterOrView *findFirstView();
    SplitterOrView *findSplitter(Core::IUAVGadget *uavGadget);
    SplitterOrView *findSplitter(SplitterOrView *child);

    SplitterOrView *findNextView(SplitterOrView *view);

    QSize sizeHint() const { return minimumSizeHint(); }
    QSize minimumSizeHint() const;

    void unsplitAll(IUAVGadget *currentGadget);

protected:
//    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *e);

private slots:
    // Called when the user moves the splitter, and updates our m_sizes.
    void onSplitterMoved( int pos, int index );

private:
    void unsplitAll_helper();
    SplitterOrView *findNextView_helper(SplitterOrView *view, bool *found);

    // The gadget manager that controls us.
    QPointer<UAVGadgetManager> m_uavGadgetManager;

    // Our layout, we use stacked so we can change stuff without visual artifacts (I think...)
    QPointer<QStackedLayout> m_layout;

    // Our view, if we are a view (showing 1 gadget) and not a splitter.
    QPointer<UAVGadgetView> m_view;

    // Out splitter, if we are a splitter.
    QPointer<QSplitter> m_splitter;

    // The splitter sizes. We keep our own copy of these, since after loading they can't realiably be retrieved.
    QList<int> m_sizes;
};


}
}
#endif // SPLITTERORVIEW_H
