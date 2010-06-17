include(../../../openpilotgcs.pri)

COLLECTIONS = default

TEMPLATE = app
TARGET = phony_target2
CONFIG -= qt
QT =
LIBS =

isEmpty(vcproj) {
    QMAKE_LINK = @: IGNORE THIS LINE
    OBJECTS_DIR =
    win32:CONFIG -= embed_manifest_exe
} else {
    CONFIG += console
    PHONY_DEPS = .
    phony_src.input = PHONY_DEPS
    phony_src.output = phony.c
    phony_src.variable_out = GENERATED_SOURCES
    phony_src.commands = echo int main() { return 0; } > phony.c
    phony_src.name = CREATE phony.c
    phony_src.CONFIG += combine
    QMAKE_EXTRA_COMPILERS += phony_src
}

sound.files = $$PWD/$$COLLECTIONS
sound.path = /share/openpilotgcs/sounds
sound.CONFIG += no_check_exist
INSTALLS += sound
