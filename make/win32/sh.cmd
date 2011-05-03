@echo off
rem
rem This file is to be put into C:\Program Files\Git\cmd\ subdirectory
rem (or similar, depeding on where the msysGit package was installed)
rem to provide a shell prompt in the unix-like build environment on Windows.
rem
rem Currently supported on NT-class systems only (Windows XP and above).
rem
rem See also:
rem   README.txt
rem   http://wiki.openpilot.org/display/Doc/GCS+Development+on+Windows
rem   http://wiki.openpilot.org/display/Doc/Firmware+Development+on+Windows
rem
rem Based on the msys.bat file from the MSYS package 
rem   http://www.mingw.org/wiki/msys
rem

rem this should let run MSYS shell on x64
if "%PROCESSOR_ARCHITECTURE%" == "AMD64" (
  SET COMSPEC=%WINDIR%\SysWOW64\cmd.exe
)

rem some MSYS environment variables
if "x%MSYSTEM%" == "x" set MSYSTEM=MINGW32
if not "x%DISPLAY%" == "x" set DISPLAY=

rem --------------------------------------------------------------------------
rem To build the OpenPilot software we need few tools in the PATH.
rem Here we attempt to guess tools location searching in the given
rem directories first, and in the PATH last, if not found where expected.
rem
rem Please note that if you have few similar tools installed somewhere but
rem not in the expected location, and they are in the PATH, then the script
rem can detect wrong directories. For instance, if you have QtSDK installed
rem on the D: drive, but have separately installed MinGW toolkit which is
rem in the PATH, then this script detects this MinGW instead of QtSDK's one.
rem As a result, the SDL headers will not be found, if they were copied into
rem QtSDK's MinGW directory. In that case make sure that you have correct
rem directories specified here.
rem
rem Also you can add any paths below just by adding extra 'call :which'
rem lines with the following parameters:
rem  - environment variable which will be set to the tool location, if found;
rem  - expected directory for the executable which will be searched first;
rem  - any executable file which will be searched in the given directory
rem    or in the PATH, if not found where expected.
rem All they will be added to the PATH in order of appearance.
rem --------------------------------------------------------------------------

set NOT_FOUND=
set PATH_DIRS=

call :which MSYSGIT       "%ProgramFiles%\Git\bin"       git.exe
call :which QTMINGW       "C:\Qt\2010.05\mingw\bin"      mingw32-make.exe
call :which QTSDK         "C:\Qt\2010.05\qt\bin"         qmake.exe
call :which CODESOURCERY  "C:\CodeSourcery\bin"          cs-make.exe
call :which PYTHON        "C:\Python27"                  python.exe
call :which UNSIS         "%ProgramFiles%\NSIS\Unicode"  makensis.exe

if "%NOT_FOUND%" == "" goto set_path

echo:
echo Some tools were not found in the PATH or expected location:
for %%f in (%NOT_FOUND%) do echo   %%f
echo You may want to install them and/or update paths in the %0 file.
echo:

rem --------------------------------------------------------------------------
rem Provide a clean environment for command line build. We remove the
rem msysGit cmd subdirectory as well, so no recursive sh call can occur.
rem --------------------------------------------------------------------------

:set_path
set PATH=%SYSTEMROOT%\system32;%SYSTEMROOT%
set PATH=%PATH_DIRS%;%PATH%
rem echo PATH: %PATH%

rem --------------------------------------------------------------------------
rem Start a shell.
rem Any shell script can be passed to it via command line of this batch file.
rem --------------------------------------------------------------------------

if not exist "%MSYSGIT%\bash.exe" goto no_bash
call "%MSYSGIT%\bash.exe" --login -i %*
goto :eof

:no_bash
echo Cannot find bash, exiting with error
exit 1

rem --------------------------------------------------------------------------
rem Attempt to find executable in the directory given or in the PATH
rem --------------------------------------------------------------------------

:which
rem search in the directory given first
for %%F in (%2) do set FP=%%~F\%3
if exist "%FP%" goto found_directly

rem search in the PATH last
for %%F in (%3) do set FP=%%~$PATH:F
if exist "%FP%" goto found_in_path

:not_found
for %%F in (%2) do set FP=%%~F
rem echo %3: not found, expected in %FP%
set FP=
set NOT_FOUND=%NOT_FOUND% %3
goto set

:found_directly
for %%F in ("%FP%") do set FP=%%~dpsF
rem echo %3: found at: %FP%
goto set

:found_in_path
for %%F in ("%FP%") do set FP=%%~dpsF
rem echo %3: found in the PATH: %FP%

:set
rem set results regardless of was it found or not
set %1=%FP%
rem echo %1=%FP%
if "%FP%" == "" goto :eof
if not "%PATH_DIRS%" == "" set PATH_DIRS=%PATH_DIRS%;
set PATH_DIRS=%PATH_DIRS%%FP%
goto :eof
