#include "gcsdirs.h"

#include <QDir>
#include <QDebug>
#include <QString>
#include <QApplication>

GCSDirs::GCSDirs()
    : d(0)
{}

QString GCSDirs::rootDir()
{
    // Figure out root : Up one from 'bin'
    QDir rootDir = QApplication::applicationDirPath();

    rootDir.cdUp();
    return rootDir.canonicalPath();
}

QString GCSDirs::libraryPath(QString provider)
{
    QString libPath = rootDir();

#ifdef Q_OS_MACX
    // TODO not correct...
    libPath += QLatin1String("/Plugins");
#else
    // GCS_LIBRARY_BASENAME is a compiler define set by qmake
    libPath += QLatin1Char('/') + QLatin1String(GCS_LIBRARY_BASENAME);
    libPath += QLatin1String("/openpilotgcs/") + provider;
#endif
    return libPath;
}

QString GCSDirs::pluginPath(QString provider)
{
    QString pluginPath = rootDir();

#ifdef Q_OS_MACX
    // TODO not correct...
    pluginPath += QLatin1String("/Plugins");
#else
    // GCS_LIBRARY_BASENAME is a compiler define set by qmake
    pluginPath += QLatin1Char('/') + QLatin1String(GCS_LIBRARY_BASENAME);
    pluginPath += QLatin1Char('/') + provider;
    pluginPath += QLatin1String("/plugins");
#endif
    return pluginPath;
}

QString GCSDirs::sharePath(QString provider)
{
    QString sharePath = rootDir();

#ifdef Q_OS_MACX
    sharePath += QLatin1String("/Resources");
#else
    sharePath += QLatin1String("/share/openpilotgcs");
    sharePath += QLatin1Char('/') + provider;
#endif
    return sharePath;
}

QString GCSDirs::gcsPluginPath()
{
    return pluginPath("openpilotgcs");
}

QString GCSDirs::gcsSharePath()
{
    return sharePath("openpilotgcs");
}

void GCSDirs::debug()
{
    qDebug() << "=== GCSDirs ===";
    qDebug() << "GCS Share Path :" << gcsSharePath();
    qDebug() << "GCS Plugin Path :" << gcsPluginPath();
    qDebug() << "===================";
}
