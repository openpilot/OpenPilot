/**
 ******************************************************************************
 *
 * @file       uavgadgetview.h
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

#ifndef UAVGADGETVIEW_H
#define UAVGADGETVIEW_H

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QSettings>
#include <QtGui/QWidget>
#include <QtGui/QAction>
#include <QtGui/QSplitter>
#include <QtGui/QVBoxLayout>
#include <QtGui/QStackedLayout>
#include <QtCore/QPointer>


QT_BEGIN_NAMESPACE
class QComboBox;
class QToolButton;
class QLabel;
class QVBoxLayout;
QT_END_NAMESPACE

namespace Utils {
class StyledBar;
}

namespace Core {

class IUAVGadget;
class UAVGadgetManager;

namespace Internal {


class UAVGadgetView : public QWidget
{
    Q_OBJECT

public:
    UAVGadgetView(UAVGadgetManager *uavGadgetManager, IUAVGadget *uavGadget = 0, QWidget *parent = 0);
    virtual ~UAVGadgetView();

    void removeGadget();
    IUAVGadget *gadget() const;
    void setGadget(IUAVGadget *uavGadget);
    bool hasGadget(IUAVGadget *uavGadget) const;
    int indexOfClassId(QString classId);
    void showToolbar(bool show);

public slots:
    void closeView();
    void listSelectionActivated(int index);

private slots:
    void currentGadgetChanged(IUAVGadget *gadget);

private:
    void updateToolBar();

    UAVGadgetManager *m_uavGadgetManager;
    IUAVGadget *m_uavGadget;
    QWidget *m_toolBar;
    QComboBox *m_defaultToolBar;
    QWidget *m_currentToolBar;
    QWidget *m_activeToolBar;
    QComboBox *m_uavGadgetList;
    QToolButton *m_closeButton;
    Utils::StyledBar *m_top;
    QVBoxLayout *tl; // top layout
    int m_defaultIndex;
    QLabel *m_activeLabel;
};

class SplitterOrView  : public QWidget
{
    Q_OBJECT
public:
    SplitterOrView(UAVGadgetManager *uavGadgetManager, Core::IUAVGadget *uavGadget = 0, bool root = false);
    ~SplitterOrView();

    void split(Qt::Orientation orientation);
    void unsplit();

    inline bool isView() const { return m_view != 0; }
    inline bool isRoot() const { return m_isRoot; }
    
    inline bool isSplitter() const { return m_splitter != 0; }
    inline Core::IUAVGadget *gadget() const { return m_view ? m_view->gadget() : 0; }
    inline bool hasGadget(Core::IUAVGadget *uavGadget) const { return m_view && m_view->hasGadget(uavGadget); }
    inline bool hasGadget() const { return m_view && (m_view->gadget() != 0); }
    inline UAVGadgetView *view() const { return m_view; }
    inline QSplitter *splitter() const { return m_splitter; }
    QList<Core::IUAVGadget*> gadgets();
    QSplitter *takeSplitter();
    UAVGadgetView *takeView();

    QByteArray saveState() const;
    void restoreState(const QByteArray &);

    SplitterOrView *findView(Core::IUAVGadget *uavGadget);
    SplitterOrView *findView(UAVGadgetView *view);
    SplitterOrView *findFirstView();
    SplitterOrView *findSplitter(Core::IUAVGadget *uavGadget);
    SplitterOrView *findSplitter(SplitterOrView *child);

    SplitterOrView *findNextView(SplitterOrView *view);

    QSize sizeHint() const { return minimumSizeHint(); }
    QSize minimumSizeHint() const;

    void unsplitAll();

protected:
//    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *e);

private:
    void unsplitAll_helper();
    SplitterOrView *findNextView_helper(SplitterOrView *view, bool *found);
    UAVGadgetManager *m_uavGadgetManager;
    bool m_isRoot;
    QStackedLayout *m_layout;
    UAVGadgetView *m_view;
    QSplitter *m_splitter;
};

}
}
#endif // UAVGADGETVIEW_H
