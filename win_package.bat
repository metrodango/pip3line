@echo off

IF "%1"=="" goto Usage
SET GOTVERSION=false
IF NOT "%1"=="2015_32" SET GOTVERSION=true 
IF NOT "%1"=="2015_64" SET GOTVERSION=true

IF "%GOTVERSION%"=="false" GOTO Usage

SET VSVERSION=%1
REM Default Qt dir
SET "BASE_DIR_QT=C:\Qt\5.7"

IF "%VSVERSION%"=="2015_32" set QT_LIBS=%BASE_DIR_QT%"\msvc2015"
IF "%VSVERSION%"=="2015_64" set QT_LIBS=%BASE_DIR_QT%"\msvc2015_64"

REM Openssl lib (default is 32 bit, change for anything else if needed)
IF "%VSVERSION%"=="2015_32" SET "OPENSSL_PATH=C:\OpenSSL-Win32"
IF "%VSVERSION%"=="2015_64" SET "OPENSSL_PATH=C:\OpenSSL-Win64"

REM needed to copy the necessary Angle libs
SET ANGLEUSE=false
IF "%VSVERSION%"=="2015_64" SET ANGLEUSE=true 

IF NOT EXIST %QT_LIBS% GOTO Nonexistentdir

REM 
REM copying binaries to the package directory
REM
@setlocal enableextensions enabledelayedexpansion

echo [1] Copying pip3line binaries 
md package > nul 2> nul
cd package
copy "..\lib\transform.dll"
copy "..\bin\pip3line.exe"
copy "..\bin\pip3linecmd.exe"
md plugins > nul 2> nul
copy "..\bin\plugins\*.dll" ".\plugins"

echo [1.1] Copying extras
md extras > nul 2> nul
xcopy "..\extras\*" ".\extras" /s /i

REM QT libraries
echo [2] Copying QT libraries
copy %QT_LIBS%"\bin\Qt5Core.dll"
copy %QT_LIBS%"\bin\Qt5Gui.dll"
copy %QT_LIBS%"\bin\Qt5Network.dll"
copy %QT_LIBS%"\bin\Qt5Svg.dll"
copy %QT_LIBS%"\bin\Qt5Widgets.dll"
copy %QT_LIBS%"\bin\Qt5XmlPatterns.dll"
copy %QT_LIBS%"\bin\Qt5Concurrent.dll"

echo [3] Copying QT mandatory plugins
REM QT mandatory plug-ins, if missing the application won't even start
md platforms
copy %QT_LIBS%"\plugins\platforms\qminimal.dll" ".\platforms"
copy %QT_LIBS%"\plugins\platforms\qwindows.dll" ".\platforms"

echo [4] Copying distorm lib (if present)
REM Distorm3 lib
SET "DISTORM_PATH=..\..\distorm"
IF NOT EXIST %DISTORM_PATH% GOTO Nonexistentdistorm
copy %DISTORM_PATH%"\distorm3.dll"

echo [5] Copying OpenSSL lib (if present)
IF NOT EXIST %OPENSSL_PATH% GOTO NonexistentOpenssl
copy %OPENSSL_PATH%"\bin\ssleay32.dll"
copy %OPENSSL_PATH%"\bin\libeay32.dll"

REM For OpenSSL 1.1.0 "NG"
REM copy %OPENSSL_PATH%"\bin\libcrypto-1_1-x64.dll"
REM copy %OPENSSL_PATH%"\bin\libssl-1_1-x64.dll"

REM if you are using the ANGLE version of the Qt windows binaries
REM you need to copy a couple of more files
IF "%ANGLEUSE%"=="false" GOTO EndInstall
echo [6] Copying ANGLE Libraries
copy %QT_LIBS%"\bin\d3dcompiler_47.dll"
copy %QT_LIBS%"\bin\libEGL.dll"
copy %QT_LIBS%"\bin\libGLESv2.dll"

:EndInstall

SET WINDEPLOY=%QT_LIBS%\bin\windeployqt.exe --release pip3line.exe

echo [7] Running Qt deployement tool %WINDEPLOY%

%WINDEPLOY%

cd ..
GOTO End

:Usage
ECHO.
ECHO Need Qt Visual Studio version, i.e.
ECHO %~n0.bat 2015
ECHO 
ECHO Available versions : 
ECHO     2015_64 : VS 2015 amd64
ECHO     2015_32 : VS 2015 x86
GOTO End

:Nonexistentdir
ECHO.
ECHO The directory %QT_LIBS% does not exist
GOTO End

:Nonexistentdistorm
ECHO The directory %DISTORM_PATH% not found, skipping.
GOTO End

:NonexistentOpenssl
ECHO The directory %OPENSSL_PATH% not found, skipping.
GOTO End

:End
ENDLOCAL
