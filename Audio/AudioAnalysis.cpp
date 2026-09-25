#include "AudioAnalysis.h"
#include <ks.h>
#include <ksmedia.h>
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstring>
#include <cstdint>

namespace audio {
double SilenceDetector::Update(double level, ULONGLONG now) {
    // Roughly -70 dBFS to enter silence, -64 dBFS to leave it. The gap prevents
    // noise around the threshold from repeatedly restarting the delay.
    if(level >= 0.0006) quiet_ = false;
    else if(level <= 0.0003 && !quiet_) { quiet_ = true; since_ = now; }
    if(now < since_) since_ = now;
    return quiet_ ? (now-since_)/1000.0 : 0;
}
bool PcmFormat::Read(const WAVEFORMATEX &f) {
    unsigned tag = f.wFormatTag, valid = f.wBitsPerSample;
    if(tag == WAVE_FORMAT_EXTENSIBLE) {
        if(f.cbSize < sizeof(WAVEFORMATEXTENSIBLE)-sizeof(WAVEFORMATEX)) return false;
        const auto &extended = reinterpret_cast<const WAVEFORMATEXTENSIBLE &>(f);
        if(extended.SubFormat == KSDATAFORMAT_SUBTYPE_PCM) tag = WAVE_FORMAT_PCM;
        else if(extended.SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT) tag = WAVE_FORMAT_IEEE_FLOAT;
        else return false;
        valid = extended.Samples.wValidBitsPerSample;
    }
    const bool isFloat = tag == WAVE_FORMAT_IEEE_FLOAT;
    if(tag != WAVE_FORMAT_PCM && !isFloat) return false;
    if(!f.nChannels || f.nChannels > 32 || f.nSamplesPerSec < 8000 || f.nSamplesPerSec > 192000) return false;
    if(isFloat ? (f.wBitsPerSample != 32 && f.wBitsPerSample != 64) :
        (f.wBitsPerSample != 8 && f.wBitsPerSample != 16 && f.wBitsPerSample != 24 && f.wBitsPerSample != 32)) return false;
    if(!valid || valid > f.wBitsPerSample || (isFloat && valid != f.wBitsPerSample) ||
       f.nBlockAlign != f.nChannels*(f.wBitsPerSample/8)) return false;
    channels = f.nChannels; rate = f.nSamplesPerSec; bytes = f.wBitsPerSample/8;
    validBits = valid; floating = isFloat;
    return true;
}
float PcmFormat::Sample(const BYTE *data) const {
    double value;
    if(floating) {
        if(bytes == 4) { float f; memcpy(&f,data,4); value = f; }
        else { double d; memcpy(&d,data,8); value = d; }
    } else if(bytes == 1) value = (int(*data)-128)/128.0;
    else {
        // PCM valid bits are left aligned, including 24 valid bits in a 32-bit container.
        uint32_t bits = 0;
        for(unsigned i = 0; i < bytes; ++i) bits |= uint32_t(data[i]) << (8*i);
        const unsigned shift = 32-bytes*8;
        int32_t signedValue; bits <<= shift; memcpy(&signedValue,&bits,4);
        value = signedValue/2147483648.0;
    }
    return std::isfinite(value) ? static_cast<float>(std::clamp(value,-1.0,1.0)) : 0.0f;
}
Analyzer::Analyzer(const PcmFormat &format, unsigned mask) : format_(format) {
    dcRate_ = 1-std::exp(-6.283185307179586*20/format.rate);
    bassRate_ = 1-std::exp(-6.283185307179586*200/format.rate);
    SetMask(mask);
}
void Analyzer::SetMask(unsigned mask) {
    mask &= AnalyzeAll;
    if(mask == mask_) return;
    if(mask & AnalyzeLegacy) samples_.resize(Capacity*format_.channels);
    else std::vector<float>().swap(samples_);
    if(mask & AnalyzeBass) bassFilters_.resize(format_.channels);
    else std::vector<BassFilter>().swap(bassFilters_);
    if(mask & AnalyzeCentroid) {
        constexpr unsigned size = 2048;
        spectrum_.resize(size); window_.resize(size);
        for(unsigned i = 0; i < size; ++i) window_[i] = 0.5-0.5*std::cos(6.283185307179586*i/(size-1));
        for(unsigned stage = 0, length = 2; stage < 11; ++stage, length *= 2)
            fftSteps_[stage] = std::polar(1.0,-6.283185307179586/length);
    } else {
        std::vector<std::complex<double>>().swap(spectrum_);
        std::vector<double>().swap(window_);
    }
    mask_ = mask; Reset();
}
void Analyzer::Reset() {
    std::fill(samples_.begin(),samples_.end(),0.0f);
    std::fill(bassFilters_.begin(),bassFilters_.end(),BassFilter{});
    cursor_ = available_ = 0; levelEnergy_ = bassEnergy_ = 0; sampleCount_ = 0;
}
void Analyzer::Push(const BYTE *data, unsigned frames, bool silent) {
    if(!mask_) return;
    for(unsigned i = 0; i < frames; ++i) {
        for(unsigned c = 0; c < format_.channels; ++c) {
            const float sample = silent ? 0 : format_.Sample(data+(i*format_.channels+c)*format_.bytes);
            if(mask_ & AnalyzeLegacy) samples_[cursor_*format_.channels+c] = sample;
            if(mask_ & AnalyzeLevel) levelEnergy_ += double(sample)*sample;
            if(mask_ & AnalyzeBass) {
                // Per-channel energy avoids cancellation in opposite-phase stereo.
                auto &filter = bassFilters_[c];
                filter.dc += dcRate_*(sample-filter.dc);
                filter.low += bassRate_*(sample-filter.dc-filter.low);
                filter.low2 += bassRate_*(filter.low-filter.low2);
                bassEnergy_ += filter.low2*filter.low2;
            }
        }
        if(mask_ & AnalyzeLegacy) {
            cursor_ = (cursor_+1)%Capacity;
            available_ = std::min(available_+1,Capacity);
        }
    }
    sampleCount_ += static_cast<unsigned long long>(frames)*format_.channels;
}
float Analyzer::At(unsigned channel, double age) const {
    const unsigned whole = static_cast<unsigned>(age);
    if(whole >= available_) return 0;
    const float a = samples_[((cursor_+Capacity-1-whole)%Capacity)*format_.channels+channel];
    const float b = whole+1 < available_ ? samples_[((cursor_+Capacity-2-whole)%Capacity)*format_.channels+channel] : 0;
    return static_cast<float>(a+(b-a)*(age-whole));
}
static void Fft(std::vector<std::complex<double>> &values, const std::complex<double> *steps) {
    const size_t n = values.size();
    for(size_t i = 1, j = 0; i < n; ++i) {
        size_t bit = n >> 1;
        for(; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if(i < j) std::swap(values[i],values[j]);
    }
    for(size_t length = 2, stage = 0; length <= n; length *= 2, ++stage) {
        const auto step = steps[stage];
        for(size_t i = 0; i < n; i += length) {
            std::complex<double> phase(1,0);
            for(size_t j = 0; j < length/2; ++j) {
                const auto a = values[i+j], b = values[i+j+length/2]*phase;
                values[i+j] = a+b; values[i+j+length/2] = a-b; phase *= step;
            }
        }
    }
}
Descriptors Analyzer::Analyze() {
    Descriptors result;
    if(sampleCount_) {
        result.level = std::sqrt(levelEnergy_/sampleCount_);
        result.bass = std::sqrt(bassEnergy_/sampleCount_);
    }
    levelEnergy_ = bassEnergy_ = 0; sampleCount_ = 0;
    // Stop decaying filter tails well before they reach denormal floating-point values.
    // Silent endpoints may keep delivering zero-filled packets indefinitely.
    for(auto &filter : bassFilters_) {
        if(std::abs(filter.dc) < 1e-20) filter.dc = 0;
        if(std::abs(filter.low) < 1e-20) filter.low = 0;
        if(std::abs(filter.low2) < 1e-20) filter.low2 = 0;
    }
    double weighted = 0, magnitude = 0;
    constexpr unsigned size = 2048;
    for(unsigned c = 0; c < format_.channels; ++c) {
        if(mask_ & AnalyzeWaveform) {
            // Compare signed PCM at a fixed 44.1 kHz reference spacing. Retain fractional
            // samples: byte quantization makes tiny zero crossings look like large jumps.
            // Use only available history so startup padding cannot turn DC into activity.
            const unsigned count = available_ ? std::min(576u,1+(available_-1)*44100/format_.rate) : 0;
            if(count > 1) {
                double previous = At(c,0), total = 0;
                for(unsigned i = 1; i < count; ++i) {
                    const double sample = At(c,double(i)*format_.rate/44100);
                    total += std::abs(sample-previous); previous = sample;
                }
                // Full-scale PCM spans -1..1, so the maximum adjacent difference is 2.
                result.waveformVariation += total/(2*(count-1)*format_.channels);
            }
        }
        if(!(mask_ & AnalyzeCentroid)) continue;
        for(unsigned i = 0; i < size; ++i)
            spectrum_[i] = At(c,size-1-i)*window_[i];
        Fft(spectrum_,fftSteps_);
        // The original Winamp frequency effect used half of a 576-bin spectrum. Keep its band at
        // 0..11.025 kHz, independent of the playback device's native sample rate.
        // Sum magnitudes per channel so opposite-phase stereo does not cancel.
        for(unsigned i = 0; i < size/2; ++i) {
            const double frequency = double(i)*format_.rate/size;
            if(frequency >= 11025) break;
            const double value = std::abs(spectrum_[i]);
            weighted += frequency/11025*value; magnitude += value;
        }
    }
    if(mask_ & AnalyzeWaveform) {
        // A raw difference of 0.02 is a useful musical midpoint, far below alternating
        // full-scale PCM. Expand that range without a running peak or a hard gain clip.
        // The fixed curve preserves 0 and 1; apply it once after averaging channels.
        constexpr double knee = 0.02;
        const double variation = result.waveformVariation;
        result.waveformVariation = (1+knee)*variation/(variation+knee);
    }
    result.spectralCentroid = magnitude > 1e-8 ? weighted/magnitude : 0;
    return result;
}
}
