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

 The -D option is used to permanently set a user setting.

 The -reset switch will clear all the user settings and will trigger a reload of the factory defaults.

 You can combine it with the -config-file=<file> command line argument to quickly switch between multiple settings files.

 [code]
 openpilotgcs -reset -config-file ./MyOpenPilotGCS.xml
 [/code]

 Relative paths are relative to <install dir>/share/openpilotgcs/default_configurations/

 The specified file will be used to load the factory defaults from but only when the user settings are empty.
 If the user settings are not empty the file will not be used.
 This switch is useful on the 1st run when the user settings are empty or in combination with -reset.


 Quickly switch configurations

 [code]
 openpilotgcs -reset -config-file <relative or absolute path>
 [/code]

 Configuring GCS from installer

 The -D option is used to permanently set a user setting.

 If the user chooses to start GCS at the end of the installer:

 [code]
 openpilotgcs -D General/OverrideLanguage=de
 [/code]

 If the user chooses not to start GCS at the end of the installer, you still need to configure GCS.
 In that case you can use -exit-after-config

 [code]
 openpilotgcs -D General/OverrideLanguage=de -exit-after-config
 [/code]

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
    typedef QMap<QString, QString> AppOptionValues;

    enum {
        OptionIndent = 4, DescriptionIndent = 24
    };

    const QLatin1String APP_NAME("OpenPilot GCS");

    const QLatin1String CORE_PLUGIN_NAME("Core");

    const QLatin1String SETTINGS_ORG_NAME("OpenPilot");
    const QLatin1String SETTINGS_APP_NAME("OpenPilotGCS_config");

#ifdef Q_OS_MAC
    const QLatin1String SHARE_PATH("/../Resources");
#else
    const QLatin1String SHARE_PATH("/../share/openpilotgcs");
