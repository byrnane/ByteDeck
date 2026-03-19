@echo off
setlocal
powershell -ExecutionPolicy Bypass -File "%~dp0package-trimui.ps1" %*
if errorlevel 1 goto :error
goto :eof

:error
echo.
echo TrimUI packaging failed.
pause
