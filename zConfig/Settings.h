// Native configuration support for ZMatrix. See LICENSE.TXT.
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <string>
#include <vector>
#include "../zsMatrix/IzsMatrix.h"
#include "../zsMatrix/IzsMatrixAppearance.h"

namespace zconfig {
struct Error {
    std::wstring message;
    DWORD code;
};
void Require(bool condition, const wchar_t *message);
struct Color { BYTE r, g, b, a; };
struct Settings {
    unsigned maxStream, speedVariance, backTrace, leading, spacePad, refresh;
    bool monotonous, randomized;
    float probability;
    DWORD priority;
    LOGFONTW font, specialFont;
    Color foreground, fade, background, specialForeground, specialFade, specialBackground;
    TBGMode backgroundMode;
    TBlendMode blendMode;
    unsigned blendStrength;
    std::vector<wchar_t> characters;
    std::vector<std::wstring> strings;
};
Settings Capture(IzsMatrix &matrix, unsigned refresh, DWORD priority);
void Apply(const Settings &settings, IzsMatrix &matrix, unsigned &refresh, DWORD &priority);
Settings Load(const std::wstring &file, const Settings &defaults);
void Save(const std::wstring &file, const Settings &settings);

std::vector<wchar_t> ParseCharacters(const std::wstring &text);
std::wstring FormatCharacters(const std::vector<wchar_t> &characters, bool escaped);
std::vector<std::wstring> ParseStrings(const std::wstring &text, wchar_t delimiter);
std::wstring FormatStrings(const std::vector<std::wstring> &strings, wchar_t delimiter, bool escaped);
std::wstring ReadText(const std::wstring &file);
void WriteText(const std::wstring &file, const std::wstring &text);
std::wstring FullPath(const std::wstring &file);
}
