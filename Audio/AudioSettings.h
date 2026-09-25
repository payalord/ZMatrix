#pragma once
#include <windows.h>

namespace audio {
// Values are persisted in Audio.cfg; keep them stable when renaming effects.
enum Mode { WaveformVariation = 0, SpectralCentroid = 1, ModeCount = 2 };
enum ResponseSource { AudioLevel = 0, BassEnergy = 1, SourceCount = 2 };
constexpr DWORD HostVersion = 3;
struct Mapping {
    double baseScale[3], baseOffset[3], peakScale[3], peakOffset[3];
    double globalScale, globalOffset;
};
// Plain data shared by the executable and Config.dll. No CRT ownership crosses the ABI.
struct Settings {
    BOOL enabled;
    UINT mode;
    wchar_t deviceId[512]; // Empty means the default multimedia playback endpoint.
    Mapping profiles[ModeCount];
    UINT responseSource;
    double sensitivity, smoothing;
    BOOL brightnessEnabled, speedEnabled, spawnEnabled, colorEnabled;
    double brightnessStrength, speedStrength, spawnStrength;
    BOOL returnOnSilence;
    UINT silenceDelaySeconds;
};
enum State { Disabled, Starting, Capturing, Unavailable };
struct Status { State state; HRESULT error; double descriptor, response; BOOL waitingForSound; };
struct HostApi {
    DWORD size, version;
    void *context;
    void (__stdcall *get)(void *, Settings *);
    DWORD (__stdcall *preview)(void *, const Settings *);
    DWORD (__stdcall *commit)(void *, const Settings *);
    void (__stdcall *status)(void *, Status *);
};
Settings Defaults();
bool Valid(const Settings &settings);
// On failure the destination is unchanged. Missing files return ERROR_FILE_NOT_FOUND.
DWORD Load(const wchar_t *path, Settings &settings, bool legacy = false);
DWORD Save(const wchar_t *path, const Settings &settings);
struct Coefficients { double scale[3], offset[3]; };
Coefficients Map(const Mapping &mapping, double descriptor);
}
