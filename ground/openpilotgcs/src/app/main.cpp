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

/*
 The GCS locale is set to the system locale by default unless the "hidden" setting General/Locale has a value.
 The user can not change General/Locale from the Options dialog.

 The GCS language will default to the GCS locale unless the General/OverrideLanguage has a value.
 The user can change General/OverrideLanguage to any available language from the Options dialog.

 Both General/Locale and General/OverrideLanguage can be set from the command line or through the the factory defaults file.




 The -reset switch will clear all the user settings and will trigger a reload of the factory defaults.

 You can combine it with the -configfile=<file> command line argument to quickly switch between multiple settings files.

 [code]
 openpilotgcs -reset -configfile=./MyOpenPilotGCS.xml
 [/code]

 The specified file will be used to load the factory defaults from but only when the user settings are empty.
 If the user settings are not empty the file will not be used.
 This switch is useful on the 1st run when the user settings are empty or in combination with -reset.

 */

#include "qtsingleapplication.h"
#include "utils/xmlconfig.h"
#include "gcssplashscreen.h"

#include <extensionsystem/pluginmanager.h>
#include <extensionsystem/pluginspec.h>
#include <extensionsystem/iplugin.h>

#include <QtCore/QDir>
#include <QtCore/QElapsedTimer>
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
#include <QtGui/QSplashScreen>
#include <QtGui/QPainter>

namespace {

typedef QList<ExtensionSystem::PluginSpec *> PluginSpecSet;
typedef QMap<QString, bool> AppOptions;
typedef QMap<QString, QString> FoundAppOptions;

enum {
    OptionIndent = 4, DescriptionIndent = 24
};

static const char *appNameC = "OpenPilot GCS";

static const char *corePluginNameC = "Core";

#ifdef Q_OS_MAC
static const char *SHARE_PATH = "/../Resources";
#else
static const char *SHARE_PATH = "/../share/openpilotgcs";
#endif

static const char *DEFAULT_CONFIG_FILENAME = "OpenPilotGCS.xml";

static const char *fixedOptionsC =
" [OPTION]... [FILE]...\n"
"Options:\n"
"    -help               Display this help\n"
"    -version            Display program version\n"
"    -client             Attempt to connect to already running instance\n"
"    -clean-config       Delete all existing configuration settings\n"
"    -exit-after-config  Exit GCS after manipulating configuration settings\n"
"    -D key=value        Override configuration settings e.g: -D General/OverrideLanguage=de\n"
"    -reset              Reset user settings to factory defaults.\n";

static QLatin1String HELP_OPTION1("-h");
static QLatin1String HELP_OPTION2("-help");
static QLatin1String HELP_OPTION3("/h");
static QLatin1String HELP_OPTION4("--help");
static QLatin1String VERSION_OPTION("-version");
static QLatin1String CLIENT_OPTION("-client");
static QLatin1String CONFIG_OPTION("-D");
static QLatin1String CLEAN_CONFIG_OPTION("-clean-config");
static QLatin1String EXIT_AFTER_CONFIG_OPTION("-exit-after-config");
static QLatin1String RESET("-reset");
static QLatin1String NO_SPLASH("-no-splash");

// Helpers for displaying messages. Note that there is no console on Windows.
#ifdef Q_OS_WIN
// Format as <pre> HTML
inline void toHtml(QString &t)
{
    t.replace(QLatin1Char('&'), QLatin1String("&amp;"));
    t.replace(QLatin1Char('<'), QLatin1String("&lt;"));
    t.replace(QLatin1Char('>'), QLatin1String("&gt;"));
    t.insert(0, QLatin1String("<html><pre>"));
    t.append(QLatin1String("</pre></html>"));
}

void displayHelpText(QString t) // No console on Windows.
{
    toHtml(t);
    QMessageBox::information(0, QLatin1String(appNameC), t);
}

void displayError(const QString &t) // No console on Windows.
{
    QMessageBox::critical(0, QLatin1String(appNameC), t);
}

#else

void displayHelpText(const QString &t)
{
    qWarning("%s", qPrintable(t));
}

void displayError(const QString &t)
{
    qCritical("%s", qPrintable(t));
}

#endif

void printVersion(const ExtensionSystem::PluginSpec *coreplugin, const ExtensionSystem::PluginManager &pm)
{
    QString version;
    QTextStream str(&version);
    str << '\n' << appNameC << ' ' << coreplugin->version() << " based on Qt " << qVersion() << "\n\n";
    pm.formatPluginVersions(str);
    str << '\n' << coreplugin->copyright() << '\n';
    displayHelpText(version);
}

void printHelp(const QString &a0, const ExtensionSystem::PluginManager &pm)
{
    QString help;
    QTextStream str(&help);
    str << "Usage: " << a0 << fixedOptionsC;
    ExtensionSystem::PluginManager::formatOptions(str, OptionIndent, DescriptionIndent);
    pm.formatPluginOptions(str, OptionIndent, DescriptionIndent);
    displayHelpText(help);
}

inline QString msgCoreLoadFailure(const QString &why)
{
    return QCoreApplication::translate("Application", "Failed to load core: %1").arg(why);
}

inline QString msgSendArgumentFailed()
{
    return QCoreApplication::translate("Application", "Unable to send command line arguments to the already running instance. It appears to be not responding.");
}

// Prepare a remote argument: If it is a relative file, add the current directory
// since the the central instance might be running in a different directory.

inline QString prepareRemoteArgument(const QString &a)
{
    QFileInfo fi(a);
    if (!fi.exists()) {
        return a;
    }
    if (fi.isRelative()) {
        return fi.absoluteFilePath();
    }
    return a;
}

// Send the arguments to an already running instance of OpenPilot GCS
bool sendArguments(SharedTools::QtSingleApplication &app, const QStringList &arguments)
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

void systemInit()
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
}

