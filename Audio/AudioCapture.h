#pragma once
#include "AudioAnalysis.h"
#include "AudioSettings.h"
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace audio {
struct Device { std::wstring id, name; };
HRESULT PlaybackDevices(std::vector<Device> &devices);
struct Snapshot {
    State state = Disabled;
    HRESULT error = S_OK;
    Descriptors signal;
    ULONGLONG updated = 0;
};
class Capture {
public:
    ~Capture();
    DWORD Start(const wchar_t *deviceId);
    void Stop();
    Snapshot Read();
private:
    std::mutex mutex_;
    Snapshot snapshot_;
    HANDLE stop_ = nullptr;
    std::thread thread_;
    void Publish(State state, HRESULT error, Descriptors signal = {});
    void Run(std::wstring deviceId);
    HRESULT Session(const std::wstring &deviceId);
    bool Wait(DWORD milliseconds);
};
}
