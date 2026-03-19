[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Release",

    [string]$BuildDir = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent $scriptDir

function Resolve-BuildDir {
    param(
        [string]$RepoRoot,
        [string]$RequestedBuildDir,
        [string]$SelectedConfig
    )

    if ($RequestedBuildDir) {
        return (Join-Path $RepoRoot $RequestedBuildDir)
    }

    return (Join-Path $RepoRoot (Join-Path "out\host" $SelectedConfig))
}

$resolvedBuildDir = Resolve-BuildDir -RepoRoot $repoRoot -RequestedBuildDir $BuildDir -SelectedConfig $Config

$exePath = Join-Path $resolvedBuildDir "bytedeck.exe"
if (-not (Test-Path $exePath)) {
    $fallbackExePath = Join-Path $resolvedBuildDir "$Config\bytedeck.exe"
    if (Test-Path $fallbackExePath) {
        $exePath = $fallbackExePath
    } else {
        throw "Executable not found. Build first: .\scripts\build-windows.ps1"
    }
}

if (-not $env:BYTEDECK_ROOT) {
    $env:BYTEDECK_ROOT = $repoRoot
}
if (-not $env:BYTEDECK_CACHE_ROOT) {
    $env:BYTEDECK_CACHE_ROOT = Join-Path $repoRoot "out\runtime"
}

Write-Host "Running $exePath"
& $exePath
