param(
    [Parameter(Mandatory = $true)]
    [string]$EngineRoot,

    [Parameter(Mandatory = $true)]
    [string]$ProjectFile,

    [Parameter(Mandatory = $true)]
    [string]$OutputFile,

    [string]$BaselineFile = "",

    [long]$ProcessorAffinity = 0,

    [ValidateRange(1, 86400)]
    [int]$TimeoutSeconds = 1800
)

$ErrorActionPreference = "Stop"

$EditorCommand = Join-Path $EngineRoot "Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
if (-not (Test-Path $EditorCommand -PathType Leaf)) {
    throw "Unreal Editor command not found: $EditorCommand"
}
if (-not (Test-Path $ProjectFile -PathType Leaf)) {
    throw "Project file not found: $ProjectFile"
}
if ($BaselineFile -and -not (Test-Path $BaselineFile -PathType Leaf)) {
    throw "Baseline file not found: $BaselineFile"
}
if ($ProcessorAffinity -lt 0) {
    throw "Processor affinity must be zero or a positive bitmask."
}
if ($ProcessorAffinity -eq 0) {
    $AffinityProcessorCount = [Math]::Min([Environment]::ProcessorCount, 16)
    $ProcessorAffinity = ([long]1 -shl $AffinityProcessorCount) - 1
}

$ProjectFile = [System.IO.Path]::GetFullPath($ProjectFile)
$OutputFile = [System.IO.Path]::GetFullPath($OutputFile)
if ($BaselineFile) {
    $BaselineFile = [System.IO.Path]::GetFullPath($BaselineFile)
}
$OutputDirectory = Split-Path $OutputFile -Parent
if ($OutputDirectory) {
    New-Item $OutputDirectory -ItemType Directory -Force | Out-Null
}
$LogFile = [System.IO.Path]::ChangeExtension($OutputFile, ".log")
$ReportDirectory = Join-Path $OutputDirectory "$([System.IO.Path]::GetFileNameWithoutExtension($OutputFile))-report"
if (Test-Path $ReportDirectory) {
    Remove-Item $ReportDirectory -Recurse -Force
}
New-Item $ReportDirectory -ItemType Directory -Force | Out-Null
$ComparisonFileName = "$([System.IO.Path]::GetFileNameWithoutExtension($OutputFile))-remove-all-comparison.csv"
$ComparisonFile = if ($OutputDirectory) {
    Join-Path $OutputDirectory $ComparisonFileName
} else {
    Join-Path (Get-Location) $ComparisonFileName
}
$AppendComparisonFileName = "$([System.IO.Path]::GetFileNameWithoutExtension($OutputFile))-append-comparison.csv"
$AppendComparisonFile = if ($OutputDirectory) {
    Join-Path $OutputDirectory $AppendComparisonFileName
} else {
    Join-Path (Get-Location) $AppendComparisonFileName
}
$InsertComparisonFileName = "$([System.IO.Path]::GetFileNameWithoutExtension($OutputFile))-insert-comparison.csv"
$InsertComparisonFile = if ($OutputDirectory) {
    Join-Path $OutputDirectory $InsertComparisonFileName
} else {
    Join-Path (Get-Location) $InsertComparisonFileName
}
$RemoveIndicesComparisonFileName = "$([System.IO.Path]::GetFileNameWithoutExtension($OutputFile))-remove-indices-comparison.csv"
$RemoveIndicesComparisonFile = if ($OutputDirectory) {
    Join-Path $OutputDirectory $RemoveIndicesComparisonFileName
} else {
    Join-Path (Get-Location) $RemoveIndicesComparisonFileName
}
$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot "../..")).Path
$Revision = ""
if (Get-Command git -ErrorAction SilentlyContinue) {
    $Revision = (& git -C $RepositoryRoot rev-parse HEAD 2>$null)
    if ($Revision -and (& git -C $RepositoryRoot status --porcelain 2>$null)) {
        $Revision = "$Revision-dirty"
    }
}
$Arguments = @(
    $ProjectFile,
    '-ExecCmds=Automation RunTests Performance.DirectiveUtilities.Runtime; Quit',
    '-TestExit=Automation Test Queue Empty',
    "-ReportExportPath=$ReportDirectory",
    "-DirectiveUtilitiesPerfOutput=$OutputFile",
    "-DirectiveUtilitiesPerfComparisonOutput=$ComparisonFile",
    "-DirectiveUtilitiesPerfAppendComparisonOutput=$AppendComparisonFile",
    "-DirectiveUtilitiesPerfRevision=$Revision",
    "-abslog=$LogFile",
    '-unattended',
    '-nop4',
    '-nosplash',
    '-nosound',
    '-NullRHI',
    '-NoEngineAnalytics',
    '-NoEpicPortal',
    '-ini:EditorSettings:[/Script/UnrealEd.AnalyticsPrivacySettings]:bSendUsageData=False'
)
if ($BaselineFile) {
    $Arguments += "-DirectiveUtilitiesPerfBaseline=$BaselineFile"
}

