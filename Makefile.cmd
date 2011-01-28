@echo off
rem
rem Windows-friendly batch file for all flight targets
rem

rem -------------------------------------------------------------------
rem Help
rem -------------------------------------------------------------------
if '%1' == 'build' goto Proceed
if '%1' == 'clean' goto Proceed
for %%F in (%0) do echo SYNTAX: %%~nF%%~xF [build / clean / help]
echo  - build: builds all flight targets including uavobjects, bootloaders and firmware
echo  - clean: cleans all flight targets including bootloaders and firmware
echo  - help:  this help
echo:
echo Environment variables:
echo  - TARGETS_BL - list of target (default is all flight bootloader targets)
echo  - TARGETS_FW - list of target (default is all flight targets)
echo  - UAVOBJGENERATOR - path and filename override for UAVObjGenerator.exe
echo:
echo Example usage (from a batch file with .cmd extension):
echo   set PATH=D:\Work\OpenPilot\Apps\CodeSourcery\bin\;%%PATH%%
echo   set TARGETS_FW=ahrs openpilot
echo   set TARGETS_BL=%%TARGETS_FW%%
echo   call svn\trunk\Makefile.cmd build
echo:
goto Abort


:Proceed
rem -------------------------------------------------------------------
rem Settings and definitions
rem -------------------------------------------------------------------

rem Set desired targets and paths
if "%TARGETS_BL%" == "" set TARGETS_BL=ahrs openpilot pipxtreme coptercontrol
if "%TARGETS_FW%" == "" set TARGETS_FW=%TARGETS_BL%
if "%TARGET_BL_SUBDIR%" == "" set TARGET_BL_SUBDIR=bootloaders
if "%TARGET_FW_SUBDIR%" == "" set TARGET_FW_SUBDIR=.

rem Set toolset paths (if you don't have them added permanently)
rem set PATH=D:\Work\OpenPilot\Apps\CodeSourcery\bin\;%PATH%

set MAKE=cs-make

rem Set some project path variables
for %%D in (%0) do set CURDIR=%%~dpD

set ROOT_DIR=%CURDIR%
set BUILD_DIR=%ROOT_DIR%\build
set UAVOBJ_XML_DIR=%ROOT_DIR%\shared\uavobjectdefinition
set UAVOBJ_OUT_DIR=%BUILD_DIR%\uavobject-synthetics

rem -------------------------------------------------------------------
rem Proceed with target
rem -------------------------------------------------------------------

set TARGET=%1
if '%TARGET%' == 'clean' goto UAVObjectsDone

rem -------------------------------------------------------------------
rem Searching for UAVObjGenerator executable
rem -------------------------------------------------------------------

set UAVOBJGENERATOR_FILENAME=uavobjgenerator.exe

rem If environment variable is set then expand it to full path and use
for %%G in (%UAVOBJGENERATOR%) do set UAVOBJGENERATOR=%%~fG
if exist "%UAVOBJGENERATOR%" goto UAVObjGeneratorFound

rem Searching in builds
for %%G in (debug release) do (
  if exist %BUILD_DIR%\ground\uavobjgenerator\%%G\%UAVOBJGENERATOR_FILENAME% (
    set UAVOBJGENERATOR="%BUILD_DIR%\ground\uavobjgenerator\%%G\%UAVOBJGENERATOR_FILENAME%"
    goto UAVObjGeneratorFound
  )
)

rem Searching in PATH
for %%G in (%UAVOBJGENERATOR_FILENAME%) do set UAVOBJGENERATOR=%%~$PATH:G
if exist %UAVOBJGENERATOR% goto UAVObjGeneratorFound

rem Report error
for %%G in (%ROOT_DIR%/ground/ground.pro) do set GROUND_PRO=%%~fG
echo UAVObjGenerator was not found, please build it first using %GROUND_PRO%
goto Abort

:UAVObjGeneratorFound
echo UAVObjGenerator found: %UAVOBJGENERATOR%

rem -------------------------------------------------------------------
rem UAVObjects for flight build
rem -------------------------------------------------------------------

mkdir %UAVOBJ_OUT_DIR% >NUL 2>&1
pushd %UAVOBJ_OUT_DIR%
%UAVOBJGENERATOR% -flight %UAVOBJ_XML_DIR% %ROOT_DIR%
if errorlevel 1 goto Abort2
popd

:UAVObjectsDone
rem -------------------------------------------------------------------
rem Bootloaders build
rem -------------------------------------------------------------------

for %%G in (%TARGETS_BL%) do (
  %MAKE% CODE_SOURCERY=YES USE_BOOTLOADER=NO OUTDIR="%BUILD_DIR%\%TARGET_BL_SUBDIR%\%%G" -C "%ROOT_DIR%\flight\Bootloaders\%%G" %TARGET%
  if errorlevel 1 goto Abort1
)

rem -------------------------------------------------------------------
rem Firmware build
rem -------------------------------------------------------------------

for %%G in (%TARGETS_FW%) do (
  %MAKE% CODE_SOURCERY=YES USE_BOOTLOADER=YES OUTDIR="%BUILD_DIR%\%TARGET_FW_SUBDIR%\%%G" -C "%ROOT_DIR%\flight\%%G" %TARGET%
  if errorlevel 1 goto Abort1
)
goto Done


rem -------------------------------------------------------------------
rem Error handling
rem -------------------------------------------------------------------

:Abort2
popd

:Abort1
echo Error returned, build aborted

:Abort
pause

:Done
