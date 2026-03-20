@echo off
setlocal
powershell -ExecutionPolicy Bypass -File "%~dp0ByteDeck.ps1" %*
exit /b %errorlevel%
