// Deterministic capture substitute and the real audio host/COM engine.
// Build with AudioRuntime.cpp, AudioSettings.cpp and AudioResponse.cpp (not AudioCapture.cpp).
// Run from the repository root; no playback device or installed settings are touched.
#define NOMINMAX
#include "../AudioRuntime.h"
#include "../Audio/AudioCapture.h"
#include "../zsMatrix/IzsMatrix.h"
#include <cstdio>
#include <stdexcept>
#include <string>

namespace audio {
static Snapshot input;
static unsigned observedMask;
Capture::~Capture() = default;
DWORD Capture::Start(const wchar_t *, unsigned mask) { SetAnalysisMask(mask); input = {}; input.state = Capturing; return ERROR_SUCCESS; }
void Capture::Stop() { input = {}; }
Snapshot Capture::Read() { observedMask = mask_.load(); return input; }
}
static void Check(bool ok, const char *message) { if(!ok) throw std::runtime_error(message); }
int wmain() {
    CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
    wchar_t temporary[MAX_PATH], folder[MAX_PATH];
    if(!GetTempPathW(MAX_PATH,temporary) || !GetTempFileNameW(temporary,L"zms",0,folder)) return 1;
    DeleteFileW(folder); if(!CreateDirectoryW(folder,nullptr)) return 1;
    HMODULE engine = LoadLibraryW(L".\\zsMatrix.dll");
    IClassFactory *factory = nullptr; IzsMatrix *matrix = nullptr;
    int result = 0;
    try {
        Check(engine != nullptr,"Cannot load engine.");
        using GetFactory = HRESULT(STDAPICALLTYPE *)(REFCLSID,REFIID,void **);
        auto getFactory = reinterpret_cast<GetFactory>(GetProcAddress(engine,"DllGetClassObject"));
        const GUID cls = {0x2e393599,0xf2e3,0x484a,{0x9b,0x03,0xd4,0x14,0xf6,0xcb,0xa5,0xeb}};
        const GUID iid = {0xcbeac95f,0x3125,0x4603,{0xa0,0xc9,0x09,0x29,0xbd,0xc6,0x0f,0xbc}};
        Check(getFactory && SUCCEEDED(getFactory(cls,IID_IClassFactory,reinterpret_cast<void **>(&factory))) &&
            SUCCEEDED(factory->CreateInstance(nullptr,iid,reinterpret_cast<void **>(&matrix))),"Cannot create engine.");
        InitializeAudio(folder); const auto host = AudioHost();
        audio::Settings settings; host->get(host->context,&settings);
        Check(!settings.enabled && settings.returnOnSilence && settings.silenceDelaySeconds == 5,"New audio host lacks default silence return.");
        settings.enabled = settings.colorEnabled = TRUE;
        settings.brightnessEnabled = FALSE; settings.smoothing = 0; settings.mode = audio::SpectralCentroid;
        settings.profiles[1].globalScale = 0; settings.profiles[1].globalOffset = 0.25;
        settings.profiles[1].peakOffset[2] = 99;
        Check(host->preview(host->context,&settings) == 0,"Cannot preview silence settings.");
        audio::input.signal.level = 0.1;
        UpdateAudioReaction(matrix);
        Check(audio::observedMask == (audio::AnalyzeLevel | audio::AnalyzeCentroid) && matrix->GetCoeffR1() == 0.5,
            "Color-only silence detection lacks level analysis or changed the active mapping.");
        auto status = [&]() { audio::Status value = {}; host->status(host->context,&value); return value; };
        audio::input.signal = {}; audio::input.silenceSeconds = 4.9;
        UpdateAudioReaction(matrix);
        Check(!status().waitingForSound && matrix->GetCoeffR1() == 0.5,"Short silence bypassed color modulation.");
        audio::input.silenceSeconds = 6;
        Sleep(350); UpdateAudioReaction(matrix);
        Check(status().state == audio::Capturing && status().waitingForSound && matrix->GetCoeffR1() == 1 && matrix->GetCoeffB0() == 0,
            "The capture silence interval did not reach the engine or waiting status.");
        audio::Settings current; host->get(host->context,&current);
        Check(current.enabled && current.returnOnSilence,"Automatic waiting changed saved switches.");
        audio::input.signal.level = 0.1; audio::input.silenceSeconds = 0;
        Sleep(350); UpdateAudioReaction(matrix);
        Check(!status().waitingForSound && matrix->GetCoeffR1() == 0.5 && matrix->GetCoeffB0() == 24.75,
            "Sound did not restore the configured audio colors.");
        audio::input.silenceSeconds = 10; Sleep(350); UpdateAudioReaction(matrix);
        Check(status().waitingForSound,"Cannot enter waiting state again.");
        settings.returnOnSilence = FALSE;
        Check(host->preview(host->context,&settings) == 0 && !status().waitingForSound,"Disabling silence return left stale status.");
        Sleep(350); UpdateAudioReaction(matrix);
        Check(matrix->GetCoeffR1() == 0.5,"Disabling silence return left neutral coefficients.");
        settings.returnOnSilence = TRUE; host->preview(host->context,&settings);
        Sleep(350); UpdateAudioReaction(matrix);
        wcscpy_s(settings.deviceId,L"test-playback-output");
        Check(host->preview(host->context,&settings) == 0 && !status().waitingForSound,"Changing outputs retained waiting state.");
        audio::input = {audio::Unavailable,E_FAIL}; UpdateAudioReaction(matrix);
        Check(!status().waitingForSound && matrix->GetCoeffR1() == 1,"Capture failure was reported as silence waiting.");
        settings.enabled = FALSE; host->preview(host->context,&settings); UpdateAudioReaction(matrix);
        Check(status().state == audio::Disabled && !status().waitingForSound && matrix->GetCoeffR1() == 1,"Manual disable retained silence state.");
        puts("PASS: Capture silence transport, color-only analysis, neutral engine coefficients, status, resume, output changes and manual disable.");
    } catch(const std::exception &error) { fprintf(stderr,"FAIL: %s\n",error.what()); result = 1; }
    ShutdownAudio();
    if(matrix) matrix->Release(); if(factory) factory->Release(); if(engine) FreeLibrary(engine);
    DeleteFileW((std::wstring(folder)+L"\\Audio.cfg").c_str()); RemoveDirectoryW(folder);
    CoUninitialize(); return result;
}
