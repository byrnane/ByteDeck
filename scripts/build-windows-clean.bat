@echo off
setlocal

set SCRIPT_DIR=%~dp0
call "%SCRIPT_DIR%build-windows.bat" -Clean %*
exit /b %ERRORLEVEL%
