TEMPLATE = lib
TARGET = Core
DEFINES += CORE_LIBRARY
QT += xml \
    network \
    script \
    svg \
    sql
include(../../openpilotgcsplugin.pri)
include(../../libs/utils/utils.pri)
include(../../shared/scriptwrapper/scriptwrapper.pri)
include(coreplugin_dependencies.pri)
INCLUDEPATH += dialogs \
    actionmanager
DEPENDPATH += dialogs \
    actionmanager
SOURCES += mainwindow.cpp \
    tabpositionindicator.cpp \
    fancyactionbar.cpp \
    fancytabwidget.cpp \
    generalsettings.cpp \
    uniqueidmanager.cpp \
    messagemanager.cpp \
    messageoutputwindow.cpp \
    viewmanager.cpp \
    versiondialog.cpp \
    actionmanager/actionmanager.cpp \
    actionmanager/command.cpp \
    actionmanager/actioncontainer.cpp \
    actionmanager/commandsfile.cpp \
    dialogs/saveitemsdialog.cpp \
    dialogs/newdialog.cpp \
    dialogs/settingsdialog.cpp \
    dialogs/shortcutsettings.cpp \
    dialogs/openwithdialog.cpp \
    basemode.cpp \
    baseview.cpp \
    coreplugin.cpp \
    variablemanager.cpp \
    modemanager.cpp \
    coreimpl.cpp \
    basefilewizard.cpp \
    plugindialog.cpp \
    manhattanstyle.cpp \
    minisplitter.cpp \
    styleanimator.cpp \
    rightpane.cpp \
    sidebar.cpp \
    mimedatabase.cpp \
    icore.cpp \
    dialogs/ioptionspage.cpp \
    dialogs/iwizard.cpp \
    settingsdatabase.cpp \
    eventfilteringmainwindow.cpp
HEADERS += mainwindow.h \
    tabpositionindicator.h \
    fancyactionbar.h \
    fancytabwidget.h \
    generalsettings.h \
    uniqueidmanager.h \
    messagemanager.h \
    messageoutputwindow.h \
    viewmanager.h \
    actionmanager/actioncontainer.h \
    actionmanager/actionmanager.h \
    actionmanager/command.h \
    actionmanager/actionmanager_p.h \
    actionmanager/command_p.h \
    actionmanager/actioncontainer_p.h \
    actionmanager/commandsfile.h \
    dialogs/saveitemsdialog.h \
    dialogs/newdialog.h \
    dialogs/settingsdialog.h \
    dialogs/shortcutsettings.h \
    dialogs/openwithdialog.h \
    dialogs/iwizard.h \
    dialogs/ioptionspage.h \
    icontext.h \
    icore.h \
    ifile.h \
    ifilefactory.h \
    imode.h \
    ioutputpane.h \
    coreconstants.h \
    iversioncontrol.h \
    iview.h \
    ifilewizardextension.h \
    icorelistener.h \
    versiondialog.h \
    core_global.h \
    basemode.h \
    baseview.h \
    coreplugin.h \
    variablemanager.h \
    modemanager.h \
    coreimpl.h \
    basefilewizard.h \
    plugindialog.h \
    manhattanstyle.h \
    minisplitter.h \
    styleanimator.h \
    rightpane.h \
    sidebar.h \
    mimedatabase.h \
    settingsdatabase.h \
    eventfilteringmainwindow.h
FORMS += dialogs/newdialog.ui \
    dialogs/settingsdialog.ui \
    dialogs/shortcutsettings.ui \
    dialogs/saveitemsdialog.ui \
    dialogs/openwithdialog.ui \
    generalsettings.ui
RESOURCES += core.qrc \
    fancyactionbar.qrc
unix:!macx { 
    images.files = images/openpilot_logo_*.png
    images.files = images/qtcreator_logo_*.png
    images.path = /share/pixmaps
    INSTALLS += images
}
OTHER_FILES += Core.pluginspec
