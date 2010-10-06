REM /**********************************************************************************/
REM /* WiX Toolset required                                                           */
REM /* http://wix.codeplex.com                                                        */
REM /* "C:\Program Files\Windows Installer XML v3" - default installation folder  */
REM /**********************************************************************************/

set WixBin=C:\Program Files\Windows Installer XML v3\bin

"%WixBin%\candle.exe" -nologo "C:\Projects\builds\OpenPilot_Release\packaging\winx86\OpenPilot.wxs" -out OpenPilot.wixobj  -ext WixUtilExtension  -ext WixUIExtension

"%WixBin%\light.exe" -nologo OpenPilot.wixobj -cultures:en-us -loc UI_translations\en-us.wxl -dWixUILicenseRtf=license\GPLv3_en.rtf -out OpenPilot_en.msi -ext WixUtilExtension  -ext WixUIExtension
"%WixBin%\light.exe" -nologo OpenPilot.wixobj -cultures:ru-ru -loc UI_translations\ru-ru.wxl -dWixUILicenseRtf=license\GPLv3_ru.rtf -out OpenPilot_ru.msi -ext WixUtilExtension  -ext WixUIExtension
"%WixBin%\light.exe" -nologo OpenPilot.wixobj -cultures:de-de -loc UI_translations\de-de.wxl -dWixUILicenseRtf=license\GPLv3_de.rtf -out OpenPilot_de.msi -ext WixUtilExtension  -ext WixUIExtension
"%WixBin%\light.exe" -nologo OpenPilot.wixobj -cultures:es-es -loc UI_translations\es-es.wxl -dWixUILicenseRtf=license\GPLv3_es.rtf -out OpenPilot_es.msi -ext WixUtilExtension  -ext WixUIExtension
"%WixBin%\light.exe" -nologo OpenPilot.wixobj -cultures:fr-fr -loc UI_translations\fr-fr.wxl -dWixUILicenseRtf=license\GPLv3_fr.rtf -out OpenPilot_fr.msi -ext WixUtilExtension  -ext WixUIExtension
REM "%WixBin%\light.exe" -nologo OpenPilot.wixobj -cultures:zh-tw -loc UI_translations\zh_cn.wxl -dWixUILicenseRtf=license\GPLv3_zh_cn.rtf -out OpenPilot_zh_ch.msi -ext WixUtilExtension  -ext WixUIExtension