inline QStringList getPluginPaths()
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

AppOptions options()
{
    AppOptions appOptions;
    appOptions.insert(HELP_OPTION1, false);
    appOptions.insert(HELP_OPTION2, false);
    appOptions.insert(HELP_OPTION3, false);
    appOptions.insert(HELP_OPTION4, false);
    appOptions.insert(VERSION_OPTION, false);
    appOptions.insert(CLIENT_OPTION, false);
    appOptions.insert(CONFIG_OPTION, true);
    appOptions.insert(CLEAN_CONFIG_OPTION, false);
    appOptions.insert(EXIT_AFTER_CONFIG_OPTION, false);
    appOptions.insert(RESET, false);
    appOptions.insert(NO_SPLASH, false);
    return appOptions;
}

FoundAppOptions parseCommandLine(SharedTools::QtSingleApplication &app, ExtensionSystem::PluginManager &pluginManager, QString &errorMessage)
{
    FoundAppOptions foundAppOptions;
    const QStringList arguments = app.arguments();
    if (arguments.size() > 1) {
        AppOptions appOptions = options();
        if (!pluginManager.parseOptions(arguments, appOptions, &foundAppOptions, &errorMessage)) {
//            displayError(errorMessage);
//            printHelp(QFileInfo(app.applicationFilePath()).baseName(), pluginManager);
        }
    }
    return foundAppOptions;
}

void loadFactoryDefaults(QSettings &settings)
{
    QDir directory(QCoreApplication::applicationDirPath() + QString(SHARE_PATH) + QString("/default_configurations"));

    qDebug() << "Looking for configuration files in:" << directory.absolutePath();

    // check if command line contains a config file name
    QString commandLine;
    foreach(QString str, qApp->arguments()) {
        if (str.contains("configfile")) {
            commandLine = str.split("=").at(1);
        }
    }
    QString filename;
    if (!commandLine.isEmpty() && QFile::exists(directory.absolutePath() + QDir::separator() + commandLine)) {
        // use file name specified on command line
        filename = directory.absolutePath() + QDir::separator() + commandLine;
        qDebug() << "Configuration file" << filename << "specified on command line will be loaded.";
    } else if (QFile::exists(directory.absolutePath() + QDir::separator() + DEFAULT_CONFIG_FILENAME)) {
        // use default file name
        filename = directory.absolutePath() + QDir::separator() + DEFAULT_CONFIG_FILENAME;
        qDebug() << "Default configuration file" << filename << "will be loaded.";
    } else {
        // TODO should we exit violently?
        qWarning() << "No default configuration file found!";
        return;
    }

    // create settings from file
    QSettings qs(filename, XmlConfig::XmlSettingsFormat);

    // transfer loaded settings to application settings
    QStringList keys = qs.allKeys();
    foreach(QString key, keys) {
        settings.setValue(key, qs.value(key));
    }

    qDebug() << "Configuration file" << filename << "was loaded.";
}

