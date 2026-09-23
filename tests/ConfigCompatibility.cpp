// Run from the repository root after building the Win32 DLLs. No COM registration,
// desktop windows, registry settings or installed configuration files are changed.
#include <windows.h>
#include <string>
#include <stdexcept>
#include <cstdio>
#include "../zsMatrix/IzsMatrix.h"

static_assert(sizeof(void *) == 4, "Build this test with the x86 compiler.");
static_assert(sizeof(_TCHAR) == 2 && sizeof(LOGFONT) == 92, "Unicode ABI required.");

static void Check(bool passed, const char *message)
{
    if(!passed) throw std::runtime_error(message);
}

struct Module
{
    HMODULE handle;
    explicit Module(const wchar_t *path) : handle(LoadLibraryW(path))
    {
        Check(handle != NULL, "Cannot load a required DLL (check build and dependencies).");
    }
    ~Module() { FreeLibrary(handle); }
};

template<class T> struct ComObject
{
    T *ptr = NULL;
    ~ComObject() { if(ptr) ptr->Release(); }
};

struct TemporaryFile
{
    std::wstring directory, path;
    TemporaryFile()
    {
        wchar_t temp[MAX_PATH], unique[MAX_PATH];
        Check(GetTempPathW(MAX_PATH, temp) != 0, "GetTempPath failed.");
        Check(GetTempFileNameW(temp, L"zmx", 0, unique) != 0, "GetTempFileName failed.");
        Check(DeleteFileW(unique) != FALSE, "Temporary file cleanup failed.");
        Check(CreateDirectoryW(unique, NULL) != FALSE, "Temporary directory creation failed.");
        directory = unique;
    }
    ~TemporaryFile()
    {
        if(!path.empty()) DeleteFileW(path.c_str());
        RemoveDirectoryW(directory.c_str());
    }
};

static const wchar_t Characters[] = L"0,\\\x0416\x65e5";
static const wchar_t SpecialString[] = L"Test; \\ \x041f\x0440\x0438\x0432\x0435\x0442 \x65e5\x672c";

static void SetValues(IzsMatrix *matrix)
{
    matrix->SetMaxStream(137);
    matrix->SetSpeedVariance(7);
    matrix->SetValidCharSet(Characters, static_cast<unsigned int>(wcslen(Characters)));
    matrix->ClearValidSpecialStringSet();
    matrix->AddSpecialStringToValidSet(SpecialString);
    LOGFONTW font = {};
    font.lfHeight = -17;
    font.lfWeight = FW_BOLD;
    font.lfCharSet = DEFAULT_CHARSET;
    wcscpy_s(font.lfFaceName, L"Terminal");
    matrix->SetLogFont(font);
}

static void CheckValues(IzsMatrix *matrix)
{
    Check(matrix->GetMaxStream() == 137, "MaxStream changed.");
    Check(matrix->GetSpeedVariance() == 7, "SpeedVariance changed.");
    Check(std::wstring(matrix->GetValidCharSet(), matrix->GetNumCharsInSet()) == Characters,
          "Unicode character set changed.");
    Check(matrix->GetNumSpecialStringsInSet() == 1 &&
          std::wstring(matrix->GetValidSpecialString(0)) == SpecialString,
          "Unicode special strings or escaped delimiters changed.");
    LOGFONTW font = {};
    Check(GetObjectW(matrix->GetFont(), sizeof(font), &font) == sizeof(font), "Cannot read LOGFONTW.");
    Check(font.lfHeight == -17 && font.lfWeight == FW_BOLD &&
          font.lfCharSet == DEFAULT_CHARSET && wcscmp(font.lfFaceName, L"Terminal") == 0,
          "Font settings changed.");
}

