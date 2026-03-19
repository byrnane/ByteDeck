@echo off
setlocal

set "ROOT_DIR=%~dp0"
for %%I in ("%ROOT_DIR%") do set "ROOT_DIR=%%~fI"

if /I "%~1"=="dev-windows" goto run_dev_windows
if /I "%~1"=="build-windows" goto run_build_windows
if /I "%~1"=="build-trimui_sps" goto run_build_trimui_sps
if /I "%~1"=="clean" goto run_clean
if /I "%~1"=="help" goto show_help
if /I "%~1"=="--help" goto show_help
if /I "%~1"=="-h" goto show_help
if not "%~1"=="" goto unknown_command

:menu
cls
echo ByteDeck Launcher
echo.
echo   1. dev-windows
echo   2. build-windows
echo   3. build-trimui_sps
echo   4. clean
echo   5. exit
echo.
set /p CHOICE=Select action: 

if "%CHOICE%"=="1" goto run_dev_windows
if "%CHOICE%"=="2" goto run_build_windows
if "%CHOICE%"=="3" goto run_build_trimui_sps
if "%CHOICE%"=="4" goto run_clean
if "%CHOICE%"=="5" goto :eof

echo.
echo Unknown selection.
pause
goto menu

:run_dev_windows
call "%ROOT_DIR%scripts\dev-windows.bat" %2 %3 %4 %5 %6 %7 %8 %9
goto :eof

:run_build_windows
call "%ROOT_DIR%scripts\build-windows.bat" %2 %3 %4 %5 %6 %7 %8 %9
goto :eof

:run_build_trimui_sps
call "%ROOT_DIR%scripts\build-trimui_sps.bat" %2 %3 %4 %5 %6 %7 %8 %9
goto :eof

:run_clean
call "%ROOT_DIR%scripts\clean.bat" %2 %3 %4 %5 %6 %7 %8 %9
goto :eof

:unknown_command
echo Unknown command: %~1
echo.
goto show_help

:show_help
echo Usage:
echo   ByteDeck.bat
echo   ByteDeck.bat dev-windows [args]
echo   ByteDeck.bat build-windows [args]
echo   ByteDeck.bat build-trimui_sps [args]
echo   ByteDeck.bat clean
echo.
echo Examples:
echo   ByteDeck.bat dev-windows
echo   ByteDeck.bat build-windows -Clean
echo   ByteDeck.bat build-trimui_sps -Clean
echo   ByteDeck.bat clean
exit /b 1