void overrideSettings(QSettings &settings, int argc, char **argv)
{
    // Options like -DMy/setting=test
    QRegExp rx("([^=]+)=(.*)");

    QMap<QString, QString> settingOptions;
    for (int i = 0; i < argc; ++i) {
        if (QString(CONFIG_OPTION).compare(QString(argv[i])) == 0) {
            if (rx.indexIn(argv[++i]) > -1) {
                settingOptions.insert(rx.cap(1), rx.cap(2));
            }
        }
        if (QString(CLEAN_CONFIG_OPTION).compare(QString(argv[i])) == 0) {
            settings.clear();
        }
    }

    QList<QString> keys = settingOptions.keys();
    foreach (QString key, keys) {
        qDebug() << "Overriding user setting:" << key << "with value" << settingOptions.value(key);
        settings.setValue(key, settingOptions.value(key));
    }

    settings.sync();
}

void loadTranslators(QString language)
{
    // TODO static!?!
    static QTranslator translator;
    static QTranslator qtTranslator;

    const QString &creatorTrPath = QCoreApplication::applicationDirPath() + QLatin1String(SHARE_PATH) + QLatin1String("/translations");
    if (translator.load(QLatin1String("openpilotgcs_") + language, creatorTrPath)) {
        const QString &qtTrPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
        const QString &qtTrFile = QLatin1String("qt_") + language;
        // Binary installer puts Qt tr files into creatorTrPath
        if (qtTranslator.load(qtTrFile, qtTrPath) || qtTranslator.load(qtTrFile, creatorTrPath)) {
            QCoreApplication::installTranslator(&translator);
            QCoreApplication::installTranslator(&qtTranslator);
        } else {
            // unload()
            translator.load(QString());
        }
    }
}

} //  namespace anonymous

