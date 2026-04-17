param(
    [Parameter(Mandatory = $true)]
    [string]$VcpkgRoot
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path -LiteralPath $VcpkgRoot -PathType Container)) {
    Write-Error "Vcpkg root path does not exist: $VcpkgRoot"
    exit 1
}

$resolvedVcpkgRoot = (Resolve-Path -LiteralPath $VcpkgRoot).Path
$toolchainPath = Join-Path $resolvedVcpkgRoot "scripts/buildsystems/vcpkg.cmake"
if (-not (Test-Path -LiteralPath $toolchainPath -PathType Leaf)) {
    Write-Error "Invalid vcpkg root. Missing file: $toolchainPath"
    exit 1
}

$env:VCPKG_ROOT = $resolvedVcpkgRoot
Write-Host "VCPKG_ROOT set to: $env:VCPKG_ROOT"

Set-Location -LiteralPath $PSScriptRoot

Write-Host "Configuring with preset: windows-vcpkg"
cmake --preset windows-vcpkg
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

$buildTarget = "SDL3Defender"
Write-Host "Building target '$buildTarget' with preset: windows-debug"
cmake --build --preset windows-debug --target $buildTarget
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

$exePath = Join-Path $PSScriptRoot "build-win-vcpkg/bin/Debug/SDL3Defender.exe"
if (-not (Test-Path -LiteralPath $exePath -PathType Leaf)) {
    Write-Error "Debug executable not found: $exePath"
    exit 1
}

Write-Host "Launching: $exePath"
& $exePath
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}
