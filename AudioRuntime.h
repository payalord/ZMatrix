#pragma once
#include "Audio/AudioSettings.h"
class IzsMatrix;
void InitializeAudio(const wchar_t *configurationFolder);
void ShutdownAudio();
void UpdateAudioReaction(IzsMatrix *matrix);
const audio::HostApi *AudioHost();
