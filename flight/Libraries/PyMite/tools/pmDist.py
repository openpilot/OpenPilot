#!/usr/bin/env python

# This file is Copyright 2009, 2010 Dean Hall.
#
# This file is part of the Python-on-a-Chip program.
# Python-on-a-Chip is free software: you can redistribute it and/or modify
# it under the terms of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1.
# 
# Python-on-a-Chip is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# A copy of the GNU LESSER GENERAL PUBLIC LICENSE Version 2.1
# is seen in the file COPYING in this directory.

"""Creates a release package for PyMite

Makes fresh export (no .svn folders), builds the docs, makes pymite-RR.tar.gz
and creates release tag in the svn repository
"""

## @file
#  @brief Creates a release package for PyMite
#
#  Makes fresh export (no .svn folders), builds the docs, makes pymite-RR.tar.gz
#  and creates release tag in the svn repository

import os, string, subprocess, sys, tempfile


REPOS = "https://python-on-a-chip.googlecode.com/svn/"

RELEASE_NUM = sys.argv[1]
assert string.atoi(RELEASE_NUM, 16) > 6
print "Building package for release", RELEASE_NUM, "..."
PM_RELEASE = "pymite-%s" % RELEASE_NUM

# Tag the release in the repository
subprocess.check_call(["svn",
                       "cp",
                       REPOS + "trunk",
                       REPOS + "tags/%s" % PM_RELEASE,
                       '-m "Tagging PyMite release %s"' % RELEASE_NUM],
                      cwd = tempdir)

# Export (no .svn folders) the tagged release in a temporary directory
tempdir = tempfile.gettempdir()
subprocess.check_call(["svn", 
                       "export", 
                       REPOS + "tags/%s" % PM_RELEASE, 
                       PM_RELEASE],
                      cwd = tempdir)

# Build html docs
subprocess.check_call(["make", "html"],
                      cwd = os.path.join(tempdir, PM_RELEASE))

# Make pymite-RR.tar.gz and copy it to the current directory
subprocess.check_call(["tar -cz %s/ > %s.tar.gz" % (PM_RELEASE, PM_RELEASE)],
                      cwd = tempdir, shell=True)
subprocess.check_call(["cp %s.tar.gz ." % os.path.join(tempdir, PM_RELEASE)],
                      shell=True)

print "Done."
