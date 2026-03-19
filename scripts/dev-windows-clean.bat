@echo off
setlocal

set SCRIPT_DIR=%~dp0
call "%SCRIPT_DIR%dev-windows.bat" -Clean %*
exit /b %ERRORLEVEL%
