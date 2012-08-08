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
; Translation file for ${LANG_RUSSIAN}
;

;--------------------------------
; Installer section descriptions

  LangString DESC_InSecCore ${LANG_RUSSIAN} "Основные компоненты GCS (исполняемый файл и библиотеки)."
  LangString DESC_InSecPlugins ${LANG_RUSSIAN} "Плагины GCS (обеспечивают большую часть функций GCS)."
  LangString DESC_InSecResources ${LANG_RUSSIAN} "Ресурсы GCS (диаграммы, приборы, пиктограммы, 3d-модели, PFD)."
  LangString DESC_InSecSounds ${LANG_RUSSIAN} "Звуковые файлы (используются для звуковых уведомлений о событиях)."
  LangString DESC_InSecLocalization ${LANG_RUSSIAN} "Файлы языковой поддержки (для поддерживаемых языков)."
  LangString DESC_InSecFirmware ${LANG_RUSSIAN} "Файлы прошивок OpenPilot."
  LangString DESC_InSecUtilities ${LANG_RUSSIAN} "Утилиты (конвертор логов для Matlab)."
  LangString DESC_InSecDrivers ${LANG_RUSSIAN} "Файлы драйверов (опциональный драйвер CDC порта)."
  LangString DESC_InSecInstallDrivers ${LANG_RUSSIAN} "Опциональный OpenPilot CDC драйвер (виртуальный USB COM порт)."
  LangString DESC_InSecAeroSimRC ${LANG_RUSSIAN} "Файлы плагина для симулятора AeroSimRC с примером конфигурации."
  LangString DESC_InSecShortcuts ${LANG_RUSSIAN} "Установка ярлыков для приложения."

;--------------------------------
; Uninstaller section descriptions

  LangString DESC_UnSecProgram ${LANG_RUSSIAN} "Основное приложение OpenPilot GCS и все его компоненты."
  LangString DESC_UnSecCache ${LANG_RUSSIAN} "Кешированные карты OpenPilot GCS."
  LangString DESC_UnSecConfig ${LANG_RUSSIAN} "Пользовательская конфигурация OpenPilot GCS."
