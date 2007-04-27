@echo off
@echo Building MSI installer...

%CPT_VCPP_LIB%\_tools\MkVersionFile\1.0\MkVersionFile.exe ..\_version.h _version.wxi.templ _version.wxi

set candle=%CPT_VCPP_LIB%\_tools\wix\2.0\candle.exe
set light=%CPT_VCPP_LIB%\_tools\wix\2.0\light.exe
set cptui=%CPT_VCPP_LIB%\Wix_CptUI\1.5\

%candle% -nologo mainscript.wxs
if NOT "%ERRORLEVEL%"=="0" (
	pause
	exit
)
%light% -nologo mainscript.wixobj -loc ProductTexts_EN-US.wxl %cptui%\Lib\CptUI_simple.wixlib -loc %cptui%\Texts_EN-US.wxl -out msi\FlashFolder_setup.msi
if NOT "%ERRORLEVEL%"=="0" (
	pause
	exit
)