# Retained configuration resources

The current DLL is built by `zConfig/zConfig.vcxproj`. Its resource file,
`zConfig/zConfig.rc`, includes original About/Hire HTML, images and sound from
this directory. Visual Studio compiles them during the normal solution build;
no standalone resource step or C++Builder installation is required.

`ConfigResources.rc`, `ConfigModern.cbproj` and VCL sources belong to the old
Borland implementation. They do not produce the DLL used by the current
solution and are not current build instructions.

About/Hire assets remain in use and await a separate review. Do not remove them
just because the VCL implementation is historical. See [BUILDING.md](../BUILDING.md)
for the current build and distribution commands.
