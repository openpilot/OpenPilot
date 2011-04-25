#
# Project: OpenPilot
# NSIS configuration file for OpenPilot GCS
# The OpenPilot Team, http://www.openpilot.org, Copyright (C) 2010-2011.
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

# This script requires Unicode NSIS 2.46 or higher:
# http://www.scratchpaper.com/

# Features:
#  - Installs to the user local appdata path, no admin rights required.
#
# TODO:
#  - optionally install for all users (to Program Files with admin rights on Vista/7).
#  - install only built/used modules, not a whole directory.
#  - remove only installed files, not a whole directory.

;--------------------------------
; Paths
  
  ; Tree root locations (relative to this script location)
  !define NSIS_DATA_TREE "."
  !define GCS_BUILD_TREE "..\..\..\..\build\ground\openpilotgcs"
  !define WINX86_PATH "packaging\winx86"

  ; Default installation folder
  InstallDir "$LOCALAPPDATA\OpenPilot"
  
  ; Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\OpenPilot" "Install Location"

;--------------------------------
; Version information

  ; Program name and installer file
  !define PRODUCT_NAME "OpenPilot GCS"
  !define INSTALLER_NAME "OpenPilot GCS Installer"

  ; Read automatically generated version info
; !define OUT_FILE "OpenPilotGCS-XXXX-install.exe"
; !define PRODUCT_VERSION "0.0.0.0"
; !define FILE_VERSION "0.0.0.0"
; !define BUILD_DESCRIPTION "Unknown revision."
  !include "${GCS_BUILD_TREE}\${WINX86_PATH}\openpilotgcs.nsh"

  Name "${PRODUCT_NAME}"
  OutFile "${GCS_BUILD_TREE}\${WINX86_PATH}\${OUT_FILE}"

  VIProductVersion ${PRODUCT_VERSION}
  VIAddVersionKey "ProductName" "${INSTALLER_NAME}"
  VIAddVersionKey "FileVersion" "${FILE_VERSION}"
  VIAddVersionKey "Comments" "${INSTALLER_NAME}. ${BUILD_DESCRIPTION}"
  VIAddVersionKey "CompanyName" "The OpenPilot Team, http://www.openpilot.org"
  VIAddVersionKey "LegalTrademarks" "${PRODUCT_NAME} is a trademark of The OpenPilot Team"
  VIAddVersionKey "LegalCopyright" "© 2010-2011 The OpenPilot Team"
  VIAddVersionKey "FileDescription" "${INSTALLER_NAME}"

;--------------------------------
; Installer interface and base settings

  !include "MUI2.nsh"
  !define MUI_ABORTWARNING

  ; Adds an XP manifest to the installer
  XPStyle on

  ; Request application privileges for Windows Vista/7
  RequestExecutionLevel user

  ; Compression level
  SetCompressor /solid lzma
  
;--------------------------------
; Branding

  BrandingText "© 2010-2011 The OpenPilot Team, http://www.openpilot.org"

  !define MUI_ICON "${NSIS_DATA_TREE}\resources\openpilot.ico"
  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "${NSIS_DATA_TREE}\resources\header.bmp"
  !define MUI_HEADERIMAGE_BITMAP_NOSTRETCH
  !define MUI_WELCOMEFINISHPAGE_BITMAP "${NSIS_DATA_TREE}\resources\welcome.bmp"
  !define MUI_WELCOMEFINISHPAGE_BITMAP_NOSTRETCH
  !define MUI_UNWELCOMEFINISHPAGE_BITMAP "${NSIS_DATA_TREE}\resources\welcome.bmp"
  !define MUI_UNWELCOMEFINISHPAGE_BITMAP_NOSTRETCH
	
;--------------------------------
; Language selection dialog settings

  ; Remember the installer language
  !define MUI_LANGDLL_REGISTRY_ROOT "HKCU" 
  !define MUI_LANGDLL_REGISTRY_KEY "Software\OpenPilot" 
  !define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

;--------------------------------
; Pages

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "$(LicenseFile)"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH
  
  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_COMPONENTS
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

; !define MUI_FINISHPAGE_RUN "$INSTDIR\bin\openpilotgcs.exe"
 
;--------------------------------
; Supported languages, license files and translations

  !include "${NSIS_DATA_TREE}\translations\languages.nsh"
  
;--------------------------------
; Reserve files
  
  ; If you are using solid compression, files that are required before
  ; the actual installation should be stored first in the data block,
  ; because this will make your installer start faster.
  
  !insertmacro MUI_RESERVEFILE_LANGDLL

;--------------------------------
; Installer sections

Section "Core files" InSecCore
  SectionIn RO
  SetOutPath "$INSTDIR\bin"
  File /r "${GCS_BUILD_TREE}\bin\*"
SectionEnd

Section "Plugins" InSecPlugins
  SectionIn RO
  SetOutPath "$INSTDIR\lib\openpilotgcs\plugins"
  File /r "${GCS_BUILD_TREE}\lib\openpilotgcs\plugins\*.dll"
  File /r "${GCS_BUILD_TREE}\lib\openpilotgcs\plugins\*.pluginspec"
SectionEnd

