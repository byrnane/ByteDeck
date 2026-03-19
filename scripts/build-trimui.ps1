[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Release",

    [string]$BuildDir = "build-trimui",

    [string]$ToolchainFile = "",

    [string]$ToolchainPrefix = "",

    [string]$Sysroot = "",

    [string]$Sdl2Root = "",

    [switch]$Clean
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repoRoot = Split-Path -Parent $scriptDir
$resolvedBuildDir = Join-Path $repoRoot $BuildDir

function Get-OfficialTrimuiSdkRoots {
    param(
        [string]$RepoRoot
    )

    return @(
        (Join-Path $RepoRoot "sdk_tg5050_linux_v1.0.0"),
        (Join-Path $RepoRoot "toolchains\sdk_tg5050_linux_v1.0.0"),
        (Join-Path $RepoRoot "toolchains\sdk_tg5050_linux_v1.0.0\sdk_tg5050_linux_v1.0.0")
    )
}

function Resolve-OfficialToolchainPrefix {
    param(
        [string]$RepoRoot
    )

    foreach ($sdkRoot in (Get-OfficialTrimuiSdkRoots -RepoRoot $RepoRoot)) {
        $candidate = Join-Path $sdkRoot "host\opt\ext-toolchain\bin\aarch64-none-linux-gnu-"
        $compiler = "${candidate}gcc"
        if (Test-Path $compiler) {
            return $candidate
        }
    }

    return ""
}

function Resolve-OfficialSysroot {
    param(
        [string]$RepoRoot
    )

    foreach ($sdkRoot in (Get-OfficialTrimuiSdkRoots -RepoRoot $RepoRoot)) {
        $candidate = Join-Path $sdkRoot "host\aarch64-buildroot-linux-gnu\sysroot"
        if (Test-Path $candidate) {
            return $candidate
        }
    }

    return ""
}

function Resolve-OfficialSdl2Root {
    param(
        [string]$RepoRoot
    )

    $sysroot = Resolve-OfficialSysroot -RepoRoot $RepoRoot
    if (-not $sysroot) {
        return ""
    }

    $candidate = Join-Path $sysroot "usr"
    if (Test-Path (Join-Path $candidate "lib\cmake\SDL2\sdl2-config.cmake")) {
        return $candidate
    }

    return ""
}

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw "cmake not found in PATH. Install CMake first."
}

if (-not $ToolchainFile) {
    $ToolchainFile = Join-Path $repoRoot "cmake\toolchains\trimui-aarch64-linux-gnu.cmake"
}

if (-not (Test-Path $ToolchainFile)) {
    throw "TrimUI toolchain file not found: $ToolchainFile"
}

if (-not $ToolchainPrefix -and $env:BYTEDECK_TRIMUI_TOOLCHAIN_PREFIX) {
    $ToolchainPrefix = $env:BYTEDECK_TRIMUI_TOOLCHAIN_PREFIX
}

if (-not $ToolchainPrefix) {
    $ToolchainPrefix = Resolve-OfficialToolchainPrefix -RepoRoot $repoRoot
}

if (-not $ToolchainPrefix) {
    throw @"
Missing cross-compiler prefix.

Pass -ToolchainPrefix explicitly or set BYTEDECK_TRIMUI_TOOLCHAIN_PREFIX.
Example:
  powershell -ExecutionPolicy Bypass -File .\scripts\build-trimui.ps1 -ToolchainPrefix C:\toolchains\aarch64-none-linux-gnu\bin\aarch64-none-linux-gnu-
"@
}

if (-not $Sysroot -and $env:BYTEDECK_TRIMUI_SYSROOT) {
    $Sysroot = $env:BYTEDECK_TRIMUI_SYSROOT
}

if (-not $Sysroot) {
    $Sysroot = Resolve-OfficialSysroot -RepoRoot $repoRoot
}

if (-not $Sdl2Root) {
    $Sdl2Root = Resolve-OfficialSdl2Root -RepoRoot $repoRoot
}

if ($Clean -and (Test-Path $resolvedBuildDir)) {
    Remove-Item -Recurse -Force $resolvedBuildDir
}

$configureArgs = @(
    "-S", $repoRoot,
    "-B", $resolvedBuildDir,
    "-DCMAKE_TOOLCHAIN_FILE=$ToolchainFile",
    "-DCMAKE_BUILD_TYPE=$Config",
    "-DBYTEDECK_TRIMUI_TOOLCHAIN_PREFIX=$ToolchainPrefix"
)

if ($Sysroot) {
    $configureArgs += "-DBYTEDECK_TRIMUI_SYSROOT=$Sysroot"
}

if ($Sdl2Root) {
    $configureArgs += "-DBYTEDECK_SDL2_ROOT=$Sdl2Root"
}

Write-Host "Configuring ByteDeck for TrimUI tg5050 ($Config)..."
Write-Host "Toolchain prefix: $ToolchainPrefix"
if ($Sysroot) {
    Write-Host "Sysroot: $Sysroot"
}
if ($Sdl2Root) {
    Write-Host "SDL2 root: $Sdl2Root"
}
& cmake @configureArgs
if ($LASTEXITCODE -ne 0) {
    throw "CMake configure failed."
}

Write-Host "Building ByteDeck for TrimUI tg5050 ($Config)..."
& cmake --build $resolvedBuildDir --parallel
if ($LASTEXITCODE -ne 0) {
    throw "CMake build failed."
}

Write-Host "TrimUI build completed."