int main(int argc, char **argv)
{
    QElapsedTimer timer;
    timer.start();

    // low level init
    systemInit();

    // create application
    SharedTools::QtSingleApplication app((QLatin1String(appNameC)), argc, argv);

    // initialize the plugin manager
    ExtensionSystem::PluginManager pluginManager;
    pluginManager.setFileExtension(QLatin1String("pluginspec"));
    pluginManager.setPluginPaths(getPluginPaths());

    // parse command line
    qDebug() << "Command line" << app.arguments();;
    QString errorMessage;
    FoundAppOptions foundAppOptions = parseCommandLine(app, pluginManager, errorMessage);
    if (!errorMessage.isEmpty()) {
        displayError(errorMessage);
        printHelp(QFileInfo(app.applicationFilePath()).baseName(), pluginManager);
        return -1;
    }

    // load user settings
    // Must be done before any QSettings class is created
    // keep this in sync with the MainWindow ctor in coreplugin/mainwindow.cpp
    QString settingsPath = QCoreApplication::applicationDirPath() + QLatin1String(SHARE_PATH);
    qDebug() << "Loading user settings from" << settingsPath;
    QSettings::setPath(XmlConfig::XmlSettingsFormat, QSettings::SystemScope, settingsPath);
    QSettings settings(XmlConfig::XmlSettingsFormat, QSettings::UserScope, QLatin1String("OpenPilot"),
            QLatin1String("OpenPilotGCS_config"));

    // need to reset all user settings?
    if (foundAppOptions.contains(RESET)) {
        qDebug() << "Resetting user settings!";
        settings.clear();
    }

    // check if we have user settings
    if (!settings.allKeys().count()) {
        // no user settings, load the factory defaults
        qDebug() << "No user settings found, loading factory defaults...";
        loadFactoryDefaults(settings);
    }

    // override settings with command line provided values
    // take notice that the overridden values will be saved in the user settings and will continue to be effective
    // in subsequent GCS runs
    overrideSettings(settings, argc, argv);

    // initialize GCS locale
    // use the value defined by the General/Locale setting or default to system locale.
    // the General/Locale setting is not available in the Options dialog, it is a hidden setting but can still be changed:
    // - through the command line
    // - editing the factory defaults XML file before 1st launch
    // - editing the user XML file
    QString localeName = settings.value("General/Locale", QLocale::system().name()).toString();
    QLocale::setDefault(localeName);

    // some debuging
    qDebug() << "main - system locale:" << QLocale::system().name();
    qDebug() << "main - GCS locale:" << QLocale().name();

    // load translation file
    // the language used is defined by the General/OverrideLanguage setting (defaults to GCS locale)
    // if the translation file for the given language is not found, GCS will default to built in English.
    QString language = settings.value("General/OverrideLanguage", localeName).toString();
    qDebug() << "main - translation language:" << language;
    loadTranslators(language);

    app.setProperty("qtc_locale", localeName); // Do we need this?

    // open the splash screen
    GCSSplashScreen *splash = 0;
    if (!foundAppOptions.contains(NO_SPLASH)) {
        splash = new GCSSplashScreen();
        // show splash
        splash->showProgressMessage(QObject::tr("Application starting..."));
        splash->show();
        // connect to track progress of plugin manager
        QObject::connect(&pluginManager, SIGNAL(pluginAboutToBeLoaded(ExtensionSystem::PluginSpec*)),
                    splash, SLOT(showPluginLoadingProgress(ExtensionSystem::PluginSpec*)));
    }

    // find and load core plugin
    const PluginSpecSet plugins = pluginManager.plugins();
    ExtensionSystem::PluginSpec *coreplugin = 0;
    foreach (ExtensionSystem::PluginSpec *spec, plugins) {
        if (spec->name() == QLatin1String(corePluginNameC)) {
            coreplugin = spec;
            break;
        }
    }
    if (!coreplugin) {
        QString nativePaths = QDir::toNativeSeparators(getPluginPaths().join(QLatin1String(",")));
        const QString reason = QCoreApplication::translate("Application", "Could not find 'Core.pluginspec' in %1").arg(
                nativePaths);
        displayError(msgCoreLoadFailure(reason));
        return 1;
    }
    if (coreplugin->hasError()) {
        displayError(msgCoreLoadFailure(coreplugin->errorString()));
        return 1;
    }

    if (foundAppOptions.contains(VERSION_OPTION)) {
        printVersion(coreplugin, pluginManager);
        return 0;
    }
    if (foundAppOptions.contains(EXIT_AFTER_CONFIG_OPTION)) {
        return 0;
    }
    if (foundAppOptions.contains(HELP_OPTION1)
            || foundAppOptions.contains(HELP_OPTION2)
            || foundAppOptions.contains(HELP_OPTION3)
            || foundAppOptions.contains(HELP_OPTION4)) {
        printHelp(QFileInfo(app.applicationFilePath()).baseName(), pluginManager);
        return 0;
    }

    const bool isFirstInstance = !app.isRunning();
    if (!isFirstInstance && foundAppOptions.contains(CLIENT_OPTION)) {
        return sendArguments(app, pluginManager.arguments()) ? 0 : -1;
    }

    pluginManager.loadPlugins();

    if (coreplugin->hasError()) {
        displayError(msgCoreLoadFailure(coreplugin->errorString()));
        return 1;
    }

    {
        QStringList errors;
        foreach (ExtensionSystem::PluginSpec *p, pluginManager.plugins()) {
            if (p->hasError()) {
                errors.append(p->errorString());
            }
        }
        if (!errors.isEmpty()) {
            QMessageBox::warning(0,
                    QCoreApplication::translate("Application", "OpenPilot GCS - Plugin loader messages"),
                    errors.join(QString::fromLatin1("\n\n")));
        }
    }

    if (isFirstInstance) {
        // Set up lock and remote arguments for the first instance only.
        // Silently fallback to unconnected instances for any subsequent instances.
        app.initialize();
        QObject::connect(&app, SIGNAL(messageReceived(QString)), coreplugin->plugin(), SLOT(remoteArgument(QString)));
    }
    QObject::connect(&app, SIGNAL(fileOpenRequest(QString)), coreplugin->plugin(), SLOT(remoteArgument(QString)));

    // Do this after the event loop has started
    QTimer::singleShot(100, &pluginManager, SLOT(startTests()));

    if (splash) {
        // Update message and postpone closing of splashscreen 3 seconds
        splash->showProgressMessage(QObject::tr("Application started."));
        QTimer::singleShot(1500, splash, SLOT(close()));
        // TODO delete splash
    }

    qDebug() << "main - main took" << timer.elapsed() << "ms";

    int ret = app.exec();

    qDebug() << "main - GCS ran for" << timer.elapsed() << "ms";

    return ret;
}
