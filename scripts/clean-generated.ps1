[CmdletBinding()]
param(
    [switch]$RemoveBuild,
    [switch]$RemoveTrimuiBuild,
    [switch]$RemoveDist = $true,
    [switch]$RemoveCache = $true
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent $scriptDir

function Remove-GeneratedPath {
    param(
        [string]$RelativePath
    )

    $targetPath = Join-Path $repoRoot $RelativePath
    if (Test-Path $targetPath) {
        Remove-Item -Recurse -Force $targetPath
        Write-Host "Removed: $RelativePath"
    }
}

if ($RemoveBuild) {
    Remove-GeneratedPath -RelativePath "build"
}

if ($RemoveTrimuiBuild) {
    Remove-GeneratedPath -RelativePath "build-trimui"
}

if ($RemoveDist) {
    Remove-GeneratedPath -RelativePath "dist"
}

if ($RemoveCache) {
    $cacheLibrary = Join-Path $repoRoot "cache\library.json"
    if (Test-Path $cacheLibrary) {
        Remove-Item -Force $cacheLibrary
        Write-Host "Removed: cache\\library.json"
    }

    $logsPath = Join-Path $repoRoot "cache\logs"
    if (Test-Path $logsPath) {
        Get-ChildItem -Path $logsPath -Filter *.log -File | Remove-Item -Force
        Write-Host "Removed: cache\\logs\\*.log"
    }
}
