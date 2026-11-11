param()
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$output = Join-Path (Split-Path -Parent $projectRoot) 'SmartFlow-Cpp-Project.zip'
if (-not (Test-Path -LiteralPath "$projectRoot/bin/SmartFlow.exe")) { throw 'Build SmartFlow first.' }
Add-Type -AssemblyName System.IO.Compression
Add-Type -AssemblyName System.IO.Compression.FileSystem
$files = @()
foreach ($name in @('README.md','THIRD_PARTY.md','.gitignore','.gitattributes','Build SmartFlow.cmd','Open SmartFlow.cmd')) {
    $files += Get-Item -LiteralPath (Join-Path $projectRoot $name)
}
foreach ($directory in @('src','include','tests','scripts','docs','vendor','bin')) {
    $files += Get-ChildItem -LiteralPath (Join-Path $projectRoot $directory) -Recurse -File
}
$stream = [System.IO.File]::Open($output,[System.IO.FileMode]::Create)
$archive = New-Object System.IO.Compression.ZipArchive($stream,[System.IO.Compression.ZipArchiveMode]::Create)
try {
    foreach ($file in $files) {
        $relative = $file.FullName.Substring($projectRoot.Length + 1).Replace('\','/')
        [System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($archive,$file.FullName,"traffic-simulation-cpp/$relative",[System.IO.Compression.CompressionLevel]::Optimal) | Out-Null
    }
} finally {
    $archive.Dispose()
    $stream.Dispose()
}
Write-Output "Created $output with $($files.Count) files. Extract the ZIP before launching."
