@echo off
setlocal
powershell -ExecutionPolicy Bypass -File "%~dp0clean-generated.ps1" %*
if errorlevel 1 goto :error
goto :eof

:error
echo.
echo Cleanup failed.
pause
