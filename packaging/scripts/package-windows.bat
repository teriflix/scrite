@echo off
:: Windows packaging script: builds NSIS installer with windeployqt and code signing.
:: Usage:
::   package-windows.bat [--build] [--sign] [--no-sign] [--type <version-type>]
::
:: Environment variables (or set in packaging.config.local.bat):
::   CodeSignTool           Path to signing tool (e.g. signtool.exe or CodeSignTool.exe)
::   SCRITE_BUSINESS_NAME   Certificate CN for signing
::   SCRITE_OPENSSL_LIBS    Root directory containing openssl-1.1\x64\bin\
::   SCRITE_CRASHPAD_ROOT   Root directory of Crashpad SDK (optional)
::   QT_BIN_DIR             Path to Qt bin directory (optional, uses PATH if not set)
::   CMAKE_DIR              Path to cmake bin directory (contains cpack.exe)
::   BUILD_DIR              CMake build directory (default: build)

setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"
pushd "%SCRIPT_DIR%\..\.."
set "PROJECT_ROOT=%CD%"
popd

if not defined BUILD_DIR set "BUILD_DIR=build"
set "STAGING_DIR=%PROJECT_ROOT%\packaging\_staging\windows"
set "ASSETS_DIR=%PROJECT_ROOT%\packaging\assets\windows"

:: Load local configuration if it exists
set "CONFIG_LOCAL=%PROJECT_ROOT%\packaging\packaging.config.local.bat"
if exist "%CONFIG_LOCAL%" (
    echo Loading configuration from %CONFIG_LOCAL%
    call "%CONFIG_LOCAL%"
) else (
    echo Note: packaging.config.local.bat not found
    echo   ^(This is OK if you set environment variables directly^)
)

:: Extract version from CMakeLists.txt
set "VERSION="
for /f "tokens=3 delims= " %%v in ('findstr /r /c:"project(Scrite VERSION" "%PROJECT_ROOT%\CMakeLists.txt"') do (
    if not defined VERSION set "VERSION=%%v"
)
if not defined VERSION (
    echo ERROR: Could not extract version from CMakeLists.txt >&2
    exit /b 1
)
echo Scrite version: %VERSION%

:: Parse arguments
set "BUILD=0"
set "SIGN=0"
set "VERSION_SUFFIX="

:parse_args
if "%~1"=="" goto :args_done
if /i "%~1"=="--build"   ( set "BUILD=1" & shift & goto :parse_args )
if /i "%~1"=="--sign"    ( set "SIGN=1"  & shift & goto :parse_args )
if /i "%~1"=="--no-sign" ( set "SIGN=0"  & shift & goto :parse_args )
if /i "%~1"=="--type" (
    if "%~2"=="" ( echo ERROR: --type requires a value >&2 & exit /b 1 )
    set "VERSION_SUFFIX=-%~2"
    shift & shift
    goto :parse_args
)
echo ERROR: Unknown option: %~1 >&2
exit /b 1
:args_done

:: Build if requested
if "%BUILD%"=="1" (
    set "BUILD_ARGS="
    if defined VERSION_SUFFIX set "BUILD_ARGS=--type !VERSION_SUFFIX:~1!"
    call "%SCRIPT_DIR%\build.bat" !BUILD_ARGS!
    if errorlevel 1 goto :error
)

:: Verify executable exists
set "EXE=%PROJECT_ROOT%\binary\Scrite.exe"
if not exist "%EXE%" (
    echo ERROR: Executable not found: %EXE% >&2
    echo Have you built the project? Try --build flag. >&2
    exit /b 1
)

echo Packaging Windows NSIS installer...
echo   Executable: %EXE%

:: Add CMAKE_DIR and QT_BIN_DIR to PATH if set
if defined CMAKE_DIR set "PATH=%CMAKE_DIR%;%PATH%"
if defined QT_BIN_DIR set "PATH=%QT_BIN_DIR%;%PATH%"

:: Locate windeployqt
set "WINDEPLOYQT="
if defined QT_BIN_DIR (
    if exist "%QT_BIN_DIR%\windeployqt.exe" set "WINDEPLOYQT=%QT_BIN_DIR%\windeployqt.exe"
)
if not defined WINDEPLOYQT (
    for %%i in (windeployqt.exe) do set "WINDEPLOYQT=%%~$PATH:i"
)
if not defined WINDEPLOYQT (
    echo ERROR: windeployqt.exe not found. Set QT_BIN_DIR or add Qt bin to PATH. >&2
    exit /b 1
)
echo   windeployqt: %WINDEPLOYQT%

:: Step 1: Create staging directory and copy executable
echo Preparing staging directory...
if exist "%STAGING_DIR%" rmdir /s /q "%STAGING_DIR%"
mkdir "%STAGING_DIR%"
copy /y "%EXE%" "%STAGING_DIR%" >nul
copy /y "%PROJECT_ROOT%\apps\desktop\appicon.ico" "%STAGING_DIR%" >nul

:: Step 2: Sign executable if requested
if "%SIGN%"=="1" (
    if not defined CodeSignTool (
        echo WARNING: --sign requested but CodeSignTool not set. Skipping exe signing.
    ) else (
        echo Signing Scrite.exe...
        "!CodeSignTool!" sign /tr http://timestamp.sectigo.com /td sha256 /fd sha256 /n "!SCRITE_BUSINESS_NAME!" "!STAGING_DIR!\Scrite.exe"
        if errorlevel 1 goto :error
    )
)

