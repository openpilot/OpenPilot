INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

# interfaces
HEADERS += $$PWD/qxtabstractfileloggerengine.h $$PWD/qxtabstractiologgerengine.h
SOURCES += $$PWD/qxtabstractfileloggerengine.cpp $$PWD/qxtabstractiologgerengine.cpp

# Basic STD Logger Engine
HEADERS += $$PWD/qxtbasicstdloggerengine.h
SOURCES += $$PWD/qxtbasicstdloggerengine.cpp

# Basic File Logger Engine
HEADERS += $$PWD/qxtbasicfileloggerengine.h
SOURCES += $$PWD/qxtbasicfileloggerengine.cpp

# XML File Logger Engine
HEADERS += $$PWD/qxtxmlfileloggerengine.h
SOURCES += $$PWD/qxtxmlfileloggerengine.cpp
