#include "AudioAnalysis.h"
#include <ks.h>
#include <ksmedia.h>
#include <algorithm>
#include <cmath>
#include <complex>
#include <cstring>
#include <cstdint>

namespace audio {
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
double CalculateWaveformVariation(const unsigned char *samples) {
    unsigned total = 0;
    for(unsigned i = 1; i < 576; ++i) total += std::abs(int(samples[i])-int(samples[i-1]));
    return std::min(total/288,255u)/255.0;
}
Analyzer::Analyzer(const PcmFormat &format) : format_(format), samples_(Capacity*format.channels) {}
void Analyzer::Reset() { std::fill(samples_.begin(),samples_.end(),0.0f); cursor_ = available_ = 0; }
void Analyzer::Push(const BYTE *data, unsigned frames, bool silent) {
    for(unsigned i = 0; i < frames; ++i) {
        for(unsigned c = 0; c < format_.channels; ++c)
            samples_[cursor_*format_.channels+c] = silent ? 0 : format_.Sample(data+(i*format_.channels+c)*format_.bytes);
        cursor_ = (cursor_+1)%Capacity;
        available_ = std::min(available_+1,Capacity);
    }
}
float Analyzer::At(unsigned channel, double age) const {
    const unsigned whole = static_cast<unsigned>(age);
    if(whole >= available_) return 0;
    const float a = samples_[((cursor_+Capacity-1-whole)%Capacity)*format_.channels+channel];
    const float b = whole+1 < available_ ? samples_[((cursor_+Capacity-2-whole)%Capacity)*format_.channels+channel] : 0;
    return static_cast<float>(a+(b-a)*(age-whole));
}
static void Fft(std::vector<std::complex<double>> &values) {
    const size_t n = values.size();
    for(size_t i = 1, j = 0; i < n; ++i) {
        size_t bit = n >> 1;
        for(; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if(i < j) std::swap(values[i],values[j]);
    }
    for(size_t length = 2; length <= n; length *= 2) {
        const auto step = std::polar(1.0,-6.283185307179586/length);
        for(size_t i = 0; i < n; i += length) {
            std::complex<double> phase(1,0);
            for(size_t j = 0; j < length/2; ++j) {
                const auto a = values[i+j], b = values[i+j+length/2]*phase;
                values[i+j] = a+b; values[i+j+length/2] = a-b; phase *= step;
            }
        }
    }
}
Descriptors Analyzer::Analyze() const {
    Descriptors result;
    double weighted = 0, magnitude = 0;
    constexpr unsigned size = 2048;
    std::vector<std::complex<double>> spectrum(size);
    for(unsigned c = 0; c < format_.channels; ++c) {
        unsigned char waveform[576];
        // A fixed 44.1 kHz reference keeps waveform variation independent of endpoint rate.
        // Winamp waveform bytes contain signed 8-bit PCM in an unsigned array.
        // Preserve the original Winamp VU effect's unsigned differences and zero-crossing jumps.
        // See WACUP/vis_classic, Vis_Satan.cpp, AtAnStDirectRender.
        for(unsigned i = 0; i < 576; ++i)
            waveform[i] = static_cast<unsigned char>(static_cast<int>(std::clamp(
                std::floor(128.0*At(c,(575-i)*format_.rate/44100.0)),-128.0,127.0)));
        result.waveformVariation += CalculateWaveformVariation(waveform)/format_.channels;
        for(unsigned i = 0; i < size; ++i)
            spectrum[i] = At(c,size-1-i)*(0.5-0.5*std::cos(6.283185307179586*i/(size-1)));
        Fft(spectrum);
        // The original Winamp frequency effect used half of a 576-bin spectrum. Keep its band at
        // 0..11.025 kHz, independent of the playback device's native sample rate.
        // Sum magnitudes per channel so opposite-phase stereo does not cancel.
        for(unsigned i = 0; i < size/2; ++i) {
            const double frequency = double(i)*format_.rate/size;
            if(frequency >= 11025) break;
            const double value = std::abs(spectrum[i]);
            weighted += frequency/11025*value; magnitude += value;
        }
    }
    result.spectralCentroid = magnitude > 1e-8 ? weighted/magnitude : 0;
    return result;
}
}
