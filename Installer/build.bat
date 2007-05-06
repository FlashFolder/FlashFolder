@echo off
@echo Building MSI installer...

tools\MkVersionFile.exe ..\_version.h _version.wxi.templ _version.wxi

%WIX_2_0_DIR%\candle.exe -nologo mainscript.wxs
if NOT "%ERRORLEVEL%"=="0" (
	exit /B 1
)
%WIX_2_0_DIR%\light.exe -nologo mainscript.wixobj -loc ProductTexts_EN-US.wxl wix-cptui\lib\CptUI_simple.wixlib -loc wix-cptui\lib\Texts_EN-US.wxl -out msi\FlashFolder_setup.msi
if NOT "%ERRORLEVEL%"=="0" (
	exit /B 1
)
exit /B 0