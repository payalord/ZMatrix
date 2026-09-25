# ZMatrix module guide

See [BUILDING.md](../BUILDING.md) for the toolchain and build commands, and the
[user guide](USER_GUIDE.md) for settings and their defaults. This document covers
source ownership, compatibility contracts and rendering constraints.

## Source layout

- `matrix.cpp`, `globals.cpp`: startup, the hidden animation controller, tray
  commands, configuration, wallpaper updates and screensaver transitions.
- `DesktopHost.cpp`: discovers and validates Explorer's background/icon
  hierarchy, including startup waiting and placement below desktop icons.
- `DesktopWindows.cpp`: owns desktop rendering windows, repairs their placement
  and recreates them when the display layout or Explorer host changes.
- `zsMatrix`: COM animation engine, streams, characters, cleanup and blending.
- `zConfig`: native Win32 configuration, audio, character, help and information
  dialogs, plus animation CFG persistence. Produces `Config.dll`.
- `Audio`: playback capture, analysis, response mappings and audio persistence.
  `AudioRuntime.cpp` connects these to the application and configuration UI.
- `MsgHook`, `RegistryListenerThread`, `TopLevelListenerWindow`: input hooks
  and Windows notifications. The hidden listener owns the animation controller
  independently of Explorer's windows.
- `ScreenSaver`: `ZMatrixSS.scr`, which starts or contacts the application.

## Desktop rendering

One engine maintains stream positions, timing, fonts and wallpaper for the
entire virtual desktop. Modern layered Explorer uses one rendering window per
monitor. Classic Explorer uses one background window. Each visible window must
remain in the verified background layer below icons; drawing stops while that
placement cannot be established.

`DesktopWindows` acquires the target DCs and maps engine coordinates to each
monitor with viewport offsets. `IzsMatrixRenderer::RenderTargets` advances the
animation once and routes drawing to intersecting targets. The caller retains
ownership of DCs; the engine restores their state and retains no target pointers.
Targets must also respect their DC clip regions. The ordinary single-DC `Render`
entry point remains available for screensaver rendering and compatibility.

Explorer surface loss triggers automatic recreation. Display/DPI notifications
are coalesced before resizing the shared canvas and refreshing the wallpaper.
The controller, audio state and timers survive surface replacement. A display
change ends an active screensaver before rebuilding the desktop target.

Rendering is incremental: new surfaces are cleared once, then streams draw and
clean up character areas. Fonts, wallpaper and scratch surfaces stay shared;
there are no per-monitor engines or additional fullscreen output bitmaps. Avoid
full-desktop repaints per frame, retained target DCs and per-frame allocation.

## Blending and glow

`zsMatrix.cpp` contains character rendering, `DrawArithmeticCharacter` and the
final output helpers. `CopyOutput`, `ClearOutput` and `DrawOutputCharacter` route
operations to either the single DC or active monitor targets. Changes to drawing
must preserve both paths, including cleanup, clipping and italic glyph overhang.

Color inversion, Dark mix and Bright mix retain the original XOR/AND/OR enum
values, CFG identifiers and GDI operations. Wallpaper shading, Soft brighten
(Screen) and Soft darken (Multiply) append new values. The arithmetic path uses
integer channels and a reusable 48 KiB DIB with three 64 x 64 tiles; larger glyphs
are tiled. `GdiFlush` synchronizes GDI writes before CPU pixel access.

Blend strength interpolates between the plain character/background colors at
0% and the selected wallpaper-blended result at 100%. It does not alpha-blend
an entire animation layer over the desktop. Preserve the original result for
legacy modes at full strength. Full strength skips the plain-character draw;
solid mode, zero strength and legacy modes release the arithmetic tile when
it is no longer needed.

Glow draws four faint one-pixel offsets followed by the original glyph inside
its character area. It shares the existing surfaces and cleanup bounds. The
configuration sample uses the same glow helper; it does not preview wallpaper
mixing or live audio colors.

## Audio processing

The WASAPI worker analyzes selected playback output roughly every 50 ms and
publishes descriptors; it never calls the rendering COM object. The rendering
thread applies RGB coefficients and transient motion overrides. Analysis is
selected by active influences: level needs no FFT, waveform uses sample history,
and centroid adds a reused 2048-point FFT. Capture stops when no influence needs
analysis. Audio samples are not saved to disk.

`AudioAnalysis.cpp` computes full-band RMS, bass energy, waveform variation and
spectral centroid. RMS includes all samples between updates. Bass filters each
channel separately to avoid opposite-phase cancellation. Waveform variation
uses signed floating-point differences at 44.1 kHz reference spacing and a fixed
response curve, with no adaptive peak or gain state. Do not reintroduce byte
quantization, unsigned wrapping or startup comparisons against missing history.

`AudioResponse.cpp` smooths responses using elapsed time, not frame count.
Brightness scales RGB equally; Speed and New streams have independent motion
multipliers. Color modulation smooths the normalized response before applying
Base/Peak mappings. A soft RMS gate restores normal brightness and motion near
silence; color mapping retains its Base behavior unless silence return bypasses it.

