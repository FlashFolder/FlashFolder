@echo off
@echo Building MSI installer...

set CULTURE=en-US

tools\MkVersionFile.exe ..\_version.h _version.wxi.templ _version.wxi

"%WIX_3_0_DIR%\candle.exe" -nologo MainScript.wxs Features.wxs ProgramFiles.wxs Regkeys.wxs MyWixUI_InstallDir.wxs MultiUserDlg.wxs -dCulture=%CULTURE% -ext WixUiExtension
if NOT "%ERRORLEVEL%"=="0" (
	exit /B 1
)
"%WIX_3_0_DIR%\light.exe" -nologo MainScript.wixobj Features.wixobj ProgramFiles.wixobj Regkeys.wixobj MyWixUI_InstallDir.wixobj MultiUserDlg.wixobj -loc ProductTexts_%CULTURE%.wxl -cultures:%CULTURE% -ext WixUtilExtension -ext WixUiExtension -out msi\FlashFolder_setup.msi
if NOT "%ERRORLEVEL%"=="0" (
	exit /B 1
)
exit /B 0