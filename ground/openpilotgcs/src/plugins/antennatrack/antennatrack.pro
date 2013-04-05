TEMPLATE = lib
TARGET = AntennaTrack
include(../../openpilotgcsplugin.pri)
include(../../plugins/coreplugin/coreplugin.pri)
include(antennatrack_dependencies.pri)
include(../../libs/qwt/qwt.pri)
HEADERS += antennatrackplugin.h
HEADERS += gpsparser.h
HEADERS += telemetryparser.h
HEADERS += antennatrackgadget.h
HEADERS += antennatrackwidget.h
HEADERS += antennatrackgadgetfactory.h
HEADERS += antennatrackgadgetconfiguration.h
HEADERS += antennatrackgadgetoptionspage.h
SOURCES += antennatrackplugin.cpp
SOURCES += gpsparser.cpp
SOURCES += telemetryparser.cpp
SOURCES += antennatrackgadget.cpp
SOURCES += antennatrackgadgetfactory.cpp
SOURCES += antennatrackwidget.cpp
SOURCES += antennatrackgadgetconfiguration.cpp
SOURCES += antennatrackgadgetoptionspage.cpp
OTHER_FILES += AntennaTrack.pluginspec
FORMS += antennatrackgadgetoptionspage.ui
FORMS += antennatrackwidget.ui
