/**
 ******************************************************************************
 *
 * @file       uavgadgetmanager.h
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

#ifndef UAVGADGETMANAGER_H
#define UAVGADGETMANAGER_H

#include "../core_global.h"

#include <coreplugin/icorelistener.h>

#include <QtGui/QWidget>
#include <QtCore/QList>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE
class QSettings;
class QModelIndex;
QT_END_NAMESPACE

namespace Core {

class IContext;
class ICore;
class IUAVGadget;
class IMode;

struct UAVGadgetManagerPrivate;

namespace Internal {

class UAVGadgetMode;
class UAVGadgetView;
class SplitterOrView;

class UAVGadgetClosingCoreListener;


} // namespace Internal

class CORE_EXPORT UAVGadgetManagerPlaceHolder : public QWidget
{
    Q_OBJECT
public:
    UAVGadgetManagerPlaceHolder(Core::Internal::UAVGadgetMode *mode, QWidget *parent = 0);
    ~UAVGadgetManagerPlaceHolder();
//    static UAVGadgetManagerPlaceHolder* current();
private slots:
    void currentModeChanged(Core::IMode *);
private:
    Core::IMode *m_mode;
    Core::Internal::UAVGadgetMode *m_uavGadgetMode;
    UAVGadgetManagerPlaceHolder* m_current;
};


class CORE_EXPORT UAVGadgetManager : public QWidget
{
    Q_OBJECT

public:

    explicit UAVGadgetManager(ICore *core, QWidget *parent);
    virtual ~UAVGadgetManager();
    void init();
    // setUAVGadgetMode should be called exactly once
    // right after the mode has been created, and never thereafter
    void setUAVGadgetMode(Core::Internal::UAVGadgetMode *mode) { m_uavGadgetMode = mode; }

    void ensureUAVGadgetManagerVisible();

    IUAVGadget *currentUAVGadget() const;

    QByteArray saveState() const;
    bool restoreState(const QByteArray &state);

    void saveSettings();
    void readSettings();
    bool toolbarsShown() { return m_showToolbars; }

signals:
    void currentUAVGadgetChanged(IUAVGadget *gadget);

private slots:
    void handleContextChange(Core::IContext *context);
    void updateActions();

public slots:
    void split(Qt::Orientation orientation);
    void split();
    void splitSideBySide();
    void removeCurrentSplit();
    void removeAllSplits();
    void gotoOtherSplit();
    void showToolbars(bool show);

private:
    void addUAVGadget(IUAVGadget *gadget);
    void removeUAVGadget(IUAVGadget *gadget);
    void setCurrentUAVGadget(IUAVGadget *gadget);
    void closeView(Core::Internal::UAVGadgetView *view);
    void emptyView(Core::Internal::UAVGadgetView *view);
    Core::Internal::SplitterOrView *currentSplitterOrView() const;

    bool m_showToolbars;
    UAVGadgetManagerPrivate *m_d;
    Core::Internal::UAVGadgetMode *m_uavGadgetMode;

    friend class Core::Internal::SplitterOrView;
    friend class Core::Internal::UAVGadgetView;
};

} // namespace Core


//===================UAVGadgetClosingCoreListener======================

namespace Core {
namespace Internal {

class UAVGadgetClosingCoreListener : public ICoreListener
{
    Q_OBJECT

public:
    UAVGadgetClosingCoreListener(UAVGadgetManager *em);
    bool uavGadgetAboutToClose(IUAVGadget *gadget);
    bool coreAboutToClose();

private:
    UAVGadgetManager *m_em;
};

} // namespace Internal
} // namespace Core

#endif // UAVGADGETMANAGER_H