int wmain(int argc, wchar_t **argv)
{
    const bool engineOnly = argc == 2 && wcscmp(argv[1], L"--engine-only") == 0;
    const HRESULT initialized = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if(FAILED(initialized)) return 1;
    int result = 0;
    try
    {
        Module engine(L".\\zsMatrix.dll");
        typedef HRESULT (STDAPICALLTYPE *GetClassObject)(REFCLSID, REFIID, void **);
        GetClassObject getFactory = reinterpret_cast<GetClassObject>(GetProcAddress(engine.handle, "DllGetClassObject"));
        Check(getFactory != NULL, "Missing engine factory export.");
        const GUID matrixClass = {0x2e393599,0xf2e3,0x484a,{0x9b,0x03,0xd4,0x14,0xf6,0xcb,0xa5,0xeb}};
        const GUID matrixInterface = {0xcbeac95f,0x3125,0x4603,{0xa0,0xc9,0x09,0x29,0xbd,0xc6,0x0f,0xbc}};
        ComObject<IClassFactory> factory;
        Check(SUCCEEDED(getFactory(matrixClass, IID_IClassFactory, reinterpret_cast<void **>(&factory.ptr))), "Cannot create factory.");
        ComObject<IzsMatrix> matrix;
        Check(SUCCEEDED(factory.ptr->CreateInstance(NULL, matrixInterface, reinterpret_cast<void **>(&matrix.ptr))), "Cannot create engine.");
        SetValues(matrix.ptr);
        CheckValues(matrix.ptr);
        puts("PASS: Win32 Unicode engine interface, characters, special strings and LOGFONTW.");

        if(!engineOnly)
        {
            Module config(L".\\Config.dll");
            const char *exports[] = {"LaunchAboutForm", "LaunchHireForm", "LaunchConfigForm", "SaveConfigToFile", "LoadConfigFromFile"};
            for(const char *name : exports) Check(GetProcAddress(config.handle, name) != NULL, "Missing Config export.");
            Check(FindResourceW(config.handle, L"COMMENTS.HTML", RT_HTML) != NULL, "Missing About HTML.");
            Check(FindResourceW(config.handle, L"HIRE.HTML", RT_HTML) != NULL, "Missing Hire HTML.");
            Check(FindResourceW(config.handle, MAKEINTRESOURCEW(102), L"WAVE") != NULL, "Missing About sound.");
            typedef int (__stdcall *Saver)(IzsMatrix *, unsigned int, DWORD, const wchar_t *);
            typedef int (__stdcall *Loader)(IzsMatrix *, unsigned int &, DWORD &, const wchar_t *);
            Saver save = reinterpret_cast<Saver>(GetProcAddress(config.handle, "SaveConfigToFile"));
            Loader load = reinterpret_cast<Loader>(GetProcAddress(config.handle, "LoadConfigFromFile"));
            unsigned int refresh = 0;
            DWORD priority = 0;
            Check(load(matrix.ptr, refresh, priority, L".\\default.cfg") != 0, "Cannot load legacy default.cfg.");
            Check(matrix.ptr->GetMaxStream() == 1000 && refresh == 50 && priority == IDLE_PRIORITY_CLASS,
                  "Legacy default.cfg values were not loaded.");
            const wchar_t *names[] = {L"settings.cfg", L"with spaces.cfg", L"\x041d\x0430\x0441\x0442\x0440\x043e\x0439\x043a\x0438.cfg", L"\x65e5\x672c\x8a9e.cfg"};
            for(const wchar_t *name : names)
            {
                TemporaryFile temp;
                temp.path = temp.directory + L"\\" + name;
                SetValues(matrix.ptr);
                Check(save(matrix.ptr, 41, NORMAL_PRIORITY_CLASS, temp.path.c_str()) != 0, "Cannot save CFG.");
                Check(GetFileAttributesW(temp.path.c_str()) != INVALID_FILE_ATTRIBUTES, "CFG saved to the wrong path.");
                matrix.ptr->SetMaxStream(1);
                matrix.ptr->SetSpeedVariance(1);
                matrix.ptr->ClearValidCharSet();
                matrix.ptr->ClearValidSpecialStringSet();
                LOGFONTW changedFont = {};
                changedFont.lfHeight = -9;
                changedFont.lfWeight = FW_NORMAL;
                wcscpy_s(changedFont.lfFaceName, L"Arial");
                matrix.ptr->SetLogFont(changedFont);
                refresh = 0;
                priority = 0;
                Check(load(matrix.ptr, refresh, priority, temp.path.c_str()) != 0, "Cannot reload CFG.");
                CheckValues(matrix.ptr);
                Check(refresh == 41 && priority == NORMAL_PRIORITY_CLASS, "Refresh time or priority changed.");
            }
            puts("PASS: Config exports/resources, legacy CFG and Unicode path round trips.");
        }
        else puts("Config.dll tests not requested (--engine-only).");
    }
    catch(const std::exception &error)
    {
        fprintf(stderr, "FAIL: %s\n", error.what());
        result = 1;
    }
    CoUninitialize();
    return result;
}
