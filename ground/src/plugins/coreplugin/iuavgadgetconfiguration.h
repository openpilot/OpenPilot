/**
 ******************************************************************************
 *
 * @file       iuavgadgetconfiguration.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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

#ifndef IUAVGADGETCONFIGURATION_H
#define IUAVGADGETCONFIGURATION_H

#include <coreplugin/core_global.h>
#include <QObject>

namespace Core {

class CORE_EXPORT IUAVGadgetConfiguration : public QObject
{
Q_OBJECT
public:
    explicit IUAVGadgetConfiguration(bool locked, QString classId, QString name, QObject *parent = 0);
    virtual QByteArray saveState() const = 0;
    QString classId() { return m_classId; }
    QString name() { return m_name; }
    void setName(QString name) { m_name = name; }
    bool locked() const { return m_locked; }
    virtual IUAVGadgetConfiguration *clone() = 0;

signals:

public slots:

private:
    bool m_locked;
    QString m_classId;
    QString m_name;

};

} // namespace Core

#endif // IUAVGADGETCONFIGURATION_H
