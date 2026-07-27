param(
    [Parameter(Mandatory = $true)]
    [string]$EngineRoot,

    [Parameter(Mandatory = $true)]
    [string]$ProjectFile,

    [Parameter(Mandatory = $true)]
    [string]$OutputFile,

    [string]$BaselineFile = ""
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
$ComparisonFileName = "$([System.IO.Path]::GetFileNameWithoutExtension($OutputFile))-remove-all-comparison.csv"
$ComparisonFile = if ($OutputDirectory) {
    Join-Path $OutputDirectory $ComparisonFileName
} else {
    Join-Path (Get-Location) $ComparisonFileName
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
    "-DirectiveUtilitiesPerfOutput=$OutputFile",
    "-DirectiveUtilitiesPerfComparisonOutput=$ComparisonFile",
    "-DirectiveUtilitiesPerfRevision=$Revision",
    "-abslog=$LogFile",
    '-unattended',
    '-nop4',
    '-nosplash',
    '-nosound',
    '-NullRHI'
)
if ($BaselineFile) {
    $Arguments += "-DirectiveUtilitiesPerfBaseline=$BaselineFile"
}

& $EditorCommand $Arguments
if ($LASTEXITCODE -ne 0) {
    throw "Runtime performance suite failed. Log: $LogFile"
}
if (-not (Test-Path $OutputFile -PathType Leaf)) {
    throw "Runtime performance results were not generated: $OutputFile"
}
if (-not (Test-Path $ComparisonFile -PathType Leaf)) {
    throw "Remove All comparison results were not generated: $ComparisonFile"
}
if (-not (Select-String -Path $LogFile -Pattern 'Test Completed\. Result=\{Success\} Name=\{Runtime\} Path=\{Performance\.DirectiveUtilities\.Runtime\}' -Quiet)) {
    throw "Runtime performance suite did not complete successfully. Log: $LogFile"
}

Write-Host "Runtime performance results: $OutputFile"
Write-Host "Remove All comparison results: $ComparisonFile"
Write-Host "Automation log: $LogFile"
