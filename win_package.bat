@echo off

IF "%QTDIR%"=="" SET QTDIR=C:\Qt\5.14.0\msvc2017_64

IF "%1"=="" goto Usage
SET GOTVERSION=false
IF NOT "%1"=="vs2017" SET GOTVERSION=true

IF "%GOTVERSION%"=="false" GOTO Usage


SET VSVERSION=%1

echo Visual Studio selected %VSVERSION%

REM Default Qt dir
SET BASE_DIR_QT=%QTDIR%

REM IF "%VSVERSION%"=="2015_32" set QT_LIBS=%BASE_DIR_QT%"\msvc2015"
REM IF "%VSVERSION%"=="2017_64" set QT_LIBS=%BASE_DIR_QT%"\msvc2017_64"
REM IF "%VSVERSION%"=="2019_64" set QT_LIBS=%BASE_DIR_QT%"\msvc2019_64"

SET QT_LIBS=%QTDIR%

echo QT libs selected %QT_LIBS%

REM Openssl lib
SET OPENSSL_PATH=C:\OpenSSL-v111-Win64

REM needed to copy the necessary Angle libs
REM SET ANGLEUSE=false
REM IF "%VSVERSION%"=="2017_64" SET ANGLEUSE=true 
REM IF "%VSVERSION%"=="2019_64" SET ANGLEUSE=true 

IF NOT EXIST %QT_LIBS% GOTO Nonexistentdir

REM 
REM copying binaries to the package directory
REM
@setlocal enableextensions enabledelayedexpansion

echo [X] Copying pip3line binaries 
md package > nul 2> nul
cd package
copy "..\lib\transform.dll"
copy "..\bin\pip3line.exe"
copy "..\bin\pip3linecmd.exe"
md plugins > nul 2> nul
copy "..\bin\plugins\*.dll" ".\plugins"

echo [X] Copying extras
md extras > nul 2> nul
xcopy "..\extras\*" ".\extras" /s /i

REM QT libraries
REM echo [2] Copying QT libraries
REM copy %QT_LIBS%"\bin\Qt5Core.dll"
REM copy %QT_LIBS%"\bin\Qt5Gui.dll"
REM copy %QT_LIBS%"\bin\Qt5Network.dll"
REM copy %QT_LIBS%"\bin\Qt5Svg.dll"
REM copy %QT_LIBS%"\bin\Qt5Widgets.dll"
REM copy %QT_LIBS%"\bin\Qt5XmlPatterns.dll"
REM copy %QT_LIBS%"\bin\Qt5Concurrent.dll"
REM copy %QT_LIBS%"\bin\Qt5PrintSupport.dll"

echo [X] Copying QT mandatory plugins
REM QT mandatory plug-ins, if missing the application won't even start
md platforms
copy %QT_LIBS%"\plugins\platforms\qminimal.dll" ".\platforms"
copy %QT_LIBS%"\plugins\platforms\qwindows.dll" ".\platforms"

echo [X] Copying distorm lib (if present)
REM Distorm3 lib
SET "DISTORM_PATH=..\ext\distorm"
IF NOT EXIST %DISTORM_PATH% GOTO Nonexistentdistorm
copy %DISTORM_PATH%"\distorm3.dll"

echo [X] Copying QScintilla lib (if present)
REM Qscintilla lib
SET "QSCINTILLA_PATH=..\ext\QScintilla_gpl\Qt4Qt5\release"
IF NOT EXIST %QSCINTILLA_PATH% GOTO Nonexistentqscintilla
copy %QSCINTILLA_PATH%"\qscintilla2_qt5.dll"

REM if you are using the ANGLE version of the Qt windows binaries
REM you need to copy a couple of more files
REM IF "%ANGLEUSE%"=="false" GOTO EndInstall
REM echo [6] Copying ANGLE Libraries
REM copy %QT_LIBS%"\bin\d3dcompiler_47.dll"
REM copy %QT_LIBS%"\bin\libEGL.dll"
REM copy %QT_LIBS%"\bin\libGLESv2.dll"

:EndInstall

SET WINDEPLOY=%QT_LIBS%\bin\windeployqt.exe --release pip3line.exe

echo [X] Running Qt deployement tool %WINDEPLOY%

%WINDEPLOY%

cd ..
GOTO End

echo [X] Copying OpenSSL lib (if present) %OPENSSL_PATH%
IF NOT EXIST "%OPENSSL_PATH%" GOTO NonexistentOpenssl

copy "%OPENSSL_PATH%\bin\*.dll"


:Usage
ECHO.
ECHO Need Qt Visual Studio version, i.e.
ECHO %~n0.bat vs2017
ECHO 
ECHO Available versions :
ECHO     vs2017 : VS 2017 amd64
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

:Nonexistentqscintilla
ECHO The directory %QSCINTILLA_PATH% not found, skipping.
GOTO End

:End
ENDLOCAL
