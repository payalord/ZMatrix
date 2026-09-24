# ZMatrix module guide

This describes the current Visual Studio build and replaces the historical
Visual C++ 6.0/Borland notes from SourceCodeReadme.txt. Original ZMatrix copyright:
Z. Shaker, 2001-2002. Retain source notices; see [LICENSE.TXT](../LICENSE.TXT).

## Application modules

- `matrix.cpp` and `globals.cpp`: startup, tray commands, configuration,
  wallpaper updates and screensaver transitions.
- `DesktopHost.cpp`: discovers Explorer's background/icon hierarchy, creates
  the rendering child, waits during startup and validates placement. Desktop
  drawing is suspended when the host is invalid.
- `zsMatrix`: COM rendering engine, streams, characters and wallpaper blending.
  The original `IzsMatrix` ABI is retained. `IzsMatrixAppearance.h` supplies the
  optional Blend strength and glow interfaces without changing existing vtables.
- `zConfig`: native Win32 configuration, audio, character, help and information
  dialogs, plus CFG persistence. Output: `Config.dll`. Original Unicode/stdcall
  entry points remain; new functionality uses additional exports.
- `Audio`: WASAPI capture, analysis, color mappings and persistence.
  `AudioRuntime.cpp` connects it to the application. The capture worker never
  calls the rendering COM object; the UI/rendering thread applies coefficients
  and transient motion overrides through `IzsMatrixMotion.h`.
- `MsgHook`, `RegistryListenerThread`, `TopLevelListenerWindow`: Windows
  notifications and screensaver input handling.
- `ScreenSaver`: `ZMatrixSS.scr`, which communicates with the running
  application or starts it for screensaver use.

See [BUILDING.md](../BUILDING.md) for toolchains and output paths.

## Rendering and configuration

Desktop rendering uses a verified Explorer background child. It must not fall
back to a generic desktop window. Screensaver rendering is a separate path.

Rendering is incremental. Blend strength mixes plain characters with the
wallpaper-blended result in the character-sized work area. Do not introduce a
fullscreen alpha layer or repaint the entire desktop each frame. Preserve the
original bitmap result for legacy modes at 100% and remove wallpaper
contribution at 0% for every mode.

The UI calls the legacy XOR/AND/OR modes Color inversion, Dark mix and Bright
mix. Their enum values and CFG identifiers remain unchanged. Wallpaper shading,
Soft brighten (Screen) and Soft darken (Multiply) append new values. They use
integer color arithmetic in DrawArithmeticCharacter, with a lazy 48 KiB DIB
containing three 64 x 64 tiles for glyph coverage, wallpaper and plain/result
pixels. Larger characters are processed in tiles. No fullscreen buffers or
per-frame allocations are added. GdiFlush synchronizes the tile before CPU
access; only dirty character regions are presented. Legacy modes retain their
GDI path. Full strength skips the plain-character draw. Zero strength bypasses
wallpaper mixing and frees the tile buffer, as do solid mode and legacy blends.

Shading scales each text channel by wallpaper brightness, approximated as
(54R + 183G + 19B) / 256. Screen and Multiply use their standard per-channel
formulas. Glyph coverage includes antialiasing and glow; Blend strength then
interpolates the result with the plain character. Preserve text-background
opacity, cleanup, clipping and viewport behavior when extending this path.

The optional minimal glow draws four faint one-pixel character offsets inside
the existing character rectangle, followed by the original glyph. It reuses
the same rendering surfaces and cleanup bounds, with no glyph cache or extra
image buffers. Disabled glow uses the original TextOut path.

Configuration preview is reversible. The outer dialog persists audio settings
on acceptance; Cancel restores animation and audio previews. Animation CFGs
and Audio.cfg are separate formats. Legacy Winamp names remain only for
configuration compatibility, not as playback dependencies.

## Audio analysis and response

`AudioResponse` selects the analysis needed by active influences and smooths
their envelopes using elapsed time, independent of Refresh time. RMS energy
includes all samples between analysis updates, not only the last FFT window.
Bass uses per-channel DC rejection and two low-pass stages around 200 Hz;
opposite-phase channels do not cancel. Brightness scales RGB equally within
the chosen palette. A soft RMS silence gate returns level-based effects to
ordinary appearance and motion. Color modulation retains the legacy mappings,
including their Base values in silence.

The worker analyzes roughly every 50 ms. Level-only analysis needs no sample
ring or FFT storage. Bass adds three filter values per channel. Waveform
variation uses the sample ring; spectral centroid additionally uses a reused
2048-point FFT buffer and cached Hann window/stage coefficients. Disabled
analyses do not run, and unneeded buffers are released when the mask changes.
Capture stops when no influence is active. No PCM is written to disk.

The engine keeps two fractional budgets for motion: virtual update ticks and
new stream births. Speed is limited to 1..2, birth rate to 0..2. Births are
scheduled once per real frame, independently of the virtual tick count, and
never exceed Maximum streams. Every intermediate tick is drawn to preserve
special strings and cleanup. There are no new rendering surfaces or per-stream
allocations. Normal multipliers retain the original single-update behavior.

Audio.cfg version 2 stores each influence and its amount separately. Version 1
loads with only legacy Color modulation enabled and no smoothing, preserving
existing appearance. The executable/Config.dll host contract is version 2;
reject other versions before copying Settings or Status structures. The five
original Config exports and all existing COM interface vtables are unchanged.

`AudioTests`, `AudioResponseTests`, `AudioRuntimeTests`, `ConfigDialogs` and
`BlendStrengthTests` cover persistence/migration, analysis, independent effects,
envelopes, endpoint failure, preview rollback and motion rendering. The response
test links AudioSettings.cpp and AudioResponse.cpp; runtime tests also need
AudioAnalysis.cpp, AudioCapture.cpp and AudioRuntime.cpp. Keep test output
outside source directories. AudioCaptureSmoke's test tone is explicitly opt-in.

## Documentation and historical material

[USER_GUIDE.md](USER_GUIDE.md) is the current manual. The archived website,
preprocessed HTML manual and empty CHM files are not part of the help system.

`Config` retains the original Borland implementation and the About/Hire HTML,
images and sound. The native build consumes these assets through
`zConfig/zConfig.rc`; it does not compile VCL. About/Hire content awaits a
separate review.

`ORIGINALREADME.md` preserves original author information.
`Matrix Code Font ReadMe.txt` records the font author and distribution notice.
`mem_manager_readme.txt` accompanies the third-party `mmgr` code still listed
in the native projects. These are historical/third-party documents, not current
build or user instructions.

See [SCRIPTS.md](SCRIPTS.md) for the active entry points and old scripts retained
for review.
