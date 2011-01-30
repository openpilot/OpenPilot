#
# Project: OpenPilot
# NSIS header file for OpenPilot GCS
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

;--------------------------------
; Languages
; Only currently available GCS translations are included.
; Default language is system-dependant, or set in the Registry for this installer.

  !insertmacro MUI_LANGUAGE "English"
  !insertmacro MUI_LANGUAGE "French"
  !insertmacro MUI_LANGUAGE "German"
  !insertmacro MUI_LANGUAGE "Russian"
  !insertmacro MUI_LANGUAGE "Spanish"
  !insertmacro MUI_LANGUAGE "TradChinese"

;--------------------------------
; Translated license files

  LicenseLangString LicenseFile ${LANG_ENGLISH} "licenses\GPLv3_en.rtf"
  LicenseLangString LicenseFile ${LANG_FRENCH} "licenses\GPLv3_fr.rtf"
  LicenseLangString LicenseFile ${LANG_GERMAN} "licenses\GPLv3_de.rtf"
  LicenseLangString LicenseFile ${LANG_RUSSIAN} "licenses\GPLv3_ru.rtf"
  LicenseLangString LicenseFile ${LANG_SPANISH} "licenses\GPLv3_es.rtf"
  LicenseLangString LicenseFile ${LANG_TRADCHINESE} "licenses\GPLv3_zh_CN.rtf"

;--------------------------------
; Translated strings

  !include "translations\strings_en.nsh"
  !include "translations\strings_fr.nsh"
  !include "translations\strings_de.nsh"
  !include "translations\strings_ru.nsh"
  !include "translations\strings_es.nsh"
  !include "translations\strings_zh_CN.nsh"
