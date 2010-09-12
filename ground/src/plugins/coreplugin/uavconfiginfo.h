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

namespace Core
{

class UAVConfigVersion{
public:
    UAVConfigVersion(QString versionString = "0.0.0");
    UAVConfigVersion(int major, int minor, int patch);

    int majorNr;
    int minorNr;
    int patchNr;

    QString toString() const;
    bool operator==(const UAVConfigVersion &other);
};

class UAVConfigInfo : public QObject
{
    Q_OBJECT
public:

    UAVConfigInfo(QSettings *qs, QObject *parent = 0);
    UAVConfigInfo(UAVConfigVersion version, QString nameOfConfigurable, QObject *parent = 0);

    enum Compatibility { FullyCompatible, MinorLossOfConfiguration, MissingConfiguration, MajorLossOfConfiguration, NotCompatible };
    void setNameOfConfigurable(const QString nameOfConfigurable){m_nameOfConfigurable = nameOfConfigurable;}

    void save(QSettings *qs);

    void setVersion(int major, int minor, int patch){m_version = UAVConfigVersion(major, minor, patch);}
    void setVersion(const QString version){m_version = UAVConfigVersion(version);}
    UAVConfigVersion version(){ return m_version;}
    int checkCompatibilityWith(UAVConfigVersion programVersion);
    bool askToAbort(int compat, QString message);
    void notifyAbort(QString message);
    bool standardVersionHandlingIsNotOK(UAVConfigVersion programVersion);

signals:

public slots:

private:
    UAVConfigVersion m_version;
    QString m_nameOfConfigurable;

};

} // namespace Core

#endif // UAVCONFIGINFO_H
