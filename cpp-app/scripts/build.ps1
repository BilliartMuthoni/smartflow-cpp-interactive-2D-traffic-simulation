param([string]$Compiler = '', [switch]$RebuildLibrary, [switch]$TestsOnly)
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
if (-not $Compiler) {
    $foundCompiler = Get-Command g++.exe -ErrorAction SilentlyContinue
    if ($foundCompiler) { $Compiler = $foundCompiler.Source }
    elseif (Test-Path 'C:/msys64/ucrt64/bin/g++.exe') { $Compiler = 'C:/msys64/ucrt64/bin/g++.exe' }
    else { throw 'Install an x64 MinGW-w64 C++ compiler, or pass -Compiler C:/path/to/g++.exe.' }
}
$compilerDir = Split-Path -Parent $Compiler
$cCompiler = Join-Path $compilerDir 'gcc.exe'
$archiver = Join-Path $compilerDir 'ar.exe'
$env:PATH = "$compilerDir;$env:PATH"
$buildDir = Join-Path $projectRoot 'build'
$binDir = Join-Path $projectRoot 'bin'
$objectDir = Join-Path $buildDir 'raylib-objects'
$raySrc = Join-Path $projectRoot 'vendor/raylib-5.5/src'
New-Item -ItemType Directory -Force -Path $buildDir,$binDir,$objectDir | Out-Null
function Invoke-Checked([string]$Program, [string[]]$Arguments) {
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Program failed with exit code $LASTEXITCODE" }
}
$common = @('-std=c++17','-O2','-Wall','-Wextra','-Wpedantic','-static','-static-libgcc','-static-libstdc++',"-I$projectRoot/include")
foreach ($name in @('simulation','graphics')) {
    Write-Output "Building $name tests"
    Invoke-Checked $Compiler ($common + @("$projectRoot/src/$name.cpp", "$projectRoot/tests/${name}_test.cpp",'-o',"$buildDir/${name}_test.exe"))
    Invoke-Checked "$buildDir/${name}_test.exe" @()
}
if ($TestsOnly) { exit 0 }
$library = Join-Path $buildDir 'libraylib.a'
if ($RebuildLibrary -or -not (Test-Path $library)) {
    $objects = @()
    foreach ($module in @('rcore','rshapes','rtextures','rtext','rmodels','raudio','utils','rglfw')) {
        Write-Output "Compiling raylib $module"
        $object = Join-Path $objectDir "$module.o"
        Invoke-Checked $cCompiler @('-std=gnu99','-O2','-w','-DPLATFORM_DESKTOP_GLFW','-DGRAPHICS_API_OPENGL_33',"-I$raySrc","-I$raySrc/external/glfw/include",'-c',"$raySrc/$module.c",'-o',$object)
        $objects += $object
    }
    Invoke-Checked $archiver (@('rcs',$library) + $objects)
}
Write-Output 'Building SmartFlow C++'
$sources = @('main','simulation','graphics','scene') | ForEach-Object { "$projectRoot/src/$_.cpp" }
Invoke-Checked $Compiler ($common + @("-I$raySrc") + $sources + @($library,'-lopengl32','-lgdi32','-lwinmm','-mwindows','-o',"$binDir/SmartFlow.exe"))
Copy-Item -LiteralPath "$projectRoot/vendor/raylib-5.5/LICENSE" -Destination "$binDir/raylib-LICENSE.txt"
Write-Output "Ready: $binDir/SmartFlow.exe"