#endif

    const char *DEFAULT_CONFIG_FILENAME = "OpenPilotGCS.xml";

    const char *fixedOptionsC = " [OPTION]...\n"
            "Options:\n"
            "    -help               Display this help\n"
            "    -version            Display application version\n"
            "    -no-splash          Don't display splash screen\n"
            "    -client             Attempt to connect to already running instance\n"
            "    -D <key>=<value>    Permanently set a user setting, e.g: -D General/OverrideLanguage=de\n"
            "    -reset              Reset user settings to factory defaults.\n"
            "    -config-file <file> Specify alternate factory defaults settings file (used with -reset)\n"
            "    -exit-after-config  Exit after manipulating configuration settings\n";

    const QLatin1String HELP1_OPTION("-h");
    const QLatin1String HELP2_OPTION("-help");
    const QLatin1String HELP3_OPTION("/h");
    const QLatin1String HELP4_OPTION("--help");
    const QLatin1String VERSION_OPTION("-version");
    const QLatin1String NO_SPLASH_OPTION("-no-splash");
    const QLatin1String CLIENT_OPTION("-client");
    const QLatin1String CONFIG_OPTION("-D");
    const QLatin1String RESET_OPTION("-reset");
    const QLatin1String CONFIG_FILE_OPTION("-config-file");
    const QLatin1String EXIT_AFTER_CONFIG_OPTION("-exit-after-config");

    // Helpers for displaying messages. Note that there is no console on Windows.
    void displayHelpText(QString t)
    {
#ifdef Q_OS_WIN
        // No console on Windows. (???)
        // TODO there is a console on windows and popups are not always desired
        t.replace(QLatin1Char('&'), QLatin1String("&amp;"));
        t.replace(QLatin1Char('<'), QLatin1String("&lt;"));
        t.replace(QLatin1Char('>'), QLatin1String("&gt;"));
        t.insert(0, QLatin1String("<html><pre>"));
        t.append(QLatin1String("</pre></html>"));
        QMessageBox::information(0, APP_NAME, t);
#else
        qWarning("%s", qPrintable(t));
#endif
    }

    void displayError(const QString &t)
    {
#ifdef Q_OS_WIN
        // No console on Windows. (???)
        // TODO there is a console on windows and popups are not always desired
        QMessageBox::critical(0, APP_NAME, t);
#else
        qCritical("%s", qPrintable(t));
#endif
    }

    void printVersion(const ExtensionSystem::PluginSpec *corePlugin, const ExtensionSystem::PluginManager &pm)
    {
        QString version;
        QTextStream str(&version);
        str << '\n' << APP_NAME << ' ' << corePlugin->version() << " based on Qt " << qVersion() << "\n\n";
        pm.formatPluginVersions(str);
        str << '\n' << corePlugin->copyright() << '\n';
        displayHelpText(version);
    }

    void printHelp(const QString &appExecName, const ExtensionSystem::PluginManager &pm)
    {
        QString help;
        QTextStream str(&help);
        str << "Usage: " << appExecName << fixedOptionsC;
        ExtensionSystem::PluginManager::formatOptions(str, OptionIndent, DescriptionIndent);
        pm.formatPluginOptions(str, OptionIndent, DescriptionIndent);
        displayHelpText(help);
    }

    inline QString msgCoreLoadFailure(const QString &reason)
    {
        return QCoreApplication::translate("Application", "Failed to load core plug-in, reason is: %1").arg(reason);
    }

    inline QString msgSendArgumentFailed()
    {
        return QCoreApplication::translate("Application",
                "Unable to send command line arguments to the already running instance. It appears to be not responding.");
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

    // Send the arguments to an already running instance of application
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
        // increase the number of file that can be opened in application
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
        appOptions.insert(HELP1_OPTION, false);
        appOptions.insert(HELP2_OPTION, false);
        appOptions.insert(HELP3_OPTION, false);
        appOptions.insert(HELP4_OPTION, false);
        appOptions.insert(VERSION_OPTION, false);
        appOptions.insert(NO_SPLASH_OPTION, false);
        appOptions.insert(CLIENT_OPTION, false);
        appOptions.insert(CONFIG_OPTION, true);
        appOptions.insert(RESET_OPTION, false);
        appOptions.insert(CONFIG_FILE_OPTION, true);
        appOptions.insert(EXIT_AFTER_CONFIG_OPTION, false);
        return appOptions;
    }

    AppOptionValues parseCommandLine(SharedTools::QtSingleApplication &app,
            ExtensionSystem::PluginManager &pluginManager, QString &errorMessage)
    {
        AppOptionValues appOptionValues;
        const QStringList arguments = app.arguments();
        if (arguments.size() > 1) {
            AppOptions appOptions = options();
            pluginManager.parseOptions(arguments, appOptions, &appOptionValues, &errorMessage);
        }
        return appOptionValues;
    }

    void loadFactoryDefaults(QSettings &settings, AppOptionValues &appOptionValues)
    {
        QDir directory(QCoreApplication::applicationDirPath() + SHARE_PATH + QString("/default_configurations"));
        qDebug() << "Looking for factory defaults configuration files in:" << directory.absolutePath();

        QString fileName;

        // check if command line option -config-file contains a file name
        QString commandLine = appOptionValues.value(CONFIG_FILE_OPTION);
        if (!commandLine.isEmpty()) {
            if (QFile::exists(directory.absolutePath() + QDir::separator() + commandLine)) {
                // file name specified on command line has a relative path
                fileName = directory.absolutePath() + QDir::separator() + commandLine;
                qDebug() << "Configuration file" << fileName << "specified on command line will be loaded.";
            } else if (QFile::exists(commandLine)) {
                // file name specified on command line has an absolutee path
                fileName = commandLine;
                qDebug() << "Configuration file" << fileName << "specified on command line will be loaded.";
            } else {
                qWarning() << "Configuration file" << commandLine << "specified on command line does not exist.";
            }
        }

        if (fileName.isEmpty()) {
            // check default file
            if (QFile::exists(directory.absolutePath() + QDir::separator() + DEFAULT_CONFIG_FILENAME)) {
                // use default file name
                fileName = directory.absolutePath() + QDir::separator() + DEFAULT_CONFIG_FILENAME;
                qDebug() << "Default configuration file" << fileName << "will be loaded.";
            } else {
                qWarning() << "No default configuration file found in" << directory.absolutePath();
            }
        }

        if (fileName.isEmpty()) {
            // TODO should we exit violently?
            qCritical() << "No default configuration file found!";
            return;
        }

        // create settings from file
        QSettings qs(fileName, XmlConfig::XmlSettingsFormat);

        // transfer loaded settings to application settings
        QStringList keys = qs.allKeys();
        foreach(QString key, keys) {
            settings.setValue(key, qs.value(key));
        }

        qDebug() << "Configuration file" << fileName << "was loaded.";
    }

    void overrideSettings(QSettings &settings, int argc, char **argv)
    {
        // Options like -D My/setting=test
        QRegExp rx("([^=]+)=(.*)");

        for (int i = 0; i < argc; ++i) {
            if (CONFIG_OPTION == QString(argv[i])) {
                if (rx.indexIn(argv[++i]) > -1) {
                    QString key = rx.cap(1);
                    QString value = rx.cap(2);
                    qDebug() << "User setting" << key << "set to value" << value;
                    settings.setValue(key, value);
                }
            }
        }

        settings.sync();
    }

    void loadTranslators(QString language, QTranslator &translator, QTranslator &qtTranslator)
    {
        const QString &creatorTrPath = QCoreApplication::applicationDirPath() + SHARE_PATH
                + QLatin1String("/translations");
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
    SharedTools::QtSingleApplication app(APP_NAME, argc, argv);

    // initialize the plugin manager
    ExtensionSystem::PluginManager pluginManager;
    pluginManager.setFileExtension(QLatin1String("pluginspec"));
    pluginManager.setPluginPaths(getPluginPaths());

    // parse command line
    qDebug() << "Command line" << app.arguments();
    QString errorMessage;
    AppOptionValues appOptionValues = parseCommandLine(app, pluginManager, errorMessage);
    if (!errorMessage.isEmpty()) {
        // this will display two popups : one error popup + one usage string popup
        // TODO merge two popups into one.
        displayError(errorMessage);
        printHelp(QFileInfo(app.applicationFilePath()).baseName(), pluginManager);
        return -1;
    }

    // load user settings
    // Must be done before any QSettings class is created
    // keep this in sync with the MainWindow ctor in coreplugin/mainwindow.cpp
    QString settingsPath = QCoreApplication::applicationDirPath() + SHARE_PATH;
    qDebug() << "Loading system settings from" << settingsPath;
    QSettings::setPath(XmlConfig::XmlSettingsFormat, QSettings::SystemScope, settingsPath);
    qDebug() << "Loading user settings from" << SETTINGS_ORG_NAME << "/" << SETTINGS_APP_NAME;
    QSettings settings(XmlConfig::XmlSettingsFormat, QSettings::UserScope, SETTINGS_ORG_NAME, SETTINGS_APP_NAME);

    // need to reset all user settings?
    if (appOptionValues.contains(RESET_OPTION)) {
        qDebug() << "Resetting user settings!";
        settings.clear();
    }

    // check if we have user settings
    if (!settings.allKeys().count()) {
        // no user settings, load the factory defaults
        qDebug() << "No user settings found, loading factory defaults...";
        loadFactoryDefaults(settings, appOptionValues);
    }

    // override settings with command line provided values
    // take notice that the overridden values will be saved in the user settings and will continue to be effective
    // in subsequent GCS runs
    overrideSettings(settings, argc, argv);

    // initialize GCS locale
    // use the value defined by the General/Locale setting or default to system Locale.
    // the General/Locale setting is not available in the Options dialog, it is a hidden setting but can still be changed:
    // - through the command line
    // - editing the factory defaults XML file before 1st launch
    // - editing the user XML file
    QString localeName = settings.value("General/Locale", QLocale::system().name()).toString();
    QLocale::setDefault(localeName);

    // some debugging
    qDebug() << "main - system locale:" << QLocale::system().name();
    qDebug() << "main - GCS locale:" << QLocale().name();

    // load translation file
    // the language used is defined by the General/OverrideLanguage setting (defaults to GCS locale)
    // if the translation file for the given language is not found, GCS will default to built in English.
    QString language = settings.value("General/OverrideLanguage", localeName).toString();
    qDebug() << "main - language:" << language;
    QTranslator translator;
    QTranslator qtTranslator;
    loadTranslators(language, translator, qtTranslator);

    app.setProperty("qtc_locale", localeName); // Do we need this?

    if (appOptionValues.contains(EXIT_AFTER_CONFIG_OPTION)) {
        qDebug() << "main - exiting after config!";
        return 0;
    }

    // open the splash screen
    GCSSplashScreen *splash = 0;
    if (!appOptionValues.contains(NO_SPLASH_OPTION)) {
        splash = new GCSSplashScreen();
        // show splash
        splash->showProgressMessage(QObject::tr("Application starting..."));
        splash->show();
        // connect to track progress of plugin manager
        QObject::connect(&pluginManager, SIGNAL(pluginAboutToBeLoaded(ExtensionSystem::PluginSpec*)), splash,
                SLOT(showPluginLoadingProgress(ExtensionSystem::PluginSpec*)));
    }

    // find and load core plugin
    const PluginSpecSet plugins = pluginManager.plugins();
    ExtensionSystem::PluginSpec *coreplugin = 0;
    foreach (ExtensionSystem::PluginSpec *spec, plugins) {
        if (spec->name() == CORE_PLUGIN_NAME) {
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

    if (appOptionValues.contains(VERSION_OPTION)) {
        printVersion(coreplugin, pluginManager);
        return 0;
    }
    if (appOptionValues.contains(HELP1_OPTION) || appOptionValues.contains(HELP2_OPTION)
            || appOptionValues.contains(HELP3_OPTION) || appOptionValues.contains(HELP4_OPTION)) {
        printHelp(QFileInfo(app.applicationFilePath()).baseName(), pluginManager);
        return 0;
    }

    const bool isFirstInstance = !app.isRunning();
    if (!isFirstInstance && appOptionValues.contains(CLIENT_OPTION)) {
        return sendArguments(app, pluginManager.arguments()) ? 0 : -1;
    }

    pluginManager.loadPlugins();

    if (coreplugin->hasError()) {
        displayError(msgCoreLoadFailure(coreplugin->errorString()));
        return 1;
    }

    {
        QStringList errors;
        foreach (ExtensionSystem::PluginSpec *p, pluginManager.plugins())
        {
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
        // close and delete splash
        splash->close();
        delete splash;
    }

    qDebug() << "main - main took" << timer.elapsed() << "ms";

    int ret = app.exec();

    qDebug() << "main - GCS ran for" << timer.elapsed() << "ms";

    return ret;
}
