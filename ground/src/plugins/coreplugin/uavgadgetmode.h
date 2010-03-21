/**
 ******************************************************************************
 *
 * @file       uavgadgetmode.h
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

#ifndef UAVGADGETMODE_H
#define UAVGADGETMODE_H

#include <coreplugin/imode.h>

#include <QtCore/QObject>
#include <QtGui/QIcon>

QT_BEGIN_NAMESPACE
class QSplitter;
class QWidget;
class QIcon;
class QVBoxLayout;
QT_END_NAMESPACE

namespace Core {

class UAVGadgetManager;

namespace Internal {

class UAVGadgetMode : public Core::IMode
{
    Q_OBJECT

public:
    UAVGadgetMode(UAVGadgetManager *uavGadgetManager, QString name, QIcon icon, int priority, QString uniqueName);
    ~UAVGadgetMode();

    // IMode
    QString name() const;
    QIcon icon() const;
    int priority() const;
    QWidget* widget();
    const char* uniqueModeName() const;
    QList<int> context() const;
    UAVGadgetManager* uavGadgetManager() const { return m_uavGadgetManager; }

private slots:
    void grabUAVGadgetManager(Core::IMode *mode);

private:
    UAVGadgetManager *m_uavGadgetManager;
    QString m_name;
    QIcon m_icon;
    QWidget *m_widget;
    int m_priority;
    QVBoxLayout *m_layout;
    QString m_uniqueName;
    QByteArray m_uniqueNameBA;
    const char *m_uniqueNameC;
};

} // namespace Internal
} // namespace Core

#endif // UAVGADGETMODE_H
