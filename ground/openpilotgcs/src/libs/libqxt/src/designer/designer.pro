CLEAN_TARGET     = QxtDesignerPlugins
DEFINES         += BUILD_QXT_DESIGNER
QT               = core gui
QXT              = core gui
CONVENIENCE     +=

include(designer.pri)
#include(../qxtbase.pri)

CONFIG          += designer plugin
target.path      = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS         = target
