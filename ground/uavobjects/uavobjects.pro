TEMPLATE  = subdirs

uavobjects.target = FORCE
uavobjects.commands += mkdir -p ../../uavobject-synthetics && 
uavobjects.commands += cd ../../uavobject-synthetics && 
uavobjects.commands += ../ground/uavobjgenerator/uavobjgenerator
uavobjects.commands += -gcs ../../shared/uavobjectdefinition ../.. 
QMAKE_EXTRA_TARGETS += uavobjects

