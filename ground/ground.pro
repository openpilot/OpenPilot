#
# Top level Qt-Creator project file
#

TEMPLATE  = subdirs

SUBDIRS = \
        sub_openpilotgcs \
        sub_uavobjects \
        sub_uavobjgenerator

# uavobjgenerator
sub_uavobjgenerator.subdir = uavobjgenerator

# uavobjects
sub_uavobjects.subdir  = uavobjects
sub_uavobjects.depends = sub_uavobjgenerator

# openpilotgcs
sub_openpilotgcs.subdir  = openpilotgcs
sub_openpilotgcs.depends = sub_uavobjects

