// Configuration persistence for the ConfigFileFormatVersion=1.0 format.
// See LICENSE.TXT.
#include "Settings.h"
#include <algorithm>
#include <cmath>
#include <cerrno>
#include <cstdio>
#include <cwchar>
#include <limits>
#include <locale>
#include <sstream>

namespace zconfig {
Settings Capture(IzsMatrix &m, unsigned refresh, DWORD priority) {
    Settings s = {};
    s.maxStream = m.GetMaxStream(); s.speedVariance = m.GetSpeedVariance();
    s.backTrace = m.GetBackTrace(); s.leading = m.GetLeading(); s.spacePad = m.GetSpacePad();
    s.monotonous = m.GetMonotonousCleanupEnabled(); s.randomized = m.GetRandomizedCleanupEnabled();
    s.refresh = refresh; s.priority = priority; s.probability = m.GetSpecialStringStreamProbability();
    Require(GetObjectW(m.GetFont(), sizeof(s.font), &s.font) == sizeof(s.font), L"Cannot read the current font.");
    Require(GetObjectW(m.GetSpecialStringFont(), sizeof(s.specialFont), &s.specialFont) == sizeof(s.specialFont), L"Cannot read the special-string font.");
    m.GetColor(s.foreground.r, s.foreground.g, s.foreground.b, s.foreground.a);
    m.GetFadeColor(s.fade.r, s.fade.g, s.fade.b, s.fade.a);
    m.GetBGColor(s.background.r, s.background.g, s.background.b, s.background.a);
    m.GetSpecialStringColor(s.specialForeground.r, s.specialForeground.g, s.specialForeground.b, s.specialForeground.a);
    m.GetSpecialStringFadeColor(s.specialFade.r, s.specialFade.g, s.specialFade.b, s.specialFade.a);
    m.GetSpecialStringBGColor(s.specialBackground.r, s.specialBackground.g, s.specialBackground.b, s.specialBackground.a);
    s.backgroundMode = m.GetBGMode(); s.blendMode = m.GetBlendMode();
    if(m.GetNumCharsInSet()) s.characters.assign(m.GetValidCharSet(), m.GetValidCharSet() + m.GetNumCharsInSet());
    for(unsigned i = 0; i < m.GetNumSpecialStringsInSet(); ++i) s.strings.emplace_back(m.GetValidSpecialString(i));
    return s;
}
void Apply(const Settings &s, IzsMatrix &m, unsigned &refresh, DWORD &priority) {
    m.SetMaxStream(s.maxStream); m.SetSpeedVariance(s.speedVariance);
    m.SetBackTrace(s.backTrace); m.SetLeading(s.leading); m.SetSpacePad(s.spacePad);
    m.SetMonotonousCleanupEnabled(s.monotonous); m.SetRandomizedCleanupEnabled(s.randomized);
    m.SetSpecialStringStreamProbability(s.probability);
    LOGFONTW font = {}, specialFont = {};
    if(GetObjectW(m.GetFont(), sizeof(font), &font) != sizeof(font) || memcmp(&font, &s.font, sizeof(font))) m.SetLogFont(s.font);
    if(GetObjectW(m.GetSpecialStringFont(), sizeof(specialFont), &specialFont) != sizeof(specialFont) || memcmp(&specialFont, &s.specialFont, sizeof(specialFont))) m.SetSpecialStringLogFont(s.specialFont);
    m.SetColor(s.foreground.r, s.foreground.g, s.foreground.b, s.foreground.a);
    m.SetFadeColor(s.fade.r, s.fade.g, s.fade.b, s.fade.a);
    m.SetBGColor(s.background.r, s.background.g, s.background.b, s.background.a);
    m.SetSpecialStringColor(s.specialForeground.r, s.specialForeground.g, s.specialForeground.b, s.specialForeground.a);
    m.SetSpecialStringFadeColor(s.specialFade.r, s.specialFade.g, s.specialFade.b, s.specialFade.a);
    m.SetSpecialStringBGColor(s.specialBackground.r, s.specialBackground.g, s.specialBackground.b, s.specialBackground.a);
    m.SetBGMode(s.backgroundMode); m.SetBlendMode(s.blendMode);
    if(s.characters.size() != m.GetNumCharsInSet() || (!s.characters.empty() && !std::equal(s.characters.begin(), s.characters.end(), m.GetValidCharSet()))) {
        if(s.characters.empty()) m.ClearValidCharSet();
        else m.SetValidCharSet(s.characters.data(), static_cast<unsigned>(s.characters.size()));
    }
    bool changedStrings = s.strings.size() != m.GetNumSpecialStringsInSet();
    for(unsigned i = 0; !changedStrings && i < s.strings.size(); ++i) changedStrings = s.strings[i] != m.GetValidSpecialString(i);
    if(changedStrings) {
        std::vector<const wchar_t *> strings;
        for(const auto &str : s.strings) strings.push_back(str.c_str());
        if(strings.empty()) m.ClearValidSpecialStringSet();
        else m.SetValidSpecialStringSet(strings.data(), static_cast<unsigned>(strings.size()));
    }
    refresh = s.refresh; priority = s.priority;
}

struct NamedValue { const wchar_t *name; unsigned value; };
// Stringification requires a second macro for MSVC's standard preprocessor.
#define WIDEN_(x) L##x
#define WIDEN(x) WIDEN_(x)
#define NAMED(x) {WIDEN(#x), x}
static const NamedValue Charsets[] = {
    NAMED(DEFAULT_CHARSET), NAMED(ANSI_CHARSET), NAMED(BALTIC_CHARSET), NAMED(CHINESEBIG5_CHARSET),
    NAMED(EASTEUROPE_CHARSET), NAMED(GB2312_CHARSET), NAMED(GREEK_CHARSET), NAMED(HANGUL_CHARSET),
    NAMED(MAC_CHARSET), NAMED(OEM_CHARSET), NAMED(RUSSIAN_CHARSET), NAMED(SHIFTJIS_CHARSET),
    NAMED(SYMBOL_CHARSET), NAMED(TURKISH_CHARSET), NAMED(JOHAB_CHARSET), NAMED(HEBREW_CHARSET),
    NAMED(ARABIC_CHARSET), NAMED(THAI_CHARSET)
};
static const NamedValue Precisions[] = {NAMED(OUT_DEFAULT_PRECIS), NAMED(OUT_DEVICE_PRECIS), NAMED(OUT_OUTLINE_PRECIS), NAMED(OUT_RASTER_PRECIS), NAMED(OUT_STRING_PRECIS), NAMED(OUT_STROKE_PRECIS), NAMED(OUT_TT_ONLY_PRECIS), NAMED(OUT_TT_PRECIS)};
static const NamedValue Clips[] = {NAMED(CLIP_DEFAULT_PRECIS), NAMED(CLIP_STROKE_PRECIS), NAMED(CLIP_EMBEDDED), NAMED(CLIP_LH_ANGLES)};
static const NamedValue Qualities[] = {NAMED(DEFAULT_QUALITY), NAMED(DRAFT_QUALITY), NAMED(PROOF_QUALITY), NAMED(ANTIALIASED_QUALITY), NAMED(NONANTIALIASED_QUALITY), NAMED(CLEARTYPE_QUALITY)};
static const NamedValue Pitches[] = {NAMED(DEFAULT_PITCH), NAMED(FIXED_PITCH), NAMED(VARIABLE_PITCH)};
static const NamedValue Families[] = {NAMED(FF_DONTCARE), NAMED(FF_DECORATIVE), NAMED(FF_MODERN), NAMED(FF_ROMAN), NAMED(FF_SCRIPT), NAMED(FF_SWISS)};
static const NamedValue Priorities[] = {NAMED(IDLE_PRIORITY_CLASS), NAMED(BELOW_NORMAL_PRIORITY_CLASS), NAMED(NORMAL_PRIORITY_CLASS), NAMED(ABOVE_NORMAL_PRIORITY_CLASS), NAMED(HIGH_PRIORITY_CLASS)};
static const NamedValue Backgrounds[] = {NAMED(bgmodeBitmap), NAMED(bgmodeColor)};
static const NamedValue Blends[] = {NAMED(blendmodeXOR), NAMED(blendmodeAND), NAMED(blendmodeOR)};
#undef NAMED
#undef WIDEN
#undef WIDEN_

template<size_t N> static std::wstring Name(unsigned value, const NamedValue (&names)[N]) {
    for(const auto &entry : names) if(entry.value == value) return entry.name;
    return std::to_wstring(value);
}
template<size_t N> static unsigned Value(const std::wstring &text, const NamedValue (&names)[N]) {
    for(const auto &entry : names) if(text == entry.name) return entry.value;
    wchar_t *end = nullptr;
    const unsigned long result = wcstoul(text.c_str(), &end, 10);
    return !text.empty() && end && *end == 0 ? result : names[0].value;
}
class Profile {
    std::wstring path;
public:
    explicit Profile(const std::wstring &file) : path(file) {}
    std::wstring Read(const wchar_t *section, const wchar_t *key, const std::wstring &fallback) const {
        for(DWORD size = 256; size <= 1024 * 1024; size *= 2) {
            std::vector<wchar_t> buffer(size);
            const DWORD count = GetPrivateProfileStringW(section, key, fallback.c_str(), buffer.data(), size, path.c_str());
            if(count < size - 1) return std::wstring(buffer.data(), count);
        }
        throw Error{L"A configuration value is too large.", ERROR_INVALID_DATA};
    }
    long Number(const wchar_t *section, const wchar_t *key, long fallback) const {
        const auto text = Read(section, key, std::to_wstring(fallback));
        if(_wcsicmp(text.c_str(), L"true") == 0) return 1;
        if(_wcsicmp(text.c_str(), L"false") == 0) return 0;
        wchar_t *end = nullptr;
        errno = 0;
        const long value = wcstol(text.c_str(), &end, 10);
        return !text.empty() && end && *end == 0 && errno != ERANGE ? value : fallback;
    }
    unsigned Unsigned(const wchar_t *key, unsigned fallback) const {
        const long n = Number(L"General", key, static_cast<long>(fallback));
        return n >= 0 ? static_cast<unsigned>(n) : fallback;
    }
    void Write(const wchar_t *section, const wchar_t *key, const std::wstring &value) const {
        Require(WritePrivateProfileStringW(section, key, value.c_str(), path.c_str()) != FALSE, L"Cannot write the configuration file.");
    }
    void PutNumber(const wchar_t *section, const wchar_t *key, long value) const { Write(section, key, std::to_wstring(value)); }
};
static LOGFONTW ReadFont(const Profile &p, const wchar_t *section) {
    LOGFONTW f = {};
    f.lfHeight = p.Number(section, L"FontHeight", 12);
    f.lfWidth = p.Number(section, L"FontWidth", 0);
    f.lfEscapement = p.Number(section, L"FontEscapement", 0);
    f.lfOrientation = p.Number(section, L"FontOrientation", 0);
    f.lfWeight = p.Number(section, L"FontWeight", 400);
    f.lfItalic = p.Number(section, L"FontItalic", 0) != 0;
    f.lfUnderline = p.Number(section, L"FontUnderline", 0) != 0;
    f.lfStrikeOut = p.Number(section, L"FontStrikeOut", 0) != 0;
    f.lfCharSet = static_cast<BYTE>(Value(p.Read(section, L"FontCharSet", L"DEFAULT_CHARSET"), Charsets));
    f.lfOutPrecision = static_cast<BYTE>(Value(p.Read(section, L"FontOutPrecision", L"OUT_DEFAULT_PRECIS"), Precisions));
    f.lfClipPrecision = static_cast<BYTE>(Value(p.Read(section, L"FontClipPrecision", L"CLIP_DEFAULT_PRECIS"), Clips));
    f.lfQuality = static_cast<BYTE>(Value(p.Read(section, L"FontQuality", L"DEFAULT_QUALITY"), Qualities));
    f.lfPitchAndFamily = static_cast<BYTE>(Value(p.Read(section, L"FontPitch", L"DEFAULT_PITCH"), Pitches) | Value(p.Read(section, L"FontFamily", L"FF_DONTCARE"), Families));
    wcsncpy_s(f.lfFaceName, p.Read(section, L"FontName", L"Terminal").c_str(), _TRUNCATE);
    return f;
}
static void WriteFont(const Profile &p, const wchar_t *section, const LOGFONTW &f) {
    p.PutNumber(section, L"FontHeight", f.lfHeight); p.PutNumber(section, L"FontWidth", f.lfWidth);
    p.PutNumber(section, L"FontEscapement", f.lfEscapement); p.PutNumber(section, L"FontOrientation", f.lfOrientation);
    p.PutNumber(section, L"FontWeight", f.lfWeight); p.PutNumber(section, L"FontItalic", f.lfItalic);
    p.PutNumber(section, L"FontUnderline", f.lfUnderline); p.PutNumber(section, L"FontStrikeOut", f.lfStrikeOut);
    p.Write(section, L"FontCharSet", Name(f.lfCharSet, Charsets));
    p.Write(section, L"FontOutPrecision", Name(f.lfOutPrecision, Precisions));
    p.Write(section, L"FontClipPrecision", Name(f.lfClipPrecision, Clips));
    p.Write(section, L"FontQuality", Name(f.lfQuality, Qualities));
    p.Write(section, L"FontPitch", Name(f.lfPitchAndFamily & 0x0f, Pitches));
    p.Write(section, L"FontFamily", Name(f.lfPitchAndFamily & 0xf0, Families));
    p.Write(section, L"FontName", f.lfFaceName);
}
static Color ReadColor(const Profile &p, const wchar_t *key, const wchar_t *fallback) {
    int r = 0, g = 0, b = 0, a = 0;
    if(swscanf_s(p.Read(L"Colors", key, fallback).c_str(), L"{%d,%d,%d,%d}", &r, &g, &b, &a) != 4)
        swscanf_s(fallback, L"{%d,%d,%d,%d}", &r, &g, &b, &a);
    auto byte = [](int n) { return static_cast<BYTE>(std::max(0, std::min(n, 255))); };
    return {byte(r), byte(g), byte(b), byte(a)};
}
static void WriteColor(const Profile &p, const wchar_t *key, Color c) {
    p.Write(L"Colors", key, L"{" + std::to_wstring(c.r) + L"," + std::to_wstring(c.g) + L"," + std::to_wstring(c.b) + L"," + std::to_wstring(c.a) + L"}");
}
static bool UnicodeProfile(const std::wstring &path) {
    FILE *input = nullptr;
    _wfopen_s(&input, path.c_str(), L"rb");
    Require(input != nullptr, L"Cannot read the configuration file.");
    const int first = fgetc(input), second = fgetc(input), third = fgetc(input);
    const bool failed = ferror(input) != 0;
    fclose(input);
    Require(!failed, L"Cannot read the configuration file.");
    // Windows profile APIs support ANSI and UTF-16LE. Other BOMs would silently
    // turn an apparently successful load into defaults, or corrupt an update.
    if((first == 0xef && second == 0xbb && third == 0xbf) || (first == 0xfe && second == 0xff))
        throw Error{L"CFG files must use the legacy ANSI or UTF-16 little-endian encoding.", ERROR_NO_UNICODE_TRANSLATION};
    return first == 0xff && second == 0xfe;
}
Settings Load(const std::wstring &file, const Settings &defaults) {
    const auto path = FullPath(file);
    const DWORD attributes = GetFileAttributesW(path.c_str());
    Require(attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY), L"The configuration file does not exist.");
    // Verify that the file can actually be read; profile APIs silently use defaults on I/O errors.
    ReadText(path);
    UnicodeProfile(path);
    Profile p(path);
    Settings s = defaults;
    s.maxStream = p.Unsigned(L"MaxStream", s.maxStream); s.speedVariance = p.Unsigned(L"SpeedVariance", s.speedVariance);
    s.backTrace = p.Unsigned(L"BackTrace", s.backTrace); s.leading = p.Unsigned(L"Leading", s.leading);
    s.spacePad = p.Unsigned(L"SpacePad", s.spacePad); s.refresh = std::max(1u, p.Unsigned(L"RefreshTime", s.refresh));
    s.monotonous = p.Number(L"General", L"MonotonousCleanupEnabled", s.monotonous) != 0;
    s.randomized = p.Number(L"General", L"RandomizedCleanupEnabled", s.randomized) != 0;
    s.priority = Value(p.Read(L"General", L"PriorityClass", L"IDLE_PRIORITY_CLASS"), Priorities);
    std::wistringstream probability(p.Read(L"General", L"SpecialStringStreamProbability", L""));
    probability.imbue(std::locale::classic());
    float parsed;
    if(probability >> parsed && std::isfinite(parsed)) s.probability = std::max(0.0f, std::min(parsed, 1.0f));
    s.font = ReadFont(p, L"Text"); s.specialFont = ReadFont(p, L"SpecialText");
    s.characters = ParseCharacters(p.Read(L"Text", L"CharSet", L"*"));
    s.strings = ParseStrings(p.Read(L"SpecialText", L"Strings", L"The matrix has you"), L';');
    s.foreground = ReadColor(p, L"FGColor", L"{150,255,100,255}");
    s.fade = ReadColor(p, L"FadeColor", L"{50,85,33,128}");
    s.background = ReadColor(p, L"BGColor", L"{0,0,0,0}");
    s.specialForeground = ReadColor(p, L"SpecialStringFGColor", L"{150,255,100,255}");
    s.specialFade = ReadColor(p, L"SpecialStringFadeColor", L"{50,85,33,128}");
    s.specialBackground = ReadColor(p, L"SpecialStringBGColor", L"{0,0,0,0}");
    s.backgroundMode = Value(p.Read(L"Colors", L"BGMode", L"bgmodeBitmap"), Backgrounds);
    s.blendMode = Value(p.Read(L"Colors", L"BlendMode", L"blendmodeXOR"), Blends);
    return s;
}
class TemporaryProfile {
public:
    std::wstring path;
    explicit TemporaryProfile(const std::wstring &target) {
        const auto folder = target.substr(0, target.find_last_of(L'\\'));
        wchar_t name[MAX_PATH];
        Require(GetTempFileNameW(folder.c_str(), L"zmx", 0, name) != 0, L"Cannot create a temporary configuration file in this folder.");
        path = name;
    }
    ~TemporaryProfile() { DeleteFileW(path.c_str()); }
};
static void CheckAnsiFont(const wchar_t *text) {
    if(GetACP() == CP_UTF8) return;
    BOOL substituted = FALSE;
    const int size = WideCharToMultiByte(CP_ACP, WC_NO_BEST_FIT_CHARS, text, -1, nullptr, 0, nullptr, &substituted);
    if(!size || substituted)
        throw Error{L"This font name cannot be stored in the existing ANSI CFG encoding. Choose a representable font name or an existing UTF-16 CFG file.", ERROR_NO_UNICODE_TRANSLATION};
}
void Save(const std::wstring &file, const Settings &s) {
    const auto path = FullPath(file);
    const DWORD attributes = GetFileAttributesW(path.c_str());
    TemporaryProfile temporary(path);
    bool unicode = false;
    if(attributes != INVALID_FILE_ATTRIBUTES) {
        Require(!(attributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_READONLY)), L"The selected configuration file is read-only.");
        Require(CopyFileW(path.c_str(), temporary.path.c_str(), FALSE) != FALSE, L"Cannot copy the existing configuration.");
        unicode = UnicodeProfile(path);
    }
    if(!unicode) { CheckAnsiFont(s.font.lfFaceName); CheckAnsiFont(s.specialFont.lfFaceName); }
    Profile p(temporary.path);
    p.Write(L"General", L"ConfigFileFormatVersion", L"1.0");
    p.PutNumber(L"General", L"MaxStream", s.maxStream); p.PutNumber(L"General", L"SpeedVariance", s.speedVariance);
    p.PutNumber(L"General", L"BackTrace", s.backTrace); p.PutNumber(L"General", L"Leading", s.leading);
    p.PutNumber(L"General", L"SpacePad", s.spacePad); p.PutNumber(L"General", L"RefreshTime", s.refresh);
    p.PutNumber(L"General", L"MonotonousCleanupEnabled", s.monotonous); p.PutNumber(L"General", L"RandomizedCleanupEnabled", s.randomized);
    p.Write(L"General", L"PriorityClass", Name(s.priority, Priorities));
    std::wostringstream probability;
    probability.imbue(std::locale::classic()); probability.precision(std::numeric_limits<float>::max_digits10);
    probability << s.probability;
    p.Write(L"General", L"SpecialStringStreamProbability", probability.str());
    WriteFont(p, L"Text", s.font); WriteFont(p, L"SpecialText", s.specialFont);
    p.Write(L"Text", L"CharSet", FormatCharacters(s.characters, true));
    p.Write(L"SpecialText", L"Strings", FormatStrings(s.strings, L';', true));
    WriteColor(p, L"FGColor", s.foreground); WriteColor(p, L"FadeColor", s.fade); WriteColor(p, L"BGColor", s.background);
    WriteColor(p, L"SpecialStringFGColor", s.specialForeground); WriteColor(p, L"SpecialStringFadeColor", s.specialFade); WriteColor(p, L"SpecialStringBGColor", s.specialBackground);
    p.Write(L"Colors", L"BGMode", Name(s.backgroundMode, Backgrounds));
    p.Write(L"Colors", L"BlendMode", Name(s.blendMode, Blends));
    WritePrivateProfileStringW(nullptr, nullptr, nullptr, temporary.path.c_str());
    Require(MoveFileExW(temporary.path.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != FALSE, L"Cannot replace the configuration file.");
    WritePrivateProfileStringW(nullptr, nullptr, nullptr, path.c_str());
}
}
