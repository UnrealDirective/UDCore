param(
    [Parameter(Mandatory = $true)]
    [string]$EngineRoot,

    [ValidateSet("Development", "Shipping")]
    [string]$ClientConfiguration = "Development"
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$ScriptRoot = $PSScriptRoot
$RepositoryRoot = (Resolve-Path (Join-Path $ScriptRoot "..\..\..")).Path
$EngineVersion = Split-Path $EngineRoot -Leaf
$WorkRoot = Join-Path $RepositoryRoot "Build\RuntimeHost\$EngineVersion\$ClientConfiguration"
$ProjectRoot = Join-Path $WorkRoot "Project"
$ProjectFile = Join-Path $ProjectRoot "DirectiveUtilitiesRuntimeHost.uproject"
$PluginRoot = Join-Path $ProjectRoot "Plugins\DirectiveUtilities"
$PluginDescriptor = Join-Path $PluginRoot "DirectiveUtilities.uplugin"
$RuntimeTestModule = Join-Path $ProjectRoot "Source\DirectiveUtilitiesRuntimeHostTests"
$RuntimeTestSourceRoot = Join-Path $RepositoryRoot "Source\DirectiveUtilitiesTests"
$ArchiveRoot = Join-Path $WorkRoot "Archive"
$ReportRoot = Join-Path $WorkRoot "Reports"
$PerformanceRoot = Join-Path $WorkRoot "Performance"
$BuildScript = Join-Path $EngineRoot "Engine\Build\BatchFiles\Build.bat"
$RunUAT = Join-Path $EngineRoot "Engine\Build\BatchFiles\RunUAT.bat"
$EditorCommand = Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
$LongestActionPath = Join-Path $ProjectRoot "Plugins\DirectiveUtilities\Intermediate\Build\Win64\x64\UnrealEditor\Development\DirectiveUtilitiesBlueprintNodes\UnrealEditor-DirectiveUtilitiesBlueprintNodes.dll.rsp"

if ($LongestActionPath.Length -ge 260) {
    throw "The Windows RuntimeHost path would exceed Unreal's 260-character action-path limit. Move the repository to a shorter path."
}

foreach ($RequiredFile in @($BuildScript, $RunUAT, $EditorCommand)) {
    if (-not (Test-Path $RequiredFile -PathType Leaf)) {
        throw "Required file not found: $RequiredFile"
    }
}

Remove-Item $WorkRoot -Recurse -Force -ErrorAction SilentlyContinue
New-Item $PluginRoot -ItemType Directory -Force | Out-Null
New-Item (Join-Path $ReportRoot "Editor") -ItemType Directory -Force | Out-Null

robocopy (Join-Path $RepositoryRoot "Tests\RuntimeHost") $ProjectRoot /E /XD Binaries Intermediate Saved Scripts Plugins | Out-Null
if ($LASTEXITCODE -ge 8) {
    throw "Failed to stage the runtime host project."
}

Copy-Item (Join-Path $RepositoryRoot "DirectiveUtilities.uplugin") $PluginDescriptor
foreach ($Directory in @("Source", "Config", "Resources")) {
    $SourceDirectory = Join-Path $RepositoryRoot $Directory
    if (-not (Test-Path $SourceDirectory -PathType Container)) {
        continue
    }

    robocopy $SourceDirectory (Join-Path $PluginRoot $Directory) /E /XD Binaries Intermediate | Out-Null
    if ($LASTEXITCODE -ge 8) {
        throw "Failed to stage plugin directory: $Directory"
    }
}

$RuntimeTestPrivateRoot = Join-Path $RuntimeTestModule "Private\Tests"
$RuntimeTestPublicRoot = Join-Path $RuntimeTestModule "Public\Tests"
New-Item $RuntimeTestPrivateRoot -ItemType Directory -Force | Out-Null
New-Item $RuntimeTestPublicRoot -ItemType Directory -Force | Out-Null
foreach ($TestSource in Get-Content (Join-Path $RepositoryRoot "Tests\RuntimeHost\RuntimeTestSources.txt")) {
    if (-not [string]::IsNullOrWhiteSpace($TestSource)) {
        Copy-Item (Join-Path $RuntimeTestSourceRoot "Private\Tests\$TestSource") $RuntimeTestPrivateRoot
    }
}
Copy-Item (Join-Path $RuntimeTestSourceRoot "Public\Tests\DirectiveUtilTestObject.h") $RuntimeTestPublicRoot

& $BuildScript DirectiveUtilitiesRuntimeHostEditor Win64 Development "-Project=$ProjectFile" -WaitMutex -NoHotReload -ForceUnity -DisableAdaptiveUnity
if ($LASTEXITCODE -ne 0) {
    throw "Editor target build failed."
}

$EditorArguments = @(
    '-ExecCmds=Automation RunTests DirectiveUtilities; Quit',
    "-ReportExportPath=$(Join-Path $ReportRoot 'Editor')",
    '-TestExit=Automation Test Queue Empty',
    "-abslog=$(Join-Path $WorkRoot 'EditorTests.log')",
    '-unattended',
    '-nop4',
    '-nosplash',
    '-NullRHI'
)
& $EditorCommand $ProjectFile @EditorArguments
if ($LASTEXITCODE -ne 0) {
    throw "Editor automation tests failed."
}

$PackageArguments = @(
    'BuildCookRun',
    "-project=$ProjectFile",
    '-noP4',
    '-platform=Win64',
    "-clientconfig=$ClientConfiguration",
    '-build',
    '-cook',
    '-stage',
    '-pak',
    '-skipzenstore',
    '-package',
    '-archive',
    "-archivedirectory=$ArchiveRoot",
    '-unattended',
    '-utf8output'
)
& $RunUAT @PackageArguments
if ($LASTEXITCODE -ne 0) {
    throw "Packaged game build failed."
}

$GameCommand = Get-ChildItem $ArchiveRoot -Filter "DirectiveUtilitiesRuntimeHost*.exe" -File -Recurse |
    Where-Object { $_.DirectoryName -like "*\Binaries\Win64" } |
    Select-Object -First 1
if (-not $GameCommand) {
    throw "Packaged game executable not found under $ArchiveRoot"
}

$GameLog = Join-Path $WorkRoot "GameTests.log"
$GameArguments = @(
    "-abslog=$GameLog",
    '-unattended',
    '-nop4',
    '-nosplash',
    '-nosound',
    '-NullRHI'
)
if ($ClientConfiguration -eq "Shipping") {
    New-Item $PerformanceRoot -ItemType Directory -Force | Out-Null
    $AppendOutput = Join-Path $PerformanceRoot "shipping-append-comparison.csv"
    $SmokeOutput = Join-Path $PerformanceRoot "shipping-smoke.csv"
    $Revision = ""
    if (Get-Command git -ErrorAction SilentlyContinue) {
        $Revision = (& git -C $RepositoryRoot rev-parse HEAD 2>$null)
        if ($Revision -and (& git -C $RepositoryRoot status --porcelain 2>$null)) {
            $Revision = "$Revision-dirty"
        }
    }
    $GameArguments += @(
        "-DirectiveUtilitiesAppendShippingBenchmarkOutput=$AppendOutput",
        "-DirectiveUtilitiesShippingSmokeOutput=$SmokeOutput",
        "-DirectiveUtilitiesPerfRevision=$Revision"
    )
} else {
    $GameArguments += @(
        '-ExecCmds=Automation RunTests DirectiveUtilities; Quit',
        '-TestExit=Automation Test Queue Empty'
    )
}
$GameCommandPath = $GameCommand.FullName
$GameArgumentLine = ($GameArguments | ForEach-Object { '"{0}"' -f $_ }) -join ' '
$GameProcess = Start-Process `
    -FilePath $GameCommandPath `
    -ArgumentList $GameArgumentLine `
    -WorkingDirectory $GameCommand.DirectoryName `
    -Wait `
    -PassThru
if ($GameProcess.ExitCode -ne 0) {
    if (Test-Path $GameLog -PathType Leaf) {
        Get-Content $GameLog -Tail 100
    }
    throw "Packaged game failed. Log: $GameLog"
}

$EditorReportPath = Join-Path $ReportRoot "Editor\index.json"
if (-not (Test-Path $EditorReportPath -PathType Leaf)) {
    throw "Editor automation report was not generated: $EditorReportPath"
}

$EditorReport = Get-Content $EditorReportPath -Raw | ConvertFrom-Json
if ($EditorReport.failed -ne 0 -or $EditorReport.succeededWithWarnings -ne 0 -or $EditorReport.notRun -ne 0) {
    throw "Editor automation tests failed or produced warnings. Report: $EditorReportPath"
}

if ($ClientConfiguration -eq "Shipping") {
    if (-not (Test-Path $AppendOutput -PathType Leaf)) {
        throw "Packaged Shipping append result was not generated: $AppendOutput"
    }
    if (-not (Select-String -Path $AppendOutput -Pattern '^#configuration,Shipping$' -Quiet)) {
        throw "Packaged benchmark did not report a Shipping configuration: $AppendOutput"
    }
    foreach ($ElementType in @("bool", "int32", "float", "FVector", "FString", "UObject")) {
        if ((Select-String -Path $AppendOutput -Pattern "^$ElementType,").Count -ne 10) {
            throw "Packaged benchmark did not report all $ElementType scenarios: $AppendOutput"
        }
    }
    if (-not (Test-Path $SmokeOutput -PathType Leaf)) {
        throw "Packaged Shipping smoke result was not generated: $SmokeOutput"
    }
    if (-not (Select-String -Path $SmokeOutput -Pattern '^#configuration,Shipping$' -Quiet)) {
        throw "Packaged smoke test did not report a Shipping configuration: $SmokeOutput"
    }
    if (Select-String -Path $SmokeOutput -Pattern ',false$' -Quiet) {
        throw "Packaged Shipping smoke test reported a failure: $SmokeOutput"
    }
    if ((Select-String -Path $SmokeOutput -Pattern ',true$').Count -ne 15) {
        throw "Packaged Shipping smoke test did not report every check: $SmokeOutput"
    }
} else {
    if (-not (Test-Path $GameLog -PathType Leaf)) {
        throw "Packaged game automation log was not generated: $GameLog"
    }
    if (-not (Select-String -Path $GameLog -Pattern 'TEST COMPLETE\. EXIT CODE: 0' -Quiet)) {
        throw "Packaged game automation did not finish cleanly. Log: $GameLog"
    }
}

Write-Host "Reports: $ReportRoot"
if ($ClientConfiguration -eq "Shipping") {
    Write-Host "Editor tests and packaged Shipping smoke and benchmark passed for $EngineVersion."
    Write-Host "Shipping performance results: $PerformanceRoot"
} else {
    Write-Host "Editor and packaged game tests passed for $EngineVersion."
    Write-Host "Packaged game log: $GameLog"
}
