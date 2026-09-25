# ZMatrix

ZMatrix draws animated Matrix-style character streams behind the Windows
desktop icons and can also run as a screensaver. This is Payalord's maintained
fork of the original project by Z. Shaker (Happy Dude).

## Download and support

- [Homepage and downloads](https://payalord.github.io/ZMatrix/)
- [Source code](https://github.com/payalord/ZMatrix)
- [Bug reports, support and feature requests](https://github.com/payalord/ZMatrix/issues)

The source tree may contain changes that have not yet been published in a
release. Download the installer from the homepage for a published build.

## Using ZMatrix

Right-click its notification-area icon to configure the animation, pause it,
refresh the wallpaper, change startup behavior or use the screensaver.

Customize fonts, colors, glow and wallpaper blending, or enable audio reaction
to sound playing through a Windows playback output. Brightness, speed, stream
creation and color modulation can be controlled independently.

Read the [user guide](docs/USER_GUIDE.md) for the settings, audio palette
controls, configuration files and troubleshooting. Help is also available
from the application's menu without an Internet connection.

## Building and contributing

- [BUILDING.md](BUILDING.md): prerequisites, compilation, packaging and checks.
- [Module guide](docs/DEVELOPMENT.md): source layout and implementation contracts.
- [Script inventory](docs/SCRIPTS.md): current commands and historical scripts.

The complete application builds with Visual Studio 2022/v143 as 32-bit Unicode;
Inno Setup 6 packages the installer. C++Builder and Winamp are not required.

When reporting a problem, include your Windows version, ZMatrix version, steps
to reproduce it, and relevant display or audio settings. Review personal
special strings and device information before sharing configuration files.

## Credits

- Original ZMatrix author: Z. Shaker (Happy Dude).
- Windows port and this fork: Payalord.
- Matrix Code Font: e-RBi; see [the font notice](Matrix%20Code%20Font%20ReadMe.txt).
- Thanks for help and articles: [Kristjan Skutta](https://github.com/Biohazard90)
  and [Gerald Degeneve](https://github.com/gdegeneve).
- The original project was based on screensaver code published at elouai.com.

The [original author's README](ORIGINALREADME.md) is preserved as a historical
document, not the instructions for the current fork. Original project:
http://zmatrix.sourceforge.net/

## License

See [LICENSE.TXT](LICENSE.TXT) and the copyright notices in the source files.
The bundled font has its own notice linked above.
