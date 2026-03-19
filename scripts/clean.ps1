[CmdletBinding()]
param()

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent $scriptDir

$pathsToRemove = @(
    "out",
    "dist",
    "build",
    "build-trimui",
    "cache"
)

foreach ($relativePath in $pathsToRemove) {
    $targetPath = Join-Path $repoRoot $relativePath
    if (Test-Path $targetPath) {
        Remove-Item -Recurse -Force $targetPath
        Write-Host "Removed: $relativePath"
    }
}
