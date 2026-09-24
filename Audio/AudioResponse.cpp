#include "AudioResponse.h"
#include <algorithm>
#include <cmath>

namespace audio {
unsigned RequiredAnalysis(const Settings &s) {
    if(!s.enabled) return 0;
    unsigned mask = 0;
    if((s.brightnessEnabled && s.brightnessStrength > 0) || (s.speedEnabled && s.speedStrength > 0) ||
       (s.spawnEnabled && s.spawnStrength > 0))
        mask |= AnalyzeLevel | (s.responseSource == BassEnergy ? AnalyzeBass : 0);
    if(s.colorEnabled) mask |= s.mode == WaveformVariation ? AnalyzeWaveform : AnalyzeCentroid;
    return mask;
}
double ResponseLevel(const Settings &s, const Descriptors &signal) {
    return std::clamp((s.responseSource == BassEnergy ? signal.bass : signal.level)*s.sensitivity,0.0,1.0);
}
static double Follow(double current, double target, double seconds, double attack, double release) {
    const double time = target > current ? attack : release;
    const double value = time > 0 ? target+(current-target)*std::exp(-seconds/time) : target;
    return std::abs(value-target) < 0.00001 ? target : value;
}
void Response::Reset() { level_ = motion_ = activity_ = color_ = 0; source_ = SourceCount; mode_ = ModeCount; }
Reaction Response::Update(const Settings &s, const Descriptors &signal, bool capturing, double seconds) {
    Reaction result;
    if(!capturing || !RequiredAnalysis(s)) { Reset(); return result; }
    seconds = std::max(seconds,0.0);
    if(source_ != s.responseSource) { level_ = motion_ = activity_ = 0; source_ = s.responseSource; }
    const double target = ResponseLevel(s,signal);
    level_ = Follow(level_,target,seconds,0.015+0.085*s.smoothing,0.1+0.7*s.smoothing);
    motion_ = Follow(motion_,target,seconds,0.1+0.2*s.smoothing,0.35+0.65*s.smoothing);
    // A soft silence gate restores ordinary brightness and motion between tracks.
    // RMS is measured before sensitivity, so a gain adjustment does not move the noise floor.
    const double activity = std::clamp((signal.level-0.0003)/0.003,0.0,1.0);
    activity_ = Follow(activity_,activity,seconds,0.1,0.35+0.65*s.smoothing);
    if(s.colorEnabled) {
        const auto &mapping = s.profiles[s.mode];
        const double descriptor = s.mode == WaveformVariation ? signal.waveformVariation : signal.spectralCentroid;
        const double mapped = std::clamp(descriptor*mapping.globalScale+mapping.globalOffset,0.0,1.0);
        if(mode_ != s.mode) { color_ = mapped; mode_ = s.mode; }
        color_ = Follow(color_,mapped,seconds,0.1*s.smoothing,0.8*s.smoothing);
        // Map once after smoothing the normalized response, including the legacy offset.
        auto normalized = mapping; normalized.globalScale = 1; normalized.globalOffset = 0;
        result.colors = Map(normalized,color_);
    } else mode_ = ModeCount;
    if(s.brightnessEnabled) {
        // Equal RGB scaling preserves hue and avoids clipping above the chosen palette.
        // Loud passages approach the original brightness; quiet passages dim it gently.
        const double brightness = 1-0.75*s.brightnessStrength*activity_*(1-level_);
        for(int c = 0; c < 3; ++c) { result.colors.scale[c] *= brightness; result.colors.offset[c] *= brightness; }
    }
    if(s.speedEnabled) result.speed = 1+s.speedStrength*activity_*motion_;
    if(s.spawnEnabled) result.spawn = 1+s.spawnStrength*activity_*(2*motion_-1);
    return result;
}
}