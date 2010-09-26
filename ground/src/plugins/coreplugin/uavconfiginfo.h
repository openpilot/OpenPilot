/**
 ******************************************************************************
 *
 * @file       uavconfiginfo.h
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
#ifndef UAVCONFIGINFO_H
#define UAVCONFIGINFO_H

#include <QObject>
#include <QString>
#include <QSettings>
#include "iuavgadgetconfiguration.h"
#include "core_global.h"

namespace Core
{

class IUAVGadgetConfiguration;

class CORE_EXPORT UAVConfigVersion{
public:
    UAVConfigVersion(QString versionString = "0.0.0");
    UAVConfigVersion(int major, int minor, int patch);

    int majorNr;
    int minorNr;
    int patchNr;

    QString toString() const;
    bool operator==(const UAVConfigVersion &other);
};

class CORE_EXPORT UAVConfigInfo : public QObject
{
    Q_OBJECT
public:

    explicit UAVConfigInfo(QObject *parent = 0);
    explicit UAVConfigInfo(QSettings *qs, QObject *parent = 0);
    explicit UAVConfigInfo(IUAVGadgetConfiguration* config, QObject *parent = 0);
    UAVConfigInfo(UAVConfigVersion version, QString nameOfConfigurable, QObject *parent = 0);

    enum Compatibility { FullyCompatible, MinorLossOfConfiguration, MissingConfiguration, MajorLossOfConfiguration, NotCompatible };
    void setNameOfConfigurable(const QString nameOfConfigurable){m_nameOfConfigurable = nameOfConfigurable;}

    void save(QSettings *qs);
    void read(QSettings *qs);

    void setVersion(int major, int minor, int patch){m_version = UAVConfigVersion(major, minor, patch);}
    void setVersion(const QString version){m_version = UAVConfigVersion(version);}
    void setVersion(const UAVConfigVersion version){m_version = version;}
    UAVConfigVersion version(){ return m_version;}
    bool locked(){ return m_locked; }
    void setLocked(bool locked){ m_locked = locked; }

    int checkCompatibilityWith(UAVConfigVersion programVersion);
    bool askToAbort(int compat, QString message);
    void notify(QString message);
    bool standardVersionHandlingOK(UAVConfigVersion programVersion);
    bool standardVersionHandlingOK(QString programVersion){ return standardVersionHandlingOK(UAVConfigVersion(programVersion));}

signals:

public slots:

private:
    UAVConfigVersion m_version;
    bool m_locked;
    QString m_nameOfConfigurable;

};

} // namespace Core

#endif // UAVCONFIGINFO_H
