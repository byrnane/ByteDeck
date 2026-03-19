@echo off
setlocal

set "REPO_ROOT=%~dp0.."
for %%I in ("%REPO_ROOT%") do set "REPO_ROOT=%%~fI"

wsl.exe -d Ubuntu bash -lc "cd \"$(wslpath '%REPO_ROOT%')\" && ./scripts/build-trimui-wsl.sh %*"
if errorlevel 1 goto :error
goto :eof

:error
echo.
echo TrimUI WSL build failed.
pause
