#!/bin/bash
#
# win_sdk_install.sh - Windows toolchain install script.
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

# This script should be launched from git bash prompt. It assumes MSYS
# environment and expects path names in the form of /C/some/path, where
# C is a drive letter, and paths are in MSYS format. It probably won't
# work under cygwin.
SCRIPT_PATH="`echo "$BASH_SOURCE" | sed 's|\\\\|/|g; s|^\(.\):|/\1|'`"
SCRIPT_NAME="`basename \"$SCRIPT_PATH\"`"
SCRIPT_DIR="`dirname \"$SCRIPT_PATH\"`"
ROOT_DIR="`pushd \"$SCRIPT_DIR/../..\" >/dev/null && pwd && popd >/dev/null`"
TOOLS_DIR="$ROOT_DIR/tools"

# Tools URLs to fetch
WGET_URL="http://wiki.openpilot.org/download/attachments/18612236/wget.exe"
MAKE_URL="http://wiki.openpilot.org/download/attachments/18612236/make.exe"

# Expected tools paths
WGET="$TOOLS_DIR/bin/`basename \"$WGET_URL\"`"
MAKE="$TOOLS_DIR/bin/`basename \"$MAKE_URL\"`"

# wget is necessary to fetch other files
WGET_NAME="`basename \"$WGET\"`"
if [ ! -x "$WGET" ]; then
    echo "$SCRIPT_NAME: $WGET_NAME not found, fetching from $WGET_URL"
    mkdir -p "`dirname \"$WGET\"`"

    VBSCRIPT="$TOOLS_DIR/fetch.$$.vbs"
    cat >"$VBSCRIPT" <<-EOF
	url  = WScript.Arguments.Item(0)
	file = WScript.Arguments.Item(1)

	Dim xHttp: Set xHttp = CreateObject("MSXML2.ServerXMLHTTP")
	xHttp.Open "GET", url, False
	xHttp.setOption 2, SXH_SERVER_CERT_IGNORE_ALL_SERVER_ERRORS
	xHttp.Send

	If xHttp.Status = 200 Then
	    Dim bStrm: Set bStrm = CreateObject("AdoDb.Stream")
	    With bStrm
	        .Type = 1 '// binary
	        .Open
	        .Write xHttp.ResponseBody
	        .SaveToFile file, 2 '// overwrite
	        .Close
	    End With
	    WScript.Quit(0)
	Else
	    WScript.Quit(1)
	End IF
EOF

    cscript "$VBSCRIPT" "$WGET_URL" "$WGET"
    rc=$?
    rm "$VBSCRIPT"

    if [ $rc -ne 0 ]; then
        echo "$SCRIPT_NAME: $WGET_NAME fetch error, hope it's in the path..."
        WGET="$WGET_NAME"
    fi
fi

# make is necessary to fetch all SDKs
MAKE_NAME="`basename \"$MAKE\"`"
if [ ! -x "$MAKE" ]; then
    echo "$SCRIPT_NAME: $MAKE_NAME not found, fetching from $MAKE_URL"
    MAKE_DIR="`dirname \"$MAKE\"`"
    mkdir -p "$MAKE_DIR"
    $WGET -N --content-disposition -P "$MAKE_DIR" "$MAKE_URL"
    if [ $? -ne 0 ]; then
        echo "$SCRIPT_NAME: $MAKE_NAME fetch error, hope it's in the path..."
        MAKE="$MAKE_NAME"
    fi
fi

# Finally we can fetch all SDKs using top level Makefile
cd "$ROOT_DIR"
echo "Run 'tools/bin/make all_sdk_install' to install the other tools"
echo " or 'tools/bin/make help' for more info on make targets"
