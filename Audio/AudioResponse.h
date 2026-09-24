#pragma once
#include "AudioAnalysis.h"
#include "AudioSettings.h"

namespace audio {
unsigned RequiredAnalysis(const Settings &settings);
double ResponseLevel(const Settings &settings, const Descriptors &signal);
struct Reaction {
    Coefficients colors = {{1,1,1},{0,0,0}};
    double speed = 1, spawn = 1;
};
// Render-thread envelope state; no buffers or ownership cross the Config.dll ABI.
class Response {
public:
    void Reset();
    Reaction Update(const Settings &settings, const Descriptors &signal, bool capturing, double seconds);
private:
    double level_ = 0, motion_ = 0, activity_ = 0, color_ = 0;
    UINT source_ = SourceCount, mode_ = ModeCount;
};
}