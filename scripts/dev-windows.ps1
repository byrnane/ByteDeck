[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Release",

    [string]$BuildDir = "",

    [string]$DistDir = "",

    [string]$ToolchainFile = "",

    [switch]$Clean
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent $scriptDir
$buildScript = Join-Path $scriptDir "build-windows.ps1"

function Resolve-DistDir {
    param(
        [string]$RepoRoot,
        [string]$RequestedDistDir,
        [string]$SelectedConfig
    )

    if ($RequestedDistDir) {
        return (Join-Path $RepoRoot $RequestedDistDir)
    }

    if ($SelectedConfig -eq "Release") {
        return (Join-Path $RepoRoot "dist\windows")
    }

    return (Join-Path $RepoRoot ("dist\windows-" + $SelectedConfig.ToLowerInvariant()))
}

$resolvedDistDir = Resolve-DistDir -RepoRoot $repoRoot -RequestedDistDir $DistDir -SelectedConfig $Config

& $buildScript -Config $Config -BuildDir $BuildDir -DistDir $DistDir -ToolchainFile $ToolchainFile -Clean:$Clean
if ($LASTEXITCODE -ne 0) {
    throw "Windows build failed."
}

$exePath = Join-Path $resolvedDistDir "bytedeck.exe"
if (-not (Test-Path $exePath)) {
    throw "Executable not found: $exePath"
}

$env:BYTEDECK_ROOT = $repoRoot
$env:BYTEDECK_ROMS_ROOT = Join-Path $repoRoot "roms"
$env:BYTEDECK_BIOS_ROOT = Join-Path $repoRoot "bios"
$env:BYTEDECK_APPS_ROOT = Join-Path $repoRoot "Apps"
$env:BYTEDECK_COLLECTIONS_ROOT = Join-Path $repoRoot "collections"
$env:BYTEDECK_CACHE_ROOT = Join-Path $repoRoot "out\runtime"

Write-Host "Running $exePath"
& $exePath
