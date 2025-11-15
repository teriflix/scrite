@echo off
setlocal enabledelayedexpansion

REM This script checks if clang-format.exe is in the system PATH
REM and then recursively formats .h, .cpp, and .mm files,
REM starting from two directories above its own location.
REM It skips any files located within a '3rdparty' directory.

REM 1. Check if clang-format.exe can be found in the system PATH.
where clang-format.exe >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: clang-format.exe could not be found in your system PATH.
    echo Please ensure the directory containing clang-format.exe is added to your PATH environment variable.
    goto :eof
)

REM Define the root directory for the search as two levels up from the script's location.
set "ROOT_DIR=%~dp0..\.."

echo clang-format.exe found in PATH.
echo Starting formatting in: "%ROOT_DIR%"
echo (Will skip files in any '3rdparty' directory)
echo.

REM 2. Recursively find and format files in the specified root directory.
for /r "%ROOT_DIR%" %%f in (*.h *.cpp *.mm) do (
    set "filepath=%%f"
    
    REM Check if the filepath contains "\3rdparty\". We add backslashes to match the directory name.
    set "checkpath=!filepath:\3rdparty\=!"
    
    REM If the path remains unchanged after substitution, "3rdparty" was not found.
    if "!filepath!"=="!checkpath!" (
        echo Formatting "%%f"
        clang-format -i "%%f"
    ) else (
        echo Skipping "%%f" (in 3rdparty folder)
    )
)

echo.
echo Formatting complete.
endlocal
goto :eof