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
goto Abort


:Proceed
rem -------------------------------------------------------------------
rem Settings and definitions
rem -------------------------------------------------------------------

rem Set desired targets
set TARGETS_FW=AHRS OpenPilot PipXtreme
set TARGETS_BL=%TARGETS_FW%

rem Set toolset paths (if you don't have them added permanently)
rem set PATH=D:\Work\OpenPilot\Apps\CodeSourcery\bin\;%PATH%
set MAKE=cs-make

rem Set some project path variables
rem for /F %%D in ('cd') do set CURDIR=%%D
for %%D in (%0) do set CURDIR=%%~dD%%~pD

set ROOT_DIR=%CURDIR%
set BUILD_DIR=%ROOT_DIR%\build
set UAVOBJ_XML_DIR=%ROOT_DIR%\shared\uavobjectdefinition
set UAVOBJ_OUT_DIR=%BUILD_DIR%\uavobject-synthetics

rem Find the UAVObjGenerator
for %%G in (debug release) do (
  if exist %BUILD_DIR%\ground\uavobjgenerator\%%G\uavobjgenerator.exe (
    set UAVOBJGENERATOR="%BUILD_DIR%\ground\uavobjgenerator\%%G\uavobjgenerator.exe"
    goto UAVObjGeneratorFound
  )
)
echo UAVObjGenerator was not found, please build it first
goto Abort
:UAVObjGeneratorFound

rem -------------------------------------------------------------------
rem Proceed with target
rem -------------------------------------------------------------------

set TARGET=%1

rem -------------------------------------------------------------------
rem UAVObjects for flight build
rem -------------------------------------------------------------------

if '%TARGET%' == 'clean' goto UAVObjectsDone
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
  %MAKE% CODE_SOURCERY=YES USE_BOOTLOADER=NO OUTDIR="%BUILD_DIR%\flight\Bootloaders\%%G" -C "%ROOT_DIR%\flight\Bootloaders\%%G" %TARGET%
  if errorlevel 1 goto Abort1
)

rem -------------------------------------------------------------------
rem Firmware build
rem -------------------------------------------------------------------

for %%G in (%TARGETS_FW%) do (
  %MAKE% CODE_SOURCERY=YES USE_BOOTLOADER=YES OUTDIR="%BUILD_DIR%\flight\%%G" -C "%ROOT_DIR%\flight\%%G" %TARGET%
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
