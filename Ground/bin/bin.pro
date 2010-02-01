include(../openpilotgcs.pri)

TEMPLATE = app
TARGET = $$IDE_APP_WRAPPER
OBJECTS_DIR =

PRE_TARGETDEPS = $$PWD/openpilotgcs

QMAKE_LINK = cp $$PWD/openpilotgcs $@ && : IGNORE REST

QMAKE_CLEAN = $$IDE_APP_WRAPPER

target.path  = /bin
INSTALLS    += target
