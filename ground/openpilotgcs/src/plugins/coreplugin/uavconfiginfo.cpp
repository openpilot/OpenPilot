/**
 ******************************************************************************
 *
 * @file       uavconfiginfo.cpp
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
/*!
    \class Core::UAVConfigInfo
    \mainclass

    \brief The Config Info is a helper-class to handle version changes in GCS
        configuration files.

    The UAVConfigInfo provides version-information for the configuration-data
    and callback functions to ask the user how to handle incompatble
    (old or newer) configurations.

    When the config is created from a \l{QSettings} instance, an UAVConfigInfo
    object is passed to the factory-method. With the version-data it can decide whether
    the presented config-data is compatible to the current implementation. It may
    migrate old data to the current format or abort the import.

    When the config is written to the \l{QSettings} instance, an UAVConfigInfo object
    is passed to the writer-function. The version of the config-format should
    be written to the UAVConfigInfo object. This version will be passed to
    factory-method when creating the config-object from this configuration.


    Typically a plugin can handle version-changes like this:
    \code
    MyGadgetConfiguration::MyGadgetConfiguration(QString classId, QSettings* qSettings, UAVConfigInfo *configInfo, QObject *parent) :
        IUAVGadgetConfiguration(classId, parent)
    {
        if ( ! qSettings )
            return;

        if ( configInfo->version() == UAVConfigVersion() )
            configInfo->setVersion("1.0.0");

        if ( !configInfo->standardVersionHandlingOK(CURRENT_VERSION))
            return;

        ... read the config ...
    }

    void MyGadgetConfiguration::saveConfig(QSettings* qSettings, Core::UAVConfigInfo *configInfo) const {

        configInfo->setVersion(CURRENT_VERSION);

        ... write the config ...
    }

    \endcode


    \section1 Version Conventions

    The Version numbers are in the form "major.minor.patch" (e.g. "3.1.4") with the
    following meaning:
    \list
    \o major: Differences in this number indicate completely incompatible formats. The
        config can't be imported.
    \o minor: Differences in this number indicate backwards compatible formats. Old
        configs can be imported or will be automatically migrated by the new program
        but configs written by this plugin can't be reasonably read by old versions of
        the plugin.
    \o patch: Differences in this number indicate backwards and forward compatible formats.
        Configs written by this plugin can be read by old versions of the plugin. Old configs
        are extended by defaults by the new plugin.
    \endlist

    All parts (major, minor, patch) must be numeric values.

    \section1 Utility Functions

    \fn bool UAVConfigInfo::standardVersionHandlingOK(UAVConfigVersion programVersion)
    \brief Default version handling.

    With this function the plugin can test compatiblility of the current version
    with the imported version. If there are differences, the user is asked whether
    he or she wants to import the settings or abort the import.

    Returns true when the import should be done, false otherwise.

*/

#include "uavconfiginfo.h"
#include <QMessageBox>

#define VERSION_DEFAULT  "0.0.0"

#define TEXT_MINOR_LOSS_OF_CONFIGURATION  tr( \
" Some of the configured features might not be supported \
by your version of the plugin. You might want to upgrade the plugin.")

#define TEXT_MISSING_CONFIGURATION  tr( \
" Some configuration is missing in the imported config and will be replaced \
by default settings.")

#define TEXT_MAJOR_LOSS_OF_CONFIGURATION  tr( \
" Major features can't be imported \
by your version of the plugin. You should upgrade the plugin to import these settings.")

#define TEXT_NOT_COMPATIBLE  tr( \
" The imported settings are not compatible with this plugin and won't be imported!")

using namespace Core;

UAVConfigInfo::UAVConfigInfo(QObject *parent) :
        QObject(parent),
        m_version(VERSION_DEFAULT),
        m_locked(false),
        m_nameOfConfigurable("")
{

}

UAVConfigInfo::UAVConfigInfo(QSettings *qs, QObject *parent) :
    QObject(parent),
    m_version(VERSION_DEFAULT)
{
    read(qs);
}

