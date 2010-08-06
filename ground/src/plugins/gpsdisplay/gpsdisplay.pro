TEMPLATE = lib
TARGET = GpsDisplayGadget
QT += svg
include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(gpsdisplay_dependencies.pri)
include(../../libs/qwt/qwt.pri)
HEADERS += gpsdisplayplugin.h \
    buffer.h
HEADERS += gpsdisplaygadget.h
HEADERS += gpsdisplaywidget.h
HEADERS += gpsdisplaygadgetfactory.h
HEADERS += gpsdisplaygadgetconfiguration.h
HEADERS += gpsdisplaygadgetoptionspage.h
SOURCES += gpsdisplayplugin.cpp \
    buffer.cpp
SOURCES += gpsdisplaygadget.cpp
SOURCES += gpsdisplaygadgetfactory.cpp
SOURCES += gpsdisplaywidget.cpp
SOURCES += gpsdisplaygadgetconfiguration.cpp
SOURCES += gpsdisplaygadgetoptionspage.cpp
OTHER_FILES += GpsDisplayGadget.pluginspec
FORMS += gpsdisplaygadgetoptionspage.ui \
    gpsdisplaywidget.ui
RESOURCES += widgetresources.qrc
