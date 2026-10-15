# Builds and tests the pure C++ graphics algorithms. No graphics library is used.
param([string]$Compiler = '')
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot

if (-not $Compiler) {
    # Prefer MSYS2 MinGW-w64; a Cygwin g++ on PATH produces Cygwin-dependent programs.
    if (Test-Path 'C:/msys64/ucrt64/bin/g++.exe') { $Compiler = 'C:/msys64/ucrt64/bin/g++.exe' }
    else {
        $found = Get-Command g++.exe -ErrorAction SilentlyContinue
        if ($found) { $Compiler = $found.Source }
        else { throw 'Install an x64 MinGW-w64 C++ compiler, or pass -Compiler C:/path/to/g++.exe.' }
    }
}
$env:PATH = "$(Split-Path -Parent $Compiler);$env:PATH"

$buildDir = Join-Path $projectRoot 'build'
New-Item -ItemType Directory -Force -Path $buildDir | Out-Null

function Invoke-Checked([string]$Program, [string[]]$Arguments) {
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Program failed with exit code $LASTEXITCODE" }
}

$common = @('-std=c++17', '-O2', '-Wall', '-Wextra', '-Wpedantic', '-static', "-I$projectRoot/include")

Write-Output "Compiler: $Compiler"
Write-Output 'Building graphics tests'
Invoke-Checked $Compiler ($common + @("$projectRoot/src/graphics.cpp", "$projectRoot/tests/graphics_test.cpp", '-o', "$buildDir/graphics_test.exe"))
Invoke-Checked "$buildDir/graphics_test.exe" @()

Write-Output 'Building renderer'
Invoke-Checked $Compiler ($common + @("$projectRoot/src/graphics.cpp", "$projectRoot/src/canvas.cpp", "$projectRoot/src/render.cpp", '-o', "$buildDir/render.exe"))
Invoke-Checked "$buildDir/render.exe" @("$projectRoot/output")
Write-Output 'Build complete.'
