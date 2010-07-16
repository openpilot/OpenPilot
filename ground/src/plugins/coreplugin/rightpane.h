/**
 ******************************************************************************
 *
 * @file       rightpane.h
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

#ifndef RIGHTPANE_H
#define RIGHTPANE_H

#include "core_global.h"

#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE
class QSettings;
QT_END_NAMESPACE

namespace Core {

class IMode;
class RightPaneWidget;

// TODO: The right pane works only for the help plugin atm.  It can't cope
// with more than one plugin publishing objects they want in the right pane
// For that the API would need to be different. (Might be that instead of
// adding objects to the pool, there should be a method
// RightPaneWidget::setWidget(QWidget *w) Anyway if a second plugin wants to
// show something there, redesign this API

class CORE_EXPORT RightPanePlaceHolder : public QWidget
{
    friend class Core::RightPaneWidget;
    Q_OBJECT

public:
    RightPanePlaceHolder(Core::IMode *mode, QWidget *parent = 0);
    ~RightPanePlaceHolder();
    static RightPanePlaceHolder *current();

private slots:
    void currentModeChanged(Core::IMode *);

private:
    void applyStoredSize(int width);
    Core::IMode *m_mode;
    static RightPanePlaceHolder* m_current;
};


class CORE_EXPORT BaseRightPaneWidget : public QObject
{
    Q_OBJECT

public:
    BaseRightPaneWidget(QWidget *widget);
    ~BaseRightPaneWidget();
    QWidget *widget() const;

private:
    QWidget *m_widget;
};


class CORE_EXPORT RightPaneWidget : public QWidget
{
    Q_OBJECT

public:
    RightPaneWidget();
    ~RightPaneWidget();

    void saveSettings(QSettings *settings);
    void readSettings(QSettings *settings);

    bool isShown();
    void setShown(bool b);

    static RightPaneWidget *instance();

    int storedWidth();

protected:
    void resizeEvent(QResizeEvent *);

private slots:
    void objectAdded(QObject *obj);
    void aboutToRemoveObject(QObject *obj);

private:
    bool m_shown;
    int m_width;
    static RightPaneWidget *m_instance;
};

} // namespace Core

#endif // RIGHTPANE_H
