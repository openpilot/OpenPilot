/**
 ******************************************************************************
 *
 * @file       main.cpp
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

#include "qtsingleapplication.h"
#include "utils/xmlconfig.h"

#include <extensionsystem/pluginmanager.h>
#include <extensionsystem/pluginspec.h>
#include <extensionsystem/iplugin.h>

#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QFileInfo>
#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <QtCore/QLibraryInfo>
#include <QtCore/QTranslator>
#include <QtCore/QSettings>
#include <QtCore/QVariant>

#include <QtGui/QMessageBox>
#include <QtGui/QApplication>
#include <QtGui/QMainWindow>

enum { OptionIndent = 4, DescriptionIndent = 24 };

static const char *appNameC = "OpenPilot GCS";
static const char *corePluginNameC = "Core";
static const char *fixedOptionsC =
" [OPTION]... [FILE]...\n"
"Options:\n"
"    -help               Display this help\n"
"    -version            Display program version\n"
"    -client             Attempt to connect to already running instance\n"
"    -clean-config       Delete all existing configuration settings\n"
"    -exit-after-config  Exit GCS after manipulating configuration settings\n"
"    -D key=value        Override configuration settings e.g: -D General/OverrideLanguage=de\n"
"    -configfile=value       Default configuration file to load if settings file is empty\n";
static const char *HELP_OPTION1 = "-h";
static const char *HELP_OPTION2 = "-help";
static const char *HELP_OPTION3 = "/h";
static const char *HELP_OPTION4 = "--help";
static const char *VERSION_OPTION = "-version";
static const char *CLIENT_OPTION = "-client";
static const char *CONFIG_OPTION = "-D";
static const char *CLEAN_CONFIG_OPTION = "-clean-config";
static const char *EXIT_AFTER_CONFIG_OPTION = "-exit-after-config";

typedef QList<ExtensionSystem::PluginSpec *> PluginSpecSet;

// Helpers for displaying messages. Note that there is no console on Windows.
#ifdef Q_OS_WIN
// Format as <pre> HTML
static inline void toHtml(QString &t)
{
    t.replace(QLatin1Char('&'), QLatin1String("&amp;"));
    t.replace(QLatin1Char('<'), QLatin1String("&lt;"));
    t.replace(QLatin1Char('>'), QLatin1String("&gt;"));
    t.insert(0, QLatin1String("<html><pre>"));
    t.append(QLatin1String("</pre></html>"));
}

static void displayHelpText(QString t) // No console on Windows.
{
    toHtml(t);
    QMessageBox::information(0, QLatin1String(appNameC), t);
}

static void displayError(const QString &t) // No console on Windows.
{
    QMessageBox::critical(0, QLatin1String(appNameC), t);
}

#else

static void displayHelpText(const QString &t)
{
    qWarning("%s", qPrintable(t));
}

static void displayError(const QString &t)
{
    qCritical("%s", qPrintable(t));
}

#endif

static void printVersion(const ExtensionSystem::PluginSpec *coreplugin,
                         const ExtensionSystem::PluginManager &pm)
{
    QString version;
    QTextStream str(&version);
    str << '\n' << appNameC << ' ' << coreplugin->version()<< " based on Qt " << qVersion() << "\n\n";
    pm.formatPluginVersions(str);
    str << '\n' << coreplugin->copyright() << '\n';
    displayHelpText(version);
}

static void printHelp(const QString &a0, const ExtensionSystem::PluginManager &pm)
{
    QString help;
    QTextStream str(&help);
    str << "Usage: " << a0  << fixedOptionsC;
    ExtensionSystem::PluginManager::formatOptions(str, OptionIndent, DescriptionIndent);
    pm.formatPluginOptions(str,  OptionIndent, DescriptionIndent);
    displayHelpText(help);
}

static inline QString msgCoreLoadFailure(const QString &why)
{
    return QCoreApplication::translate("Application", "Failed to load core: %1").arg(why);
}

static inline QString msgSendArgumentFailed()
{
    return QCoreApplication::translate("Application", "Unable to send command line arguments to the already running instance. It appears to be not responding.");
}

// Prepare a remote argument: If it is a relative file, add the current directory
// since the the central instance might be running in a different directory.

static inline QString prepareRemoteArgument(const QString &a)
{
    QFileInfo fi(a);
    if (!fi.exists())
        return a;
    if (fi.isRelative())
        return fi.absoluteFilePath();
    return a;
}

// Send the arguments to an already running instance of OpenPilot GCS
static bool sendArguments(SharedTools::QtSingleApplication &app, const QStringList &arguments)
{
    if (!arguments.empty()) {
        // Send off arguments
        const QStringList::const_iterator acend = arguments.constEnd();
        for (QStringList::const_iterator it = arguments.constBegin(); it != acend; ++it) {
            if (!app.sendMessage(prepareRemoteArgument(*it))) {
                displayError(msgSendArgumentFailed());
                return false;
            }
        }
    }
    // Special empty argument means: Show and raise (the slot just needs to be triggered)
    if (!app.sendMessage(QString())) {
        displayError(msgSendArgumentFailed());
        return false;
    }
    return true;
}

static inline QStringList getPluginPaths()
{
    QStringList rc;
    // Figure out root:  Up one from 'bin'
    QDir rootDir = QApplication::applicationDirPath();
    rootDir.cdUp();
    const QString rootDirPath = rootDir.canonicalPath();
    // 1) "plugins" (Win/Linux)
    QString pluginPath = rootDirPath;
    pluginPath += QLatin1Char('/');
    pluginPath += QLatin1String(GCS_LIBRARY_BASENAME);
    pluginPath += QLatin1Char('/');
    pluginPath += QLatin1String("openpilotgcs");
    pluginPath += QLatin1Char('/');
    pluginPath += QLatin1String("plugins");
    rc.push_back(pluginPath);
    // 2) "PlugIns" (OS X)
    pluginPath = rootDirPath;
    pluginPath += QLatin1Char('/');
    pluginPath += QLatin1String("Plugins");
    rc.push_back(pluginPath);
    return rc;
}

#ifdef Q_OS_MAC
#  define SHARE_PATH "/../Resources"
#else
#  define SHARE_PATH "/../share/openpilotgcs"
#endif

static void overrideSettings(QSettings &settings, int argc, char **argv){

    QMap<QString, QString> settingOptions;
    // Options like -DMy/setting=test
    QRegExp rx("([^=]+)=(.*)");

    for(int i = 0; i < argc; ++i ){
        if ( QString(CONFIG_OPTION).compare(QString(argv[i])) == 0 ){
            if ( rx.indexIn(argv[++i]) > -1 ){
                settingOptions.insert(rx.cap(1), rx.cap(2));
            }
        }
        if ( QString(CLEAN_CONFIG_OPTION).compare(QString(argv[i])) == 0 ){
            settings.clear();
        }
    }

    QList<QString> keys = settingOptions.keys();
    foreach ( QString key, keys ){
        settings.setValue(key, settingOptions.value(key));
    }
    settings.sync();
}

int main(int argc, char **argv)
{
#ifdef Q_OS_MAC
    // increase the number of file that can be opened in OpenPilot GCS
    struct rlimit rl;
    getrlimit(RLIMIT_NOFILE, &rl);
    rl.rlim_cur = rl.rlim_max;
    setrlimit(RLIMIT_NOFILE, &rl);
#endif
#ifdef Q_OS_LINUX
    QApplication::setAttribute(Qt::AA_X11InitThreads, true);
#endif

    //Set the default locale to EN, if this is not set the system locale will be used
    //and as of now we dont want that behaviour.
    QLocale::setDefault(QLocale::English);

    QApplication::setGraphicsSystem("raster");

    SharedTools::QtSingleApplication app((QLatin1String(appNameC)), argc, argv);

    QString locale = QLocale::system().name();

    // Must be done before any QSettings class is created
    QSettings::setPath(XmlConfig::XmlSettingsFormat, QSettings::SystemScope,
            QCoreApplication::applicationDirPath()+QLatin1String(SHARE_PATH));
    // keep this in sync with the MainWindow ctor in coreplugin/mainwindow.cpp
    QSettings settings(XmlConfig::XmlSettingsFormat, QSettings::UserScope,
                                 QLatin1String("OpenPilot"), QLatin1String("OpenPilotGCS_config"));

    overrideSettings(settings, argc, argv);
    locale = settings.value("General/OverrideLanguage", locale).toString();

    QTranslator translator;
    QTranslator qtTranslator;

    const QString &creatorTrPath = QCoreApplication::applicationDirPath()
                                   + QLatin1String(SHARE_PATH "/translations");
    if (translator.load(QLatin1String("openpilotgcs_") + locale, creatorTrPath)) {
        const QString &qtTrPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
        const QString &qtTrFile = QLatin1String("qt_") + locale;
        // Binary installer puts Qt tr files into creatorTrPath
        if (qtTranslator.load(qtTrFile, qtTrPath) || qtTranslator.load(qtTrFile, creatorTrPath)) {
            QCoreApplication::installTranslator(&translator);
            QCoreApplication::installTranslator(&qtTranslator);
        } else {
            translator.load(QString()); // unload()
        }
    }
    app.setProperty("qtc_locale", locale); // Do we need this?

    // Load
    ExtensionSystem::PluginManager pluginManager;
    pluginManager.setFileExtension(QLatin1String("pluginspec"));

    const QStringList pluginPaths = getPluginPaths();
    pluginManager.setPluginPaths(pluginPaths);

    const QStringList arguments = app.arguments();
    QMap<QString, QString> foundAppOptions;
    if (arguments.size() > 1) {
        QMap<QString, bool> appOptions;
        appOptions.insert(QLatin1String(HELP_OPTION1), false);
        appOptions.insert(QLatin1String(HELP_OPTION2), false);
        appOptions.insert(QLatin1String(HELP_OPTION3), false);
        appOptions.insert(QLatin1String(HELP_OPTION4), false);
        appOptions.insert(QLatin1String(VERSION_OPTION), false);
        appOptions.insert(QLatin1String(CLIENT_OPTION), false);
        appOptions.insert(QLatin1String(CONFIG_OPTION), true);
        appOptions.insert(QLatin1String(CLEAN_CONFIG_OPTION), false);
        appOptions.insert(QLatin1String(EXIT_AFTER_CONFIG_OPTION), false);
        QString errorMessage;
        if (!pluginManager.parseOptions(arguments,
                                        appOptions,
                                        &foundAppOptions,
                                        &errorMessage)) {
            displayError(errorMessage);
            printHelp(QFileInfo(app.applicationFilePath()).baseName(), pluginManager);
            return -1;
        }
    }

    const PluginSpecSet plugins = pluginManager.plugins();
    ExtensionSystem::PluginSpec *coreplugin = 0;
    foreach (ExtensionSystem::PluginSpec *spec, plugins) {
        if (spec->name() == QLatin1String(corePluginNameC)) {
            coreplugin = spec;
            break;
        }
    }
    if (!coreplugin) {
        QString nativePaths = QDir::toNativeSeparators(pluginPaths.join(QLatin1String(",")));
        const QString reason = QCoreApplication::translate("Application", "Could not find 'Core.pluginspec' in %1").arg(nativePaths);
        displayError(msgCoreLoadFailure(reason));
        return 1;
    }
    if (coreplugin->hasError()) {
        displayError(msgCoreLoadFailure(coreplugin->errorString()));
        return 1;
    }
    if (foundAppOptions.contains(QLatin1String(VERSION_OPTION))) {
        printVersion(coreplugin, pluginManager);
        return 0;
    }
    if (foundAppOptions.contains(QLatin1String(EXIT_AFTER_CONFIG_OPTION))) {
        return 0;
    }
    if (foundAppOptions.contains(QLatin1String(HELP_OPTION1))
            || foundAppOptions.contains(QLatin1String(HELP_OPTION2))
            || foundAppOptions.contains(QLatin1String(HELP_OPTION3))
            || foundAppOptions.contains(QLatin1String(HELP_OPTION4))) {
        printHelp(QFileInfo(app.applicationFilePath()).baseName(), pluginManager);
        return 0;
    }
    const bool isFirstInstance = !app.isRunning();
    if (!isFirstInstance && foundAppOptions.contains(QLatin1String(CLIENT_OPTION)))
        return sendArguments(app, pluginManager.arguments()) ? 0 : -1;

    pluginManager.loadPlugins();
    if (coreplugin->hasError()) {
        displayError(msgCoreLoadFailure(coreplugin->errorString()));
        return 1;
    }
    {
        QStringList errors;
        foreach (ExtensionSystem::PluginSpec *p, pluginManager.plugins())
            if (p->hasError())
                errors.append(p->errorString());
        if (!errors.isEmpty())
            QMessageBox::warning(0,
                QCoreApplication::translate("Application", "OpenPilot GCS - Plugin loader messages"),
                errors.join(QString::fromLatin1("\n\n")));
    }

    if (isFirstInstance) {
        // Set up lock and remote arguments for the first instance only.
        // Silently fallback to unconnected instances for any subsequent
        // instances.
        app.initialize();
        QObject::connect(&app, SIGNAL(messageReceived(QString)), coreplugin->plugin(), SLOT(remoteArgument(QString)));
    }
    QObject::connect(&app, SIGNAL(fileOpenRequest(QString)), coreplugin->plugin(), SLOT(remoteArgument(QString)));

    // Do this after the event loop has started
    QTimer::singleShot(100, &pluginManager, SLOT(startTests()));
    return app.exec();
}
