#
# Top level Qt-Creator project file
#

# This meta-project allows qt-creator users to open and configure a
# single project and to build all required software to produce a GCS.
# This includes regenerating all uavobject output.
#
# NOTE: to use this meta-project, you MUST perform these steps once
# for each source tree checkout:
# - Open <top>/ground/ground.pro in qt-creator
# - Select the "Projects" tab
# - Under Build Settings/General heading, click "Show Details"
# - Activate "Shadow Build"
# - Set your Build Directory to <top>/build/ground
#
# <top> = the full path to the base of your git source tree which
# should contain "flight", "ground", etc.

# There is a small problem with dependencies. qmake needs synthetic
# files when it generates GCS Makefiles. But we do not have
# uavobjgenerator at that time (on the 1st build). So we use the
# following trick: at make stage in uavobjects we rerun qmake for
# openpilotgcs.pro and regenerate GCS Makefiles using just built
# synthetic files. It takes some extra time but solves the
# dependency problem.

# Please note that this meta-project intended only for qt-creator
# users. Top level Makefile handles all dependencies itself and
# doesn't use ground.pro.

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
