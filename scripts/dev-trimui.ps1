[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Release",

    [string]$BuildDir = "build-trimui",

    [string]$StageDir = "dist\trimui-sd-overlay",

    [string]$RuntimeLibDir = "",

    [switch]$Clean
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$buildScript = Join-Path $scriptDir "build-trimui-wsl.bat"
$packageScript = Join-Path $scriptDir "package-trimui.ps1"

$buildArgs = @("--config", $Config, "--build-dir", $BuildDir)
if ($Clean) {
    $buildArgs += "--clean"
}

& $buildScript @buildArgs
if ($LASTEXITCODE -ne 0) {
    throw "TrimUI WSL build failed."
}

$packageArgs = @{
    BuildDir = $BuildDir
    StageDir = $StageDir
    Clean = $Clean
}

if ($RuntimeLibDir) {
    $packageArgs.RuntimeLibDir = $RuntimeLibDir
}

& $packageScript @packageArgs
if ($LASTEXITCODE -ne 0) {
    throw "TrimUI packaging failed."
}

Write-Host "TrimUI dev package is ready: $(Join-Path (Split-Path -Parent $scriptDir) $StageDir)"
