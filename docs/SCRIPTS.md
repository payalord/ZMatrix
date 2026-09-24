# Build script inventory

Current distribution uses `scripts/Build-Distribution.ps1` and
`Setup/ZMatrix_payalord.iss`. Visual Studio builds `matrix.sln` without calling
the old distribution BAT files.

## Active entry points

| File | Purpose |
| --- | --- |
| `scripts/Build-Distribution.ps1` | Rebuild Release/x86, check inputs, stage files and compile the installer. `-SkipBuild` packages existing binaries. |
| `CreateDistroNT.bat` | Convenience wrapper for the normal PowerShell build. |
| `CompleteDistroNT.bat` | Convenience wrapper for the same script with `-SkipBuild`. |
| `scripts/Test-Documentation.ps1` | Checks the required offline documentation and local Markdown links; called before packaging. |

The wrappers contain no independent packaging logic. PowerShell can be called
directly; the two BAT files are optional conveniences.

## Historical scripts retained for review

These are not supported release commands. Their dependencies and old source
layouts have not been restored.

| File | Original purpose and current limitation |
| --- | --- |
| `CreateDistro.bat` | Runs unsupported Win9x packaging before NT packaging. Not an alias for the current build. |
| `CreateDistro9x.bat` | Uses Visual C++ 6.0, Borland, old HTML help and Inno Setup Extensions. |
| `CreateDistroSRC.bat` | Hand-maintained source ZIP; omits current modules/projects and references missing files and the removed website/manual. Can rewrite versions when passed an argument. Use the Git repository for current sources. |
| `scripts/ReVersionAll.bat` | Runs Perl over old Borland/native resources and `Setup/ZMatrix.iss`, not the actual `ZMatrix_payalord.iss`. |
| `Help/help.zmatrix.n3.net/src/make.bat` | Old Filepp/Perl help preprocessing; its input tree has been removed. |
| `Web/zmatrix.n3.net/src/make.bat` | Old website assembly; its input tree has been removed. |
| `Web/zmatrix.n3.net/scripts/ZipRelease.bat` | Old website ZIP packaging. |
| `Web/zmatrix.n3.net/scripts/ZipReleaseAndUploadToSourceForge.bat` | Old ZIP/SCP upload to the original author's SourceForge account. Not deployment for this fork's homepage. |

All historical BAT files, including those in the former website directories,
are intentionally retained for a separate decision. They do not make the old
website or HTML-help sources a dependency of the current application.

`Setup/ZMatrix.iss` and `Setup/ZMatrix.ise` are historical installer definitions.
Use the active PowerShell script and `Setup/ZMatrix_payalord.iss` for releases.
Update versions explicitly in the active resources and installer rather than
running the historical version rewriter.
