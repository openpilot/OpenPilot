/**
 ******************************************************************************
 *
 * @file       hitlil2configuration.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   hitlil2plugin
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

#ifndef HITLIL2CONFIGURATION_H
#define HITLIL2CONFIGURATION_H

#include <coreplugin/iuavgadgetconfiguration.h>
#include <QtGui/QColor>
#include <QString>

using namespace Core;

class HITLIL2Configuration : public IUAVGadgetConfiguration
{

Q_OBJECT
Q_PROPERTY(QString m_il2HostName READ il2HostName WRITE setIl2HostName)
Q_PROPERTY(int m_il2Port READ il2Port WRITE setIl2Port)
Q_PROPERTY(bool m_il2ManualControl READ il2ManualControl WRITE setIl2ManualControl)

public:
    explicit HITLIL2Configuration(QString classId, const QByteArray &state = 0, QObject *parent = 0);
    QByteArray saveState() const;
    IUAVGadgetConfiguration *clone();

    QString il2HostName() const { return m_il2HostName; }
    int il2Port() const { return m_il2Port; }
    bool il2ManualControl() const { return m_il2ManualControl; }

signals:

public slots:
    void setIl2HostName(QString HostName) { m_il2HostName = HostName; }
    void setIl2Port(int Port) { m_il2Port = Port; }
    void setIl2ManualControl(bool val) { m_il2ManualControl = val; }

private:
    QString m_il2HostName;
    int m_il2Port;
    bool m_il2ManualControl;

};

#endif // HITLIL2CONFIGURATION_H
