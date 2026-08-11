param(
    [Parameter(Mandatory = $true)]
    [string[]]$EngineRoots,

    [switch]$IncludeLinux,

    [string]$LinuxToolchainBase = "C:\UnrealToolchains",

    [string]$LinuxPackageBase = ""
)

$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

if ($EngineRoots.Count -ne 3) {
    throw "Provide the UE 5.6, 5.7, and 5.8 engine roots in that order."
}

$ExpectedVersions = @("UE_5.6", "UE_5.7", "UE_5.8")
for ($Index = 0; $Index -lt $ExpectedVersions.Count; $Index++) {
    if ((Split-Path $EngineRoots[$Index] -Leaf) -ne $ExpectedVersions[$Index]) {
        throw "Expected $($ExpectedVersions[$Index]) at argument $($Index + 1): $($EngineRoots[$Index])"
    }
}

$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
Push-Location $RepositoryRoot
try {
    py -3 -m unittest Tests\Packaging\test_package_fab.py Tests\Release\test_check_release.py
    if ($LASTEXITCODE -ne 0) { throw "Python tests failed." }

    py -3 Tools\Release\check_release.py
    if ($LASTEXITCODE -ne 0) { throw "Release identity check failed." }

    py -3 Tools\Packaging\package_fab.py --check
    if ($LASTEXITCODE -ne 0) { throw "Fab source validation failed." }

    git diff --check
    if ($LASTEXITCODE -ne 0) { throw "Git whitespace validation failed." }

    $ReleaseFiles = & git ls-files -co --exclude-standard
    if ($LASTEXITCODE -ne 0) { throw "Release file discovery failed." }
    $ReleaseFiles = $ReleaseFiles | Where-Object {
        $_ -notlike "Build/*" -and
        $_ -notlike "Binaries/*" -and
        $_ -notlike "Intermediate/*" -and
        $_ -notlike "Tools/Release/run-local-release-gate.*"
    }
    $Markers = Select-String -LiteralPath $ReleaseFiles -Pattern "co-authored-by:|generated (with|by)|chatgpt|openai|claude|copilot|delve|seamless|robust|leverage|comprehensive|streamline|\u2014|\u2013"
    if ($Markers) {
        $Markers
        throw "Release text contains an attribution or generated-content marker."
    }

    foreach ($EngineRoot in $EngineRoots) {
        & Tests\RuntimeHost\Scripts\run-windows.ps1 -EngineRoot $EngineRoot -ClientConfiguration Development
    }

    if ($IncludeLinux) {
        if (-not $LinuxPackageBase) {
            $LinuxPackageBase = Join-Path ([System.IO.Path]::GetPathRoot($RepositoryRoot)) "DULinux"
        }
        $LinuxToolchainVersions = @(
            "v25_clang-18.1.0-rockylinux8",
            "v26_clang-20.1.8-rockylinux8",
            "v26_clang-20.1.8-rockylinux8"
        )
        $PreviousLinuxToolchainRoot = $env:LINUX_MULTIARCH_ROOT
        try {
            for ($Index = 0; $Index -lt $EngineRoots.Count; $Index++) {
                $LinuxToolchainRoot = Join-Path $LinuxToolchainBase $LinuxToolchainVersions[$Index]
                if (-not (Test-Path $LinuxToolchainRoot -PathType Container)) {
                    throw "Linux toolchain not found: $LinuxToolchainRoot"
                }

                $env:LINUX_MULTIARCH_ROOT = "$LinuxToolchainRoot\"
                $RunUAT = Join-Path $EngineRoots[$Index] "Engine\Build\BatchFiles\RunUAT.bat"
                $LinuxPackage = Join-Path $LinuxPackageBase $ExpectedVersions[$Index]
                Remove-Item $LinuxPackage -Recurse -Force -ErrorAction SilentlyContinue
                & $RunUAT BuildPlugin `
                    "-Plugin=$(Join-Path $RepositoryRoot 'DirectiveUtilities.uplugin')" `
                    "-Package=$LinuxPackage" `
                    "-TargetPlatforms=Linux" `
                    -Rocket
                if ($LASTEXITCODE -ne 0) {
                    throw "Linux plugin build failed for $($ExpectedVersions[$Index])."
                }
            }
        }
        finally {
            $env:LINUX_MULTIARCH_ROOT = $PreviousLinuxToolchainRoot
        }
    }

    $PerformanceRoot = Join-Path $RepositoryRoot "Build\Performance\ReleaseGate"
    $PerformanceProject = Join-Path $RepositoryRoot "Build\RuntimeHost\UE_5.8\Project\DirectiveUtilitiesRuntimeHost.uproject"
    $PerformanceWarmup = Join-Path $PerformanceRoot "warmup.csv"
    $PerformanceBaseline = Join-Path $PerformanceRoot "baseline.csv"
    $PerformanceCandidate = Join-Path $PerformanceRoot "candidate.csv"
    & Tests\Performance\run-runtime-benchmarks.ps1 `
        -EngineRoot $EngineRoots[2] -ProjectFile $PerformanceProject -OutputFile $PerformanceWarmup
    & Tests\Performance\run-runtime-benchmarks.ps1 `
        -EngineRoot $EngineRoots[2] -ProjectFile $PerformanceProject -OutputFile $PerformanceBaseline
    & Tests\Performance\run-runtime-benchmarks.ps1 `
        -EngineRoot $EngineRoots[2] -ProjectFile $PerformanceProject -OutputFile $PerformanceCandidate `
        -BaselineFile $PerformanceBaseline

    & Tests\RuntimeHost\Scripts\run-windows.ps1 -EngineRoot $EngineRoots[2] -ClientConfiguration Shipping
    Write-Host "Local release gate passed."
}
finally {
    Pop-Location
}
