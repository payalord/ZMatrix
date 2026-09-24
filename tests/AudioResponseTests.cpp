// Deterministic envelopes and independent audio influences; no device or windows required.
// Build with AudioSettings.cpp and AudioResponse.cpp.
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
        puts("PASS: Independent audio influences, hue preservation, level/bass, time-based envelopes, silence, failure, zero strength and legacy color compatibility.");
        return 0;
    } catch(const std::exception &error) { fprintf(stderr,"FAIL: %s\n",error.what()); return 1; }
}