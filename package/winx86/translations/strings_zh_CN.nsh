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
; Translation file for ${LANG_TRADCHINESE}
;

;--------------------------------
; Installer section descriptions

  LangString DESC_InSecCore ${LANG_TRADCHINESE} "地面站核心组件 （可执行文件和库文件）."
  LangString DESC_InSecPlugins ${LANG_TRADCHINESE} "地面站插件（提供地面站大部分功能）."
  LangString DESC_InSecResources ${LANG_TRADCHINESE} "地面站资源库（图表，地图，模型，PFD（主要飞行数据图））."
  LangString DESC_InSecSounds ${LANG_TRADCHINESE} "地面站音频文件（用于对于特定事件的提醒）."
  LangString DESC_InSecLocalization ${LANG_TRADCHINESE} "地面站本土化（适用于它所支持的语言）."
  LangString DESC_InSecFirmware ${LANG_TRADCHINESE} "OpenPilot firmware binaries."
  LangString DESC_InSecUtilities ${LANG_TRADCHINESE} "OpenPilot utilities (Matlab log parser)."
  LangString DESC_InSecDrivers ${LANG_TRADCHINESE} "OpenPilot hardware driver files (optional OpenPilot CDC driver)."
  LangString DESC_InSecInstallDrivers ${LANG_TRADCHINESE} "Optional OpenPilot CDC driver (virtual USB COM port)."
  LangString DESC_InSecAeroSimRC ${LANG_TRADCHINESE} "AeroSimRC plugin files with sample configuration."
  LangString DESC_InSecShortcuts ${LANG_TRADCHINESE} "安装开始菜单的快捷方式."

;--------------------------------
; Uninstaller section descriptions

  LangString DESC_UnSecProgram ${LANG_TRADCHINESE} "OpenPilot GCS（地面站）程序及其所有的文件."
  LangString DESC_UnSecCache ${LANG_TRADCHINESE} "OpenPilot GCS（地面站）缓存的地图数据."
  LangString DESC_UnSecConfig ${LANG_TRADCHINESE} "OpenPilot GCS（地面站）配置文件."