Section "Resources" InSecResources
  SetOutPath "$INSTDIR\share\openpilotgcs\diagrams"
  File /r "${GCS_BUILD_TREE}\share\openpilotgcs\diagrams\*"
  SetOutPath "$INSTDIR\share\openpilotgcs\dials"
  File /r "${GCS_BUILD_TREE}\share\openpilotgcs\dials\*"
  SetOutPath "$INSTDIR\share\openpilotgcs\mapicons"
  File /r "${GCS_BUILD_TREE}\share\openpilotgcs\mapicons\*"
  SetOutPath "$INSTDIR\share\openpilotgcs\models"
  File /r "${GCS_BUILD_TREE}\share\openpilotgcs\models\*"
  SetOutPath "$INSTDIR\share\openpilotgcs\pfd"
  File /r "${GCS_BUILD_TREE}\share\openpilotgcs\pfd\*"
SectionEnd

Section "Sound files" InSecSounds
  SetOutPath "$INSTDIR\share\openpilotgcs\sounds"
  File /r "${GCS_BUILD_TREE}\share\openpilotgcs\sounds\*"
SectionEnd

Section "Localization" InSecLocalization
  SetOutPath "$INSTDIR\share\openpilotgcs\translations"
  File /r "${GCS_BUILD_TREE}\share\openpilotgcs\translations\*.qm"
SectionEnd

Section "Shortcuts" InSecShortcuts
  ; Create desktop and start menu shortcuts
  SetOutPath "$INSTDIR"
  CreateDirectory "$SMPROGRAMS\OpenPilot"
  CreateShortCut "$SMPROGRAMS\OpenPilot\OpenPilot GCS.lnk" "$INSTDIR\bin\openpilotgcs.exe" \
	"" "$INSTDIR\bin\openpilotgcs.exe" 0 "" "" "${PRODUCT_NAME} ${PRODUCT_VERSION}. ${BUILD_DESCRIPTION}"
  CreateShortCut "$DESKTOP\OpenPilot GCS.lnk" "$INSTDIR\bin\openpilotgcs.exe" \
  	"" "$INSTDIR\bin\openpilotgcs.exe" 0 "" "" "${PRODUCT_NAME} ${PRODUCT_VERSION}. ${BUILD_DESCRIPTION}"
  CreateShortCut "$SMPROGRAMS\OpenPilot\Uninstall.lnk" "$INSTDIR\Uninstall.exe" "" "$INSTDIR\Uninstall.exe" 0
SectionEnd

Section ; create uninstall info
  ; Write the installation path into the registry
  WriteRegStr HKCU "Software\OpenPilot" "Install Location" $INSTDIR
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenPilot" "DisplayName" "OpenPilot GCS"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenPilot" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenPilot" "NoModify" 1
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenPilot" "NoRepair" 1

  ; Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

;--------------------------------
; Installer section descriptions

  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${InSecCore} $(DESC_InSecCore)
    !insertmacro MUI_DESCRIPTION_TEXT ${InSecPlugins} $(DESC_InSecPlugins)
    !insertmacro MUI_DESCRIPTION_TEXT ${InSecResources} $(DESC_InSecResources)
    !insertmacro MUI_DESCRIPTION_TEXT ${InSecSounds} $(DESC_InSecSounds)
    !insertmacro MUI_DESCRIPTION_TEXT ${InSecLocalization} $(DESC_InSecLocalization)
    !insertmacro MUI_DESCRIPTION_TEXT ${InSecShortcuts} $(DESC_InSecShortcuts)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END
 
;--------------------------------
; Installer functions

Function .onInit

  !insertmacro MUI_LANGDLL_DISPLAY

FunctionEnd

;--------------------------------
; Uninstaller sections

Section "un.OpenPilot GCS" UnSecProgram
  ; Remove installed files and/or directories
  RMDir /r /rebootok "$INSTDIR\bin"
  RMDir /r /rebootok "$INSTDIR\lib"
  RMDir /r /rebootok "$INSTDIR\share"
  Delete /rebootok "$INSTDIR\Uninstall.exe"

  ; Remove directory
  RMDir /rebootok "$INSTDIR"

  ; Remove registry keys
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\OpenPilot"
  DeleteRegKey HKCU "Software\OpenPilot"

  ; Remove shortcuts, if any
  Delete /rebootok "$DESKTOP\OpenPilot GCS.lnk"
  Delete /rebootok "$SMPROGRAMS\OpenPilot\*"
  RMDir /rebootok "$SMPROGRAMS\OpenPilot"
SectionEnd

Section "un.Maps cache" UnSecCache
  ; Remove maps cache
  RMDir /r /rebootok "$PROFILE\OpenPilot"
SectionEnd

Section /o "un.Configuration" UnSecConfig
  ; Remove configuration
  RMDir /r /rebootok "$APPDATA\OpenPilot"
SectionEnd

;--------------------------------
; Uninstall section descriptions

  !insertmacro MUI_UNFUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${UnSecProgram} $(DESC_UnSecProgram)
    !insertmacro MUI_DESCRIPTION_TEXT ${UnSecCache} $(DESC_UnSecCache)
    !insertmacro MUI_DESCRIPTION_TEXT ${UnSecConfig} $(DESC_UnSecConfig)
  !insertmacro MUI_UNFUNCTION_DESCRIPTION_END
 
;--------------------------------
; Uninstaller functions

Function un.onInit

  !insertmacro MUI_UNGETLANGUAGE
  
FunctionEnd
