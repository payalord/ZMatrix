#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mmreg.h>
#include <vector>
#include <complex>

namespace audio {
// Describes a validated, interleaved WASAPI mix format.
struct PcmFormat {
    unsigned channels = 0, rate = 0, bytes = 0, validBits = 0;
    bool floating = false;
    bool Read(const WAVEFORMATEX &format);
    float Sample(const BYTE *data) const;
};
// RMS values use full-scale PCM; spectralCentroid is relative to 11.025 kHz.
struct Descriptors { double waveformVariation = 0, spectralCentroid = 0, level = 0, bass = 0; };
enum AnalysisMask { AnalyzeWaveform = 1, AnalyzeCentroid = 2, AnalyzeLevel = 4, AnalyzeBass = 8,
    AnalyzeLegacy = AnalyzeWaveform | AnalyzeCentroid, AnalyzeAll = AnalyzeLegacy | AnalyzeLevel | AnalyzeBass };
// Preserves the original Winamp VU effect: 576 samples with integer division.
double CalculateWaveformVariation(const unsigned char *samples);
class Analyzer {
public:
    explicit Analyzer(const PcmFormat &format, unsigned mask = AnalyzeLegacy);
    void SetMask(unsigned mask);
    void Reset();
    void Push(const BYTE *data, unsigned frames, bool silent);
    Descriptors Analyze();
private:
    static constexpr unsigned Capacity = 4096;
    PcmFormat format_;
    std::vector<float> samples_;
    std::vector<std::complex<double>> spectrum_;
    std::vector<double> window_;
    std::complex<double> fftSteps_[11];
    struct BassFilter { double dc = 0, low = 0, low2 = 0; };
    std::vector<BassFilter> bassFilters_;
    unsigned mask_ = 0;
    double dcRate_ = 0, bassRate_ = 0, levelEnergy_ = 0, bassEnergy_ = 0;
    unsigned long long sampleCount_ = 0;
    unsigned cursor_ = 0, available_ = 0;
    float At(unsigned channel, double age) const;
};
}
