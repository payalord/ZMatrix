# ZMatrix user guide

This guide describes the current ZMatrix source build. Features in an older
published installer may differ. ZMatrix runs as an animated desktop background
and can also be used as a screensaver.

[Homepage and downloads](https://payalord.github.io/ZMatrix/)

[Support and bug reports](https://github.com/payalord/ZMatrix/issues)

## Getting started

Install ZMatrix and start it from the Start menu or the installer's Launch
ZMatrix option. Find its icon in the Windows notification area; it may be in
the overflow area. Right-click the icon to open its menu.

- Configure opens the animation settings.
- Pause stops stream updates; selecting it again resumes them.
- Refresh reloads the desktop background and refreshes the animation target.
- Help opens this guide.
- Exit stops ZMatrix and removes its notification-area icon.

In desktop mode, the animation belongs behind the desktop icons. At startup,
the background starts solid and is filled progressively by moving streams.
ZMatrix waits for a usable Explorer background layer before starting. If it
cannot connect within a minute, choose Retry to wait again or Cancel to exit.
It does not intentionally draw over application windows in desktop mode.

The Help window works offline. Choose a document or section from its lists,
or search for a setting by name. Help and Readme can also be opened from the
Start menu without starting the animation.

## Configuration and preview

Most changes are previewed immediately. OK accepts and saves the settings;
Cancel restores the settings from before the dialog opened. Audio reaction
opens its own editor. Accepting that editor keeps its preview, but the outer
configuration dialog must also be accepted to save the audio changes.

### Text and special strings

Font selects the ordinary-character font. Special font selects the font for
special strings. A font must contain a glyph for a character to display it
correctly. The installer includes Matrix Code Font. JapaneseSet.txt and
MatrixCodeFontSet.txt provide character sets for suitable fonts.

Characters and special strings opens the text editor. Characters are comma
separated; special strings use one line per string. Use \\, for a literal comma
in the character list and \\\\ for a literal backslash. The legacy \\0xNNNN format
represents a Unicode character, for example \\0x0041 for A. Character and string
lists can be loaded and saved separately as text files.

Special strings (%) controls how often streams select a special string. At
0%, none are selected; at 100%, streams select special strings when available.

### Colors

Ordinary text and Special strings each have Lead and Trail colors. Lead is
the bright leading character; Trail is the faded character left behind it.
Background selects the background color. Copy ordinary colors to special
copies the ordinary Lead and Trail colors to the special-string colors.

Enable glow adds a narrow halo to ordinary and special text. It is off by
default and adds drawing work. The sample updates immediately; the running
animation shows changes as new characters are drawn. The halo can be clipped
with tightly packed fonts.

Audio reaction can change visible colors even if these selections have not
changed. See Keeping the original palette below.

### Streams and performance

Maximum streams limits the total number of streams across the desktop. Speed
variance controls how different their speeds can be; zero gives them the same
speed. Slower streams also use dimmer character colors. Refresh time (ms)
controls the interval between updates. A larger interval reduces update
frequency and slows the animation; a smaller interval updates more often.

To reduce animation work, lower Maximum streams, increase Refresh time or
disable glow. Audio-driven speed and stream creation can add drawing work.
Process priority controls how Windows schedules ZMatrix against other tasks;
the default is Idle. Raising priority does not reduce the work performed.

Desktop animation also creates GPU work in Windows' Desktop Window Manager
(dwm.exe). ZMatrix's own CPU usage alone does not show the total cost. To compare
settings, keep the wallpaper, monitor layout and audio playback the same, and
use Pause to check the baseline with stream updates stopped.

### Background and blending

Background mode selects Bitmap (use the desktop wallpaper) or Solid color
(use the selected Background color). Set a wallpaper in Windows before using
bitmap blending.

Bitmap blending controls how character colors mix with the wallpaper:

- Color inversion changes colors dramatically. White characters invert the
  wallpaper colors; other character colors produce different color shifts.
- Dark mix combines colors into a darker result. Some color
  combinations can become almost invisible.
- Bright mix combines colors into a brighter result.
- Wallpaper shading uses the brightness of wallpaper details while retaining
  the character's hue. A colorful picture appears in shades of the text color.
- Soft brighten (Screen) lightens the result with smooth color transitions.
  It suits dark wallpapers; pale areas can reduce character contrast.
- Soft darken (Multiply) darkens and colors wallpaper details with smooth
  transitions. It suits light wallpapers; dark areas can hide characters.

The color/font sample shows text colors and glow on the selected solid color;
wallpaper mixing and audio color changes are visible in the running animation.

Text background selects Transparent or Opaque for the area behind individual
characters. It does not set the transparency of the entire desktop animation.

Blend strength (%) controls the wallpaper contribution to each character area:

- 100% uses the selected wallpaper-blending result.
- 0% uses the selected character and Background colors, without wallpaper
  contribution.
- Intermediate values mix these two results.

The background starts solid and streams gradually reveal the blended image.
Existing trails update as the animation progresses. Bitmap blending and Blend
strength are disabled in Solid color mode.

### Trail cleanup

Monotonous cleanup erases the trail at a fixed distance behind its leading
character. Back trace sets that distance in character positions.

Randomized cleanup erases positions in a range behind the leading character.
Leading sets the distance at which the range begins; Space padding sets how
far it extends beyond that point. Both methods can be enabled independently.

Cleanup affects trails as streams move. Switching it off does not immediately
restore pixels that were already cleared or change the selected Trail colors.

## Audio reaction

Open Configure, then Audio reaction, and select Enable audio reaction.
Playback output chooses the Windows output whose sound is analyzed. The
default output follows the Windows default playback device. Refresh updates
the list after connecting or removing an output.

Capture uses Windows WASAPI loopback: sound played through the selected output,
including other applications and system sounds. It does not select a microphone
and does not save an audio recording. A player using another output will not
drive the selected output's reaction.

### Effects and response controls

Each influence has its own checkbox. Brightness, Speed and New streams also
have independent strength controls; unchecking an influence keeps its saved
strength. A strength of 0% has no effect. Enable just the influences you want.

- Brightness gently dims the chosen character colors during quieter passages
  and approaches their original brightness during louder passages. It uses
  equal RGB scaling, without adding colors or increasing them beyond the chosen
  palette. At maximum strength, dimming can reach 25% of the original brightness.
- Speed smoothly accelerates the streams, preserving their individual speed
  differences. Maximum strength allows up to twice the normal speed. It does
  not change Refresh time or the saved Speed variance.
- New streams varies how often new streams appear: fewer in quieter passages,
  more in louder passages. Maximum strength ranges from zero to twice the normal
  birth rate. Existing streams finish naturally, and Maximum streams still
  applies. This changes the arrival rate, not the length of existing trails.
- Color modulation enables the RGB mappings described below. It can
  change the palette and works independently of the other three influences.

Source selects Audio level (overall loudness, measured as RMS) or Bass energy
(low-frequency energy). It controls Brightness, Speed and New streams; the
color mapping has its own Effect selector. Bass energy follows bass activity,
not a detected musical tempo or a guaranteed beat.

Sensitivity (%) multiplies the level/bass response: 100% means unchanged,
400% means four times the measured level. Increase it for quiet playback or
lower it if the response stays near 100%. Smoothness (%) softens transitions,
including Color modulation. Motion always has a short smoothing period to
avoid abrupt speed jumps; 0% gives its fastest response.

Near silence, brightness and motion gradually return to their ordinary values.
Color modulation retains its Base behavior unless silence return is enabled.
Changes affect newly drawn characters; existing trails keep their pixels until
redrawn or cleared.

For a new configuration, audio reaction is disabled. Its initial settings are:

- Playback output: System default output; Source: Audio level.
- Sensitivity: 400%; Smoothness: 50%.
- Brightness: selected, strength 50%.
- Speed: unchecked, saved strength 35%.
- New streams: unchecked, saved strength 50%.
- Color modulation: unchecked; Effect: Waveform variation.
- Return to normal during silence: selected, delay 5 seconds.

### Returning to normal during silence

Return to normal during silence temporarily removes all audio influences after
Silence delay (seconds) of continuous silence. The delay accepts whole numbers
from 1 to 60. Turning this option off preserves the usual audio behavior.

The transition to ordinary colors, brightness, speed and stream creation takes
about 0.3 seconds. Enable audio reaction stays checked. Capture continues, and
the selected effects return over about 0.3 seconds when sound resumes, without
waiting for the silence delay again. The status reports "Waiting for sound.
Ordinary appearance is active." during the bypass.

Silence is measured from all sound on the selected playback output, including
system sounds, independently of Source and Sensitivity. A low fixed threshold
and a small gap between the silence and sound thresholds prevent noise from
repeatedly switching the effect. Short pauses do not activate the bypass.
This does not restart the animation or immediately recolor existing trails.

### Color modulation

Waveform variation reacts to changes between signed audio samples. Both level
and frequency affect it; it is not simply a volume meter. Lowering a signal's
amplitude lowers its response, including very quiet signals near zero.
Its response does not automatically turn down sensitivity after a loud passage.

Spectral centroid reacts to the balance of frequencies. Higher-frequency
content increases the response; it is not a beat detector.

Each effect has its own color mapping. Base describes the mapping at response
0%; Peak describes it at response 100%. R, G and B (%) multiply the respective
color channels. Offset adds an RGB color. Between Base and Peak, the mapping
changes gradually with the response. Strong scales and offsets can saturate
channels and produce colors quite different from the ordinary green palette.

Global scale (%) multiplies the measured response. Global offset (%) shifts
it, and the result is limited to 0%-100%. These settings control how quickly
the mapping moves from Base to Peak.

Reset effect restores the following defaults for the selected effect:

- Both effects: Base R/G/B 0%; Peak R/G/B 200%; Base Offset RGB (0, 0, 0).
- Waveform variation: Peak Offset RGB (0, 24, 48), Global scale 300%,
  Global offset -30%.
- Spectral centroid: Peak Offset RGB (0, 0, 0), Global scale 500%,
  Global offset 0%.

Offsets are added to scaled character colors; they are not the final colors.
Brightness and wallpaper blending also affect the result. The Waveform
variation Peak offset therefore does not mean that the text will be blue.

Reset leaves the other effect and all output, response, influence and silence
settings unchanged. Existing/imported mappings stay as saved until changed or
reset. Accept both the audio editor and Configuration to save a reset; Cancel
restores the previous preview. Changing only Base/Peak percentages does not
restore the complete profile.

### Keeping the original palette

Leave Color modulation unchecked to keep the selected hues. Enable Brightness
for a pulse in those hues, or leave Brightness unchecked too if you want only
motion changes with completely unchanged character colors. Wallpaper blending
influences the final image independently of audio; bitwise blending can change
the apparent hue even when the character's RGB channels are scaled equally.

### Device status and old settings

The status text reports capture state or an error. If nothing reacts, check
that the correct output is selected and sound is playing through it. If an
output is disconnected, select an available output or the default output.
When capture is unavailable, the renderer immediately uses ordinary appearance
and motion. Capture stops when no influence is active, including when all
selected strengths are zero.

Import legacy Winamp settings reads an existing vis_zmx.cfg mapping. It does
not install or require Winamp. Import enables Color modulation but leaves the
master capture switch and other influences unchanged. Old section names remain
for compatibility; current playback capture is built into ZMatrix.

Older audio configurations load automatically and retain their saved mappings.
The earliest format selects only Color modulation with Smoothness 0%. Files
predating silence return receive that option enabled with a 5-second delay.
Current waveform analysis also applies to old and imported mappings, so their
response can differ from the original Winamp implementation.

## Saving, loading and resetting settings

The File menu saves or loads an animation CFG and can load defaults from the
default.cfg bundled with ZMatrix. Loaded changes remain a preview until OK.
Saving a named CFG is an explicit file operation; Cancel does not undo that save.

Audio settings are separate. Loading or saving an animation CFG does not
transfer the audio profile. To back up all settings, exit ZMatrix and copy its
entire configuration directory.

Each Windows user's files are in `%APPDATA%\.ZMatrix`:

- ZMatrix.cfg: normal animation settings.
- ZMatrixScreenSaver.cfg: optional screensaver animation settings.
- ZMatrixMisc.cfg: additional startup/screensaver preferences.
- Audio.cfg: audio output, independent influences, response controls and color mappings.

Paste `%APPDATA%\.ZMatrix` into File Explorer's address bar to find the directory.
For a complete reset, exit ZMatrix, back up this directory and rename it. The
next launch recreates settings as needed. For animation appearance alone, use
File > Load defaults and accept the configuration dialog.

Animation CFGs support legacy ANSI and UTF-16 little-endian formats. Use the
application to edit them; changing them to UTF-8 is not supported by the legacy
CFG reader. The documentation files themselves use UTF-8.

Older animation and screensaver CFGs load directly without an installer
conversion. Special-string probability accepts either a decimal point or the
decimal comma used by some older configurations (for example, 0,25 means 25%).
Loading leaves the file unchanged; saving preserves its supported encoding.

## Automatic startup

Auto-Start Options controls startup shortcuts for the current user and for all
users. Changing all-users startup can require additional Windows permissions.
These options start the desktop animation at sign-in and are separate from
Windows' screensaver timeout.

## Screensaver

Install the optional Screensaver Component to use ZMatrixSS as the Windows
screensaver. Screen Saver Options in the tray menu contains:

- Screen Save: enter ZMatrix's screensaver mode immediately.
- Set As Screen Saver: select ZMatrixSS as the Windows screensaver.
- Always Set As Screen Saver While Running: temporarily select it while
  ZMatrix runs, restoring the previous selection when ZMatrix exits normally.
- Blend Screen Saver With Background Only: use the wallpaper for bitmap
  blending; when unchecked, screensaver blending can use visible screen
  contents, including application windows.
- Add/Edit Screen Saver Configuration: edit separate animation settings.
- Delete Screen Saver Configuration: return to using the normal animation
  settings for the screensaver.

Audio settings are shared. A separate screensaver CFG changes animation
settings, not the selected audio output or audio profile.

For screensaver-only use, disable automatic desktop startup, turn off Always
Set As Screen Saver While Running, select ZMatrixSS in Windows' screensaver
settings and exit ZMatrix. Windows can then launch it for screensaver mode.
Screen Save from the tray is an animation command, not a workstation lock;
use Windows' lock command when you need to lock the session.

## Troubleshooting

### Unexpected colors

Check Enable audio reaction first, then Base/Peak RGB scales and offsets.
Check Background mode, Bitmap blending and Blend strength next. Different
wallpapers produce different results with the same color selections.

### Old wallpaper or incorrect display arrangement

ZMatrix rebuilds its desktop target when monitor layout or scaling changes.
Select Refresh from the tray menu if the wallpaper is stale. If the arrangement
remains wrong after the display change settles, restart ZMatrix and report the
monitor resolutions, scaling and relative positions.

### Startup waits or drawing appears above desktop icons

Startup waits for a usable Explorer background layer. If that layer becomes
unavailable later, ZMatrix suspends desktop drawing and retries automatically.
It recreates rendering windows after Explorer replaces them. If animation does
not return after Explorer recovers, restart ZMatrix and report the problem.

Drawing over icons in desktop mode is a defect. Report the Windows build,
display layout, scaling, ZMatrix version and reproduction steps. If it happens
during an antivirus check, include the exact product message; timing alone
does not identify the cause.

### Multiple monitors

The animation spans the Windows virtual desktop, including monitors left of or
above the primary display. Maximum streams is shared across monitors. Modern
Explorer uses a separate rendering window for each monitor automatically; no
additional setting is needed. Mixed-DPI layouts and Explorer behavior can differ
between Windows versions.

### Missing characters

Choose a font containing the selected characters or load a suitable character
set. Ordinary characters and special strings have separate fonts.

## About and the original author's message

Open Help > About ZMatrix from Configuration. Play original author message
starts the recording; the green triangle changes to a square while it plays.
Press the same button to stop. Playback also stops when the dialog closes and
never starts automatically when About opens.

## Uninstalling

Exit ZMatrix and uninstall it through Windows' installed-apps list or the
Uninstall ZMatrix shortcut. The uninstaller asks whether to also remove your
settings from `%APPDATA%\.ZMatrix`. Choose Yes for a fresh configuration on the
next installation, or No (the default) to keep your settings. This removes only
ZMatrix.cfg, ZMatrixScreenSaver.cfg, ZMatrixMisc.cfg and Audio.cfg; named presets
and other files are retained. The directory is removed only if it is empty.

The prompt shows the Windows account and exact settings directory. If you run
the uninstaller under another account, the choice applies to that account;
other users' settings are not removed. Silent uninstall keeps settings without
prompting. Updating or reinstalling ZMatrix also keeps settings.

The bundled Matrix Code Font is retained. If settings cannot be removed, the
uninstaller reports this instead of silently claiming a complete reset.
