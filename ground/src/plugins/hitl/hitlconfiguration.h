/**
 ******************************************************************************
 *
 * @file       hitlconfiguration.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   hitl
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

#ifndef HITLCONFIGURATION_H
#define HITLCONFIGURATION_H

#include <coreplugin/iuavgadgetconfiguration.h>
#include <QtGui/QColor>
#include <QString>

using namespace Core;

class HITLConfiguration : public IUAVGadgetConfiguration
{

Q_OBJECT
Q_PROPERTY(QString m_fgPathBin READ fgPathBin WRITE setFGPathBin)
Q_PROPERTY(QString m_fgPathData READ fgPathData WRITE setFGPathData)
Q_PROPERTY(bool m_fgManualControl READ fgManualControl WRITE setFGManualControl)

public:
    explicit HITLConfiguration(QString classId, const QByteArray &state = 0, QObject *parent = 0);
    QByteArray saveState() const;
    IUAVGadgetConfiguration *clone();

    QString fgPathBin() const { return m_fgPathBin; }
    QString fgPathData() const { return m_fgPathData; }
    bool fgManualControl() const { return m_fgManualControl; }

signals:

public slots:
    void setFGPathBin(QString fgPath) { m_fgPathBin = fgPath; }
    void setFGPathData(QString fgPath) { m_fgPathData = fgPath; }
    void setFGManualControl(bool val) { m_fgManualControl = val; }

private:
    QString m_fgPathBin;
    QString m_fgPathData;
    bool m_fgManualControl;

};

#endif // HITLCONFIGURATION_H
