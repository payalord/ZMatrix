# Retained configuration resources

The current DLL is built by `zConfig/zConfig.vcxproj`. Its resource file,
`zConfig/zConfig.rc`, includes original About/Hire HTML, images and sound from
this directory. Visual Studio compiles them during the normal solution build;
no standalone resource step or C++Builder installation is required.

`ConfigResources.rc`, `ConfigModern.cbproj` and VCL sources belong to the old
Borland implementation. They do not produce the DLL used by the current
solution and are not current build instructions.

The About dialog plays the original author's sound only when requested, with
one button to play or stop it. Keep the HTML, images and WAV referenced by the
native resource file even though the VCL implementation is unused. See
[BUILDING.md](../BUILDING.md) for build and distribution commands.
