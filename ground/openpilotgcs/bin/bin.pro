include(../openpilotgcs.pri)

TEMPLATE = app
TARGET = $$GCS_APP_WRAPPER
OBJECTS_DIR =

PRE_TARGETDEPS = $$PWD/openpilotgcs

QMAKE_LINK = cp $$PWD/openpilotgcs $@ && : IGNORE REST

QMAKE_CLEAN = $$GCS_APP_WRAPPER

target.path  = /bin
INSTALLS    += target
