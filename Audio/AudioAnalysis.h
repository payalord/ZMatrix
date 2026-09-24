#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mmreg.h>
#include <vector>

namespace audio {
// Describes a validated, interleaved WASAPI mix format.
struct PcmFormat {
    unsigned channels = 0, rate = 0, bytes = 0, validBits = 0;
    bool floating = false;
    bool Read(const WAVEFORMATEX &format);
    float Sample(const BYTE *data) const;
};
struct Descriptors { double vu = 0, frequency = 0; };
double LegacyVariation(const unsigned char *samples); // 576 samples, integer division as in WinampVis.
class Analyzer {
public:
    explicit Analyzer(const PcmFormat &format);
    void Reset();
    void Push(const BYTE *data, unsigned frames, bool silent);
    Descriptors Analyze() const;
private:
    static constexpr unsigned Capacity = 4096;
    PcmFormat format_;
    std::vector<float> samples_;
    unsigned cursor_ = 0, available_ = 0;
    float At(unsigned channel, double age) const;
};
}
