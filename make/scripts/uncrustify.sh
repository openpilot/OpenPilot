#!/bin/bash -e
#
# uncrustify.sh - source code processing script.
# Copyright (c) 2013, The OpenPilot Team, http://www.openpilot.org
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

# The following environment variables must be set
: ${UNCRUSTIFY?} ${UNCRUSTIFY_CONFIG?}

# No format flag file name
NO_FORMAT=.no-auto-format

# Recursive directory tree walk function
recursive()
{
    if [ -d "$1" ]; then
        # skip directory if no-format file is found
        if [ -f "$1/$NO_FORMAT" ]; then
            echo "Skipping: $1/"
            return
        fi
        echo "Processing: $1/"

        # process all files and subdirectories
        local name
        for name in $1/*; do
            recursive "$name"
        done
    else
        # process only known file extensions
        case "$1" in
            *.c|*.h|*.cc|*.cpp|*.hpp)
                ${UNCRUSTIFY} -c "${UNCRUSTIFY_CONFIG}" --no-backup "$1"
                ;;
        esac
    fi
}

# Entry point
for dir in $*; do
    recursive "$dir"
done
