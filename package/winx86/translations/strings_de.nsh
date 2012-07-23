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
; Translation file for ${LANG_GERMAN}
;

;--------------------------------
; Installer section descriptions

  LangString DESC_InSecCore ${LANG_GERMAN} "GCS Kernkomponenten (Programm und Bibliotheken)."
  LangString DESC_InSecPlugins ${LANG_GERMAN} "GCS Plugins (Der Großteil der GCS Funktionalität)."
  LangString DESC_InSecResources ${LANG_GERMAN} "GCS Resourcen (Diagramme, Ziffernblätter, Kartensymbole, 3d-Modelle, PFD)."
  LangString DESC_InSecSounds ${LANG_GERMAN} "GCS Sounddateien (benötigt für akustische Ereignisbenachrichtigungen)."
  LangString DESC_InSecLocalization ${LANG_GERMAN} "GCS Lokalisierung (für unterstützte Sprachen)."
  LangString DESC_InSecFirmware ${LANG_GERMAN} "OpenPilot firmware binaries."
  LangString DESC_InSecUtilities ${LANG_GERMAN} "OpenPilot Dienstprogramme (Matlab Log Parser)."
  LangString DESC_InSecDrivers ${LANG_GERMAN} "OpenPilot Hardware Treiberdateien (optionaler OpenPilot CDC Treiber)."
  LangString DESC_InSecInstallDrivers ${LANG_GERMAN} "OpenPilot CDC Treiber (optional)."
  LangString DESC_InSecAeroSimRC ${LANG_GERMAN} "AeroSimRC plugin files with sample configuration."
  LangString DESC_InSecShortcuts ${LANG_GERMAN} "Installiere Verknüpfungen unter Startmenü->Anwendungen."

;--------------------------------
; Uninstaller section descriptions

  LangString DESC_UnSecProgram ${LANG_GERMAN} "OpenPilot GCS Programm inklusive aller Komponenten."
  LangString DESC_UnSecCache ${LANG_GERMAN} "OpenPilot GCS zwischengespeichertes Kartenmaterial."
  LangString DESC_UnSecConfig ${LANG_GERMAN} "OpenPilot GCS gespeicherte Konfigurationsdaten."
