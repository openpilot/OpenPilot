@echo off
rem
rem NSIS installer script file for OpenPilot GCS
rem The OpenPilot Team, http://www.openpilot.org, Copyright (C) 2010-2011.
rem
rem This script requires Unicode NSIS 2.46 or higher:
rem http://www.scratchpaper.com/
rem

rem Set path to NSIS compiler
set NSIS=%ProgramFiles%/NSIS/Unicode
set NSISC=%NSIS%/makensis.exe

rem Input script file (in the same directory as this batch file)
for %%D in (%0) do set NSI=%%~dD%%~pD\openpilotgcs.nsi

rem Build installer
echo Generating Windows installer...
"%NSISC%" /V2 %NSI%