:: Step 3: Copy OpenSSL DLLs
echo Copying OpenSSL 1.1 DLLs...
if not defined SCRITE_OPENSSL_LIBS (
    echo ERROR: SCRITE_OPENSSL_LIBS not set. Cannot find OpenSSL DLLs. >&2
    exit /b 1
)
copy /y "%SCRITE_OPENSSL_LIBS%\openssl-1.1\x64\bin\libcrypto-1_1-x64.dll" "%STAGING_DIR%" >nul
if errorlevel 1 goto :error
copy /y "%SCRITE_OPENSSL_LIBS%\openssl-1.1\x64\bin\libssl-1_1-x64.dll" "%STAGING_DIR%" >nul
if errorlevel 1 goto :error

:: Step 4: Copy Visual C++ redistributable
echo Copying VC++ redistributable...
if not exist "%ASSETS_DIR%\vcredist_x64.exe" (
    echo   vcredist_x64.exe not found, downloading...
    curl -L -o "%ASSETS_DIR%\vcredist_x64.exe" "https://aka.ms/vs/17/release/vc_redist.x64.exe"
    if errorlevel 1 (
        echo ERROR: Failed to download vcredist_x64.exe >&2
        goto :error
    )
)
copy /y "%ASSETS_DIR%\vcredist_x64.exe" "%STAGING_DIR%" >nul

:: Step 5: Run windeployqt (on Scrite.exe only, before adding non-Qt executables)
echo Running windeployqt...
"%WINDEPLOYQT%" --qmldir "%PROJECT_ROOT%\apps\desktop\qml" --no-compiler-runtime --no-translations "%STAGING_DIR%\Scrite.exe"
if errorlevel 1 goto :error

:: Step 6: Copy crashpad_handler after windeployqt (it is not a Qt executable)
if defined SCRITE_CRASHPAD_ROOT (
    if exist "!SCRITE_CRASHPAD_ROOT!\bin\crashpad_handler.exe" (
        echo Copying crashpad_handler...
        copy /y "!SCRITE_CRASHPAD_ROOT!\bin\crashpad_handler.exe" "!STAGING_DIR!" >nul
    )
)

:: Step 7: Copy project-built dependencies
echo Copying project dependencies...
copy /y "%PROJECT_ROOT%\binary\KF6SonnetCore.dll" "%STAGING_DIR%" >nul
copy /y "%PROJECT_ROOT%\binary\phtranslator.dll" "%STAGING_DIR%" >nul
xcopy /y /i /e "%PROJECT_ROOT%\binary\kf6" "%STAGING_DIR%\kf6" >nul
set "QUAZIP_VCPKG=%PROJECT_ROOT%\thirdparty\static\quazip\.vcpkg\packages"
copy /y "%QUAZIP_VCPKG%\zlib_x64-windows\bin\zlib1.dll" "%STAGING_DIR%" >nul
copy /y "%QUAZIP_VCPKG%\bzip2_x64-windows\bin\bz2.dll" "%STAGING_DIR%" >nul

:: Step 9: Copy additional assets
echo Copying additional assets...
copy /y "%ASSETS_DIR%\FileAssociation.nsh" "%STAGING_DIR%" >nul
copy /y "%ASSETS_DIR%\license.txt" "%STAGING_DIR%" >nul

:: Step 10: Refresh CMake configuration to regenerate CPackConfig.cmake.
:: Pass SCRITE_STAGING_DIR so that CPACK_INSTALLED_DIRECTORIES is baked in.
echo Refreshing CMake configuration...
set "STAGING_FWD=%STAGING_DIR:\=/%"
cmake -B "%PROJECT_ROOT%\%BUILD_DIR%" -S "%PROJECT_ROOT%" "-DSCRITE_STAGING_DIR=%STAGING_FWD%"
if errorlevel 1 goto :error

:: Step 11: Run CPack NSIS generator
echo Creating NSIS installer with CPack...
set "CPACK_ARGS="
if defined VERSION_SUFFIX set "CPACK_ARGS=-DSCRITE_VERSION_SUFFIX=%VERSION_SUFFIX%"

cpack --config "%PROJECT_ROOT%\%BUILD_DIR%\CPackConfig.cmake" -G NSIS -B "%STAGING_DIR%" %CPACK_ARGS%
if errorlevel 1 goto :error

:: Find generated installer
set "SETUP_EXE=%PROJECT_ROOT%\binary\packages\Scrite-%VERSION%%VERSION_SUFFIX%-64bit-Setup.exe"
if not exist "%SETUP_EXE%" (
    echo ERROR: Setup executable not found after packaging: %SETUP_EXE% >&2
    exit /b 1
)

:: Step 11: Sign installer if requested
if "%SIGN%"=="1" (
    if defined CodeSignTool (
        echo Signing installer...
        "!CodeSignTool!" sign /tr http://timestamp.sectigo.com /td sha256 /fd sha256 /n "!SCRITE_BUSINESS_NAME!" "!SETUP_EXE!"
        if errorlevel 1 goto :error
    )
)

:: Clean up staging
rmdir /s /q "%STAGING_DIR%"

:: Print manifest
for %%f in ("%SETUP_EXE%") do (
    echo Package created: %%~nxf
    echo   Path: %%f
    echo   Size: %%~zf bytes
)

echo.
echo =========================================================
echo Windows packaging complete^^!
echo =========================================================
exit /b 0

:error
echo.
echo ERROR: Packaging failed. >&2
exit /b 1
