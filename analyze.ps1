param(
    [switch]$Clean
)

$ErrorActionPreference = "Stop"

Set-Location -LiteralPath $PSScriptRoot

$analyzeBuildDir = Join-Path $PSScriptRoot "build-win-vcpkg-analyze"
if ($Clean) {
    if (Test-Path -LiteralPath $analyzeBuildDir -PathType Container) {
        Write-Host "Removing analyze build directory: $analyzeBuildDir"
        Remove-Item -Recurse -Force -LiteralPath $analyzeBuildDir
    } else {
        Write-Host "Analyze build directory does not exist: $analyzeBuildDir"
    }
}

Write-Host "Configuring with preset: windows-vcpkg-analyze"
cmake --preset windows-vcpkg-analyze
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

$buildTarget = "SDL3Defender"
Write-Host "Building target '$buildTarget' with preset: windows-debug-analyze"
cmake --build --preset windows-debug-analyze --target $buildTarget
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}
