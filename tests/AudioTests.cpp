// Build with AudioSettings.cpp and AudioAnalysis.cpp; no device or user settings are touched.
#include "../Audio/AudioAnalysis.h"
#include "../Audio/AudioSettings.h"
#include <ks.h>
#include <ksmedia.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <stdexcept>
#include <string>

static void Check(bool value, const char *message) { if(!value) throw std::runtime_error(message); }
static bool Near(double a, double b, double tolerance = 1e-10) { return std::abs(a-b) <= tolerance; }
static audio::PcmFormat Format(unsigned rate, unsigned channels, unsigned bits = 32, bool floating = true) {
    WAVEFORMATEX f = {};
    f.wFormatTag = floating ? WAVE_FORMAT_IEEE_FLOAT : WAVE_FORMAT_PCM;
    f.nChannels = static_cast<WORD>(channels); f.nSamplesPerSec = rate; f.wBitsPerSample = static_cast<WORD>(bits);
    f.nBlockAlign = static_cast<WORD>(channels*bits/8); f.nAvgBytesPerSec = rate*f.nBlockAlign;
    audio::PcmFormat result; Check(result.Read(f),"Supported PCM format rejected."); return result;
}
static audio::Descriptors Tone(unsigned rate, unsigned channels, double frequency, double amplitude, bool reversed = false) {
    auto format = Format(rate,channels); audio::Analyzer analyzer(format);
    std::vector<float> pcm(8192*channels);
    for(unsigned i = 0; i < 8192; ++i) for(unsigned c = 0; c < channels; ++c)
        pcm[i*channels+c] = static_cast<float>(amplitude*std::sin(6.283185307179586*frequency*i/rate)*(reversed && c%2 ? -1 : 1));
    // Exercise arbitrary packet boundaries and ring wraparound.
    analyzer.Push(reinterpret_cast<const BYTE *>(pcm.data()),31,false);
    analyzer.Push(reinterpret_cast<const BYTE *>(pcm.data()+31*channels),8192-31,false);
    return analyzer.Analyze();
}
static void WriteFixture(const wchar_t *path, const char *text) {
    HANDLE file = CreateFileW(path,GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
    Check(file != INVALID_HANDLE_VALUE,"Cannot write fixture.");
    DWORD written; const BOOL ok = WriteFile(file,text,static_cast<DWORD>(strlen(text)),&written,nullptr); CloseHandle(file);
    Check(ok != FALSE,"Fixture write failed.");
}
int wmain() {
    wchar_t temporary[MAX_PATH], folder[MAX_PATH];
    Check(GetTempPathW(MAX_PATH,temporary) && GetTempFileNameW(temporary,L"zma",0,folder),"Cannot create temporary path.");
    DeleteFileW(folder); Check(CreateDirectoryW(folder,nullptr) != FALSE,"Cannot create temporary directory.");
    const std::wstring file = std::wstring(folder)+L"\\audio-\x65e5\x672c.cfg";
    int result = 0;
    try {
        auto settings = audio::Defaults();
        Check(!settings.enabled && settings.mode == audio::WaveformVariation && audio::Valid(settings),"Audio must default to disabled.");
        const auto base = audio::Map(settings.profiles[0],0), peak = audio::Map(settings.profiles[0],1);
        Check(base.scale[0] == 0 && base.offset[1] == 64 && base.offset[2] == 128,"Waveform variation base colors changed.");
        Check(peak.scale[2] == 2 && peak.offset[0] == 128 && peak.offset[1] == 255,"Waveform variation peak colors changed.");
        auto mapping = settings.profiles[1]; mapping.globalScale = 1;
        const auto middle = audio::Map(mapping,0.25);
        Check(middle.scale[0] == 0.5 && middle.offset[0] == 0,"Fractional color coefficients were truncated.");
        Check(audio::Map(mapping,-1).scale[0] == 0 && audio::Map(mapping,2).scale[0] == 2,"Mapping clamp failed.");
        settings.enabled = TRUE; settings.mode = audio::SpectralCentroid;
        wcscpy_s(settings.deviceId,L"{endpoint-\x65e5\x672c}");
        settings.profiles[0].baseScale[1] = 0.375; settings.profiles[1].globalOffset = -0.125;
        Check(audio::Save(file.c_str(),settings) == 0,"Audio settings save failed.");
        auto loaded = audio::Defaults();
        Check(audio::Load(file.c_str(),loaded) == 0 && loaded.enabled && loaded.mode == audio::SpectralCentroid &&
            wcscmp(loaded.deviceId,settings.deviceId) == 0 && loaded.profiles[0].baseScale[1] == 0.375 &&
            loaded.profiles[1].globalOffset == -0.125,"Unicode settings or separate profiles failed to round-trip.");
        settings.enabled = FALSE;
        Check(audio::Save(file.c_str(),settings) == 0 && audio::Load(file.c_str(),loaded) == 0 && !loaded.enabled,"Replacing settings returned stale data.");
        HANDLE locked = CreateFileW(file.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);
        Check(locked != INVALID_HANDLE_VALUE,"Cannot lock settings for failure test.");
        settings.enabled = TRUE;
        const DWORD blocked = audio::Save(file.c_str(),settings); CloseHandle(locked);
        Check(blocked && audio::Load(file.c_str(),loaded) == 0 && !loaded.enabled,"Failed save damaged the existing configuration.");
        WriteFixture(file.c_str(),"[VU Modulate]\r\nBaseColorScales={0.5,1.25,2}\r\nGlobalOffset=-0,125\r\n[Frequency Modulate]\r\nGlobalScale=7.5\r\n");
        loaded = audio::Defaults();
        Check(audio::Load(file.c_str(),loaded,true) == 0 && loaded.profiles[0].baseScale[1] == 1.25 &&
            loaded.profiles[1].globalScale == 7.5 && loaded.profiles[0].globalOffset == -0.125 && !loaded.enabled,"Legacy import failed or enabled capture.");
        const auto imported = loaded;
        for(const auto bad : {"{1,2}","{1,2,3}junk","{1,2,11}","{1,nan,3}","{1,2,inf}"}) {
            WriteFixture(file.c_str(),(std::string("[VU Modulate]\r\nBaseColorScales=")+bad+"\r\n").c_str());
            Check(audio::Load(file.c_str(),loaded,true) == ERROR_INVALID_DATA && loaded.profiles[0].baseScale[1] == imported.profiles[0].baseScale[1],"Invalid legacy import partially applied.");
        }
        auto invalid = settings; invalid.profiles[0].globalScale = std::numeric_limits<double>::quiet_NaN();
        Check(!audio::Valid(invalid) && audio::Save(file.c_str(),invalid) == ERROR_INVALID_DATA,"NaN settings accepted.");
        unsigned char wave[576] = {};
        Check(audio::CalculateWaveformVariation(wave) == 0,"Constant waveform was not silent.");
        for(unsigned i = 0; i < 576; ++i) wave[i] = i%2 ? 255 : 0;
        Check(audio::CalculateWaveformVariation(wave) == 1,"Waveform variation saturation changed.");
        wave[0] = 255; std::fill(wave+1,wave+576,static_cast<unsigned char>(0));
        Check(audio::CalculateWaveformVariation(wave) == 0,"Waveform variation integer division changed.");
        for(unsigned rate : {22050u,44100u,48000u,96000u,192000u}) {
            for(unsigned channels : {1u,2u,6u}) {
                const auto low = Tone(rate,channels,1000,0.5,true), high = Tone(rate,channels,4000,0.5,true);
                Check(Near(low.spectralCentroid,1000.0/11025,0.002) && Near(high.spectralCentroid,4000.0/11025,0.002),"Spectral centroid or sample-rate compensation failed.");
                Check(high.waveformVariation > low.waveformVariation,"Waveform variation lost its frequency sensitivity.");
                const auto quiet = Tone(rate,channels,1000,0.05,true);
                Check(Near(quiet.spectralCentroid,low.spectralCentroid,1e-5),"Spectral centroid depends on amplitude.");
            }
        }
        audio::Analyzer analyzer(Format(48000,2));
        const float notFinite[] = {std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity()};
        analyzer.Push(reinterpret_cast<const BYTE *>(notFinite),1,false);
        Check(analyzer.Analyze().spectralCentroid == 0,"Nonfinite PCM reached the FFT.");
        analyzer.Push(nullptr,8192,true);
        Check(analyzer.Analyze().spectralCentroid == 0 && analyzer.Analyze().waveformVariation == 0,"WASAPI silent packets were not handled.");
        const BYTE minimum32[] = {0,0,0,128}, maximum24[] = {255,255,127}, minimum16[] = {0,128}, zero8[] = {128};
        Check(Format(44100,1,32,false).Sample(minimum32) == -1 && Near(Format(44100,1,24,false).Sample(maximum24),1,1e-6) &&
            Format(44100,1,16,false).Sample(minimum16) == -1 && Format(44100,1,8,false).Sample(zero8) == 0,"Integer PCM conversion failed.");
        WAVEFORMATEXTENSIBLE extensible = {};
        extensible.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE; extensible.Format.cbSize = sizeof(extensible)-sizeof(WAVEFORMATEX);
        extensible.Format.nChannels = 6; extensible.Format.nSamplesPerSec = 48000;
        extensible.Format.wBitsPerSample = 32; extensible.Format.nBlockAlign = 24;
        extensible.Samples.wValidBitsPerSample = 24; extensible.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
        audio::PcmFormat format;
        Check(format.Read(extensible.Format) && format.Sample(minimum32) == -1,"Extensible 24-in-32 PCM failed.");
        extensible.Format.nBlockAlign = 12;
        Check(!format.Read(extensible.Format),"Malformed frame alignment accepted.");
        puts("PASS: Audio defaults, mapping, Unicode/atomic persistence, legacy import validation, PCM, silence, multichannel FFT and sample rates.");
    } catch(const std::exception &error) { fprintf(stderr,"FAIL: %s\n",error.what()); result = 1; }
    DeleteFileW(file.c_str()); RemoveDirectoryW(folder);
    return result;
}
