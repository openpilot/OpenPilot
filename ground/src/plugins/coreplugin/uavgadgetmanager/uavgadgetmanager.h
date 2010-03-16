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
class IUAVGadgetFactory;
class IMode;

struct UAVGadgetManagerPrivate;

namespace Internal {

class UAVGadgetView;
class SplitterOrView;

class UAVGadgetClosingCoreListener;


} // namespace Internal

class CORE_EXPORT UAVGadgetManagerPlaceHolder : public QWidget
{
    Q_OBJECT
public:
    UAVGadgetManagerPlaceHolder(Core::IMode *mode, QWidget *parent = 0);
    ~UAVGadgetManagerPlaceHolder();
    static UAVGadgetManagerPlaceHolder* current();
private slots:
    void currentModeChanged(Core::IMode *);
private:
    Core::IMode *m_mode;
    static UAVGadgetManagerPlaceHolder* m_current;
};

class CORE_EXPORT UAVGadgetManager : public QWidget
{
    Q_OBJECT

public:
    typedef QList<IUAVGadgetFactory*> UAVGadgetFactoryList;

    explicit UAVGadgetManager(ICore *core, QWidget *parent);
    virtual ~UAVGadgetManager();
    void init();
    static UAVGadgetManager *instance() { return m_instance; }

    void ensureUAVGadgetManagerVisible();

    IUAVGadget *currentUAVGadget() const;
    IUAVGadget *activateUAVGadget(IUAVGadget *gadget);

    bool closeUAVGadgets(const QList<IUAVGadget *> uavGadgetsToClose);

    QByteArray saveState() const;
    bool restoreState(const QByteArray &state);

    void saveSettings();
    void readSettings();

    UAVGadgetFactoryList uavGadgetFactories() const;

signals:

public slots:
    bool closeAllUAVGadgets();

    void closeUAVGadget();

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
    void hideToolbars(bool hide);

private:
    void addUAVGadget(IUAVGadget *gadget);
    IUAVGadget *createUAVGadget(const QString &uavGadgetKind = QString());
    void removeUAVGadget(IUAVGadget *gadget);

    IUAVGadget *placeUAVGadget(Core::Internal::UAVGadgetView *view, Core::IUAVGadget *gadget);
    void setCurrentUAVGadget(IUAVGadget *gadget);
    void setCurrentView(Core::Internal::SplitterOrView *view);
    IUAVGadget *activateUAVGadget(Core::Internal::UAVGadgetView *view, Core::IUAVGadget *gadget);

    Core::Internal::SplitterOrView *currentSplitterOrView() const;

    void closeUAVGadget(Core::IUAVGadget *gadget);
    void closeView(Core::Internal::UAVGadgetView *view);
    void emptyView(Core::Internal::UAVGadgetView *view);
    Core::Internal::UAVGadgetView *currentUAVGadgetView() const;
    IUAVGadget *pickUnusedUAVGadget() const;

    static UAVGadgetManager *m_instance;
    UAVGadgetManagerPrivate *m_d;

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
