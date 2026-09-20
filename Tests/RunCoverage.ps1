param(
    [string]$BuildDirectory = "build/coverage-gcc",
    [string]$OutputFile = "coverage/cobertura.xml"
)

$ErrorActionPreference = "Stop"
$projectRoot = Split-Path -Parent $PSScriptRoot
$gcov = Get-Command gcov.exe -ErrorAction SilentlyContinue
$python = Get-Command py.exe -ErrorAction SilentlyContinue
$msysBash = "C:\msys64\usr\bin\bash.exe"

if ($null -eq $gcov)
{
    throw "gcov.exe was not found. Install MSYS2's UCRT64 GCC toolchain first."
}

if ($null -eq $python)
{
    throw "The Python launcher was not found. Install Python, then run: py -m pip install --user gcovr"
}

if (-not (Test-Path $msysBash))
{
    throw "MSYS2 bash was not found at $msysBash. Install MSYS2 UCRT64 to use this coverage configuration."
}

$buildPath = Join-Path $projectRoot $BuildDirectory
$testExecutable = Join-Path $buildPath "ECSEngineUnitTests.exe"
$coverageFile = Join-Path $projectRoot $OutputFile
$htmlDirectory = Join-Path $projectRoot "coverage/html"
$htmlReport = Join-Path $htmlDirectory "index.html"
$gcovrRoot = $projectRoot.Replace("\", "/")
$gcovrBuildPath = $buildPath.Replace("\", "/")
$gcovrCoverageFile = $coverageFile.Replace("\", "/")
$gcovrHtmlReport = $htmlReport.Replace("\", "/")

if (-not (Test-Path $testExecutable))
{
    throw "Test executable not found: $testExecutable. Run 'cmake --preset coverage' and 'cmake --build --preset coverage' first."
}

New-Item -ItemType Directory -Force -Path (Split-Path -Parent $coverageFile) | Out-Null
New-Item -ItemType Directory -Force -Path $htmlDirectory | Out-Null

& $testExecutable

if ($LASTEXITCODE -ne 0)
{
    exit $LASTEXITCODE
}

$gcovrCommand = @"
export PATH=/ucrt64/bin:`$PATH
/c/Windows/py.exe -m gcovr --root '$gcovrRoot' --object-directory '$gcovrBuildPath' --gcov-executable gcov --gcov-exclude-directory '.*/CMakeFiles/[0-9.]+-msvc[0-9]+/CompilerId.*' --filter '$gcovrRoot/Base' --filter '$gcovrRoot/Managers' --filter '$gcovrRoot/Systems' --filter '$gcovrRoot/DataClasses' --filter '$gcovrRoot/Tools' --filter '$gcovrRoot/HotReloading' --filter '$gcovrRoot/Collision' --filter '$gcovrRoot/Components' --filter '$gcovrRoot/ProjectExplorer' --filter '$gcovrRoot/Serialization' --filter '$gcovrRoot/Dialogs' --filter '$gcovrRoot/UI' --filter '$gcovrRoot/Engine.cpp' --filter '$gcovrRoot/imguiHandler.cpp' --xml-pretty --output '$gcovrCoverageFile' --html-details '$gcovrHtmlReport'
"@

& $msysBash -lc $gcovrCommand

if ($LASTEXITCODE -ne 0)
{
    exit $LASTEXITCODE
}

Write-Host "Coverage reports written to $coverageFile and $htmlDirectory/index.html"