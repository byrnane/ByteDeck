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

function Resolve-ToolchainFile {
    param(
        [string]$RequestedToolchainFile
    )

    if ($RequestedToolchainFile) {
        return $RequestedToolchainFile
    }

    $candidates = New-Object System.Collections.Generic.List[string]

    if ($env:VCPKG_ROOT) {
        $candidates.Add((Join-Path $env:VCPKG_ROOT "scripts\buildsystems\vcpkg.cmake"))
    }

    $vcpkgCommand = Get-Command vcpkg -ErrorAction SilentlyContinue
    if ($vcpkgCommand) {
        $vcpkgRoot = Split-Path -Parent $vcpkgCommand.Source
        $candidates.Add((Join-Path $vcpkgRoot "scripts\buildsystems\vcpkg.cmake"))
    }

    $commonRoots = @(
        "C:\vcpkg",
        "D:\vcpkg",
        (Join-Path $env:USERPROFILE "vcpkg"),
        (Join-Path $env:USERPROFILE "source\repos\vcpkg")
    )

    foreach ($root in $commonRoots) {
        if ($root) {
            $candidates.Add((Join-Path $root "scripts\buildsystems\vcpkg.cmake"))
        }
    }

    foreach ($candidate in $candidates) {
        if ($candidate -and (Test-Path $candidate)) {
            return $candidate
        }
    }

    return ""
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

function Stage-WindowsDist {
    param(
        [string]$RepoRoot,
        [string]$ResolvedBuildDir,
        [string]$ResolvedDistDir
    )

    if (Test-Path $ResolvedDistDir) {
        Remove-Item -Recurse -Force $ResolvedDistDir
    }

    New-Item -ItemType Directory -Force $ResolvedDistDir | Out-Null
    New-Item -ItemType Directory -Force (Join-Path $ResolvedDistDir "scripts") | Out-Null
    New-Item -ItemType Directory -Force (Join-Path $ResolvedDistDir "cache\logs") | Out-Null
    New-Item -ItemType Directory -Force (Join-Path $ResolvedDistDir "ROMS") | Out-Null
    New-Item -ItemType Directory -Force (Join-Path $ResolvedDistDir "BIOS") | Out-Null
    New-Item -ItemType Directory -Force (Join-Path $ResolvedDistDir "Apps") | Out-Null
    New-Item -ItemType Directory -Force (Join-Path $ResolvedDistDir "collections") | Out-Null

    $exePath = Join-Path $ResolvedBuildDir "bytedeck.exe"
    if (-not (Test-Path $exePath)) {
        throw "Built executable not found: $exePath"
    }

    Copy-Item $exePath (Join-Path $ResolvedDistDir "bytedeck.exe") -Force

    Get-ChildItem -Path $ResolvedBuildDir -Filter *.dll -File -ErrorAction SilentlyContinue |
        ForEach-Object { Copy-Item $_.FullName $ResolvedDistDir -Force }

    Copy-TreeIfExists -SourcePath (Join-Path $RepoRoot "config") -DestinationPath (Join-Path $ResolvedDistDir "config")
    Copy-TreeIfExists -SourcePath (Join-Path $RepoRoot "themes") -DestinationPath (Join-Path $ResolvedDistDir "themes")
    Copy-TreeIfExists -SourcePath (Join-Path $RepoRoot "assets") -DestinationPath (Join-Path $ResolvedDistDir "assets")
    Copy-TreeIfExists -SourcePath (Join-Path $RepoRoot "collections") -DestinationPath (Join-Path $ResolvedDistDir "collections")
    Copy-Item (Join-Path $RepoRoot "scripts\launch_item.sh") (Join-Path $ResolvedDistDir "scripts\launch_item.sh") -Force
}

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw "cmake not found in PATH. Install CMake first."
}

$resolvedBuildDir = Resolve-BuildDir -RepoRoot $repoRoot -RequestedBuildDir $BuildDir -SelectedConfig $Config
$resolvedDistDir = Resolve-DistDir -RepoRoot $repoRoot -RequestedDistDir $DistDir -SelectedConfig $Config
$resolvedToolchain = Resolve-ToolchainFile -RequestedToolchainFile $ToolchainFile
$cacheFile = Join-Path $resolvedBuildDir "CMakeCache.txt"

if ($Clean) {
    if (Test-Path $resolvedBuildDir) {
        Remove-Item -Recurse -Force $resolvedBuildDir
    }
    if (Test-Path $resolvedDistDir) {
        Remove-Item -Recurse -Force $resolvedDistDir
    }
}

$configureArgs = @(
    "-S", $repoRoot,
    "-B", $resolvedBuildDir
)

if ($resolvedToolchain -and -not (Test-Path $cacheFile)) {
    $configureArgs += "-DCMAKE_TOOLCHAIN_FILE=$resolvedToolchain"
    Write-Host "Using toolchain: $resolvedToolchain"
} elseif ($resolvedToolchain) {
    Write-Host "Using existing CMake cache in: $resolvedBuildDir"
} else {
    throw @"
Unable to find the vcpkg toolchain automatically.

Fix one of these and run again:
  1. Set VCPKG_ROOT to your vcpkg folder
  2. Put vcpkg.exe in PATH
  3. Pass -ToolchainFile explicitly
"@
}

$runtimeOutputConfig = $Config.ToUpperInvariant()
$configureArgs += "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=$resolvedBuildDir"
$configureArgs += "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_${runtimeOutputConfig}=$resolvedBuildDir"

Write-Host "Configuring ByteDeck for windows ($Config)..."
& cmake @configureArgs
if ($LASTEXITCODE -ne 0) {
    throw "CMake configure failed."
}

Write-Host "Building ByteDeck for windows ($Config)..."
& cmake --build $resolvedBuildDir --config $Config
if ($LASTEXITCODE -ne 0) {
    throw "CMake build failed."
}

Stage-WindowsDist -RepoRoot $repoRoot -ResolvedBuildDir $resolvedBuildDir -ResolvedDistDir $resolvedDistDir
Write-Host "Windows distribution is ready: $resolvedDistDir"
