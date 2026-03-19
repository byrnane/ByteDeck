[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Release",

    [string]$BuildDir = "build"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent $scriptDir
$resolvedBuildDir = Join-Path $repoRoot $BuildDir

$exePath = Join-Path $resolvedBuildDir "$Config\bytedeck.exe"
if (-not (Test-Path $exePath)) {
    $fallbackExePath = Join-Path $resolvedBuildDir "bytedeck.exe"
    if (Test-Path $fallbackExePath) {
        $exePath = $fallbackExePath
    } else {
        throw "Executable not found. Build first: .\scripts\build-windows.ps1"
    }
}

Write-Host "Running $exePath"
& $exePath
