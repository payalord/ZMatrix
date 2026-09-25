// Run from the repository root with an available playback endpoint. Uses an
// unregistered engine and an isolated temporary configuration directory.
#define NOMINMAX
#include "../AudioRuntime.h"
#include "../zsMatrix/IzsMatrix.h"
#include "../zsMatrix/IzsMatrixMotion.h"
#include <cstdio>
#include <stdexcept>
#include <string>

static void Check(bool ok, const char *message) { if(!ok) throw std::runtime_error(message); }
static bool WaitFor(const audio::HostApi &host, audio::State state) {
    const auto deadline = GetTickCount64()+5000;
    do {
        audio::Status status; host.status(host.context,&status);
        if(status.state == state) return true;
        Sleep(20);
    } while(GetTickCount64() < deadline);
    return false;
}
int wmain() {
    CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
    wchar_t temporary[MAX_PATH], folder[MAX_PATH];
    if(!GetTempPathW(MAX_PATH,temporary) || !GetTempFileNameW(temporary,L"zma",0,folder)) return 1;
    DeleteFileW(folder); if(!CreateDirectoryW(folder,nullptr)) return 1;
    HMODULE engine = LoadLibraryW(L".\\zsMatrix.dll");
    IClassFactory *factory = nullptr; IzsMatrix *matrix = nullptr;
    int result = 0;
    try {
        Check(engine != nullptr,"Cannot load engine.");
        typedef HRESULT(STDAPICALLTYPE *GetFactory)(REFCLSID,REFIID,void **);
        auto getFactory = reinterpret_cast<GetFactory>(GetProcAddress(engine,"DllGetClassObject"));
        const GUID cls = {0x2e393599,0xf2e3,0x484a,{0x9b,0x03,0xd4,0x14,0xf6,0xcb,0xa5,0xeb}};
        const GUID iid = {0xcbeac95f,0x3125,0x4603,{0xa0,0xc9,0x09,0x29,0xbd,0xc6,0x0f,0xbc}};
        Check(getFactory && SUCCEEDED(getFactory(cls,IID_IClassFactory,reinterpret_cast<void **>(&factory))),"Cannot create engine factory.");
        Check(SUCCEEDED(factory->CreateInstance(nullptr,iid,reinterpret_cast<void **>(&matrix))),"Cannot create engine.");
        matrix->SetColor(17,83,201,231);
        InitializeAudio(folder);
        const auto host = AudioHost(); Check(host != nullptr,"Missing audio host.");
        audio::Settings settings; host->get(host->context,&settings);
        Check(!settings.enabled && WaitFor(*host,audio::Disabled),"New configuration started capture.");
        matrix->SetCoeffR1(9.0); UpdateAudioReaction(matrix);
        Check(matrix->GetCoeffR1() == 1 && matrix->GetCoeffB0() == 0,"Disabled audio did not restore ordinary coefficients.");
        settings.enabled = TRUE; settings.mode = audio::SpectralCentroid;
        settings.colorEnabled = TRUE; settings.brightnessEnabled = FALSE; settings.smoothing = 0;
        // A fixed custom mapping tests fractional transport independently of silence or default profiles.
        settings.returnOnSilence = FALSE;
        settings.profiles[1] = {};
        for(int c = 0; c < 3; ++c) settings.profiles[1].peakScale[c] = 2;
        settings.profiles[1].globalScale = 0; settings.profiles[1].globalOffset = 0.25;
        settings.profiles[1].peakOffset[2] = 99;
        Check(host->preview(host->context,&settings) == 0 && WaitFor(*host,audio::Capturing),"Cannot start default loopback.");
        UpdateAudioReaction(matrix);
        Check(matrix->GetCoeffR1() == 0.5 && matrix->GetCoeffG1() == 0.5 && matrix->GetCoeffB0() == 24.75,"Fractional coefficients did not reach the engine intact.");
        BYTE r,g,b,a; matrix->GetColor(r,g,b,a);
        Check(r == 17 && g == 83 && b == 201 && a == 231,"Audio changed the user's base colors.");
        settings.colorEnabled = FALSE;
        Check(host->preview(host->context,&settings) == 0 && WaitFor(*host,audio::Disabled),"Capture kept running with no active influences.");
        ApplyAudioMotion(*matrix,2,0); UpdateAudioReaction(matrix);
        IzsMatrixMotion *motion = nullptr;
        Check(SUCCEEDED(matrix->QueryInterface(IID_IZSMATRIXMOTION,reinterpret_cast<void **>(&motion))),"Missing motion interface.");
        const bool neutral = motion->GetAudioSpeed() == 1 && motion->GetAudioSpawn() == 1;
        motion->Release();
        Check(neutral && matrix->GetCoeffR1() == 1,"Inactive effects left appearance or motion overrides active.");
        settings.brightnessEnabled = TRUE; settings.speedEnabled = TRUE; settings.spawnEnabled = TRUE;
        settings.responseSource = audio::BassEnergy;
        Check(host->preview(host->context,&settings) == 0 && WaitFor(*host,audio::Capturing),"Could not restart with independent bass effects.");
        wcscpy_s(settings.deviceId,L"ZMatrix-nonexistent-test-endpoint");
        Check(host->preview(host->context,&settings) == 0 && WaitFor(*host,audio::Unavailable),"Missing endpoint was not reported.");
        UpdateAudioReaction(matrix);
        Check(matrix->GetCoeffR1() == 1 && matrix->GetCoeffB0() == 0,"Device failure left audio coefficients active.");
        settings.enabled = FALSE;
        Check(host->preview(host->context,&settings) == 0 && host->commit(host->context,&settings) == 0,"Cannot persist audio settings.");
        audio::Settings saved = audio::Defaults();
        const std::wstring file = std::wstring(folder)+L"\\Audio.cfg";
        Check(audio::Load(file.c_str(),saved) == 0 && !saved.enabled && saved.profiles[1].globalOffset == 0.25,"Runtime committed the wrong settings.");
        ShutdownAudio(); settings.enabled = TRUE;
        Check(!AudioHost() && host->preview(host->context,&settings) == ERROR_SHUTDOWN_IN_PROGRESS,"A closing dialog restarted capture after shutdown.");
        puts("PASS: Audio host, direct fractional engine coefficients, base-color preservation, device failure, persistence and shutdown guard.");
    } catch(const std::exception &error) { fprintf(stderr,"FAIL: %s\n",error.what()); result = 1; }
    ShutdownAudio();
    if(matrix) matrix->Release(); if(factory) factory->Release(); if(engine) FreeLibrary(engine);
    DeleteFileW((std::wstring(folder)+L"\\Audio.cfg").c_str()); RemoveDirectoryW(folder);
    CoUninitialize(); return result;
}
