# Building ZMatrix

The supported build is a 32-bit Unicode application. Use the solution's
`Debug|x86` or `Release|x86` configuration; individual C++ projects call this
platform `Win32`. A 32-bit application can run on 64-bit Windows. There is no
complete x64 application build.

## Requirements

- Visual Studio 2022 with Desktop development with C++, the MSVC v143 toolset,
  and a Windows 10 SDK.
- Inno Setup 6 for building the installer.
- Windows PowerShell 5.1, included with Windows, for distribution packaging.

All application modules, including `Config.dll`, build with Visual Studio.
Borland/C++Builder, VCL, MFC, Winamp, Perl/Filepp and HTML Help Workshop are not
required. The native DLL embeds the original About/Hire resources from `Config`.

## Compile the application

From an x86 Native Tools Command Prompt for VS 2022, run in the repository root:

```bat
msbuild matrix.sln /t:Build /p:Configuration=Debug /p:Platform=x86
msbuild matrix.sln /t:Build /p:Configuration=Release /p:Platform=x86
```

The solution builds `matrix.exe`, `Config.dll`, `zsMatrix.dll` and `MsgHook.dll`
in the repository root, plus `ScreenSaver\ZMatrixSS.scr`. The `zConfig` project
produces the application's `Config.dll`. Debug and Release replace the same
output files, so build them sequentially. For a clean rebuild use `/t:Rebuild`.

Compilation alone does not install the font, register the engine's COM server,
or configure the screensaver. Use the installer for a normal application test.
Standalone tests load the DLLs directly without registration.

Historical `Release Win9x` and `Template` configurations and Borland projects
are not supported build paths. The current installer allows Windows 7 and
later, but that setting does not prove runtime compatibility. Target Windows
versions, Explorer layouts, multiple monitors and mixed DPI need actual testing.

## Build the installer

From PowerShell in the repository root:

```powershell
.\scripts\Build-Distribution.ps1
```

The script discovers MSBuild and Inno Setup, rebuilds Release/x86, checks inputs,
stages `DistroNT`, and compiles `Setup\ZMatrix_payalord.iss`. It does not run
the installer or change version numbers. Distribution files are excluded from
Git. For an already complete, matching Release build:

```powershell
.\scripts\Build-Distribution.ps1 -SkipBuild
```

Do not use `-SkipBuild` after changing code or compiled resources until you
rebuild the solution. Custom tool locations can be supplied with `-MSBuildPath`
and `-InnoSetupPath`. Set the application version resources and installer
version explicitly before publishing. The installer is written to
`DistroNT\Output\ZMatrixSetupNT_<major>_<minor>_<release>.exe`, using the version
defined in `Setup\ZMatrix_payalord.iss`.

`CreateDistroNT.bat` invokes the normal PowerShell build.
`CompleteDistroNT.bat` invokes it with `-SkipBuild`. Other retained BAT files
are described in [docs/SCRIPTS.md](docs/SCRIPTS.md); do not use them for current
releases.

## Documentation

[docs/USER_GUIDE.md](docs/USER_GUIDE.md) is the current user manual. Update it
when a setting, menu command or supported behavior changes. The native help
window reads installed UTF-8 Markdown documents with no browser or CHM compiler.

Check documentation without rebuilding the application:

```powershell
.\scripts\Test-Documentation.ps1
```

Documentation-only edits do not require recompiling the binaries. Package them
with `-SkipBuild` when a matching Release build is available. The installer
copies the documents beside the application; an existing installation keeps
its previous copies until updated.

Keep documentation, comments and UI text in English. Preserve original author
credits and notices for bundled code and fonts. See
[docs/DEVELOPMENT.md](docs/DEVELOPMENT.md) for the module layout.

## Verification

From the x86 VS tools prompt in the repository root, for example:

```bat
mkdir tests\Debug
cl /nologo /std:c++17 /EHsc /W4 /DUNICODE /D_UNICODE tests\ConfigCompatibility.cpp /Fotests\Debug\ /Fetests\Debug\ConfigCompatibility.exe ole32.lib gdi32.lib
tests\Debug\ConfigCompatibility.exe
```

This checks the Unicode interface, Config exports/resources, legacy CFG
compatibility and save/load behavior with temporary files. `--engine-only`
deliberately skips Config checks and does not replace the full test.
Keep generated test files in ignored output folders or outside the repository.

See the test inventory in [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md) for coverage
and dependencies. Dialog tests open test-owned windows, and audio smoke tests
use playback capture. Do not overwrite an installed user's configuration.

Before release, verify Help and Readme from the application and installed
shortcuts, configuration OK/Cancel, audio device changes, startup, screensaver
behavior, installation and upgrades. Include monitor connect/disconnect, scaling
changes and Explorer restart. Isolated tests do not replace these system checks.
