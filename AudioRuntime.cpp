#include "Audio/AudioCapture.h"
#include "Audio/AudioResponse.h"
#include "AudioRuntime.h"
#include "zsMatrix/IzsMatrix.h"
#include "zsMatrix/IzsMatrixMotion.h"

namespace {
audio::Capture capture;
audio::Settings settings = audio::Defaults();
std::wstring settingsPath;
bool closed = true;
DWORD loadError = 0;
audio::Response response;
ULONGLONG lastUpdate = 0;
void __stdcall Get(void *, audio::Settings *out) { if(out) *out = settings; }
DWORD __stdcall Preview(void *, const audio::Settings *next) {
    if(closed) return ERROR_SHUTDOWN_IN_PROGRESS;
    if(!next || !audio::Valid(*next)) return ERROR_INVALID_DATA;
    const unsigned mask = audio::RequiredAnalysis(*next), previousMask = audio::RequiredAnalysis(settings);
    const bool restart = bool(mask) != bool(previousMask) || wcscmp(next->deviceId,settings.deviceId) != 0;
    if(restart) {
        if(mask) {
            const DWORD error = capture.Start(next->deviceId,mask);
            if(error) { if(previousMask) capture.Start(settings.deviceId,previousMask); return error; }
        } else capture.Stop();
        response.Reset(); lastUpdate = 0;
    } else capture.SetAnalysisMask(mask);
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
        settings.mode == audio::WaveformVariation ? snapshot.signal.waveformVariation : snapshot.signal.spectralCentroid,
        audio::ResponseLevel(settings,snapshot.signal)};
}
const audio::HostApi host = {sizeof(audio::HostApi),audio::HostVersion,nullptr,Get,Preview,Commit,Status};
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
void ShutdownAudio() { closed = true; capture.Stop(); response.Reset(); lastUpdate = 0; settings = audio::Defaults(); }
const audio::HostApi *AudioHost() { return closed ? nullptr : &host; }
void UpdateAudioReaction(IzsMatrix *matrix) {
    if(!matrix) return;
    const auto snapshot = capture.Read();
    const ULONGLONG now = GetTickCount64();
    const double seconds = lastUpdate ? (now-lastUpdate)/1000.0 : 0;
    lastUpdate = now;
    const auto reaction = response.Update(settings,snapshot.signal,!closed && snapshot.state == audio::Capturing,seconds);
    const auto &coefficients = reaction.colors;
    // Only the rendering/UI thread touches the COM engine. Fractional values remain intact.
    matrix->SetCoeffR1(coefficients.scale[0]); matrix->SetCoeffG1(coefficients.scale[1]); matrix->SetCoeffB1(coefficients.scale[2]);
    matrix->SetCoeffR0(coefficients.offset[0]); matrix->SetCoeffG0(coefficients.offset[1]); matrix->SetCoeffB0(coefficients.offset[2]);
    ApplyAudioMotion(*matrix,reaction.speed,reaction.spawn);
}
