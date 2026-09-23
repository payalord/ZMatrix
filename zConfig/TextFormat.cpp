// Legacy ZMatrix text syntax, independent of VCL. See LICENSE.TXT.
#include "Settings.h"
#include <cstdio>
#include <memory>

namespace zconfig {
void Require(bool condition, const wchar_t *message) {
    if(!condition) { const DWORD code = GetLastError(); throw Error{message, code ? code : ERROR_INVALID_DATA}; }
}
std::wstring FullPath(const std::wstring &file) {
    Require(!file.empty(), L"A file name is required.");
    DWORD size = GetFullPathNameW(file.c_str(), 0, nullptr, nullptr);
    Require(size != 0, L"Cannot resolve the file name.");
    std::vector<wchar_t> path(size);
    DWORD copied = GetFullPathNameW(file.c_str(), size, path.data(), nullptr);
    Require(copied != 0 && copied < size, L"Cannot resolve the file name.");
    return path.data();
}
static int Hex(wchar_t c) {
    if(c >= L'0' && c <= L'9') return c - L'0';
    if(c >= L'a' && c <= L'f') return c - L'a' + 10;
    if(c >= L'A' && c <= L'F') return c - L'A' + 10;
    return -1;
}
static bool Printable(wchar_t c) { return c >= L' ' && c != 0x7f; }
static bool ReadHex(const std::wstring &text, size_t &pos, wchar_t &value) {
    if(pos + 3 >= text.size() || text[pos] != L'\\' || text[pos+1] != L'0' ||
       (text[pos+2] != L'x' && text[pos+2] != L'X') || Hex(text[pos+3]) < 0) return false;
    pos += 3;
    unsigned number = 0, count = 0;
    while(pos < text.size() && count < 4 && Hex(text[pos]) >= 0) {
        number = number * 16 + Hex(text[pos++]);
        ++count;
    }
    value = static_cast<wchar_t>(number);
    return true;
}
static std::wstring Escape(wchar_t c) {
    wchar_t buffer[12];
    // Four digits prevent a following A-F/0-9 character being swallowed by the parser.
    swprintf_s(buffer, L"\\0x%04X", static_cast<unsigned>(c));
    return buffer;
}
std::vector<wchar_t> ParseCharacters(const std::wstring &text) {
    std::vector<wchar_t> result;
    for(size_t pos = 0; pos < text.size();) {
        wchar_t c = text[pos];
        if(c == L'\\') {
            if(ReadHex(text, pos, c)) { if(Printable(c)) result.push_back(c); continue; }
            ++pos;
            if(pos < text.size() && (text[pos] == L',' || text[pos] == L'\\')) result.push_back(text[pos++]);
        } else {
            ++pos;
            if(Printable(c) && c != L' ' && c != L',') result.push_back(c);
        }
    }
    return result;
}
std::wstring FormatCharacters(const std::vector<wchar_t> &characters, bool escaped) {
    std::wstring result;
    for(wchar_t c : characters) {
        if(!Printable(c)) continue;
        if(!result.empty()) result += L", ";
        if(c == L',' || c == L'\\') result += L'\\';
        result += c == L' ' || (escaped && c >= 0x80) ? Escape(c) : std::wstring(1, c);
    }
    return result;
}
std::vector<std::wstring> ParseStrings(const std::wstring &text, wchar_t delimiter) {
    std::vector<std::wstring> result;
    std::wstring current;
    auto finish = [&]() { if(!current.empty()) result.push_back(current); current.clear(); };
    for(size_t pos = 0; pos < text.size();) {
        wchar_t c = text[pos];
        if(c == delimiter) { ++pos; finish(); }
        else if(c == L'\\') {
            if(pos + 1 < text.size() && (text[pos+1] == delimiter || text[pos+1] == L'\\')) {
                current += text[pos+1]; pos += 2;
            } else if(ReadHex(text, pos, c)) {
                // The legacy format treats a hexadecimal delimiter as a separator.
                if(c == delimiter) finish(); else if(Printable(c)) current += c;
            } else { ++pos; }
        } else { ++pos; if(Printable(c)) current += c; }
    }
    finish();
    return result;
}
std::wstring FormatStrings(const std::vector<std::wstring> &strings, wchar_t delimiter, bool escaped) {
    std::wstring result;
    for(size_t i = 0; i < strings.size(); ++i) {
        if(i) result += delimiter;
        for(size_t j = 0; j < strings[i].size(); ++j) {
            const wchar_t c = strings[i][j];
            if(!Printable(c)) continue;
            if(c == delimiter || c == L'\\') result += L'\\';
            // INI readers strip enclosing quotes and outside whitespace.
            const bool quote = c == L'"' || (c == L' ' && (j == 0 || j+1 == strings[i].size()));
            result += escaped && (c >= 0x80 || quote) ? Escape(c) : std::wstring(1, c);
        }
    }
    return result;
}
std::wstring ReadText(const std::wstring &file) {
    FILE *raw = nullptr;
    _wfopen_s(&raw, file.c_str(), L"rb");
    Require(raw != nullptr, L"Cannot open the selected file for reading.");
    std::unique_ptr<FILE, decltype(&fclose)> input(raw, fclose);
    Require(_fseeki64(raw, 0, SEEK_END) == 0, L"Cannot read the file size.");
    const auto length = _ftelli64(raw);
    Require(length >= 0 && length <= 16 * 1024 * 1024, L"The text file is too large.");
    rewind(raw);
    std::string bytes(static_cast<size_t>(length), '\0');
    Require(fread(&bytes[0], 1, bytes.size(), raw) == bytes.size(), L"Cannot read the file.");
    if(bytes.empty()) return {};
    if(bytes.size() >= 2 && static_cast<unsigned char>(bytes[0]) == 0xff && static_cast<unsigned char>(bytes[1]) == 0xfe) {
        Require(bytes.size() % 2 == 0, L"Invalid UTF-16 text file.");
        std::wstring result;
        for(size_t i = 2; i < bytes.size(); i += 2)
            result += static_cast<wchar_t>(static_cast<unsigned char>(bytes[i]) | (static_cast<unsigned char>(bytes[i+1]) << 8));
        return result;
    }
    const bool utf8 = bytes.size() >= 3 && bytes.compare(0, 3, "\xef\xbb\xbf") == 0;
    const char *data = bytes.data() + (utf8 ? 3 : 0);
    const int count = static_cast<int>(bytes.size() - (utf8 ? 3 : 0));
    if(!count) return {};
    int size = MultiByteToWideChar(utf8 ? CP_UTF8 : CP_ACP, utf8 ? MB_ERR_INVALID_CHARS : 0, data, count, nullptr, 0);
    Require(size != 0, L"Cannot decode the text file.");
    std::wstring result(size, L'\0');
    MultiByteToWideChar(utf8 ? CP_UTF8 : CP_ACP, utf8 ? MB_ERR_INVALID_CHARS : 0, data, count, &result[0], size);
    return result;
}
void WriteText(const std::wstring &file, const std::wstring &text) {
    // Exported sets/strings use the legacy ASCII escape syntax, never a new encoding.
    std::string bytes;
    for(wchar_t c : text) {
        Require(c <= 0x7f, L"The exported text must use Unicode escapes.");
        if(c == L'\n' && (bytes.empty() || bytes.back() != '\r')) bytes += '\r';
        bytes += static_cast<char>(c);
    }
    FILE *raw = nullptr;
    _wfopen_s(&raw, file.c_str(), L"wb");
    Require(raw != nullptr, L"Cannot open the selected file for writing.");
    const bool success = fwrite(bytes.data(), 1, bytes.size(), raw) == bytes.size();
    const int closed = fclose(raw);
    Require(success && closed == 0, L"Cannot finish writing the file.");
}
}
