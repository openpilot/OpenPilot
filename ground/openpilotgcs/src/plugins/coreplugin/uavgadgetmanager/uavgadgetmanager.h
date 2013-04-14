/**
 ******************************************************************************
 *
 * @file       uavgadgetmanager.h
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

#ifndef UAVGADGETMANAGER_H
#define UAVGADGETMANAGER_H

#include "../core_global.h"

#include <coreplugin/icorelistener.h>
#include <coreplugin/imode.h>

#include <QtGui/QWidget>
#include <QtCore/QList>
#include <QtCore/QPointer>
#include <QtCore/QSettings>
#include <QtGui/QIcon>

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

namespace Core {

class IContext;
class ICore;
class IUAVGadget;

namespace Internal {

class UAVGadgetView;
class SplitterOrView;

} // namespace Internal

class CORE_EXPORT UAVGadgetManager : public IMode
{
    Q_OBJECT

public:

    explicit UAVGadgetManager(ICore *core, QString name, QIcon icon, int priority, QString uniqueName, QWidget *parent);
    virtual ~UAVGadgetManager();

    // IMode
    QString name() const { return m_name; }
    QIcon icon() const { return m_icon; }
    int priority() const { return m_priority; }
    void setPriority(int priority) { m_priority = priority; }
    const char *uniqueModeName() const { return m_uniqueModeName; }
    QList<int> context() const;

    void init();
    QWidget *widget() { return m_widget; }

    void ensureUAVGadgetManagerVisible();

    IUAVGadget *currentGadget() const;

    void saveState(QSettings*) const;
    bool restoreState(QSettings* qSettings);

    void saveSettings(QSettings* qs);
    void readSettings(QSettings* qs);
    bool toolbarsShown() { return m_showToolbars; }

signals:
    void currentGadgetChanged(IUAVGadget *gadget);
    void showUavGadgetMenus(bool show, bool hasSplitter);
    void updateSplitMenus(bool hasSplitter);

private slots:
    void handleContextChange(Core::IContext *context);
    void updateUavGadgetMenus();
    void modeChanged(Core::IMode *mode);


public slots:
    void split(Qt::Orientation orientation);
    void split();
    void splitSideBySide();
    void removeCurrentSplit();
    void removeAllSplits();
    void gotoOtherSplit();
    void showToolbars(bool show);

private:
    void setCurrentGadget(IUAVGadget *gadget);
    void addGadgetToContext(IUAVGadget *gadget);
    void removeGadget(IUAVGadget *gadget);
    void closeView(Core::Internal::UAVGadgetView *view);
    void emptyView(Core::Internal::UAVGadgetView *view);
    Core::Internal::SplitterOrView *currentSplitterOrView() const;

    bool m_showToolbars;
    Core::Internal::SplitterOrView *m_splitterOrView;
    Core::IUAVGadget *m_currentGadget;
    Core::ICore *m_core;

    QString m_name;
    QIcon m_icon;
    int m_priority;
    QString m_uniqueName;
    QByteArray m_uniqueNameBA;
    const char* m_uniqueModeName;
    QWidget *m_widget;

    friend class Core::Internal::SplitterOrView;
    friend class Core::Internal::UAVGadgetView;
};

} // namespace Core

#endif // UAVGADGETMANAGER_H
