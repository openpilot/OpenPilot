#
# make command replacement to run from command prompt under bash
#
# This file should be put into C:\Program Files\Git\cmd\ subdirectory
# (or similar, depeding on where the msysGit package was installed)
# to provide a shell prompt in the unix-like build environment on Windows.
#

exec /bin/make $*
