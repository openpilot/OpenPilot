REM /**********************************************************************************/
REM /* WiX Toolset required                                                           */
REM /* http://wix.codeplex.com                                                        */
REM /* "C:\Program Files\Windows Installer XML v3" - default installation folder  */
REM /**********************************************************************************/

set WixBin=C:\Program Files\Windows Installer XML v3\bin

"%WixBin%\candle.exe" -nologo OpenPilot.wxs -out OpenPilot.wixobj  -ext WixUtilExtension  -ext WixUIExtension

"%WixBin%\light.exe" -nologo OpenPilot.wixobj -sval -spdb -cultures:en-us -loc UI_translations\en-us.wxl -dWixUILicenseRtf=license\GPLv3_en.rtf -out OpenPilotGCS.msi -ext WixUtilExtension  -ext WixUIExtension
"%WixBin%\light.exe" -nologo OpenPilot.wixobj -sval -spdb -cultures:ru-ru -loc UI_translations\ru-ru.wxl -dWixUILicenseRtf=license\GPLv3_ru.rtf -out OpenPilot_ru.msi -ext WixUtilExtension  -ext WixUIExtension
"%WixBin%\light.exe" -nologo OpenPilot.wixobj -sval -spdb -cultures:de-de -loc UI_translations\de-de.wxl -dWixUILicenseRtf=license\GPLv3_de.rtf -out OpenPilot_de.msi -ext WixUtilExtension  -ext WixUIExtension
"%WixBin%\light.exe" -nologo OpenPilot.wixobj -sval -spdb -cultures:es-es -loc UI_translations\es-es.wxl -dWixUILicenseRtf=license\GPLv3_es.rtf -out OpenPilot_es.msi -ext WixUtilExtension  -ext WixUIExtension
"%WixBin%\light.exe" -nologo OpenPilot.wixobj -sval -spdb -cultures:fr-fr -loc UI_translations\fr-fr.wxl -dWixUILicenseRtf=license\GPLv3_fr.rtf -out OpenPilot_fr.msi -ext WixUtilExtension  -ext WixUIExtension
"%WixBin%\light.exe" -nologo OpenPilot.wixobj -sval -spdb -cultures:zh-tw -loc UI_translations\zh_cn.wxl -loc UI_translations\wixui_zh-tw.wxl -dWixUILicenseRtf=license\GPLv3_zh_cn.rtf -out OpenPilot_zh_ch.msi -ext WixUtilExtension  -ext WixUIExtension


MsiTools\MsiTran.exe -g OpenPilotGCS.msi OpenPilot_ru.msi ru.mst
MsiTools\MsiTran.exe -g OpenPilotGCS.msi OpenPilot_de.msi de.mst
MsiTools\MsiTran.exe -g OpenPilotGCS.msi OpenPilot_es.msi es.mst
MsiTools\MsiTran.exe -g OpenPilotGCS.msi OpenPilot_fr.msi fr.mst
MsiTools\MsiTran.exe -g OpenPilotGCS.msi OpenPilot_zh_ch.msi zn.mst

MsiTools\wisubstg.vbs OpenPilotGCS.msi ru.mst 1049
MsiTools\wisubstg.vbs OpenPilotGCS.msi de.mst 1031
MsiTools\wisubstg.vbs OpenPilotGCS.msi es.mst 3082
MsiTools\wisubstg.vbs OpenPilotGCS.msi fr.mst 1036
MsiTools\wisubstg.vbs OpenPilotGCS.msi zn.mst 1028

del OpenPilot.wixobj OpenPilot_ru.msi ru.mst OpenPilot_de.msi de.mst OpenPilot_es.msi es.mst OpenPilot_fr.msi fr.mst OpenPilot_zh_ch.msi zn.mst
