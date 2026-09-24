// Audio settings persistence and audio-to-color mapping.
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "AudioSettings.h"
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>

namespace audio {
Settings Defaults() {
    Settings s = {};
    auto &variation = s.profiles[WaveformVariation];
    auto &centroid = s.profiles[SpectralCentroid];
    for(int c = 0; c < 3; ++c) variation.peakScale[c] = centroid.peakScale[c] = 2;
    variation.baseOffset[1] = 64; variation.baseOffset[2] = 128;
    variation.peakOffset[0] = 128; variation.peakOffset[1] = variation.peakOffset[2] = 255;
    variation.globalScale = 3; variation.globalOffset = -0.3;
    centroid.globalScale = 5;
    return s;
}
static bool Range(double value, double low, double high) {
    return std::isfinite(value) && value >= low && value <= high;
}
bool Valid(const Settings &s) {
    if((s.enabled != FALSE && s.enabled != TRUE) || s.mode >= ModeCount ||
       !wmemchr(s.deviceId, 0, _countof(s.deviceId))) return false;
    for(const auto &m : s.profiles) {
        for(int c = 0; c < 3; ++c)
            if(!Range(m.baseScale[c],0,10) || !Range(m.peakScale[c],0,10) ||
               !Range(m.baseOffset[c],0,255) || !Range(m.peakOffset[c],0,255)) return false;
        if(!Range(m.globalScale,0,20) || !Range(m.globalOffset,-5,5)) return false;
    }
    // A device ID is serialized as a single unquoted INI value.
    return wcspbrk(s.deviceId, L"\r\n\"[]") == nullptr;
}
Coefficients Map(const Mapping &m, double descriptor) {
    const double t = std::clamp(descriptor * m.globalScale + m.globalOffset, 0.0, 1.0);
    Coefficients result = {};
    for(int c = 0; c < 3; ++c) {
        result.scale[c] = (1-t)*m.baseScale[c] + t*m.peakScale[c];
        result.offset[c] = (1-t)*m.baseOffset[c] + t*m.peakOffset[c];
    }
    return result;
}
static std::wstring Value(const wchar_t *path, const wchar_t *section, const wchar_t *key) {
    wchar_t value[1024];
    const DWORD n = GetPrivateProfileStringW(section, key, L"", value, _countof(value), path);
    if(n == _countof(value)-1) throw DWORD(ERROR_INVALID_DATA);
    return value;
}
static bool Parse(std::wstring value, double *dest, int count, bool legacy) {
    // VCL could write localized scalar floats. Comma-separated RGB arrays with
    // decimal commas are ambiguous and must not be guessed at.
    if(legacy && count == 1 && value.find(L'.') == std::wstring::npos && std::count(value.begin(),value.end(),L',') == 1)
        std::replace(value.begin(),value.end(),L',',L'.');
    std::wistringstream in(value); in.imbue(std::locale::classic());
    wchar_t delimiter;
    if(count > 1 && (!(in >> delimiter) || delimiter != L'{')) return false;
    for(int i = 0; i < count; ++i) {
        if(!(in >> dest[i]) || !std::isfinite(dest[i])) return false;
        if(count > 1 && (!(in >> delimiter) || delimiter != (i+1 == count ? L'}' : L','))) return false;
    }
    in >> std::ws;
    return in.eof();
}
// Section names and keys are shared by Audio.cfg version 1 and legacy Winamp imports.
// Keep these serialized names stable even when effect names change.
static const wchar_t *Sections[] = {L"VU Modulate", L"Frequency Modulate"};
static const wchar_t *Keys[] = {L"BaseColorScales", L"BaseColorOffsets", L"PeakColorScales", L"PeakColorOffsets", L"GlobalScale", L"GlobalOffset"};
DWORD Load(const wchar_t *path, Settings &settings, bool legacy) {
    try {
        const DWORD attr = GetFileAttributesW(path);
        if(attr == INVALID_FILE_ATTRIBUTES) return GetLastError();
        if(attr & FILE_ATTRIBUTE_DIRECTORY) return ERROR_INVALID_DATA;
        Settings next = legacy ? settings : Defaults();
        if(!legacy) {
            if(Value(path,L"Audio",L"Version") != L"1") return ERROR_INVALID_DATA;
            const auto enabled = Value(path,L"Audio",L"Enabled");
            const auto mode = Value(path,L"Audio",L"Mode");
            if((enabled != L"0" && enabled != L"1") || (mode != L"0" && mode != L"1")) return ERROR_INVALID_DATA;
            next.enabled = enabled == L"1"; next.mode = mode == L"1" ? SpectralCentroid : WaveformVariation;
            const auto device = Value(path,L"Audio",L"Device");
            if(device.size() >= _countof(next.deviceId)) return ERROR_INVALID_DATA;
            wcscpy_s(next.deviceId, device.c_str());
        }
        bool found = false;
        for(int mode = 0; mode < ModeCount; ++mode) {
            auto &m = next.profiles[mode];
            double *fields[] = {m.baseScale,m.baseOffset,m.peakScale,m.peakOffset,&m.globalScale,&m.globalOffset};
            for(int k = 0; k < 6; ++k) {
                const auto value = Value(path,Sections[mode],Keys[k]);
                if(value.empty()) { if(!legacy) return ERROR_INVALID_DATA; continue; }
                found = true;
                if(!Parse(value,fields[k],k < 4 ? 3 : 1,legacy)) return ERROR_INVALID_DATA;
            }
        }
        if(!found || !Valid(next)) return ERROR_INVALID_DATA;
        settings = next;
        return ERROR_SUCCESS;
    } catch(DWORD error) { return error; }
    catch(...) { return ERROR_NOT_ENOUGH_MEMORY; }
}
DWORD Save(const wchar_t *path, const Settings &s) {
    if(!Valid(s)) return ERROR_INVALID_DATA;
    try {
        std::wostringstream out; out.imbue(std::locale::classic()); out << std::setprecision(17);
        out << L"\xFEFF[Audio]\r\nVersion=1\r\nEnabled=" << s.enabled << L"\r\nMode=" << s.mode << L"\r\nDevice=" << s.deviceId << L"\r\n";
        for(int mode = 0; mode < ModeCount; ++mode) {
            const auto &m = s.profiles[mode];
            const double *fields[] = {m.baseScale,m.baseOffset,m.peakScale,m.peakOffset,&m.globalScale,&m.globalOffset};
            out << L"\r\n[" << Sections[mode] << L"]\r\n";
            for(int k = 0; k < 6; ++k) {
                out << Keys[k] << L"=";
                if(k < 4) out << L"{" << fields[k][0] << L"," << fields[k][1] << L"," << fields[k][2] << L"}";
                else out << fields[k][0];
                out << L"\r\n";
            }
        }
        const std::wstring text = out.str(), destination(path);
        const auto separator = destination.find_last_of(L"\\/");
        if(separator == std::wstring::npos) return ERROR_BAD_PATHNAME;
        wchar_t temp[MAX_PATH];
        if(!GetTempFileNameW(destination.substr(0,separator).c_str(), L"zma", 0, temp)) return GetLastError();
        HANDLE file = CreateFileW(temp, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        DWORD error = ERROR_SUCCESS, written = 0;
        const DWORD bytes = static_cast<DWORD>(text.size()*sizeof(wchar_t));
        if(file == INVALID_HANDLE_VALUE) error = GetLastError();
        else {
            if(!WriteFile(file,text.data(),bytes,&written,nullptr) || written != bytes || !FlushFileBuffers(file))
                error = GetLastError() ? GetLastError() : ERROR_WRITE_FAULT;
            CloseHandle(file);
        }
        if(!error && !MoveFileExW(temp,path,MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) error = GetLastError();
        if(error) DeleteFileW(temp);
        return error;
    } catch(...) { return ERROR_NOT_ENOUGH_MEMORY; }
}
}
