/**
 ******************************************************************************
 *
 * @file       ioutputpane.h
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

#ifndef IOUTPUTPANE_H
#define IOUTPUTPANE_H

#include "core_global.h"

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

namespace Core {

class CORE_EXPORT IOutputPane : public QObject
{
    Q_OBJECT
public:
    IOutputPane(QObject *parent = 0) : QObject(parent) {}
    virtual ~IOutputPane() {}

    virtual QWidget *outputWidget(QWidget *parent) = 0;
    virtual QList<QWidget*> toolBarWidgets() const = 0;
    virtual QString name() const = 0;

    // -1 don't show in statusBar
    // 100...0 show at front...end
    virtual int priorityInStatusBar() const = 0;

    virtual void clearContents() = 0;
    virtual void visibilityChanged(bool visible) = 0;

    // This function is called to give the outputwindow focus
    virtual void setFocus() = 0;
    // Wheter the outputpane has focus
    virtual bool hasFocus() = 0;
    // Wheter the outputpane can be focused at the moment.
    // (E.g. the search result window doesn't want to be focussed if the are no results.)
    virtual bool canFocus() = 0;

    virtual bool canNavigate() = 0;
    virtual bool canNext() = 0;
    virtual bool canPrevious() = 0;
    virtual void goToNext() = 0;
    virtual void goToPrev() = 0;
public slots:
    void popup()
    {
        popup(true);
    }
    void popup(bool withFocus)
    {
        emit showPage(withFocus);
    }

    void hide()
    {
        emit hidePage();
    }

    void toggle()
    {
        toggle(true);
    }

    void toggle(bool withFocusIfShown)
    {
        emit togglePage(withFocusIfShown);
    }

    void navigateStateChanged()
    {
        emit navigateStateUpdate();
    }

signals:
    void showPage(bool withFocus);
    void hidePage();
    void togglePage(bool withFocusIfShown);
    void navigateStateUpdate();
};

} // namespace Core

#endif // IOUTPUTPANE_H
