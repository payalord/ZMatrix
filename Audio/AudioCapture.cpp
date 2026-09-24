#include "AudioCapture.h"
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <wrl/client.h>
#include <memory>

namespace audio {
using Microsoft::WRL::ComPtr;
struct TaskMemory { void operator()(void *p) const { CoTaskMemFree(p); } };
static HRESULT Enumerator(ComPtr<IMMDeviceEnumerator> &enumerator) {
    return CoCreateInstance(__uuidof(MMDeviceEnumerator),nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&enumerator));
}
static HRESULT DeviceId(IMMDevice *device, std::wstring &id) {
    LPWSTR raw = nullptr;
    const HRESULT hr = device->GetId(&raw);
    std::unique_ptr<wchar_t,TaskMemory> owned(raw);
    if(SUCCEEDED(hr)) id = raw;
    return hr;
}
HRESULT PlaybackDevices(std::vector<Device> &devices) {
    ComPtr<IMMDeviceEnumerator> enumerator;
    HRESULT hr = Enumerator(enumerator); if(FAILED(hr)) return hr;
    ComPtr<IMMDeviceCollection> collection;
    hr = enumerator->EnumAudioEndpoints(eRender,DEVICE_STATE_ACTIVE,&collection); if(FAILED(hr)) return hr;
    UINT count = 0; hr = collection->GetCount(&count); if(FAILED(hr)) return hr;
    std::vector<Device> next;
    for(UINT i = 0; i < count; ++i) {
        ComPtr<IMMDevice> device; Device item;
        if(FAILED(collection->Item(i,&device)) || FAILED(DeviceId(device.Get(),item.id))) continue;
        item.name = item.id;
        ComPtr<IPropertyStore> properties;
        if(SUCCEEDED(device->OpenPropertyStore(STGM_READ,&properties))) {
            PROPVARIANT name; PropVariantInit(&name);
            if(SUCCEEDED(properties->GetValue(PKEY_Device_FriendlyName,&name)) && name.vt == VT_LPWSTR && name.pwszVal)
                item.name = name.pwszVal;
            PropVariantClear(&name);
        }
        next.push_back(std::move(item));
    }
    devices.swap(next);
    return S_OK;
}
Capture::~Capture() { Stop(); }
void Capture::Publish(State state, HRESULT error, Descriptors signal) {
    std::lock_guard<std::mutex> lock(mutex_);
    snapshot_ = {state,error,signal,GetTickCount64()};
}
Snapshot Capture::Read() {
    std::lock_guard<std::mutex> lock(mutex_);
    Snapshot result = snapshot_;
    if(result.state == Capturing && GetTickCount64()-result.updated > 1000)
        result = {Unavailable,HRESULT_FROM_WIN32(ERROR_TIMEOUT),{},GetTickCount64()};
    return result;
}
DWORD Capture::Start(const wchar_t *deviceId) {
    Stop();
    stop_ = CreateEventW(nullptr,TRUE,FALSE,nullptr);
    if(!stop_) return GetLastError();
    Publish(Starting,S_OK);
    try { thread_ = std::thread(&Capture::Run,this,std::wstring(deviceId)); }
    catch(...) { CloseHandle(stop_); stop_ = nullptr; Publish(Unavailable,E_OUTOFMEMORY); return ERROR_NOT_ENOUGH_MEMORY; }
    return ERROR_SUCCESS;
}
void Capture::Stop() {
    if(stop_) {
        SetEvent(stop_);
        if(thread_.joinable()) thread_.join();
        CloseHandle(stop_); stop_ = nullptr;
    }
    Publish(Disabled,S_OK);
}
bool Capture::Wait(DWORD milliseconds) {
    // Pump the worker's STA (also suitable for the first IAudioClient use on Windows 8).
    const ULONGLONG until = GetTickCount64()+milliseconds;
    for(;;) {
        MSG message;
        while(PeekMessageW(&message,nullptr,0,0,PM_REMOVE)) {
            TranslateMessage(&message); DispatchMessageW(&message);
        }
        const ULONGLONG now = GetTickCount64();
        const DWORD left = now < until ? static_cast<DWORD>(until-now) : 0;
        const DWORD result = MsgWaitForMultipleObjects(1,&stop_,FALSE,left,QS_ALLINPUT);
        if(result == WAIT_OBJECT_0 || result == WAIT_FAILED) return true;
        if(result == WAIT_TIMEOUT) return false;
    }
}
void Capture::Run(std::wstring deviceId) {
    const HRESULT initialized = CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
    if(FAILED(initialized)) { Publish(Unavailable,initialized); return; }
    try {
        while(!Wait(0)) {
            const HRESULT hr = Session(deviceId);
            if(Wait(0)) break;
            Publish(Unavailable,FAILED(hr) ? hr : AUDCLNT_E_DEVICE_INVALIDATED);
            if(Wait(1000)) break;
        }
    } catch(...) { Publish(Unavailable,E_OUTOFMEMORY); }
    CoUninitialize();
}
HRESULT Capture::Session(const std::wstring &id) {
    ComPtr<IMMDeviceEnumerator> enumerator;
    HRESULT hr = Enumerator(enumerator); if(FAILED(hr)) return hr;
    ComPtr<IMMDevice> device;
    hr = id.empty() ? enumerator->GetDefaultAudioEndpoint(eRender,eMultimedia,&device) : enumerator->GetDevice(id.c_str(),&device);
    if(FAILED(hr)) return hr;
    std::wstring activeId;
    hr = DeviceId(device.Get(),activeId); if(FAILED(hr)) return hr;
    // Explicit IDs must also designate playback endpoints, never a microphone.
    ComPtr<IMMEndpoint> endpoint;
    hr = device.As(&endpoint); if(FAILED(hr)) return hr;
    EDataFlow flow; hr = endpoint->GetDataFlow(&flow);
    if(FAILED(hr) || flow != eRender) return E_INVALIDARG;
    ComPtr<IAudioClient> client;
    hr = device->Activate(__uuidof(IAudioClient),CLSCTX_INPROC_SERVER,nullptr,&client); if(FAILED(hr)) return hr;
    WAVEFORMATEX *raw = nullptr;
    hr = client->GetMixFormat(&raw);
    std::unique_ptr<WAVEFORMATEX,TaskMemory> mix(raw);
    if(FAILED(hr)) return hr;
    PcmFormat format;
    if(!format.Read(*mix)) return AUDCLNT_E_UNSUPPORTED_FORMAT;
    // Timer-based shared loopback works on Windows 7, including silent endpoints.
    hr = client->Initialize(AUDCLNT_SHAREMODE_SHARED,AUDCLNT_STREAMFLAGS_LOOPBACK,1000000,0,mix.get(),nullptr);
    if(FAILED(hr)) return hr;
    ComPtr<IAudioCaptureClient> capture;
    hr = client->GetService(IID_PPV_ARGS(&capture)); if(FAILED(hr)) return hr;
    Analyzer analyzer(format);
    hr = client->Start(); if(FAILED(hr)) return hr;
    struct StopClient { IAudioClient *p; ~StopClient() { p->Stop(); } } stopClient{client.Get()};
    Publish(Capturing,S_OK);
    ULONGLONG lastPacket = GetTickCount64(), lastAnalysis = 0, lastDeviceCheck = lastPacket;
    bool empty = false;
    while(!Wait(10)) {
        UINT32 frames = 0;
        for(;;) {
            hr = capture->GetNextPacketSize(&frames); if(FAILED(hr)) return hr;
            if(!frames || Wait(0)) break;
            BYTE *data = nullptr; DWORD flags = 0;
            hr = capture->GetBuffer(&data,&frames,&flags,nullptr,nullptr); if(FAILED(hr)) return hr;
            if(flags & AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY) analyzer.Reset();
            analyzer.Push(data,frames,(flags & AUDCLNT_BUFFERFLAGS_SILENT) != 0);
            hr = capture->ReleaseBuffer(frames); if(FAILED(hr)) return hr;
            lastPacket = GetTickCount64(); empty = false;
        }
        const ULONGLONG now = GetTickCount64();
        if(now-lastPacket > 200 && !empty) { analyzer.Reset(); empty = true; }
        if(now-lastAnalysis >= 50) {
            Publish(Capturing,S_OK,empty ? Descriptors{} : analyzer.Analyze());
            lastAnalysis = now;
        }
        if(now-lastDeviceCheck >= 1000) {
            DWORD state = 0;
            hr = device->GetState(&state);
            if(FAILED(hr) || !(state & DEVICE_STATE_ACTIVE)) return AUDCLNT_E_DEVICE_INVALIDATED;
            if(id.empty()) {
                ComPtr<IMMDevice> current; std::wstring currentId;
                hr = enumerator->GetDefaultAudioEndpoint(eRender,eMultimedia,&current);
                if(FAILED(hr)) return hr;
                hr = DeviceId(current.Get(),currentId); if(FAILED(hr)) return hr;
                if(currentId != activeId) return AUDCLNT_E_DEVICE_INVALIDATED;
            }
            lastDeviceCheck = now;
        }
    }
    return S_OK;
}
}
