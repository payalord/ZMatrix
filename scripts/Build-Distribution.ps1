[CmdletBinding()]
param(
    [switch]$SkipBuild,
    [string]$MSBuildPath,
    [string]$InnoSetupPath
)

$ErrorActionPreference = 'Stop'
$repoRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path

function Find-Tool([string]$ExplicitPath, [string]$Name) {
    if ($ExplicitPath) {
        return (Get-Item -LiteralPath $ExplicitPath -ErrorAction Stop).FullName
    }
    $command = Get-Command $Name -ErrorAction SilentlyContinue
    if ($command) { return $command.Source }
    throw "$Name was not found. See BUILDING.md for the required build environment."
}

if (-not $MSBuildPath) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (Test-Path -LiteralPath $vswhere) {
        $MSBuildPath = & $vswhere -latest -products '*' -requires Microsoft.Component.MSBuild -find 'MSBuild\Current\Bin\MSBuild.exe' | Select-Object -First 1
    }
}
if (-not $InnoSetupPath) {
    $candidate = Join-Path ${env:ProgramFiles(x86)} 'Inno Setup 6\ISCC.exe'
    if (Test-Path -LiteralPath $candidate) { $InnoSetupPath = $candidate }
}
$innoSetup = Find-Tool $InnoSetupPath 'ISCC.exe'

Push-Location -LiteralPath $repoRoot
try {
    # These separately built components are required by the existing installer.
    # Check them before building or replacing any distribution files.
    foreach ($required in @('ZMatrixHelp.chm', 'WinampVis\vis_zmx.dll')) {
        if (-not (Test-Path -LiteralPath $required -PathType Leaf)) {
            throw "Missing $required. Build it separately as described in BUILDING.md."
        }
    }

    if (-not $SkipBuild) {
        $msbuild = Find-Tool $MSBuildPath 'MSBuild.exe'
        if (-not $env:BDS -or -not (Test-Path -LiteralPath (Join-Path $env:BDS 'Bin\CodeGear.Cpp.Targets'))) {
            throw 'C++Builder is required. Run its rsvars.bat before starting this script.'
        }
        & $msbuild matrix.sln /t:Rebuild /p:Configuration=Release /p:Platform=x86 /nologo
        if ($LASTEXITCODE -ne 0) { throw "Visual Studio build failed ($LASTEXITCODE)." }
        & $msbuild Config\ConfigModern.cbproj /t:Rebuild /p:Config=Release /p:Platform=Win32 /nologo
        if ($LASTEXITCODE -ne 0) { throw "C++Builder build failed ($LASTEXITCODE)." }
    }

    $files = [ordered]@{
        'default.cfg' = 'default.cfg'
        'matrix.exe' = 'matrix.exe'
        'zsMatrix.dll' = 'zsMatrix.dll'
        'Config.dll' = 'Config.dll'
        'MsgHook.dll' = 'MsgHook.dll'
        'README.md' = 'README.md'
        'LICENSE.TXT' = 'LICENSE.TXT'
        'ORIGINALREADME.md' = 'ORIGINALREADME.md'
        'JapaneseSet.txt' = 'JapaneseSet.txt'
        'MatrixCodeFontSet.txt' = 'MatrixCodeFontSet.txt'
        'Matrix Code Font.ttf' = 'Matrix Code Font.ttf'
        'ZMatrixHelp.chm' = 'ZMatrixHelp.chm'
        'WinampVis\vis_zmx.dll' = 'vis_zmx.dll'
        'ScreenSaver\ZMatrixSS.scr' = 'ScreenSaver\ZMatrixSS.scr'
    }
    foreach ($source in $files.Keys) {
        if (-not (Test-Path -LiteralPath $source -PathType Leaf)) {
            throw "Missing distribution input: $source"
        }
    }

    $stagePath = [IO.Path]::GetFullPath((Join-Path $repoRoot 'DistroNT'))
    if ([IO.Path]::GetDirectoryName($stagePath) -ne $repoRoot) {
        throw 'The distribution directory must be directly inside the repository.'
    }
    if (Test-Path -LiteralPath $stagePath) {
        $stageItems = @(Get-Item -LiteralPath $stagePath) + @(Get-ChildItem -LiteralPath $stagePath -Force -Recurse)
        if ($stageItems | Where-Object { $_.Attributes -band [IO.FileAttributes]::ReparsePoint }) {
            throw 'Refusing to replace a distribution directory containing links or junctions.'
        }
        Remove-Item -LiteralPath $stagePath -Recurse -Force
    }
    New-Item -ItemType Directory -Path (Join-Path $stagePath 'ScreenSaver') -Force | Out-Null
    foreach ($source in $files.Keys) {
        Copy-Item -LiteralPath $source -Destination (Join-Path $stagePath $files[$source])
    }
    & $innoSetup 'Setup\ZMatrix_payalord.iss'
    if ($LASTEXITCODE -ne 0) { throw "Installer compilation failed ($LASTEXITCODE)." }
}
finally {
    Pop-Location
}
