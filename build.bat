@echo off
setlocal

:: Check for "clean" argument
if "%1"=="clean" (
    echo Cleaning build directory...
    rmdir /s /q build
    echo Build directory removed.
    exit /b 0
)

:: Create build directory if it doesn't exist
if not exist build (
    mkdir build
)

:: Run CMake configuration and build
cd build
cmake ..
cmake --build . --config Debug

:: Run the executable
cd Debug
ccraft.exe