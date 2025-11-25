@echo off
REM Windows Native Build Script for IED Simulators
REM Run this on a Windows machine with Visual Studio or MinGW

echo ==================================================
echo IEC61850 IED Simulator Windows Builder
echo ==================================================
echo.

SET "SCRIPT_DIR=%~dp0"
SET "DEPS_DIR=%SCRIPT_DIR%\deps"
SET "LIBIEC_DIR=%DEPS_DIR%\libiec61850"
SET "BUILD_DIR=%LIBIEC_DIR%\build_windows"
SET "OUTPUT_DIR=%SCRIPT_DIR%\ied_simulators_binaries\windows_x64"

REM Check if libiec61850 exists
if not exist "%LIBIEC_DIR%" (
    echo Cloning libiec61850...
    cd "%DEPS_DIR%"
    git clone https://github.com/mz-automation/libiec61850.git
)

REM Create build directory
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
cd "%BUILD_DIR%"

echo.
echo Configuring with CMake...
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release

echo.
echo Building...
cmake --build . --config Release -j 4

echo.
echo Packaging binaries...
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

REM Copy executables
for /r "examples" %%f in (server_example_*.exe) do (
    copy "%%f" "%OUTPUT_DIR%\" >nul 2>&1
)

REM Copy DLLs
for /r %%f in (*.dll) do (
    copy "%%f" "%OUTPUT_DIR%\" >nul 2>&1
)

echo.
echo ================================================
echo Build Complete!
echo Output: %OUTPUT_DIR%
echo ================================================
echo.
echo To run:
echo   cd %OUTPUT_DIR%
echo   server_example_basic_io.exe 10102
echo.
pause
