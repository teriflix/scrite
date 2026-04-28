@echo off
:: Build script for Scrite on Windows.
:: Usage:
::   build.bat [--clean] [--type <version-type>]
::
:: Options:
::   --clean              Remove build directory and rebuild from scratch
::   --type <type>        Set SCRITE_VERSION_TYPE (e.g. "beta", "rc", "dev")
::                        Default: empty string (release)
::
:: Environment variables (or set in packaging.config.local.bat):
::   CMAKE_DIR            Path to cmake bin directory (optional, uses PATH if not set)
::   NINJA_DIR            Path to ninja.exe directory (optional; enables Ninja generator)
::   VCVARS_BAT           Path to vcvarsall.bat (required when using Ninja generator)
::   MSVC_ARCH            Architecture for vcvarsall.bat (default: amd64)
::   BUILD_DIR            CMake build directory (default: build)
::   PARALLEL_JOBS        Number of parallel build jobs (default: 4)
::   QT_BIN_DIR           Path to Qt bin directory (optional, uses PATH if not set)

setlocal enabledelayedexpansion

set "SCRIPT_DIR=%~dp0"
set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"
pushd "%SCRIPT_DIR%\..\.."
set "PROJECT_ROOT=%CD%"
popd

if not defined BUILD_DIR set "BUILD_DIR=build"
if not defined PARALLEL_JOBS set "PARALLEL_JOBS=4"
if not defined MSVC_ARCH set "MSVC_ARCH=amd64"

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
set "CLEAN=0"
set "VERSION_TYPE="

:parse_args
if "%~1"=="" goto :args_done
if /i "%~1"=="--clean" ( set "CLEAN=1" & shift & goto :parse_args )
if /i "%~1"=="--type" (
    if "%~2"=="" ( echo ERROR: --type requires a value >&2 & exit /b 1 )
    set "VERSION_TYPE=%~2"
    shift & shift
    goto :parse_args
)
echo ERROR: Unknown option: %~1 >&2
exit /b 1
:args_done

:: Clean build directory if requested
if "%CLEAN%"=="1" (
    echo Removing build directory...
    if exist "%PROJECT_ROOT%\%BUILD_DIR%" rmdir /s /q "%PROJECT_ROOT%\%BUILD_DIR%"
)

:: Add CMAKE_DIR, NINJA_DIR, and QT_BIN_DIR to PATH if set
if defined CMAKE_DIR set "PATH=%CMAKE_DIR%;%PATH%"
if defined NINJA_DIR set "PATH=%NINJA_DIR%;%PATH%"
if defined QT_BIN_DIR set "PATH=%QT_BIN_DIR%;%PATH%"

:: Choose generator: Ninja (parallel custom commands) or default MSBuild
set "CMAKE_GENERATOR_ARGS="
if defined NINJA_DIR (
    where ninja >nul 2>&1
    if not errorlevel 1 (
        echo Using Ninja generator ^(parallel QML compilation^)
        set "CMAKE_GENERATOR_ARGS=-G Ninja"

        :: Ninja requires the MSVC environment to be initialised
        if not defined VCVARS_BAT (
            echo ERROR: NINJA_DIR is set but VCVARS_BAT is not. >&2
            echo   Set VCVARS_BAT to the path of vcvarsall.bat, e.g.: >&2
            echo   C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat >&2
            exit /b 1
        )
        echo Initialising MSVC environment ^(%MSVC_ARCH%^)...
        call "!VCVARS_BAT!" %MSVC_ARCH%
        if errorlevel 1 goto :error
    )
)

:: Configure CMake
echo Configuring CMake...
set "CMAKE_ARGS=-DCMAKE_BUILD_TYPE=Release"
if not defined NINJA_DIR (
    :: /MP only helps with MSBuild; Ninja parallelises natively
    set "CMAKE_ARGS=!CMAKE_ARGS! -DCMAKE_CXX_FLAGS=/MP%PARALLEL_JOBS%"
)
if defined VERSION_TYPE (
    set "CMAKE_ARGS=!CMAKE_ARGS! -DSCRITE_VERSION_TYPE=%VERSION_TYPE%"
    echo   Version type: %VERSION_TYPE%
)

cmake -B "%PROJECT_ROOT%\%BUILD_DIR%" -S "%PROJECT_ROOT%" !CMAKE_GENERATOR_ARGS! !CMAKE_ARGS!
if errorlevel 1 goto :error

:: Build
echo Building Scrite with %PARALLEL_JOBS% parallel jobs...
cmake --build "%PROJECT_ROOT%\%BUILD_DIR%" --parallel %PARALLEL_JOBS%
if errorlevel 1 goto :error

echo.
echo Build complete: %PROJECT_ROOT%\%BUILD_DIR%
exit /b 0

:error
echo.
echo ERROR: Build failed. >&2
exit /b 1
