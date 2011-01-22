@echo off
rem
rem Project: OpenPilot
rem NSIS installer script file for OpenPilot GCS
rem The OpenPilot Team, http://www.openpilot.org, Copyright (C) 2010-2011.
rem
rem This script requires Unicode NSIS 2.46 or higher:
rem http://www.scratchpaper.com/
rem
rem Optional SVN utility to get currently used SVN revision is SubWCRev.exe,
rem it installed by TortoiseSVN or available separately:
rem http://sourceforge.net/projects/tortoisesvn/files/Tools/1.6.7/SubWCRev-1.6.7.18415.msi/download
rem
rem See OpenPilotGCS.nsi for more details.
rem

rem Set path to NSIS compiler
set NSIS=%ProgramFiles%/NSIS/Unicode
set NSISC=%NSIS%/makensis.exe

rem Input script file
set NSI=OpenPilotGCS.nsi

rem Build installer
"%NSISC%" /V2 %NSI%
