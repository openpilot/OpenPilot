include(../../plugins/coreplugin/coreplugin.pri)
include(../../libs/utils/utils.pri)

# Provide the path to the auto-generated uavobject source files for the GCS.
UAVOBJECT_SYNTHETICS=$${GCS_BUILD_TREE}/../../uavobject-synthetics/gcs
#message(UAVOBJECT_SYNTHETICS is $$UAVOBJECT_SYNTHETICS)

# Add the include path to the auto-generated uavobject include files.
INCLUDEPATH += $$UAVOBJECT_SYNTHETICS
