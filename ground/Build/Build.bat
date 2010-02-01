@echo off
rem
rem This file is generated
rem
echo Setting up a MinGW/Qt only environment...

set QTDIR=C:\Qt\2010.01\qt
set PATH=C:\Qt\2010.01\qt\bin
set PATH=%PATH%;C:\Qt\2010.01\bin;C:\Qt\2010.01\mingw\bin
set PATH=%PATH%;%SystemRoot%\System32
set QMAKESPEC=win32-g++

qmake C:\Users\David\Documents\Code\OpenPilot\ground\openpilotgcs.pro

mingw32-make.exe

pause
