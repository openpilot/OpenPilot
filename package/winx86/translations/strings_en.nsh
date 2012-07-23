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

;
; Translation file for ${LANG_ENGLISH}
;

;--------------------------------
; Installer section descriptions

  LangString DESC_InSecCore ${LANG_ENGLISH} "Core GCS components (executable and libraries)."
  LangString DESC_InSecPlugins ${LANG_ENGLISH} "GCS plugins (provide most of GCS functionality)."
  LangString DESC_InSecResources ${LANG_ENGLISH} "GCS resources (diagrams, dials, mapicons, 3d-models, PFD)."
  LangString DESC_InSecSounds ${LANG_ENGLISH} "GCS sound files (used for audible event notifications)."
  LangString DESC_InSecLocalization ${LANG_ENGLISH} "GCS localization (for supported languages)."
  LangString DESC_InSecFirmware ${LANG_ENGLISH} "OpenPilot firmware binaries."
  LangString DESC_InSecUtilities ${LANG_ENGLISH} "OpenPilot utilities (Matlab log parser)."
  LangString DESC_InSecDrivers ${LANG_ENGLISH} "OpenPilot hardware driver files (optional OpenPilot CDC driver)."
  LangString DESC_InSecInstallDrivers ${LANG_ENGLISH} "Optional OpenPilot CDC driver (virtual USB COM port)."
  LangString DESC_InSecAeroSimRC ${LANG_ENGLISH} "AeroSimRC plugin files with sample configuration."
  LangString DESC_InSecShortcuts ${LANG_ENGLISH} "Install application start menu shortcuts."

;--------------------------------
; Uninstaller section descriptions

  LangString DESC_UnSecProgram ${LANG_ENGLISH} "OpenPilot GCS application and all components."
  LangString DESC_UnSecCache ${LANG_ENGLISH} "OpenPilot GCS cached maps data."
  LangString DESC_UnSecConfig ${LANG_ENGLISH} "OpenPilot GCS configuration files."