Silence return measures continuous silence on the capture thread, independently
of rendering, Source and Sensitivity. Its RMS thresholds are 0.0003 to enter and
0.0006 to leave silence. Capture gaps, discontinuities and device restarts reset
the timer. After the configured delay, coefficients and motion blend to exact
identity over 0.3 seconds. Capture continues while waiting; resumed sound reverses
the transition. The master enable setting is unchanged.

The engine keeps fractional budgets for motion ticks and stream births. Speed
is limited to 1..2 and birth rate to 0..2. Births are scheduled once per real
frame, independently of virtual ticks, and obey Maximum streams. Intermediate
motion ticks are drawn to preserve special strings and cleanup.

## Interfaces and settings compatibility

Preserve the original `IzsMatrix` vtable and Config exports. Optional COM
interfaces add appearance/glow (`IzsMatrixAppearance.h`), motion
(`IzsMatrixMotion.h`) and multiple render targets (`IzsMatrixRenderer.h`).
The executable/Config.dll audio host contract is version 3; validate the version
and structure size before exchanging settings or status.

Configuration is a preview transaction. The outer dialog saves animation and
audio settings on acceptance; Cancel restores both previews. Animation CFGs and
`Audio.cfg` remain separate formats. Defaults live in `default.cfg`, engine
initializers, `zConfig/Settings.cpp` and `Audio/AudioSettings.cpp`.

Animation CFG format remains 1.0. The reader accepts legacy ANSI and UTF-16 LE,
including a decimal point or single decimal comma in special-string probability.
Malformed or non-finite probabilities leave the current value unchanged; valid
values clamp to 0..1. Loading does not rewrite files. Atomic save preserves the
existing encoding and writes decimal points.

`Audio.cfg` saves version 3 and loads versions 1, 2 and 3:

- Version 1 keeps its mappings, selects only Color modulation and uses zero
  smoothing.
- Version 2 adds independent influences and strengths.
- Versions 1 and 2 receive silence return enabled with a 5-second delay;
  version 3 preserves the saved choice.

The current waveform analysis applies to every settings version. Existing
mappings are preserved; Reset effect restores only the selected mapping.
Legacy Winamp section/key names remain serialization identifiers. Importing
`vis_zmx.cfg` does not introduce a playback dependency.

Interactive uninstall offers removal of four known files in the uninstalling
account's settings directory. The default answer is No. Silent uninstall and
upgrades preserve settings. Cleanup leaves named presets and other accounts
alone, rejects redirected profile directories and reports failures.

## Verification

Tests are standalone executables, not solution projects. Build with the x86 VS
tools, run from the repository root and keep outputs in an ignored directory.
[BUILDING.md](../BUILDING.md) provides a compile/run example.

- `ConfigCompatibility`, `ConfigTextFormat`, `ConfigDialogs`: CFG encodings and
  migration, text formats, exports, dialogs and preview rollback.
- `BlendStrengthTests`: blend endpoints and reference pixels, glow, cleanup,
  motion, large glyphs, GDI lifetime and equivalent single/multiple-target output.
  An optional DLL argument compares against a retained reference build.
- `DesktopHostTests`: isolated Explorer layouts, icon ordering, startup waits,
  surface loss and rebuilds. Link `DesktopHost.cpp` and `DesktopWindows.cpp`,
  and embed `manifest.xml`. Its optional `--explorer-smoke` uses a hidden child
  of the real desktop.
- `AudioTests`, `AudioResponseTests`, `AudioRuntimeTests`: persistence, analysis,
  response envelopes and runtime state. `AudioSilenceRuntimeTests` substitutes
  capture data to test silence/resume with the real engine.
- `AudioCaptureSmoke`: real playback capture; its generated tone is opt-in.
- `DocumentationTests`: bundled documents, links, search, navigation and
  standalone `/help` and `/readme`. Uses test-owned windows.
- `tests/UninstallSettingsTests.ps1 -WorkDirectory <temporary-directory>`:
  builds an Inno Setup fixture and tests uninstall events with an isolated
  profile, including preservation, removal failures and junctions.

Isolated tests do not replace real Explorer, monitor hot-plug, DPI, screensaver
and target-Windows testing. For performance comparisons, keep the build settings,
wallpaper, display layout and audio workload identical and include a paused
baseline. Distinguish ZMatrix CPU/memory from DWM and per-adapter GPU engines;
process memory counters do not include every driver/compositor allocation.

## Documentation and retained material

The native help viewer reads the UTF-8 documents listed in
`zConfig/DocumentDialog.cpp`. Packaging checks them with
`scripts/Test-Documentation.ps1` and copies them through
`scripts/Build-Distribution.ps1` and `Setup/ZMatrix_payalord.iss`. Keep these lists
aligned when adding or renaming a bundled document. Use simple headings,
paragraphs, lists, code and links; the viewer supports a limited Markdown subset.

`Config` contains the unused VCL implementation and original About/Hire resources.
The native DLL still embeds the HTML, images and WAV through `zConfig/zConfig.rc`;
these assets are not a website mirror or build-tool dependency.
`ORIGINALREADME.md`, the font notice and `mem_manager_readme.txt` preserve original
or third-party information. Retain copyright/license notices; see
[LICENSE.TXT](../LICENSE.TXT). [SCRIPTS.md](SCRIPTS.md) identifies historical build
and website scripts that are outside the current build.
