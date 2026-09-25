# ZMatrix

This project originally forked from: [http://zmatrix.sourceforge.net/](http://zmatrix.sourceforge.net/)

## v2.0.0
[Download version 2.0.0](https://github.com/payalord/ZMatrix/releases/download/v2.0.0/ZMatrixSetupNT_2_0_0.exe)
* File: ZMatrixSetupNT_2_0_0.exe
* SHA-256: `7a2883d3dc89b662bbe6bc348c781cccb21ef2ef22ed43a586ffcbd585799a47`
### What's Changed
- Improved multi-monitor support, including contributions from @latin-programmer in [#7](https://github.com/payalord/ZMatrix/pull/7).
- Optimized desktop rendering with per-monitor windows and fixed a GDI resource leak.
- Improved desktop icon layering, startup handling, and recovery after Explorer or display changes.
- Replaced the Borland/VCL configuration dialogs with a native interface built with Visual Studio C++.
- Added built-in Audio reaction for Windows playback audio, with independent controls for brightness, speed, stream generation, and color modulation.
- Added automatic return to normal animation during silence.
- Added adjustable blend strength, new wallpaper blending modes, and clearer mode names.
- Added optional text glow with a live configuration preview.
- Replaced audio autoplay in the About dialog with a play/stop button.
- Added native offline help and updated documentation and project links.
- Improved compatibility with older settings and added optional settings removal during uninstall.
### Breaking Changes
- The legacy Winamp visualization plugin is no longer included. Audio reaction is now built directly into ZMatrix and configured through its settings.
Existing ZMatrix configuration files remain supported. Legacy Winamp audio mappings can be imported through the Audio reaction settings.

## v1.5.4
[Download version 1.5.4](https://github.com/payalord/ZMatrix/releases/download/v1.5.4/ZMatrixSetupNT_1_5_4.exe)
* File: ZMatrixSetupNT_1_5_4.exe
* CRC-32: `217e9453`
* SHA-1: `6d263dab014a77bf338b59b5aa0c91d59977c917`
* SHA-256: `7fa6e2e8dff8d6af317d90e8088b254475667bc9ab89bf989c0be2b5908d9c30`

### Change log:
1. Added High-DPI support

## v1.5.3
[Download version 1.5.3](https://github.com/payalord/ZMatrix/releases/download/v1.5.3/ZMatrixSetupNT_1_5_3.exe)
* File: ZMatrixSetupNT_1_5_3.exe
* CRC-32: `94e87ebd`
* MD4: `e4abbb0cd070b60f2b1630644ae79b87`
* MD5: `a66c6e0f074e214651ad0d4ac8e2b1d1`
* SHA-1: `f75b557494d439f77f22a62d0255f72f21f58a41`

### Change log:
1. Upgraded to work on Visual Studio 2017 Community (not everything, only main program).
2. Fixed to work on Windows 7 (Aero).

# Description
ZMatrix is desktop enhancement program. It attempts to reproduce the streaming characters shown in the film **The Matrix** on your computer desktop. Besides attempting to reproduce this look, the program also allows you to choose your own color, font, and background blending methods to suit your taste. As is implied by the mentioning of **background blending methods**, the program will not necessarily hide your current background image, rather, it can be told to blend with it in a few different ways (see the Configuration Dialog). Furthermore, ZMatrix can also function as a screensaver... [Read Full Original Author's README](ORIGINALREADME)

# Credits
Fixed by [Payalord](https://github.com/payalord) for Windows 7 (Aero)

Original Author: [Z. Shaker](http://zmatrix.sourceforge.net/help/frames_index.html)

### Special Thanks for the help/articles to:

* [Kristjan Skutta](https://github.com/Biohazard90)
* [Gerald Degeneve](https://github.com/gdegeneve)

# License
[GPL version 2](LICENSE.TXT)
