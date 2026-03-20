[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Release",

    [string]$BuildDir = "",

    [string]$DistDir = "",

    [string]$RuntimeLibDir = "",

    [switch]$Clean
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent $scriptDir
$wslBuildScript = Join-Path $scriptDir "_build-trimui_sps-wsl.sh"
$packageRoot = Join-Path $repoRoot "device\trimui_sps\package-root"

function Resolve-BuildDir {
    param(
        [string]$RepoRoot,
        [string]$RequestedBuildDir,
        [string]$SelectedConfig
    )

    if ($RequestedBuildDir) {
        return (Join-Path $RepoRoot $RequestedBuildDir)
    }

    return (Join-Path $RepoRoot (Join-Path "out\trimui_sps" $SelectedConfig))
}

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
        return (Join-Path $RepoRoot "dist\trimui_sps")
    }

    return (Join-Path $RepoRoot ("dist\trimui_sps-" + $SelectedConfig.ToLowerInvariant()))
}

function Convert-ToWslPath {
    param(
        [string]$WindowsPath
    )

    $trimmed = [System.IO.Path]::GetFullPath($WindowsPath)
    $drive = $trimmed.Substring(0, 1).ToLowerInvariant()
    $rest = $trimmed.Substring(2).Replace('\', '/')
    return "/mnt/$drive$rest"
}

function Resolve-BuiltBinaryPath {
    param(
        [string]$ResolvedBuildDir
    )

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

    throw "Built TrimUI SPS binary not found in $ResolvedBuildDir"
}

function Copy-TreeIfExists {
    param(
        [string]$SourcePath,
        [string]$DestinationPath
    )

    if (Test-Path $SourcePath) {
        New-Item -ItemType Directory -Force $DestinationPath | Out-Null
        Copy-Item (Join-Path $SourcePath "*") $DestinationPath -Recurse -Force
    }
}

function Stage-TrimuiSpsDist {
    param(
        [string]$RepoRoot,
        [string]$ResolvedBuildDir,
        [string]$ResolvedDistDir,
        [string]$PackageRoot,
        [string]$RuntimeLibDir
    )

    if (Test-Path $ResolvedDistDir) {
        Remove-Item -Recurse -Force $ResolvedDistDir
    }

    New-Item -ItemType Directory -Force $ResolvedDistDir | Out-Null
    Copy-Item (Join-Path $PackageRoot "Apps") $ResolvedDistDir -Recurse -Force

    $appRoot = Join-Path $ResolvedDistDir "Apps\ByteDeck"
    New-Item -ItemType Directory -Force (Join-Path $appRoot "bin") | Out-Null
    New-Item -ItemType Directory -Force (Join-Path $appRoot "lib") | Out-Null
    New-Item -ItemType Directory -Force (Join-Path $appRoot "assets") | Out-Null
    New-Item -ItemType Directory -Force (Join-Path $appRoot "scripts") | Out-Null
    New-Item -ItemType Directory -Force (Join-Path $appRoot "cache\logs") | Out-Null

    $binaryPath = Resolve-BuiltBinaryPath -ResolvedBuildDir $ResolvedBuildDir
    Copy-Item $binaryPath (Join-Path $appRoot "bin\bytedeck") -Force
    Copy-TreeIfExists -SourcePath (Join-Path $RepoRoot "config") -DestinationPath (Join-Path $appRoot "config")
    Copy-TreeIfExists -SourcePath (Join-Path $RepoRoot "themes") -DestinationPath (Join-Path $appRoot "themes")
    Copy-TreeIfExists -SourcePath (Join-Path $RepoRoot "assets") -DestinationPath (Join-Path $appRoot "assets")
    Copy-Item (Join-Path $RepoRoot "scripts\launch_item.sh") (Join-Path $appRoot "scripts\launch_item.sh") -Force

    if ($RuntimeLibDir) {
        if (-not (Test-Path $RuntimeLibDir)) {
            throw "Runtime library directory not found: $RuntimeLibDir"
        }

        Copy-Item (Join-Path $RuntimeLibDir "*") (Join-Path $appRoot "lib") -Recurse -Force
    }
}

if (-not (Get-Command wsl.exe -ErrorAction SilentlyContinue)) {
    throw "wsl.exe not found. Install WSL with Ubuntu first."
}

if (-not (Test-Path $wslBuildScript)) {
    throw "Internal WSL build script not found: $wslBuildScript"
}

if (-not (Test-Path $packageRoot)) {
    throw "TrimUI SPS package template not found: $packageRoot"
}

$resolvedBuildDir = Resolve-BuildDir -RepoRoot $repoRoot -RequestedBuildDir $BuildDir -SelectedConfig $Config
$resolvedDistDir = Resolve-DistDir -RepoRoot $repoRoot -RequestedDistDir $DistDir -SelectedConfig $Config

if ($Clean -and (Test-Path $resolvedDistDir)) {
    Remove-Item -Recurse -Force $resolvedDistDir
}

$repoRootWsl = Convert-ToWslPath -WindowsPath $repoRoot
$buildDirArg = if ($BuildDir) { $BuildDir.Replace('\', '/') } else { "out/trimui_sps/$Config" }
$cleanArg = if ($Clean) { " --clean" } else { "" }
$bashCommand = "cd ""$repoRootWsl"" && ./scripts/_build-trimui_sps-wsl.sh --config $Config --build-dir ""$buildDirArg""$cleanArg"

Write-Host "Building ByteDeck for trimui_sps ($Config)..."
& wsl.exe -d Ubuntu bash -lc $bashCommand
if ($LASTEXITCODE -ne 0) {
    throw "TrimUI SPS WSL build failed."
}

Stage-TrimuiSpsDist -RepoRoot $repoRoot -ResolvedBuildDir $resolvedBuildDir -ResolvedDistDir $resolvedDistDir -PackageRoot $packageRoot -RuntimeLibDir $RuntimeLibDir
Write-Host "TrimUI SPS distribution is ready: $resolvedDistDir"