UAVConfigInfo::UAVConfigInfo(UAVConfigVersion version, QString nameOfConfigurable, QObject *parent) :
        QObject(parent),
        m_version(version),
        m_locked(false),
        m_nameOfConfigurable(nameOfConfigurable)
{

}

UAVConfigInfo::UAVConfigInfo(IUAVGadgetConfiguration *config, QObject *parent) :
        QObject(parent)
{
    m_locked = config->locked();
    m_nameOfConfigurable = config->classId() + "-" + config->name();
}

void UAVConfigInfo::save(QSettings *qs)
{
    qs->beginGroup("configInfo");
    qs->setValue("version", m_version.toString());
    qs->setValue("locked", m_locked);
    qs->endGroup();
}

void UAVConfigInfo::read(QSettings *qs)
{
    qs->beginGroup("configInfo");
    m_version = UAVConfigVersion( qs->value("version", VERSION_DEFAULT ).toString());
    m_locked = qs->value("locked", false ).toBool();
    qs->endGroup();
}

bool UAVConfigInfo::askToAbort(int compat, QString message)
{
    QMessageBox msgBox;
    msgBox.setInformativeText(tr("Do you want to continue the import?"));
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);

    int result;

    switch(compat){

    case FullyCompatible:
        return false;

    case MinorLossOfConfiguration:
        msgBox.setText(tr("INFO: ") + message + TEXT_MINOR_LOSS_OF_CONFIGURATION);
        msgBox.setDefaultButton(QMessageBox::Ok);
        result = msgBox.exec();
        break;

    case MissingConfiguration:
        msgBox.setText(tr("WARNING: ") + message + TEXT_MISSING_CONFIGURATION);
        msgBox.setDefaultButton(QMessageBox::Ok);
        result = msgBox.exec();
        break;

    case MajorLossOfConfiguration:
        msgBox.setText(tr("ERROR: ") + message + TEXT_MAJOR_LOSS_OF_CONFIGURATION);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        result = msgBox.exec();
        break;

    case NotCompatible:
        msgBox.setText("ERROR: " + message + TEXT_NOT_COMPATIBLE);
        msgBox.setInformativeText(tr(""));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return true;

    default:
        msgBox.setText("INTERNAL ERROR: " + message + tr("Unknown compatibility level: " + compat));
    }
    if ( result == QMessageBox::Ok )
        return false;
    else
        return true;

}

void UAVConfigInfo::notify(QString message)
{
    QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.exec();
}

int UAVConfigInfo::checkCompatibilityWith(UAVConfigVersion programVersion)
{
    if ( m_version.majorNr != programVersion.majorNr )
        return NotCompatible;
    if ( m_version.minorNr < programVersion.minorNr )
        return MissingConfiguration;
    if ( m_version.minorNr > programVersion.minorNr )
        return MajorLossOfConfiguration;
    if ( m_version.patchNr > programVersion.patchNr )
        return MinorLossOfConfiguration;

    return FullyCompatible;
}

bool UAVConfigInfo::standardVersionHandlingOK(UAVConfigVersion programVersion)
{
    return !askToAbort(
            checkCompatibilityWith(programVersion),
            "("+m_nameOfConfigurable+")");
}

UAVConfigVersion::UAVConfigVersion(int majorNum, int minorNum, int patchNum)
    :majorNr(majorNum)
    ,minorNr(minorNum)
    ,patchNr(patchNum)
{
}

UAVConfigVersion::UAVConfigVersion(QString versionString)
{
    int begin;
    int end = 0;

    begin = end;
    end = versionString.indexOf(".", begin);
    majorNr = versionString.mid(begin, end-begin).toInt();

    begin = end+1;
    end = versionString.indexOf(".", begin);
    minorNr = versionString.mid(begin, end-begin).toInt();

    begin = end+1;
    patchNr = versionString.mid(begin).toInt();
}

QString UAVConfigVersion::toString() const
{
    return QString("%1.%2.%3").arg(majorNr).arg(minorNr).arg(patchNr);
}

bool UAVConfigVersion::operator==(const UAVConfigVersion &other)
{
    return toString() == other.toString();
}

/**
 * @}
 * @}
 */
