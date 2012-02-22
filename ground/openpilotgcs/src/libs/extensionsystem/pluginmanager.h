/**
 ******************************************************************************
 *
 * @file       pluginmanager.h
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010.
 *             Parts by Nokia Corporation (qt-info@nokia.com) Copyright (C) 2009.
 * @brief      
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   
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

#ifndef EXTENSIONSYSTEM_PLUGINMANAGER_H
#define EXTENSIONSYSTEM_PLUGINMANAGER_H

#include "extensionsystem_global.h"
#include <aggregation/aggregate.h>

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QReadWriteLock>

QT_BEGIN_NAMESPACE
class QTextStream;
QT_END_NAMESPACE

namespace ExtensionSystem {

namespace Internal {
    class PluginManagerPrivate;
}

class IPlugin;
class PluginSpec;

class EXTENSIONSYSTEM_EXPORT PluginManager : public QObject
{
    Q_DISABLE_COPY(PluginManager)
    Q_OBJECT

public:
    static PluginManager *instance();
    bool allPluginsLoaded(){return m_allPluginsLoaded;}
    PluginManager();
    virtual ~PluginManager();

    // Object pool operations
    void addObject(QObject *obj);
    void removeObject(QObject *obj);
    QList<QObject *> allObjects() const;
    template <typename T> QList<T *> getObjects() const
    {
        QReadLocker lock(&m_lock);
        QList<T *> results;
        QList<QObject *> all = allObjects();
        QList<T *> result;
        foreach (QObject *obj, all) {
            result = Aggregation::query_all<T>(obj);
            if (!result.isEmpty())
                results += result;
        }
        return results;
    }
    template <typename T> T *getObject() const
    {
        QReadLocker lock(&m_lock);
        QList<QObject *> all = allObjects();
        T *result = 0;
        foreach (QObject *obj, all) {
            if ((result = Aggregation::query<T>(obj)) != 0)
                break;
        }
        return result;
    }

    // Plugin operations
    void loadPlugins();
    QStringList pluginPaths() const;
    void setPluginPaths(const QStringList &paths);
    QList<PluginSpec *> plugins() const;
    void setFileExtension(const QString &extension);
    QString fileExtension() const;

    // command line arguments
    QStringList arguments() const;
    bool parseOptions(const QStringList &args,
        const QMap<QString, bool> &appOptions,
        QMap<QString, QString> *foundAppOptions,
        QString *errorString);
    static void formatOptions(QTextStream &str, int optionIndentation, int descriptionIndentation);
    void formatPluginOptions(QTextStream &str, int optionIndentation, int descriptionIndentation) const;
    void formatPluginVersions(QTextStream &str) const;

    bool runningTests() const;
    QString testDataDirectory() const;

signals:
    void objectAdded(QObject *obj);
    void aboutToRemoveObject(QObject *obj);

    void pluginsChanged();
    void pluginsLoadEnded();
private slots:
    void startTests();

private:
    Internal::PluginManagerPrivate *d;
    static PluginManager *m_instance;
    mutable QReadWriteLock m_lock;
    bool m_allPluginsLoaded;

    friend class Internal::PluginManagerPrivate;
};

} // namespace ExtensionSystem

#endif // EXTENSIONSYSTEM_PLUGINMANAGER_H
