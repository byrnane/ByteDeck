[CmdletBinding()]
param(
    [string]$BuildDir = "",

    [string]$StageDir = "",

    [string]$BinaryPath = "",

    [string]$RuntimeLibDir = "",

    [switch]$Clean
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent $scriptDir
$packageRoot = Join-Path $repoRoot "device\trimui\package-root"

function Resolve-BuildDir {
    param(
        [string]$RepoRoot,
        [string]$RequestedBuildDir
    )

    if ($RequestedBuildDir) {
        return (Join-Path $RepoRoot $RequestedBuildDir)
    }

    return (Join-Path $RepoRoot "out\trimui\Release")
}

function Resolve-StageDir {
    param(
        [string]$RepoRoot,
        [string]$RequestedStageDir
    )

    if ($RequestedStageDir) {
        return (Join-Path $RepoRoot $RequestedStageDir)
    }

    return (Join-Path $RepoRoot "out\package\trimui-sd-overlay")
}

function Resolve-BinaryPath {
    param(
        [string]$RequestedPath,
        [string]$ResolvedBuildDir
    )

    if ($RequestedPath) {
        return $RequestedPath
    }

    $candidates = @(
        (Join-Path $ResolvedBuildDir "bytedeck"),
        (Join-Path $ResolvedBuildDir "Release\bytedeck"),
        (Join-Path $ResolvedBuildDir "bin\bytedeck")
    )

    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) {
            return $candidate
        }
    }

    throw "Unable to find built TrimUI binary. Pass -BinaryPath explicitly."
}

if (-not (Test-Path $packageRoot)) {
    throw "TrimUI package template not found: $packageRoot"
}

$resolvedBuildDir = Resolve-BuildDir -RepoRoot $repoRoot -RequestedBuildDir $BuildDir
$resolvedStageDir = Resolve-StageDir -RepoRoot $repoRoot -RequestedStageDir $StageDir
$appRoot = Join-Path $resolvedStageDir "Apps\ByteDeck"
$resolvedBinaryPath = Resolve-BinaryPath -RequestedPath $BinaryPath -ResolvedBuildDir $resolvedBuildDir

if ($Clean -and (Test-Path $resolvedStageDir)) {
    Remove-Item -Recurse -Force $resolvedStageDir
}

New-Item -ItemType Directory -Force $resolvedStageDir | Out-Null
if (Test-Path (Join-Path $resolvedStageDir "Apps")) {
    Remove-Item -Recurse -Force (Join-Path $resolvedStageDir "Apps")
}
Copy-Item (Join-Path $packageRoot "Apps") $resolvedStageDir -Recurse -Force

New-Item -ItemType Directory -Force (Join-Path $appRoot "bin") | Out-Null
New-Item -ItemType Directory -Force (Join-Path $appRoot "lib") | Out-Null
New-Item -ItemType Directory -Force (Join-Path $appRoot "assets") | Out-Null
New-Item -ItemType Directory -Force (Join-Path $appRoot "scripts") | Out-Null
New-Item -ItemType Directory -Force (Join-Path $appRoot "cache\logs") | Out-Null

Copy-Item $resolvedBinaryPath (Join-Path $appRoot "bin\bytedeck") -Force
Copy-Item (Join-Path $repoRoot "config\*") (Join-Path $appRoot "config") -Recurse -Force
Copy-Item (Join-Path $repoRoot "assets\*") (Join-Path $appRoot "assets") -Recurse -Force -ErrorAction SilentlyContinue
Copy-Item (Join-Path $repoRoot "scripts\launch_item.sh") (Join-Path $appRoot "scripts\launch_item.sh") -Force

if ($RuntimeLibDir) {
    if (-not (Test-Path $RuntimeLibDir)) {
        throw "Runtime library directory not found: $RuntimeLibDir"
    }

    Copy-Item (Join-Path $RuntimeLibDir "*") (Join-Path $appRoot "lib") -Recurse -Force
}

Write-Host "TrimUI package staged at: $resolvedStageDir"
