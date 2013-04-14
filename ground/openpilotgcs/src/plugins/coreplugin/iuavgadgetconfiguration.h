/**
 ******************************************************************************
 *
 * @file       iuavgadgetconfiguration.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
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

#ifndef IUAVGADGETCONFIGURATION_H
#define IUAVGADGETCONFIGURATION_H

#include <coreplugin/core_global.h>
#include <coreplugin/uavconfiginfo.h>
#include <QObject>
#include <QSettings>

namespace Core {

class UAVConfigInfo;

class CORE_EXPORT IUAVGadgetConfiguration : public QObject
{
Q_OBJECT
public:
    explicit IUAVGadgetConfiguration(QString classId, QObject *parent = 0);
    QString classId() { return m_classId; }
    QString name() { return m_name; }
    void setName(QString name) { m_name = name; }
    QString provisionalName() { return m_provisionalName; }
    void setProvisionalName(QString name) { m_provisionalName = name; }
    bool locked() const { return m_locked; }
    void setLocked(bool locked) { m_locked = locked; }

    virtual void saveConfig(QSettings* /*settings*/) const {};
    virtual void saveConfig(QSettings* settings, UAVConfigInfo* /*configInfo*/) const { saveConfig(settings); }

    virtual IUAVGadgetConfiguration *clone() = 0;

signals:

public slots:

private:
    bool m_locked;
    QString m_classId;
    QString m_name;
    QString m_provisionalName;

};

} // namespace Core

#endif // IUAVGADGETCONFIGURATION_H
