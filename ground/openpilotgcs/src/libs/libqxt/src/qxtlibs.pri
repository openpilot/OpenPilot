INCLUDEPATH += $${QXT_SOURCE_TREE}/include
DEPENDPATH += $${QXT_SOURCE_TREE}/include
unix:!macx:LIBS += -Wl,-rpath,$${QXT_BUILD_TREE}/lib
macx:LIBS += -F$${QXT_BUILD_TREE}/lib
LIBS += -L$${QXT_BUILD_TREE}/lib

defineTest(qxtAddLibrary) {
    INCLUDEPATH += $${QXT_SOURCE_TREE}/src/$$1
    INCLUDEPATH += $${QXT_SOURCE_TREE}/include/$$2
    DEPENDPATH += $${QXT_SOURCE_TREE}/src/$$1
    DEPENTPATH += $${QXT_SOURCE_TREE}/include/$$2
    qtAddLibrary($$2)
}

contains(QXT, berkeley) {
    qxtAddLibrary(berkeley, QxtBerkeley)
    QXT += core
}

contains(QXT, web) {
    qxtAddLibrary(web, QxtWeb)
    QXT += core network
    QT  += network
}

contains(QXT, gui) {
    qxtAddLibrary(gui, QxtGui)
    QXT += core
    QT  += gui
}

contains(QXT, network) {
    qxtAddLibrary(network, QxtNetwork)
    QXT += core
    QT  += network
}

contains(QXT, sql) {
    qxtAddLibrary(sql, QxtSql)
    QXT += core
    QT  += sql
}

contains(QXT, core) {
    qxtAddLibrary(core, QxtCore)
}
