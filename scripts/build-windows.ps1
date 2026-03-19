[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Release",

    [string]$BuildDir = "",

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

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw "cmake not found in PATH. Install CMake first."
}

$resolvedBuildDir = Resolve-BuildDir -RepoRoot $repoRoot -RequestedBuildDir $BuildDir -SelectedConfig $Config
$resolvedToolchain = Resolve-ToolchainFile -RequestedToolchainFile $ToolchainFile
$cacheFile = Join-Path $resolvedBuildDir "CMakeCache.txt"

if ($Clean -and (Test-Path $resolvedBuildDir)) {
    Remove-Item -Recurse -Force $resolvedBuildDir
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

Example:
  powershell -ExecutionPolicy Bypass -File .\scripts\dev-windows.ps1 -ToolchainFile C:\vcpkg\scripts\buildsystems\vcpkg.cmake
"@
}

$runtimeOutputConfig = $Config.ToUpperInvariant()
$configureArgs += "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY=$resolvedBuildDir"
$configureArgs += "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_${runtimeOutputConfig}=$resolvedBuildDir"

Write-Host "Configuring ByteDeck ($Config)..."
& cmake @configureArgs
if ($LASTEXITCODE -ne 0) {
    throw "CMake configure failed."
}

Write-Host "Building ByteDeck ($Config)..."
& cmake --build $resolvedBuildDir --config $Config
if ($LASTEXITCODE -ne 0) {
    throw "CMake build failed."
}

Write-Host "Build completed."
