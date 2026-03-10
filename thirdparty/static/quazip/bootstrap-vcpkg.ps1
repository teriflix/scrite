param(
    [string]$RepoUrl = "https://github.com/microsoft/vcpkg.git",
    [string]$Baseline = "1e199d32ad53aab1defda61ce41c380302e3f95c",
    [string]$Triplet = "x64-windows"
)

$ErrorActionPreference = "Stop"

Write-Host "Scrite QuaZip vcpkg bootstrap (Windows-only helper)"
if ($env:OS -ne "Windows_NT") {
    throw "This helper is Windows-only. On macOS/Linux, do not run this script; use the platform's normal dependency setup."
}

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$vcpkgDir = Join-Path $scriptDir ".vcpkg"
$bootstrapScript = Join-Path $vcpkgDir "bootstrap-vcpkg.bat"
$installRoot = Join-Path $scriptDir "vcpkg_installed"

if (-not (Get-Command git -ErrorAction SilentlyContinue)) {
    throw "git is required but was not found in PATH."
}

if (-not (Test-Path $vcpkgDir)) {
    Write-Host "Cloning vcpkg into $vcpkgDir"
    git clone $RepoUrl $vcpkgDir
} else {
    Write-Host "Using existing vcpkg checkout at $vcpkgDir"
}

Push-Location $vcpkgDir
try {
    git fetch --all --tags --prune
    git checkout $Baseline
} finally {
    Pop-Location
}

if (-not (Test-Path $bootstrapScript)) {
    throw "bootstrap-vcpkg.bat not found under $vcpkgDir"
}

Write-Host "Bootstrapping vcpkg executable"
& $bootstrapScript
if ($LASTEXITCODE -ne 0) {
    throw "bootstrap-vcpkg.bat failed with exit code $LASTEXITCODE"
}

$vcpkgExe = Join-Path $vcpkgDir "vcpkg.exe"
if (-not (Test-Path $vcpkgExe)) {
    throw "vcpkg.exe not found under $vcpkgDir"
}

Write-Host "Installing manifest dependencies into $installRoot ($Triplet)"
& $vcpkgExe install --triplet $Triplet --x-manifest-root $scriptDir --x-install-root $installRoot --disable-metrics
if ($LASTEXITCODE -ne 0) {
    throw "vcpkg install failed with exit code $LASTEXITCODE"
}

Write-Host ""
Write-Host "QuaZip local vcpkg setup complete (Windows)."
Write-Host "Toolchain: $vcpkgDir\scripts\buildsystems\vcpkg.cmake"
Write-Host "Manifest:  $scriptDir\vcpkg.json"
Write-Host "Packages:  $installRoot\$Triplet"
