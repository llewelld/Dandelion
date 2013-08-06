@ECHO OFF
IF "%~1"=="" ECHO Error: Must specify a version number as a parameter. E.g. "buildSetup 1.0.0"
IF "%~1"=="" GOTO End
SET productversion=%1

cd windows\wixui

REM Create the msi installer user interface
ECHO Creating MSI installer user interface

candle.exe dandelion\WixUI_Dandelion.wxs *.wxs

lit.exe -out WixUI_Dandelion.wixlib *.wixobj

cd ..\..

REM Create the MSI installer
ECHO Creating MSI installer

candle windows\dandelion.wxs -out windows\dandelion.wixobj

light.exe -out dandelion-%productversion%-win32-bin.msi windows\dandelion.wixobj "windows\wixui\WixUI_Dandelion.wixlib" -loc "windows\wixui\WixUI_en-us.wxl"

REM Create the BootStrapper installer
ECHO Creating BootStrapper installer

candle windows\bootstrap\bootstrap.wxs -out windows\bootstrap\bootstrap.wixobj

light -out DandelionSetup-%productversion%-win32-bin.exe -ext WixBalExtension windows\bootstrap\bootstrap.wixobj


:End

