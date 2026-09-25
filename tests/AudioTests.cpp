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
static audio::Descriptors Tone(unsigned rate, unsigned channels, double frequency, double amplitude, bool reversed = false, unsigned mask = audio::AnalyzeLegacy, double dc = 0) {
    auto format = Format(rate,channels); audio::Analyzer analyzer(format,mask);
    std::vector<float> pcm(8192*channels);
    for(unsigned i = 0; i < 8192; ++i) for(unsigned c = 0; c < channels; ++c)
        pcm[i*channels+c] = static_cast<float>(dc+amplitude*std::sin(6.283185307179586*frequency*i/rate)*(reversed && c%2 ? -1 : 1));
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
        const auto &waveform = settings.profiles[audio::WaveformVariation];
        const auto &centroid = settings.profiles[audio::SpectralCentroid];
        for(int c = 0; c < 3; ++c)
            Check(waveform.baseScale[c] == 0 && waveform.peakScale[c] == 2 &&
                centroid.baseScale[c] == 0 && centroid.peakScale[c] == 2 &&
                centroid.baseOffset[c] == 0 && centroid.peakOffset[c] == 0,"Original color scales or centroid offsets changed.");
        Check(waveform.baseOffset[0] == 0 && waveform.baseOffset[1] == 64 && waveform.baseOffset[2] == 128 &&
            waveform.peakOffset[0] == 128 && waveform.peakOffset[1] == 255 && waveform.peakOffset[2] == 255 &&
            waveform.globalScale == 3 && waveform.globalOffset == -0.3 && centroid.globalScale == 5 && centroid.globalOffset == 0,
            "Original color offsets or response ranges changed.");
        auto mapping = settings.profiles[1]; mapping.globalScale = 1;
        for(int c = 0; c < 3; ++c) { mapping.baseScale[c] = 0; mapping.peakScale[c] = 2; mapping.peakOffset[c] = 0; }
        const auto middle = audio::Map(mapping,0.25);
        Check(middle.scale[0] == 0.5 && middle.offset[0] == 0,"Fractional color coefficients were truncated.");
        Check(audio::Map(mapping,-1).scale[0] == 0 && audio::Map(mapping,2).scale[0] == 2,"Mapping clamp failed.");
        settings.enabled = TRUE; settings.mode = audio::SpectralCentroid;
        wcscpy_s(settings.deviceId,L"{endpoint-\x65e5\x672c}");
        settings.profiles[0].baseScale[1] = 0.375; settings.profiles[1].globalOffset = -0.125;
        settings.responseSource = audio::BassEnergy; settings.speedEnabled = TRUE; settings.spawnEnabled = TRUE;
        settings.brightnessEnabled = FALSE; settings.colorEnabled = TRUE;
        settings.sensitivity = 6.5; settings.smoothing = 0.625;
        settings.brightnessStrength = 0.8; settings.speedStrength = 0.375; settings.spawnStrength = 0.125;
        Check(audio::Save(file.c_str(),settings) == 0,"Audio settings save failed.");
        auto loaded = audio::Defaults();
        Check(audio::Load(file.c_str(),loaded) == 0 && loaded.enabled && loaded.mode == audio::SpectralCentroid &&
            wcscmp(loaded.deviceId,settings.deviceId) == 0 && loaded.profiles[0].baseScale[1] == 0.375 &&
            loaded.profiles[1].globalOffset == -0.125,"Unicode settings or separate profiles failed to round-trip.");
        for(int mode = 0; mode < audio::ModeCount; ++mode) {
            const auto &before = settings.profiles[mode], &after = loaded.profiles[mode];
            Check(before.globalScale == after.globalScale && before.globalOffset == after.globalOffset,"Saved response mapping was replaced.");
            for(int c = 0; c < 3; ++c)
                Check(before.baseScale[c] == after.baseScale[c] && before.baseOffset[c] == after.baseOffset[c] &&
                    before.peakScale[c] == after.peakScale[c] && before.peakOffset[c] == after.peakOffset[c],"Saved RGB mapping was replaced.");
        }
        Check(loaded.responseSource == audio::BassEnergy && loaded.speedEnabled && loaded.spawnEnabled && !loaded.brightnessEnabled &&
            loaded.colorEnabled && loaded.sensitivity == 6.5 && loaded.smoothing == 0.625 && loaded.brightnessStrength == 0.8 &&
            loaded.speedStrength == 0.375 && loaded.spawnStrength == 0.125,"Independent response settings did not round-trip.");
        Check(WritePrivateProfileStringW(L"Audio",L"Version",L"1",file.c_str()) &&
            WritePrivateProfileStringW(L"Reaction",nullptr,nullptr,file.c_str()),"Cannot create version 1 fixture.");
        Check(audio::Load(file.c_str(),loaded) == 0 && loaded.enabled && loaded.colorEnabled && !loaded.brightnessEnabled &&
            !loaded.speedEnabled && !loaded.spawnEnabled && loaded.smoothing == 0 && loaded.profiles[0].baseScale[1] == 0.375,
            "Version 1 migration changed saved mappings or enabled new influences.");
        Check(audio::Save(file.c_str(),settings) == 0,"Cannot restore version 2 fixture.");
        for(const auto bad : {L"nan",L"1.001",L"-0.1",L"0.5junk"}) {
            Check(WritePrivateProfileStringW(L"Reaction",L"SpeedStrength",bad,file.c_str()) != FALSE,"Cannot corrupt fixture.");
            const auto previous = loaded;
            Check(audio::Load(file.c_str(),loaded) == ERROR_INVALID_DATA && loaded.speedStrength == previous.speedStrength &&
                loaded.colorEnabled == previous.colorEnabled,"Invalid reaction settings partially applied.");
        }
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
        // Zero-crossing regression: reducing amplitude must reduce the response at every frequency.
        for(unsigned rate : {8000u,22050u,44100u,48000u,96000u,192000u}) for(unsigned channels : {1u,2u,6u}) {
            for(double frequency : {80.0,1000.0,3000.0,8000.0}) {
                if(frequency >= rate/2.0) continue;
                const auto loud = Tone(rate,channels,frequency,0.5,true,audio::AnalyzeWaveform);
                const auto quiet = Tone(rate,channels,frequency,0.05,true,audio::AnalyzeWaveform);
                const auto tiny = Tone(rate,channels,frequency,1e-6,true,audio::AnalyzeWaveform);
                Check(loud.waveformVariation > 0 && loud.waveformVariation <= 1 && loud.level == 0 && loud.spectralCentroid == 0,
                    "Waveform-only analysis has an invalid response or runs unrelated analysis.");
                Check(quiet.waveformVariation > tiny.waveformVariation && quiet.waveformVariation < loud.waveformVariation &&
                    tiny.waveformVariation > 0 && tiny.waveformVariation < 0.0001,
                    "Tiny zero crossings cause a false response or quantization loses amplitude changes.");
                const auto inPhase = Tone(rate,channels,frequency,0.5,false,audio::AnalyzeWaveform);
                Check(Near(inPhase.waveformVariation,loud.waveformVariation),"Waveform response depends on channel polarity.");
            }
            const auto reference = Tone(44100,channels,1000,0.5,false,audio::AnalyzeWaveform);
            const auto native = Tone(rate,channels,1000,0.5,false,audio::AnalyzeWaveform);
            Check(std::abs(native.waveformVariation/reference.waveformVariation-1) < 0.08,"Waveform reference spacing changed with the device sample rate.");
            const auto shifted = Tone(rate,channels,1000,0.5,false,audio::AnalyzeWaveform,0.2);
            Check(Near(shifted.waveformVariation,native.waveformVariation,1e-6),"A DC offset changed waveform variation.");
            audio::Analyzer dcAnalyzer(Format(rate,channels),audio::AnalyzeWaveform);
            std::vector<float> dcPacket(4096*channels,0.25f);
            for(unsigned frames : {1u,7u,100u,4096u}) {
                dcAnalyzer.Push(reinterpret_cast<const BYTE *>(dcPacket.data()),frames,false);
                Check(dcAnalyzer.Analyze().waveformVariation == 0,"Startup padding or ring wrap turned DC into activity.");
            }
            dcAnalyzer.Reset(); dcAnalyzer.Push(nullptr,4096,true);
            Check(dcAnalyzer.Analyze().waveformVariation == 0,"Silent packets left a waveform response.");
        }
        audio::Analyzer alternating(Format(44100,1),audio::AnalyzeWaveform);
        std::vector<float> extremes(8192);
        for(unsigned i = 0; i < extremes.size(); ++i) extremes[i] = i%2 ? 1.0f : -1.0f;
        alternating.Push(reinterpret_cast<const BYTE *>(extremes.data()),static_cast<unsigned>(extremes.size()),false);
        Check(alternating.Analyze().waveformVariation == 1,"Full-scale alternating PCM must define the waveform range.");
        alternating.SetMask(0); alternating.SetMask(audio::AnalyzeWaveform);
        Check(alternating.Analyze().waveformVariation == 0,"Switching waveform analysis reused stale samples.");
        for(unsigned rate : {22050u,44100u,48000u,96000u,192000u}) {
            for(unsigned channels : {1u,2u,6u}) {
                const auto low = Tone(rate,channels,1000,0.5,true), high = Tone(rate,channels,4000,0.5,true);
                Check(Near(low.spectralCentroid,1000.0/11025,0.002) && Near(high.spectralCentroid,4000.0/11025,0.002),"Spectral centroid or sample-rate compensation failed.");
                Check(high.waveformVariation > low.waveformVariation,"Waveform variation lost its frequency sensitivity.");
                const auto quiet = Tone(rate,channels,1000,0.05,true);
                Check(Near(quiet.spectralCentroid,low.spectralCentroid,1e-5),"Spectral centroid depends on amplitude.");
            }
        }
        for(unsigned rate : {8000u,44100u,48000u,192000u}) for(unsigned channels : {1u,2u,6u}) {
            const auto low = Tone(rate,channels,80,0.5,true,audio::AnalyzeAll);
            const auto high = Tone(rate,channels,3000,0.5,true,audio::AnalyzeAll);
            const auto quiet = Tone(rate,channels,80,0.05,true,audio::AnalyzeAll);
            Check(Near(low.level,0.5/std::sqrt(2.0),0.004) && Near(high.level,low.level,0.004),"RMS depends on frequency, sample rate or channel phase.");
            Check(Near(quiet.level,low.level*0.1,1e-7) && Near(quiet.bass,low.bass*0.1,1e-7),"Level/bass response does not follow amplitude.");
            Check(low.bass > high.bass*20 && low.bass > low.level*0.7,"Bass filtering did not isolate low-frequency energy.");
        }
        audio::Analyzer levelOnly(Format(48000,1),audio::AnalyzeLevel);
        std::vector<float> impulse(4800,0); impulse[0] = 1;
        levelOnly.Push(reinterpret_cast<const BYTE *>(impulse.data()),4800,false);
        auto measured = levelOnly.Analyze();
        Check(Near(measured.level,std::sqrt(1.0/4800)) && measured.spectralCentroid == 0 && measured.waveformVariation == 0 && measured.bass == 0,
            "RMS lost an early transient or calculated an unrequested descriptor.");
        Check(levelOnly.Analyze().level == 0,"An analysis window reused old energy.");
        levelOnly.SetMask(audio::AnalyzeCentroid);
        Check(levelOnly.Analyze().spectralCentroid == 0,"A changed analysis mask retained stale samples.");
        levelOnly.SetMask(audio::AnalyzeLevel | audio::AnalyzeBass);
        std::vector<float> dc(48000,0.25f);
        levelOnly.Push(reinterpret_cast<const BYTE *>(dc.data()),48000,false); levelOnly.Analyze();
        levelOnly.Push(reinterpret_cast<const BYTE *>(dc.data()),48000,false); measured = levelOnly.Analyze();
        Check(measured.bass < 1e-6 && Near(measured.level,0.25),"DC offset was mistaken for bass.");
        std::fill(dc.begin(),dc.end(),0.0f);
        for(int i = 0; i < 200; ++i) {
            levelOnly.Push(reinterpret_cast<const BYTE *>(dc.data()),2400,false);
            measured = levelOnly.Analyze();
        }
        Check(measured.bass == 0 && measured.level == 0,"Continuous digital silence retained filter tails.");
        levelOnly.Reset(); levelOnly.Push(nullptr,4800,true); measured = levelOnly.Analyze();
        Check(measured.bass == 0 && measured.level == 0,"Reset/silent packets retained level or bass energy.");
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
