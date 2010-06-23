/**
 ******************************************************************************
 *
 * @file       TCPtelemetryconfiguration.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   map
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

#ifndef TCPtelemetryCONFIGURATION_H
#define TCPtelemetryCONFIGURATION_H

#include <coreplugin/iuavgadgetconfiguration.h>
#include <QtCore/QString>

using namespace Core;

class TCPtelemetryConfiguration : public IUAVGadgetConfiguration
{
Q_OBJECT
Q_PROPERTY(QString HostName READ HostName WRITE setHostName)
Q_PROPERTY(int Port READ Port WRITE setPort)

public:
    explicit TCPtelemetryConfiguration(QString classId, const QByteArray &state = 0, QObject *parent = 0);
    QByteArray saveState() const;
    IUAVGadgetConfiguration *clone();

    QString HostName() const { return m_HostName; }
    int Port() const { return m_Port; }

public slots:
    void setHostName(QString HostName) { m_HostName = HostName; }
    void setPort(int Port) { m_Port = Port; }

private:
    QString m_HostName;
    int m_Port;


};

#endif // TCPtelemetryCONFIGURATION_H
