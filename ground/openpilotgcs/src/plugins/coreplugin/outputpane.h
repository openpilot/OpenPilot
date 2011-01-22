/**
 ******************************************************************************
 *
 * @file       outputpane.h
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

#ifndef OUTPUTPANE_H
#define OUTPUTPANE_H

#include "core_global.h"

#include <QtCore/QMap>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE
class QAction;
class QComboBox;
class QToolButton;
class QStackedWidget;
class QPushButton;
QT_END_NAMESPACE

namespace Core {

class IMode;
class IOutputPane;

namespace Internal {
class OutputPaneManager;
class MainWindow;
}


class CORE_EXPORT OutputPanePlaceHolder : public QWidget
{
    friend class Core::Internal::OutputPaneManager; // needs to set m_visible and thus access m_current
    Q_OBJECT
public:
    OutputPanePlaceHolder(Core::IMode *mode, QWidget *parent = 0);
    ~OutputPanePlaceHolder();
    void setCloseable(bool b);
    bool closeable();
    static OutputPanePlaceHolder *getCurrent() { return m_current; }

private slots:
    void currentModeChanged(Core::IMode *);
private:
    Core::IMode *m_mode;
    bool m_closeable;
    static OutputPanePlaceHolder* m_current;
};

namespace Internal {

class OutputPaneManager : public QWidget
{
    Q_OBJECT

public:
    void init();
    static OutputPaneManager *instance();
    void setCloseable(bool b);
    bool closeable();
    QWidget *buttonsWidget();
    void updateStatusButtons(bool visible);

public slots:
    void slotHide();
    void slotNext();
    void slotPrev();
    void shortcutTriggered();

protected:
    void focusInEvent(QFocusEvent *e);

private slots:
    void changePage();
    void showPage(bool focus);
    void togglePage(bool focus);
    void clearPage();
    void updateToolTip();
    void buttonTriggered();
    void updateNavigateState();

private:
    // the only class that is allowed to create and destroy
    friend class MainWindow;

    static void create();
    static void destroy();

    OutputPaneManager(QWidget *parent = 0);
    ~OutputPaneManager();

    void showPage(int idx, bool focus);
    void ensurePageVisible(int idx);
    int findIndexForPage(IOutputPane *out);
    QComboBox *m_widgetComboBox;
    QToolButton *m_clearButton;
    QToolButton *m_closeButton;

    QAction *m_nextAction;
    QAction *m_prevAction;
    QToolButton *m_prevToolButton;
    QToolButton *m_nextToolButton;
    QWidget *m_toolBar;

    QMap<int, Core::IOutputPane*> m_pageMap;
    int m_lastIndex;

    QStackedWidget *m_outputWidgetPane;
    QStackedWidget *m_opToolBarWidgets;
    QWidget *m_buttonsWidget;
    QMap<int, QPushButton *> m_buttons;
    QMap<QAction *, int> m_actions;
};

} // namespace Internal
} // namespace Core

#endif // OUTPUTPANE_H
