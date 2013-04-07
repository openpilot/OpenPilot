TEMPLATE = lib
TARGET = GpsDisplayGadget
QT += svg
include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(gpsdisplay_dependencies.pri)
include(../../libs/qwt/qwt.pri)
HEADERS += gpsdisplayplugin.h
HEADERS += gpsconstellationwidget.h
HEADERS += gpsparser.h
HEADERS += telemetryparser.h
HEADERS += gpssnrwidget.h
HEADERS += buffer.h
HEADERS += nmeaparser.h
HEADERS += gpsdisplaygadget.h
HEADERS += gpsdisplaywidget.h
HEADERS += gpsdisplaygadgetfactory.h
HEADERS += gpsdisplaygadgetconfiguration.h
HEADERS += gpsdisplaygadgetoptionspage.h
SOURCES += gpsdisplayplugin.cpp
SOURCES += gpsconstellationwidget.cpp
SOURCES += gpsparser.cpp
SOURCES += telemetryparser.cpp
SOURCES += gpssnrwidget.cpp
SOURCES += buffer.cpp
SOURCES += nmeaparser.cpp
SOURCES += gpsdisplaygadget.cpp
SOURCES += gpsdisplaygadgetfactory.cpp
SOURCES += gpsdisplaywidget.cpp
SOURCES += gpsdisplaygadgetconfiguration.cpp
SOURCES += gpsdisplaygadgetoptionspage.cpp
OTHER_FILES += GpsDisplayGadget.pluginspec
FORMS += gpsdisplaygadgetoptionspage.ui
FORMS += gpsdisplaywidget.ui
RESOURCES += widgetresources.qrc
