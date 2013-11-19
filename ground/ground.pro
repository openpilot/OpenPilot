#
# Top level Qt-Creator project file for OpenPilot GCS
# Copyright (c) 2009-2013, The OpenPilot Team, http://www.openpilot.org
#
# This meta-project allows qt-creator users to open and configure a
# single project and build all required software to produce the GCS.
# This includes regenerating all uavobject-synthetic files.
#
# NOTE: to use this meta-project you MUST perform these steps once
# for each source tree checkout:
# - Open <top>/ground/ground.pro in qt-creator
# - Select the "Projects" tab
# - Under Build Settings/General heading activate "Shadow build"
# - Set "Build directory" below to <top>/build
# Here <top> = the full path to the base of your git source tree which
# should contain "flight", "ground", etc.
#
# NOTE: only debug qt-creator builds are supported on Windows.
#       Release builds may fail because it seems that qt-creator does not
#       define QTMINGW variable used to copy MinGW DLLs in release builds.
#
# There is a minor problem with dependencies. qmake needs synthetic
# files when it generates GCS Makefiles. But we do not have
# uavobjgenerator built yet. So we use the following trick: at make
# stage in uavobject-synthetics we rerun qmake for openpilotgcs.pro
# and regenerate GCS Makefiles using just built synthetic files.
# It takes some extra time but solves the dependency problem.
#
# Please note that this meta-project is only intended to be used by
# qt-creator users. Top level Makefile handles all dependencies itself
# and does not use ground.pro.

cache()

message("Make sure you have shadow build path set as noted in ground.pro. Build will fail otherwise")

TEMPLATE  = subdirs

SUBDIRS = \
        sub_openpilotgcs \
        sub_uavobject-synthetics \
        sub_uavobjgenerator

# uavobjgenerator
sub_uavobjgenerator.subdir = uavobjgenerator

# uavobject-synthetics
sub_uavobject-synthetics.subdir = uavobject-synthetics
sub_uavobject-synthetics.depends = sub_uavobjgenerator

# openpilotgcs
sub_openpilotgcs.subdir  = openpilotgcs
sub_openpilotgcs.depends = sub_uavobject-synthetics
