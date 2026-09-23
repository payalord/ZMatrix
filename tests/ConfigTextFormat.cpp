// Legacy text fixtures and edge cases. Build together with zConfig/TextFormat.cpp.
#include "../zConfig/Settings.h"
#include <cstdio>
#include <stdexcept>

static void Check(bool ok,const char *message) { if(!ok) throw std::runtime_error(message); }
int wmain() {
    using namespace zconfig;
    wchar_t temp[MAX_PATH], file[MAX_PATH];
    if(!GetTempPathW(MAX_PATH,temp) || !GetTempFileNameW(temp,L"zmx",0,file)) return 1;
    int result=0;
    try {
        auto japanese=ParseCharacters(ReadText(L"JapaneseSet.txt"));
        Check(japanese.size()==182 && japanese.front()==0x3041 && japanese.back()==0x30f6,"JapaneseSet.txt was not parsed correctly.");
        auto matrix=ParseCharacters(ReadText(L"MatrixCodeFontSet.txt"));
        Check(matrix.size()==40 && matrix[0]==L'1' && matrix[9]==L'0' && matrix.back()==L'd',"MatrixCodeFontSet.txt was not parsed correctly.");
        Check(ParseCharacters(L" A,\\,,\\\\,\\0X0416,\\0x20 \t\r\n") == std::vector<wchar_t>({L'A',L',',L'\\',0x0416,L' '}),"Character delimiters/escapes changed.");
        Check(ParseCharacters(L"\\").empty() && ParseStrings(L"\\",L';').empty(),"Trailing escape was not handled.");
        const std::vector<std::wstring> strings={L"First; \\ ",L"\x00e9" L"ABC123",L"\x0416\x65e5\x672c"};
        Check(ParseStrings(L"First\\; \\\\ ;\\0x00E9ABC123;\\0x0416\\0x65E5\\0x672C",L';')==strings,"Legacy strings fixture was not parsed correctly.");
        Check(ParseStrings(L"one\\0x003Btwo;;",L';')==std::vector<std::wstring>({L"one",L"two"}),"Hexadecimal delimiter compatibility changed.");
        for(bool escaped : {false,true}) {
            Check(ParseCharacters(FormatCharacters(japanese,escaped))==japanese,"Japanese set round trip failed.");
            Check(ParseStrings(FormatStrings(strings,L';',escaped),L';')==strings,"Escaped string round trip failed.");
        }
        WriteText(file,FormatStrings(strings,L'\n',true));
        Check(ParseStrings(ReadText(file),L'\n')==strings,"Special string file round trip failed.");
        WriteText(file,L""); Check(ReadText(file).empty(),"Empty text file failed.");
        FILE *stream=nullptr; _wfopen_s(&stream,file,L"wb"); Check(stream!=nullptr,"Cannot create UTF-8 fixture.");
        const char utf8[]="\xef\xbb\xbf\xe6\x97\xa5\xe6\x9c\xac";
        fwrite(utf8,1,sizeof(utf8)-1,stream); fclose(stream);
        Check(ReadText(file)==L"\x65e5\x672c","UTF-8 BOM import failed.");
        _wfopen_s(&stream,file,L"wb"); Check(stream!=nullptr,"Cannot create UTF-16 fixture.");
        const wchar_t utf16[]=L"\xfeff\x0416\x65e5\x672c";
        fwrite(utf16,sizeof(wchar_t),wcslen(utf16),stream); fclose(stream);
        Check(ReadText(file)==L"\x0416\x65e5\x672c","UTF-16 import failed.");
        puts("PASS: Bundled character sets; escaped commas/backslashes/Unicode; hexadecimal delimiters; TXT round trips; UTF-8/UTF-16 imports.");
    } catch(const std::exception &error) { fprintf(stderr,"FAIL: %s\n",error.what()); result=1; }
    catch(const Error &error) { fwprintf(stderr,L"FAIL: %ls\n",error.message.c_str()); result=1; }
    DeleteFileW(file);
    return result;
}