$ArgumentString = ($Arguments | ForEach-Object {
    if ($_ -match '[\s"]') {
        '"' + ($_ -replace '"', '\"') + '"'
    } else {
        $_
    }
}) -join ' '
$EditorProcess = Start-Process -FilePath $EditorCommand -ArgumentList $ArgumentString -PassThru
try {
    $EditorProcess.ProcessorAffinity = [IntPtr]$ProcessorAffinity
    $EditorProcess.PriorityClass = [System.Diagnostics.ProcessPriorityClass]::High
    if (-not $EditorProcess.WaitForExit($TimeoutSeconds * 1000)) {
        $EditorProcess.Kill()
        $EditorProcess.WaitForExit()
        throw "Runtime performance suite timed out after $TimeoutSeconds seconds. Log: $LogFile"
    }
    $EditorProcess.WaitForExit()
    $EditorExitCode = $EditorProcess.ExitCode
} finally {
    if (-not $EditorProcess.HasExited) {
        $EditorProcess.Kill()
    }
}
if ($EditorExitCode -ne 0) {
    throw "Runtime performance suite failed. Log: $LogFile"
}
$ReportPath = Join-Path $ReportDirectory 'index.json'
if (-not (Test-Path $ReportPath -PathType Leaf)) {
    throw "Runtime performance automation report was not generated: $ReportPath"
}
$Report = ((Get-Content $ReportPath -Raw) -replace "^\uFEFF", "") | ConvertFrom-Json
if ($Report.succeeded -ne 1 -or $Report.failed -ne 0 -or $Report.succeededWithWarnings -ne 0 -or
    $Report.notRun -ne 0 -or $Report.tests.Count -ne 1 -or
    $Report.tests[0].fullTestPath -ne 'Performance.DirectiveUtilities.Runtime' -or
    $Report.tests[0].state -ne 'Success') {
    throw "Runtime performance report is not the exact clean one-test census: $ReportPath"
}
if (-not (Test-Path $OutputFile -PathType Leaf)) {
    throw "Runtime performance results were not generated: $OutputFile"
}
if (-not (Test-Path $ComparisonFile -PathType Leaf)) {
    throw "Remove All comparison results were not generated: $ComparisonFile"
}
if (-not (Test-Path $AppendComparisonFile -PathType Leaf)) {
    throw "Append comparison results were not generated: $AppendComparisonFile"
}
if (-not (Test-Path $InsertComparisonFile -PathType Leaf)) {
    throw "Insert comparison results were not generated: $InsertComparisonFile"
}
if (-not (Test-Path $RemoveIndicesComparisonFile -PathType Leaf)) {
    throw "Remove At Indices comparison results were not generated: $RemoveIndicesComparisonFile"
}
if (-not (Select-String -Path $LogFile -Pattern 'Test Completed\. Result=\{Success\} Name=\{Runtime\} Path=\{Performance\.DirectiveUtilities\.Runtime\}' -Quiet)) {
    throw "Runtime performance suite did not complete successfully. Log: $LogFile"
}

Write-Host "Runtime performance results: $OutputFile"
Write-Host "Remove All comparison results: $ComparisonFile"
Write-Host "Append comparison results: $AppendComparisonFile"
Write-Host "Insert comparison results: $InsertComparisonFile"
Write-Host "Remove At Indices comparison results: $RemoveIndicesComparisonFile"
Write-Host "Automation log: $LogFile"
