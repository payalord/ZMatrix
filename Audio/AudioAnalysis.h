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
// Both responses are normalized to 0..1; spectralCentroid is relative to 11.025 kHz.
struct Descriptors { double waveformVariation = 0, spectralCentroid = 0; };
// Preserves the original Winamp VU effect: 576 samples with integer division.
double CalculateWaveformVariation(const unsigned char *samples);
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
