#
# Top level Qt-Creator project file
#

# This meta-project allows qt-creator users to open
# and configure a single project and to build all
# required software to produce a GCS.  This includes
# regenerating all uavobject output.
#
# NOTE: To use this meta-project, you MUST perform these
# steps once for each SVN checkout:
# - Open <top>/ground/ground.pro in qt-creator
# - Select the "Projects" tab
# - Under Build Settings/General heading, click "Show Details"
# - Activate "Shadow Build"
# - Set your Build Directory to <top>/build/ground
#
# <top> = The full path to the base of your svn tree which should
# contain "flight", "ground", etc.

# There is a known problem with dependencies which should be fixed.
# Until that the following workaround for qt-creator users exists:
#
# - run qmake in ground directory (generated GCS Makefiles lack some
#   uavobject targets - it is known problem);
# - build in uavobjgenerator;
# - build in uavobjects;
# - run qmake in ground again (IMPORTANT - now correct GCS Makefiles
#   will be generated).
#
# Now you may build GCS using "Build project 'ground'" qt-creator
# command. If some uavobjects were added/removed, you need to repeat
# all steps after uavobjects.pro modification.
#
# Please note that this workaround (and meta-project at all) is only
# for qt-creator users. Top level Makefile handles all dependencies
# itself and don't use ground.pro.


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
