# Build script inventory

Current distribution uses `scripts/Build-Distribution.ps1` and
`Setup/ZMatrix_payalord.iss`. Visual Studio builds `matrix.sln` without calling
the old distribution BAT files.

## Active entry points

- `scripts/Build-Distribution.ps1`: rebuild Release/x86, check inputs, stage
  files and compile the installer. `-SkipBuild` packages existing binaries.
- `CreateDistroNT.bat`: wrapper for the normal PowerShell build.
- `CompleteDistroNT.bat`: wrapper for the same script with `-SkipBuild`.
- `scripts/Test-Documentation.ps1`: check offline documentation and local
  Markdown links; also called before packaging.

The wrappers contain no independent packaging logic. PowerShell can be called
directly; the two BAT files are optional conveniences.

## Historical scripts

These are not supported release commands. Their dependencies and old source
layouts have not been restored.

- `CreateDistro.bat`: runs unsupported Win9x packaging before NT packaging.
- `CreateDistro9x.bat`: depends on Visual C++ 6.0, Borland, old HTML help and
  Inno Setup Extensions.
- `CreateDistroSRC.bat`: source packaging that omits current modules and
  references removed files. Can rewrite versions when given an argument.
  Use the Git repository for current sources.
- `scripts/ReVersionAll.bat` and `scripts/ReVersion*.pl`: rewrite old resources
  and `Setup/ZMatrix.iss`, not the active `Setup/ZMatrix_payalord.iss`.
- `Help/help.zmatrix.n3.net/src/make.bat`: Filepp/Perl help preprocessing with
  a removed input tree.
- `Web/zmatrix.n3.net/src/make.bat`: website assembly with a removed input tree.
- `Web/zmatrix.n3.net/scripts/ZipRelease.bat`: original website ZIP packaging.
- `Web/zmatrix.n3.net/scripts/ZipReleaseAndUploadToSourceForge.bat`: original
  ZIP/SCP upload to the author's SourceForge account, unrelated to this fork's
  homepage deployment.

The remaining `Help` and `Web` trees contain these historical BAT files, not
the old website/manual content. None is called by the current build or installer.

`Setup/ZMatrix.iss` and `Setup/ZMatrix.ise` are historical installer definitions.
Use the active PowerShell script and `Setup/ZMatrix_payalord.iss` for releases.
Update versions explicitly in the active resources and installer rather than
running the historical version rewriter.
