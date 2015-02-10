/**
 ******************************************************************************
 *
 * @file       oplinkwatchdog.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2012.
 * @addtogroup [Group]
 * @{
 * @addtogroup OPLinkWatchdog
 * @{
 * @brief [Brief]
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

#ifndef OPLINKWATCHDOG_H
#define OPLINKWATCHDOG_H

#include <QTimer>

class OPLinkStatus;
class OPLinkWatchdog : public QObject {
    Q_OBJECT
public:
    enum OPLinkType {
        OPLINK_MINI,
        OPLINK_REVOLUTION,
        OPLINK_UNKNOWN
    };

    OPLinkWatchdog();
    ~OPLinkWatchdog();
    bool isConnected() const
    {
        return m_isConnected;
    }
    OPLinkWatchdog::OPLinkType opLinkType() const
    {
        return m_opLinkType;
    }

signals:
    void connected();
    void opLinkMiniConnected();
    void opLinkRevolutionConnected();
    void disconnected();

private slots:
    void onOPLinkStatusUpdate();
    void onTimeout();

private:
    bool m_isConnected;
    OPLinkType m_opLinkType;
    QTimer *m_watchdog;
    OPLinkStatus *m_opLinkStatus;
};
#endif // OPLINKWATCHDOG_H
