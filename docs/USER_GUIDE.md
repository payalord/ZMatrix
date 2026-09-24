# ZMatrix user guide

This guide describes the current ZMatrix source build. Features in an older
published installer may differ. ZMatrix runs as an animated desktop background
and can also be used as a screensaver.

Homepage and downloads: https://payalord.github.io/ZMatrix/
Support and bug reports: https://github.com/payalord/ZMatrix/issues

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

Enable glow adds a fixed, narrow halo in the character's color. It is off by
default and applies to both ordinary and special text, including audio color
changes. The effect stays inside each character area, so its edges may be
clipped with tightly packed fonts. It adds drawing work without extra image
buffers. Changes appear as streams draw new characters; existing trails remain
until they are redrawn or cleared.

Audio reaction can change visible colors even if these selections have not
changed. See Keeping the original palette below.

### Streams and performance

Maximum streams limits the number of streams. Speed variance controls how
different their speeds can be; zero gives them the same speed. Refresh time
(ms) controls the interval between animation updates. A larger interval reduces
update frequency and slows the animation; a smaller interval updates more often.

To reduce animation work, lower Maximum streams or increase Refresh time.
Process priority controls how Windows schedules ZMatrix against other tasks;
the default is Idle. Raising priority does not reduce the work performed.

### Background and blending

Background mode selects Bitmap (use the desktop wallpaper) or Solid color
(use the selected Background color). Set a wallpaper in Windows before using
bitmap blending.

Bitmap blending controls how character colors mix with the wallpaper:

- Color inversion (formerly XOR) changes colors dramatically. White characters
  invert the wallpaper colors; other character colors invert selected bits.
- Dark mix (formerly AND) combines colors into a darker result. Some color
  combinations can become almost invisible.
- Bright mix (formerly OR) combines colors into a brighter result.
- Wallpaper shading uses the brightness of wallpaper details while retaining
  the character's hue. A colorful picture appears in shades of the text color.
- Soft brighten (Screen) lightens the result with smooth color transitions.
  It suits dark wallpapers; pale areas can reduce character contrast.
- Soft darken (Multiply) darkens and colors wallpaper details with smooth
  transitions. It suits light wallpapers; dark areas can hide characters.

The first three modes retain their original appearance and CFG identifiers.
Wallpaper shading uses the current character color, including audio changes.
These modes change colors; Enable glow separately adds a narrow character halo.
The color/font sample shows text colors and glow on the selected solid color;
wallpaper mixing is previewed in the running animation.

Text background selects Transparent or Opaque for the area behind individual
characters. It does not set the transparency of the entire desktop animation.

Blend strength (%) controls the wallpaper contribution to each character area:

- 100% uses the selected wallpaper-blending result.
- 0% uses the selected character and Background colors, without wallpaper
  contribution.
- Intermediate values mix these two results.

This does not make the animation window transparent over a visible wallpaper.
The solid start and gradual filling by streams are preserved. Existing trails
update as the animation progresses. Bitmap blending and Blend strength are
disabled in Solid color mode.

### Trail cleanup

Monotonous cleanup erases the trail at a fixed distance behind its leading
character. Back trace sets that distance in character positions.

Randomized cleanup erases positions in a range behind the leading character.
Leading sets the distance at which the range begins; Space padding sets how
far it extends beyond that point. Both methods can be enabled independently.

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
- Color modulation enables the older RGB mappings described below. It can
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

After sustained silence, brightness and motion gradually return to their
ordinary values. The older Color modulation mapping retains its Base behavior
in silence. Changes affect newly drawn characters; existing trails keep their
pixels until redrawn or cleared.

For a new configuration, audio reaction is disabled. Brightness is the only
influence selected when it is first enabled. Speed, New streams and Color
modulation are opt-in. Faster motion and more streams add drawing work; modest
strengths are usually more suitable for a desktop background.

### Color modulation

Waveform variation reacts to differences between adjacent audio samples. Both
level and frequency affect it; it is not simply a volume meter.

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

Existing Audio.cfg version 1 files retain their previous color behavior when
loaded: Color modulation is selected, the three new influences are off, and
Smoothness is 0%. They are saved in version 2 format when settings are accepted.

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

Select Refresh from the tray menu. If monitor layout or scaling changed and
the result remains wrong, exit and start ZMatrix again.

### Startup waits or drawing appears above desktop icons

Startup waits for a usable Explorer background layer and offers Retry or Cancel
if it cannot connect. If validation later fails, desktop drawing is suspended.
If Explorer destroys the background window, restart ZMatrix after Explorer
has recovered.

Drawing over icons in desktop mode is a defect. Report the Windows build,
display layout, scaling, ZMatrix version and reproduction steps. If it happens
during an antivirus check, include the exact product message; timing alone
does not identify the cause.

### Multiple monitors

The animation uses the Windows virtual desktop, including monitors left of or
above the primary display. If there is a problem, report each monitor's
resolution, scaling and relative position. Mixed-DPI layouts and Explorer
behavior can differ between Windows versions.

### Missing characters

Choose a font containing the selected characters or load a suitable character
set. Ordinary characters and special strings have separate fonts.

## Uninstalling

Exit ZMatrix and uninstall it through Windows' installed-apps list or the
Uninstall ZMatrix shortcut. User files in `%APPDATA%\.ZMatrix` and the bundled
Matrix Code Font are retained. Remove backed-up user settings separately only
if you want to discard them.
