#include "Audio/AudioCapture.h"
#include "AudioRuntime.h"
#include "zsMatrix/IzsMatrix.h"

namespace {
audio::Capture capture;
audio::Settings settings = audio::Defaults();
std::wstring settingsPath;
bool closed = true;
DWORD loadError = 0;
void __stdcall Get(void *, audio::Settings *out) { if(out) *out = settings; }
DWORD __stdcall Preview(void *, const audio::Settings *next) {
    if(closed) return ERROR_SHUTDOWN_IN_PROGRESS;
    if(!next || !audio::Valid(*next)) return ERROR_INVALID_DATA;
    const bool restart = next->enabled != settings.enabled || wcscmp(next->deviceId,settings.deviceId) != 0;
    if(restart) {
        if(next->enabled) {
            const DWORD error = capture.Start(next->deviceId);
            if(error) { if(settings.enabled) capture.Start(settings.deviceId); return error; }
        } else capture.Stop();
    }
    settings = *next;
    return ERROR_SUCCESS;
}
DWORD __stdcall Commit(void *, const audio::Settings *next) {
    if(closed) return ERROR_SHUTDOWN_IN_PROGRESS;
    if(!next || !audio::Valid(*next)) return ERROR_INVALID_DATA;
    // The editor previews first. Persist only when the enclosing dialog accepts.
    const DWORD error = audio::Save(settingsPath.c_str(),*next);
    if(!error) loadError = 0;
    return error;
}
void __stdcall Status(void *, audio::Status *out) {
    if(!out) return;
    const auto snapshot = capture.Read();
    *out = {snapshot.state,loadError ? HRESULT_FROM_WIN32(loadError) : snapshot.error,
        settings.mode == audio::WaveformVariation ? snapshot.signal.waveformVariation : snapshot.signal.spectralCentroid};
}
const audio::HostApi host = {sizeof(audio::HostApi),1,nullptr,Get,Preview,Commit,Status};
}
void InitializeAudio(const wchar_t *folder) {
    closed = false;
    try {
        settingsPath = folder;
        if(!settingsPath.empty() && settingsPath.back() != L'\\') settingsPath += L'\\';
        settingsPath += L"Audio.cfg";
        audio::Settings loaded = audio::Defaults();
        loadError = audio::Load(settingsPath.c_str(),loaded);
        if(loadError == ERROR_FILE_NOT_FOUND || loadError == ERROR_PATH_NOT_FOUND) loadError = 0;
        Preview(nullptr,&loaded);
    } catch(...) { loadError = ERROR_NOT_ENOUGH_MEMORY; }
}
void ShutdownAudio() { closed = true; capture.Stop(); }
const audio::HostApi *AudioHost() { return closed ? nullptr : &host; }
void UpdateAudioReaction(IzsMatrix *matrix) {
    if(!matrix) return;
    audio::Coefficients coefficients = {{1,1,1},{0,0,0}};
    const auto snapshot = capture.Read();
    if(settings.enabled && snapshot.state == audio::Capturing) {
        const double signal = settings.mode == audio::WaveformVariation ? snapshot.signal.waveformVariation : snapshot.signal.spectralCentroid;
        coefficients = audio::Map(settings.profiles[settings.mode],signal);
    }
    // Only the rendering/UI thread touches the COM engine. Fractional values remain intact.
    matrix->SetCoeffR1(coefficients.scale[0]); matrix->SetCoeffG1(coefficients.scale[1]); matrix->SetCoeffB1(coefficients.scale[2]);
    matrix->SetCoeffR0(coefficients.offset[0]); matrix->SetCoeffG0(coefficients.offset[1]); matrix->SetCoeffB0(coefficients.offset[2]);
}
