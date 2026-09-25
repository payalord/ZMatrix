// Deterministic envelopes and independent audio influences; no device or windows required.
// Build with AudioSettings.cpp, AudioAnalysis.cpp and AudioResponse.cpp.
#include "../Audio/AudioResponse.h"
#include <cmath>
#include <cstdio>
#include <stdexcept>

static void Check(bool value, const char *message) { if(!value) throw std::runtime_error(message); }
static bool Neutral(const audio::Reaction &r) {
    return r.speed == 1 && r.spawn == 1 && r.colors.scale[0] == 1 && r.colors.scale[1] == 1 &&
        r.colors.scale[2] == 1 && r.colors.offset[0] == 0 && r.colors.offset[1] == 0 && r.colors.offset[2] == 0;
}
static audio::Reaction Settle(audio::Response &response, const audio::Settings &settings, const audio::Descriptors &signal, int frames = 1000, double dt = 0.02) {
    audio::Reaction result;
    for(int i = 0; i < frames; ++i) result = response.Update(settings,signal,true,dt);
    return result;
}
static void CheckWaveformPipeline() {
    WAVEFORMATEX wave = {};
    wave.wFormatTag = WAVE_FORMAT_IEEE_FLOAT; wave.nChannels = 2; wave.nSamplesPerSec = 48000;
    wave.wBitsPerSample = 32; wave.nBlockAlign = 8; wave.nAvgBytesPerSec = 384000;
    audio::PcmFormat format; Check(format.Read(wave),"Cannot create waveform test format.");
    auto tone = [](double frequency, double amplitude) {
        std::vector<float> samples(4800);
        for(unsigned i = 0; i < 2400; ++i) {
            samples[2*i] = static_cast<float>(amplitude*std::sin(6.283185307179586*frequency*i/48000));
            samples[2*i+1] = -samples[2*i];
        }
        return samples;
    };
    const auto tiny = tone(3000,1e-6), quiet = tone(80,0.04), middle = tone(1000,0.2), loud = tone(3000,0.5);
    const std::vector<float> silence(4800,0);
    auto settings = audio::Defaults(); settings.enabled = settings.colorEnabled = TRUE;
    settings.brightnessEnabled = FALSE; settings.smoothing = 0.2;
    // The complete profile used before the signed-PCM fix, including its response threshold.
    auto &profile = settings.profiles[audio::WaveformVariation]; profile = {};
    for(int c = 0; c < 3; ++c) profile.peakScale[c] = 2;
    profile.baseOffset[1] = 64; profile.baseOffset[2] = 128;
    profile.peakOffset[0] = 128; profile.peakOffset[1] = profile.peakOffset[2] = 255;
    profile.globalScale = 3; profile.globalOffset = -0.3;
    audio::Analyzer analyzer(format,audio::RequiredAnalysis(settings)); audio::Response response;
    auto feed = [&](const std::vector<float> &samples) {
        audio::Reaction result;
        for(int i = 0; i < 200; ++i) {
            analyzer.Push(reinterpret_cast<const BYTE *>(samples.data()),2400,false);
            result = response.Update(settings,analyzer.Analyze(),true,0.05);
        }
        return result;
    };
    const auto base = feed(tiny), intermediate = feed(middle), peak = feed(loud);
    Check(base.colors.scale[0] == 0 && base.colors.offset[1] == 64,"Tiny zero crossings activated the old color profile.");
    Check(intermediate.colors.scale[0] > 0.8 && intermediate.colors.scale[0] < 1.8,
        "Moderate waveform input is trapped at Base or clipped to Peak with the old profile.");
    Check(peak.colors.scale[0] == 2 && peak.colors.offset[1] == 255,"Strong waveform input cannot reach the old Peak.");
    settings.brightnessEnabled = settings.speedEnabled = TRUE;
    settings.brightnessStrength = 0.9; settings.speedStrength = 0.3; settings.sensitivity = 4;
    auto same = [](const audio::Reaction &a, const audio::Reaction &b) {
        Check(std::abs(a.speed-b.speed) < 1e-5 && std::abs(a.spawn-b.spawn) < 1e-5,"Motion response drifted over repeated sound sequences.");
        for(int c = 0; c < 3; ++c)
            Check(std::abs(a.colors.scale[c]-b.colors.scale[c]) < 1e-5 && std::abs(a.colors.offset[c]-b.colors.offset[c]) < 1e-5,
                "Color response depends on previous loud passages or accumulated runtime.");
    };
    const std::vector<float> *sequence[] = {&middle,&loud,&quiet,&silence,&middle};
    for(UINT source = 0; source < audio::SourceCount; ++source) {
        settings.responseSource = source; analyzer.SetMask(audio::RequiredAnalysis(settings)); response.Reset();
        audio::Reaction reference[5];
        // Ten simulated minutes per source, with no reset between passages or cycles.
        for(int cycle = 0; cycle < 12; ++cycle) {
            for(int stage = 0; stage < 5; ++stage) {
                const auto current = feed(*sequence[stage]);
                if(cycle == 0) reference[stage] = current;
                else same(current,reference[stage]);
                if(stage == 4) same(current,reference[0]);
            }
        }
        Check(reference[1].colors.scale[0] > 0.6,"Brightness suppressed the recovered waveform Peak.");
    }
    const auto beforeDisable = feed(middle);
    settings.enabled = FALSE; Check(Neutral(feed(loud)),"Disabled audio retained calibrated colors.");
    settings.enabled = TRUE; same(feed(middle),beforeDisable);
}
int main() {
    try {
        auto settings = audio::Defaults();
        const audio::Descriptors sound = {0.2,0.3,0.1,0.04};
        audio::Response response;
        Check(Neutral(Settle(response,settings,sound)) && !audio::RequiredAnalysis(settings),"Disabled audio changed rendering or requested analysis.");
        settings.enabled = TRUE;
        Check(audio::RequiredAnalysis(settings) == audio::AnalyzeLevel,"Brightness unnecessarily requested legacy analysis.");
        for(unsigned combination = 0; combination < 16; ++combination) {
            settings.brightnessEnabled = (combination & 1) != 0; settings.speedEnabled = (combination & 2) != 0;
            settings.spawnEnabled = (combination & 4) != 0; settings.colorEnabled = (combination & 8) != 0;
            response.Reset();
            const auto active = Settle(response,settings,sound);
            Check((active.speed > 1) == bool(settings.speedEnabled) && (active.spawn < 1) == bool(settings.spawnEnabled),"Motion influences are coupled.");
            if(!settings.colorEnabled) {
                Check(active.colors.scale[0] == active.colors.scale[1] && active.colors.scale[1] == active.colors.scale[2] &&
                    active.colors.offset[0] == 0 && active.colors.offset[1] == 0 && active.colors.offset[2] == 0,"Brightness altered hue or added a color offset.");
                Check((active.colors.scale[0] < 1) == bool(settings.brightnessEnabled),"Brightness was not independent.");
                Check(Neutral(Settle(response,settings,{})),"Silence did not restore ordinary appearance and motion.");
            }
            Check(Neutral(response.Update(settings,sound,false,0.02)),"Capture failure left active overrides.");
        }
        settings = audio::Defaults(); settings.enabled = TRUE; settings.speedEnabled = settings.spawnEnabled = TRUE;
        response.Reset();
        const auto quiet = Settle(response,settings,sound);
        auto loud = sound; loud.level = 0.25;
        const auto first = response.Update(settings,loud,true,0.02);
        const auto sustained = Settle(response,settings,loud);
        Check(first.speed > quiet.speed && first.speed < sustained.speed && first.colors.scale[0] > quiet.colors.scale[0] &&
            first.colors.scale[0] < sustained.colors.scale[0],"Attack is not gradual or does not follow loudness.");
        const auto released = response.Update(settings,{},true,0.02);
        Check(released.speed > 1 && released.speed < sustained.speed,"Motion release jumped directly to idle.");
        Check(Neutral(Settle(response,settings,{})),"Envelope tails never reached neutral.");
        audio::Response slow, fast;
        const auto slowResult = Settle(slow,settings,sound,20,0.05), fastResult = Settle(fast,settings,sound,100,0.01);
        Check(std::abs(slowResult.speed-fastResult.speed) < 1e-10 && std::abs(slowResult.colors.scale[0]-fastResult.colors.scale[0]) < 1e-10,
            "Smoothing depends on animation refresh frequency.");
        slow.Reset(); fast.Reset();
        const auto infrequent = Settle(slow,settings,sound,4,0.5), frequent = Settle(fast,settings,sound,200,0.01);
        Check(std::abs(infrequent.speed-frequent.speed) < 0.00001 && std::abs(infrequent.colors.scale[0]-frequent.colors.scale[0]) < 0.00001,
            "Long refresh intervals slowed the response envelope.");
        const auto levelResult = Settle(response,settings,sound);
        settings.responseSource = audio::BassEnergy;
        const auto bassResult = Settle(response,settings,sound);
        Check(bassResult.speed < levelResult.speed && audio::RequiredAnalysis(settings) == (audio::AnalyzeLevel | audio::AnalyzeBass),"Bass source was not isolated.");
        settings.brightnessStrength = settings.speedStrength = settings.spawnStrength = 0;
        Check(!audio::RequiredAnalysis(settings) && Neutral(Settle(response,settings,sound)),"Zero strength kept analysis/overrides active.");
        settings = audio::Defaults(); settings.enabled = TRUE; settings.brightnessEnabled = FALSE; settings.colorEnabled = TRUE; settings.smoothing = 0;
        for(UINT mode = 0; mode < audio::ModeCount; ++mode) {
            settings.mode = mode; response.Reset();
            Check(audio::RequiredAnalysis(settings) == static_cast<unsigned>(mode == audio::WaveformVariation ? audio::AnalyzeWaveform : audio::AnalyzeCentroid),
                "Color mapping requested an unused analysis.");
            for(const auto &signal : {sound,audio::Descriptors{}}) {
                const auto mapped = response.Update(settings,signal,true,0.02);
                const auto original = audio::Map(settings.profiles[mode],mode == audio::WaveformVariation ? signal.waveformVariation : signal.spectralCentroid);
                for(int c = 0; c < 3; ++c) Check(mapped.colors.scale[c] == original.scale[c] && mapped.colors.offset[c] == original.offset[c],"Migrated legacy color response changed.");
            }
        }
        settings.colorEnabled = FALSE;
        Check(Neutral(response.Update(settings,sound,true,0.02)),"Turning off the final effect left coefficients active.");
        CheckWaveformPipeline();
        puts("PASS: Independent effects, time-based envelopes, silence/failure, old-profile PCM calibration and 20 simulated minutes without response drift.");
        return 0;
    } catch(const std::exception &error) { fprintf(stderr,"FAIL: %s\n",error.what()); return 1; }
}
