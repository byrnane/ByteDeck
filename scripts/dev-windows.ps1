[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release")]
    [string]$Config = "Release",

    [string]$BuildDir = "build",

    [string]$ToolchainFile = "",

    [switch]$Clean
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$buildScript = Join-Path $scriptDir "build-windows.ps1"
$runScript = Join-Path $scriptDir "run-windows.ps1"

& $buildScript -Config $Config -BuildDir $BuildDir -ToolchainFile $ToolchainFile -Clean:$Clean
& $runScript -Config $Config -BuildDir $BuildDir
