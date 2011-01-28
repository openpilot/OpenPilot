include(../../../openpilotgcs.pri)

LANGUAGES = de es fr ru zh_CN

# var, prepend, append
defineReplace(prependAll) {
    for(a,$$1):result += $$2$${a}$$3
    return($$result)
}

XMLPATTERNS = $$targetPath($$[QT_INSTALL_BINS]/xmlpatterns)
LUPDATE = $$targetPath($$[QT_INSTALL_BINS]/lupdate) -locations relative -no-ui-lines -no-sort -noobsolete
LRELEASE = $$targetPath($$[QT_INSTALL_BINS]/lrelease)

TRANSLATIONS = $$prependAll(LANGUAGES, $$PWD/openpilotgcs_,.ts)

MIME_TR_H = $$PWD/mime_tr.h

contains(QT_VERSION, ^4\\.[0-5]\\..*) {
    ts.commands = @echo This Qt version is too old for the ts target. Need Qt 4.6+.
} else {
    for(dir, $$list($$files($$GCS_SOURCE_TREE/src/plugins/*))):MIMETYPES_FILES += $$files($$dir/*.mimetypes.xml)
    MIMETYPES_FILES = \"$$join(MIMETYPES_FILES, \", \")\"
    QMAKE_SUBSTITUTES += extract-mimetypes.xq.in
    ts.commands += \
        $$XMLPATTERNS -output $$MIME_TR_H $$PWD/extract-mimetypes.xq && \
        (cd $$GCS_SOURCE_TREE && $$LUPDATE src $$MIME_TR_H -ts $$TRANSLATIONS) && \
        $$QMAKE_DEL_FILE $$targetPath($$MIME_TR_H)
}
QMAKE_EXTRA_TARGETS += ts

TEMPLATE = app
TARGET = phony_target2
CONFIG -= qt
QT =
LIBS =

updateqm.input = TRANSLATIONS
updateqm.output = $$GCS_DATA_PATH/translations/${QMAKE_FILE_BASE}.qm
isEmpty(vcproj):updateqm.variable_out = PRE_TARGETDEPS
updateqm.commands = $$LRELEASE ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
updateqm.name = LRELEASE ${QMAKE_FILE_IN}
updateqm.CONFIG += no_link
QMAKE_EXTRA_COMPILERS += updateqm

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

qmfiles.files = $$prependAll(LANGUAGES, $$OUT_PWD/openpilotgcs_,.qm)
qmfiles.path = /share/openpilotgcs/translations
qmfiles.CONFIG += no_check_exist
INSTALLS += qmfiles

#========= begin block copying qt_*.qm files ==========
win32 {
    defineReplace(QtQmExists) {
        for(lang,$$1) {
            qm_file = $$[QT_INSTALL_TRANSLATIONS]/qt_$${lang}.qm
            exists($$qm_file) : result += $$qm_file
        }
        return($$result)
    }
    QT_TRANSLATIONS = $$QtQmExists(LANGUAGES)

    copyQT_QMs.input = QT_TRANSLATIONS
    copyQT_QMs.output = $$GCS_DATA_PATH/translations/${QMAKE_FILE_BASE}.qm
    isEmpty(vcproj):copyQT_QMs.variable_out = PRE_TARGETDEPS
    copyQT_QMs.commands = $(COPY_FILE) ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
    copyQT_QMs.name = Copy ${QMAKE_FILE_IN}
    copyQT_QMs.CONFIG += no_link
    QMAKE_EXTRA_COMPILERS += copyQT_QMs
}
#========= end block copying qt_*.qm files ============
