[CmdletBinding()]
param(
    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$Arguments
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$scriptsDir = Join-Path $repoRoot "scripts"

$commands = [ordered]@{
    "dev-windows" = @{
        Label = "Dev Windows"
        Description = "Build and run the desktop launcher from dist/windows."
        Script = "dev-windows.ps1"
    }
    "run-windows" = @{
        Label = "Run Windows"
        Description = "Run the existing desktop package from dist/windows."
        Script = "run-windows.ps1"
    }
    "build-windows" = @{
        Label = "Build Windows"
        Description = "Build the desktop package into dist/windows."
        Script = "build-windows.ps1"
    }
    "build-trimui_sps" = @{
        Label = "Build TrimUI SPS"
        Description = "Cross-build and package the SD overlay into dist/trimui_sps."
        Script = "build-trimui_sps.ps1"
    }
    "clean" = @{
        Label = "Clean"
        Description = "Remove generated build outputs and runtime caches."
        Script = "clean.ps1"
    }
}

function Write-LauncherHeader {
    Clear-Host
    Write-Host ""
    Write-Host "  ByteDeck Launcher" -ForegroundColor Cyan
    Write-Host "  Build and packaging shortcuts for everyday work." -ForegroundColor DarkGray
    Write-Host ""
    Write-Host "  Repo:  $repoRoot" -ForegroundColor Gray
    Write-Host "  Dist:  $(Join-Path $repoRoot 'dist')" -ForegroundColor Gray
    Write-Host "  Out:   $(Join-Path $repoRoot 'out')" -ForegroundColor Gray
    Write-Host ""
}

function Write-LauncherHelp {
    Write-LauncherHeader
    Write-Host "Usage:" -ForegroundColor Yellow
    Write-Host "  .\ByteDeck.bat dev-windows" -ForegroundColor White
    Write-Host "  .\ByteDeck.bat run-windows" -ForegroundColor White
    Write-Host "  .\ByteDeck.bat build-windows -Clean" -ForegroundColor White
    Write-Host "  .\ByteDeck.bat build-trimui_sps -Clean" -ForegroundColor White
    Write-Host "  .\ByteDeck.bat clean" -ForegroundColor White
    Write-Host ""
    Write-Host "Available commands:" -ForegroundColor Yellow

    foreach ($entry in $commands.GetEnumerator()) {
        $name = $entry.Key.PadRight(18)
        Write-Host "  $name $($entry.Value.Description)" -ForegroundColor White
    }

    Write-Host ""
}

function Invoke-LauncherCommand {
    param(
        [string]$CommandName,
        [string[]]$ForwardedArgs
    )

    if (-not $commands.Contains($CommandName)) {
        throw "Unknown command: $CommandName"
    }

    $scriptPath = Join-Path $scriptsDir $commands[$CommandName].Script
    if (-not (Test-Path $scriptPath)) {
        throw "Script not found: $scriptPath"
    }

    $cleanArgs = @($ForwardedArgs | Where-Object { -not [string]::IsNullOrWhiteSpace($_) })

    Write-Host ""
    Write-Host "Running $CommandName..." -ForegroundColor Green
    Write-Host ""

    & $scriptPath @cleanArgs
}

function Read-InteractiveCommand {
    while ($true) {
        Write-LauncherHeader
        Write-Host "Choose an action:" -ForegroundColor Yellow
        Write-Host ""
        Write-Host "  1. Dev Windows        Build and run the desktop app" -ForegroundColor White
        Write-Host "  2. Run Windows        Run existing dist/windows build" -ForegroundColor White
        Write-Host "  3. Build Windows      Build dist/windows" -ForegroundColor White
        Write-Host "  4. Build TrimUI SPS   Build dist/trimui_sps" -ForegroundColor White
        Write-Host "  5. Clean              Remove out/, dist/ and runtime cache" -ForegroundColor White
        Write-Host "  H. Help               Show command examples" -ForegroundColor White
        Write-Host "  Q. Exit" -ForegroundColor White
        Write-Host ""

        $choice = (Read-Host "Select").Trim().ToLowerInvariant()

        switch ($choice) {
            "1" { return @{ Name = "dev-windows"; Args = @() } }
            "2" { return @{ Name = "run-windows"; Args = @() } }
            "3" { return @{ Name = "build-windows"; Args = @() } }
            "4" { return @{ Name = "build-trimui_sps"; Args = @() } }
            "5" { return @{ Name = "clean"; Args = @() } }
            "h" {
                Write-LauncherHelp
                Read-Host "Press Enter to return to the menu" | Out-Null
            }
            "q" { return $null }
            default {
                Write-Host ""
                Write-Host "Unknown selection: $choice" -ForegroundColor Red
                Start-Sleep -Seconds 1
            }
        }
    }
}

try {
    $normalizedArguments = @()
    if ($null -ne $Arguments) {
        $normalizedArguments = @($Arguments | Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
    }

    if ($normalizedArguments.Count -gt 0) {
        $commandName = $normalizedArguments[0].ToLowerInvariant()
        $forwardedArgs = if ($normalizedArguments.Count -gt 1) { $normalizedArguments[1..($normalizedArguments.Count - 1)] } else { @() }

        if ($commandName -in @("help", "-h", "--help", "/?")) {
            Write-LauncherHelp
            exit 0
        }

        Invoke-LauncherCommand -CommandName $commandName -ForwardedArgs $forwardedArgs
        exit $LASTEXITCODE
    }

    $selection = Read-InteractiveCommand
    if ($null -eq $selection) {
        exit 0
    }

    Invoke-LauncherCommand -CommandName $selection.Name -ForwardedArgs $selection.Args
    exit $LASTEXITCODE
}
catch {
    Write-Host ""
    Write-Host $_.Exception.Message -ForegroundColor Red
    Write-Host ""
    exit 1
}
