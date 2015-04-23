include(uavobjects_dependencies.pri)

# Add the include path to the built-in uavobject include files.
INCLUDEPATH += $$PWD
# Add the include path to the generated uavobject include files.
INCLUDEPATH += $$shadowed($$PWD)

LIBS *= -l$$qtLibraryName(UAVObjects)
