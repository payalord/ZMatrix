#pragma once
#include "AudioAnalysis.h"
#include "AudioSettings.h"
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <atomic>

namespace audio {
struct Device { std::wstring id, name; };
HRESULT PlaybackDevices(std::vector<Device> &devices);
struct Snapshot {
    State state = Disabled;
    HRESULT error = S_OK;
    Descriptors signal;
    ULONGLONG updated = 0;
    double silenceSeconds = 0;
};
class Capture {
public:
    ~Capture();
    DWORD Start(const wchar_t *deviceId, unsigned mask = AnalyzeLegacy);
    void SetAnalysisMask(unsigned mask) { mask_.store(mask); }
    void Stop();
    Snapshot Read();
private:
    std::mutex mutex_;
    Snapshot snapshot_;
    HANDLE stop_ = nullptr;
    std::thread thread_;
    std::atomic<unsigned> mask_{AnalyzeLegacy};
    void Publish(State state, HRESULT error, Descriptors signal = {}, double silenceSeconds = 0);
    void Run(std::wstring deviceId);
    HRESULT Session(const std::wstring &deviceId);
    bool Wait(DWORD milliseconds);
};
}
