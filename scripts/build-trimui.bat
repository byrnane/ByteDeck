@echo off
setlocal
powershell -ExecutionPolicy Bypass -File "%~dp0build-trimui.ps1" %*
if errorlevel 1 goto :error
goto :eof

:error
echo.
echo TrimUI build failed.
pause
