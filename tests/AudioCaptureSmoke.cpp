// Opt-in device integration test. Plays a quiet two-second tone on the default
// multimedia output; captures only in memory and never changes endpoint settings.
#include "../Audio/AudioCapture.h"
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <wrl/client.h>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <stdexcept>

using Microsoft::WRL::ComPtr;
static void Check(bool ok, const char *message) { if(!ok) throw std::runtime_error(message); }
static void Success(HRESULT hr, const char *message) {
    if(FAILED(hr)) { fprintf(stderr,"HRESULT: 0x%08lX\n",static_cast<unsigned long>(hr)); throw std::runtime_error(message); }
}
static void Encode(BYTE *out, const audio::PcmFormat &format, double value) {
    if(format.floating) {
        if(format.bytes == 4) { const float sample = static_cast<float>(value); memcpy(out,&sample,4); }
        else memcpy(out,&value,8);
    } else if(format.bytes == 1) *out = static_cast<BYTE>(128+value*127);
    else {
        const int sample = static_cast<int>(value*2147483647);
        const unsigned bits = static_cast<unsigned>(sample) >> (32-format.bytes*8);
        for(unsigned b = 0; b < format.bytes; ++b) out[b] = static_cast<BYTE>(bits >> (8*b));
    }
}
int wmain(int argc, wchar_t **argv) {
    if(argc != 2 || wcscmp(argv[1],L"--play-test-tone")) {
        puts("Usage: AudioCaptureSmoke.exe --play-test-tone"); return 2;
    }
    const HRESULT initialized = CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
    if(FAILED(initialized)) return 1;
    int result = 0;
    {
        audio::Capture capture;
        ComPtr<IAudioClient> output;
        WAVEFORMATEX *mix = nullptr;
        try {
            Check(capture.Read().state == audio::Disabled,"Capture did not start disabled.");
            Check(capture.Start(L"ZMatrix-nonexistent-test-endpoint") == 0,"Cannot start failure/retry test.");
            Sleep(300);
            Check(capture.Read().state == audio::Unavailable,"Invalid output did not become unavailable.");
            const auto stopped = GetTickCount64(); capture.Stop();
            Check(GetTickCount64()-stopped < 1000 && capture.Read().state == audio::Disabled,"Stopping retry did not finish promptly.");
            std::vector<audio::Device> devices;
            Success(audio::PlaybackDevices(devices),"Cannot enumerate playback devices.");
            Check(!devices.empty(),"No playback device available for this integration test.");
            printf("Active playback devices: %zu\n",devices.size());
            ComPtr<IMMDeviceEnumerator> enumerator;
            Success(CoCreateInstance(__uuidof(MMDeviceEnumerator),nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&enumerator)),"Cannot create endpoint enumerator.");
            ComPtr<IMMDevice> device;
            Success(enumerator->GetDefaultAudioEndpoint(eRender,eMultimedia,&device),"Default multimedia output unavailable.");
            Success(device->Activate(__uuidof(IAudioClient),CLSCTX_INPROC_SERVER,nullptr,&output),"Cannot open playback output.");
            Success(output->GetMixFormat(&mix),"Cannot get mix format.");
            audio::PcmFormat format; Check(format.Read(*mix),"Unsupported test output format.");
            printf("Mix format: %u Hz, %u channels, %u-bit %s\n",format.rate,format.channels,format.bytes*8,format.floating ? "float" : "PCM");
            Success(output->Initialize(AUDCLNT_SHAREMODE_SHARED,0,1000000,0,mix,nullptr),"Cannot initialize playback client.");
            ComPtr<IAudioRenderClient> render;
            Success(output->GetService(IID_PPV_ARGS(&render)),"Cannot get render service.");
            UINT32 capacity = 0; Success(output->GetBufferSize(&capacity),"Cannot get render buffer size.");
            Check(capture.Start(L"") == 0,"Cannot start default output capture.");
            Success(output->Start(),"Cannot start test playback.");
            unsigned long long frame = 0;
            bool observed = false;
            double strongest = 0;
            const ULONGLONG begin = GetTickCount64();
            while(GetTickCount64()-begin < 2000) {
                UINT32 padding = 0; Success(output->GetCurrentPadding(&padding),"Cannot get render padding.");
                const UINT32 count = capacity-padding;
                if(count) {
                    BYTE *data = nullptr; Success(render->GetBuffer(count,&data),"Cannot acquire render packet.");
                    for(UINT32 i = 0; i < count; ++i, ++frame) {
                        const double sample = 0.002*std::sin(6.283185307179586*1000*frame/format.rate);
                        for(unsigned c = 0; c < format.channels; ++c) Encode(data+(i*format.channels+c)*format.bytes,format,sample);
                    }
                    Success(render->ReleaseBuffer(count,0),"Cannot submit render packet.");
                }
                const auto snapshot = capture.Read();
                observed |= snapshot.state == audio::Capturing;
                Check(std::isfinite(snapshot.signal.waveformVariation) && std::isfinite(snapshot.signal.spectralCentroid),"Nonfinite capture descriptor.");
                strongest = std::max(strongest,snapshot.signal.spectralCentroid);
                Sleep(10);
            }
            output->Stop();
            Check(observed && strongest > 0,"Loopback did not receive rendered audio.");
            printf("Observed frequency response: %.6f\n",strongest);
            Sleep(400);
            const auto after = capture.Read();
            Check(after.state == audio::Capturing,"Stopping playback stopped the capture client.");
            printf("After playback: waveform variation %.6f, spectral centroid %.6f (other applications may still be audible).\n",after.signal.waveformVariation,after.signal.spectralCentroid);
            capture.Stop();
            Check(capture.Read().state == audio::Disabled,"Disable did not clear the capture state.");
            puts("PASS: Disabled default, missing device, interruptible retry, recovery, real WASAPI loopback packets and shutdown.");
        } catch(const std::exception &error) { fprintf(stderr,"FAIL: %s\n",error.what()); result = 1; }
        if(output) output->Stop();
        CoTaskMemFree(mix);
    }
    CoUninitialize();
    return result;
}
