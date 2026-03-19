@echo off
setlocal

set SCRIPT_DIR=%~dp0
powershell -ExecutionPolicy Bypass -File "%SCRIPT_DIR%dev-windows.ps1" %*
set EXIT_CODE=%ERRORLEVEL%

if not "%EXIT_CODE%"=="0" (
    echo.
    echo Dev workflow failed with exit code %EXIT_CODE%.
    pause
)

exit /b %EXIT_CODE%
