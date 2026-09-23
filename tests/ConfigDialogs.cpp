// Integration smoke test for native dialogs. Run from the repository root.
// Uses an unregistered engine and test-owned windows; never launches matrix.exe.
// Optional argument: directory for BMP captures of these windows.
#define NOMINMAX
#include <windows.h>
#include <commctrl.h>
#include <dlgs.h>
#include <cstdio>
#include <string>
#include <vector>
#include <stdexcept>
#include "../zsMatrix/IzsMatrix.h"
#include "../zConfig/resource.h"

static void Check(bool ok, const char *text) { if(!ok) throw std::runtime_error(text); }
static IzsMatrix *matrix;
static HHOOK hook;
static bool accepted, failed, sawConfig, sawCharacters, sawInfo;
static std::wstring captureFolder;
static unsigned refresh;
static DWORD priority;
static bool fontDialog, colorDialog;

static void Capture(HWND window, const wchar_t *name) {
    if(captureFolder.empty()) return;
    UpdateWindow(window);
    RECT rect; GetWindowRect(window, &rect);
    const int width = rect.right-rect.left, height = rect.bottom-rect.top;
    HDC dc = GetDC(window), memory = CreateCompatibleDC(dc);
    BITMAPINFO info = {};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = width; info.bmiHeader.biHeight = -height;
    info.bmiHeader.biPlanes = 1; info.bmiHeader.biBitCount = 32; info.bmiHeader.biCompression = BI_RGB;
    void *pixels = nullptr;
    HBITMAP bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &pixels, nullptr, 0);
    HGDIOBJ previous = SelectObject(memory, bitmap);
    SendMessageW(window,WM_PRINT,reinterpret_cast<WPARAM>(memory),PRF_CLIENT | PRF_NONCLIENT | PRF_CHILDREN | PRF_ERASEBKGND);
    GdiFlush();
    BITMAPFILEHEADER header = {};
    header.bfType = 0x4d42; header.bfOffBits = sizeof(header) + sizeof(info.bmiHeader);
    header.bfSize = header.bfOffBits + width*height*4;
    FILE *file = nullptr;
    _wfopen_s(&file, (captureFolder+L"\\"+name+L".bmp").c_str(), L"wb");
    if(file) { fwrite(&header, sizeof(header), 1, file); fwrite(&info.bmiHeader, sizeof(info.bmiHeader), 1, file); fwrite(pixels, 1, width*height*4, file); fclose(file); }
    SelectObject(memory, previous); DeleteObject(bitmap); DeleteDC(memory); ReleaseDC(window, dc);
}
static void Click(HWND window, int id) { SendMessageW(GetDlgItem(window,id), BM_CLICK, 0, 0); }
static void Select(HWND window, int id, int item) {
    SendDlgItemMessageW(window,id,CB_SETCURSEL,item,0);
    SendMessageW(window,WM_COMMAND,MAKEWPARAM(id,CBN_SELCHANGE),reinterpret_cast<LPARAM>(GetDlgItem(window,id)));
}
static void CALLBACK Exercise(HWND window, UINT, UINT_PTR timer, DWORD) {
    KillTimer(window,timer);
    try {
        if(GetDlgItem(window,IDC_MAX_STREAM)) {
            sawConfig = true;
            Capture(window,L"config");
            Check(GetDlgItemInt(window,IDC_MAX_STREAM,nullptr,FALSE)==137,"Initial stream value not shown.");
            SetDlgItemInt(window,IDC_MAX_STREAM,173,FALSE);
            SetDlgItemInt(window,IDC_REFRESH,61,FALSE);
            SetDlgItemInt(window,IDC_PROBABILITY,27,FALSE);
            Check(matrix->GetMaxStream()==173 && refresh==61,"Live numerical preview failed.");
            Check(matrix->GetSpecialStringStreamProbability()==0.27f,"Probability preview failed.");
            // Invalid edits must not modify the engine or underflow unsigned values.
            SetDlgItemTextW(window,IDC_REFRESH,L"0");
            Check(refresh==61,"Invalid refresh time was applied.");
            SetDlgItemInt(window,IDC_REFRESH,61,FALSE);
            Select(window,IDC_BG_MODE,1);
            Check(matrix->GetBGMode()==bgmodeColor && !IsWindowEnabled(GetDlgItem(window,IDC_BLEND)),"Background mode dependencies failed.");
            const bool wasEnabled=matrix->GetMonotonousCleanupEnabled();
            Click(window,IDC_MONOTONOUS);
            Check(matrix->GetMonotonousCleanupEnabled()!=wasEnabled,"Cleanup toggle failed.");
            Check(IsWindowEnabled(GetDlgItem(window,IDC_BACKTRACE))==!wasEnabled,"Cleanup dependency failed.");
            Select(window,IDC_PRIORITY,1);
            Check(priority==BELOW_NORMAL_PRIORITY_CLASS && GetPriorityClass(GetCurrentProcess())==priority,"Priority preview failed.");
            fontDialog=true; Click(window,IDC_FONT); fontDialog=false;
            colorDialog=true; Click(window,IDC_FOREGROUND); colorDialog=false;
            Click(window,IDC_CHARACTERS);
            if(accepted) {
                Check(matrix->GetNumCharsInSet()==4 && std::wstring(matrix->GetValidCharSet(),4)==L"A,\\\x0416","Character editor result lost.");
                Check(matrix->GetNumSpecialStringsInSet()==2 && std::wstring(matrix->GetValidSpecialString(1))==L"\x65e5\x672c; \\ text","Special string editor result lost.");
            } else Check(matrix->GetNumCharsInSet()==2,"Child dialog Cancel changed characters.");
            Click(window,accepted ? IDOK : IDCANCEL);
        } else if(GetDlgItem(window,IDC_CHAR_TEXT)) {
            sawCharacters = true;
            SetDlgItemTextW(window,IDC_CHAR_TEXT,L"A, \\, , \\\\, \x0416");
            SetDlgItemTextW(window,IDC_STRING_TEXT,L"First\r\n\x65e5\x672c; \\\\ text");
            Capture(window,L"characters");
            Click(window,accepted ? IDOK : IDCANCEL);
        } else if(GetDlgItem(window,IDC_INFO_TEXT)) {
            sawInfo = true;
            const int count=GetWindowTextLengthW(GetDlgItem(window,IDC_INFO_TEXT));
            Check(count>1000,"Author content was not loaded.");
            std::vector<wchar_t> text(count+1); GetDlgItemTextW(window,IDC_INFO_TEXT,text.data(),count+1);
            const std::wstring content(text.data());
            Check(content.find(L"zmatrix_background@hotmail.com")!=std::wstring::npos,"Author contact missing.");
            wchar_t title[128]; GetWindowTextW(window,title,128);
            Capture(window,wcscmp(title,L"About ZMatrix")==0 ? L"about" : L"hire");
            Click(window,IDOK);
        } else if(fontDialog) {
            Check(GetDlgItem(window,chx1) && GetDlgItem(window,chx2),"Font effects controls missing.");
            Capture(window,L"font");
            Click(window,IDCANCEL);
        } else if(colorDialog) {
            Capture(window,L"color");
            Click(window,IDCANCEL);
        } else {
            failed = true;
            fprintf(stderr,"FAIL: Unexpected dialog, possibly an error message.\n");
            wchar_t title[256]; GetWindowTextW(window,title,256); fwprintf(stderr,L"Window: %ls\n",title);
            EnumChildWindows(window,[](HWND child,LPARAM)->BOOL {
                wchar_t text[1024]; GetWindowTextW(child,text,1024); fwprintf(stderr,L"  %ls\n",text); return TRUE;
            },0);
            Capture(window,L"unexpected");
            PostMessageW(window,WM_CLOSE,0,0);
        }
    } catch(const std::exception &error) {
        failed = true; fprintf(stderr,"FAIL: %s\n",error.what());
        PostMessageW(window,WM_CLOSE,0,0);
    }
}
static LRESULT CALLBACK Observe(int code, WPARAM wparam, LPARAM lparam) {
    if(code==HCBT_ACTIVATE) {
        const HWND window=reinterpret_cast<HWND>(wparam);
        wchar_t name[64]; GetClassNameW(window,name,64);
        if(wcscmp(name,L"#32770")==0 && !GetPropW(window,L"ZMatrixTestObserved")) {
            SetPropW(window,L"ZMatrixTestObserved",reinterpret_cast<HANDLE>(1));
            SetTimer(window,99,100,Exercise);
        }
    }
    return CallNextHookEx(hook,code,wparam,lparam);
}
int wmain(int argc,wchar_t **argv) {
    if(argc>1) captureFolder=argv[1];
    CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
    HMODULE engine=nullptr, config=nullptr;
    IClassFactory *factory=nullptr;
    const DWORD initialPriority=GetPriorityClass(GetCurrentProcess());
    int result=0;
    try {
        engine=LoadLibraryW(L".\\zsMatrix.dll"); config=LoadLibraryW(L".\\Config.dll");
        Check(engine && config,"Cannot load DLLs.");
        typedef HRESULT(STDAPICALLTYPE *GetFactory)(REFCLSID,REFIID,void **);
        auto getFactory=reinterpret_cast<GetFactory>(GetProcAddress(engine,"DllGetClassObject"));
        const GUID cls={0x2e393599,0xf2e3,0x484a,{0x9b,0x03,0xd4,0x14,0xf6,0xcb,0xa5,0xeb}};
        const GUID iid={0xcbeac95f,0x3125,0x4603,{0xa0,0xc9,0x09,0x29,0xbd,0xc6,0x0f,0xbc}};
        Check(getFactory && SUCCEEDED(getFactory(cls,IID_IClassFactory,reinterpret_cast<void **>(&factory))),"Cannot get engine factory.");
        Check(SUCCEEDED(factory->CreateInstance(nullptr,iid,reinterpret_cast<void **>(&matrix))),"Cannot create test engine.");
        typedef int(__stdcall *Configure)(IzsMatrix *,unsigned &,DWORD &);
        typedef void(__stdcall *Info)(void *);
        auto configure=reinterpret_cast<Configure>(GetProcAddress(config,"LaunchConfigForm"));
        auto about=reinterpret_cast<Info>(GetProcAddress(config,"LaunchAboutForm"));
        auto hire=reinterpret_cast<Info>(GetProcAddress(config,"LaunchHireForm"));
        Check(configure && about && hire,"Missing dialog exports.");
        hook=SetWindowsHookExW(WH_CBT,Observe,nullptr,GetCurrentThreadId());
        Check(hook!=nullptr,"Cannot observe test windows.");
        for(bool accept : {false,true}) {
            accepted=accept; sawConfig=false; sawCharacters=false;
            matrix->SetMaxStream(137); matrix->SetValidCharSet(L"01",2);
            matrix->SetBGMode(bgmodeBitmap); matrix->SetSpecialStringStreamProbability(0.1f);
            refresh=41; priority=initialPriority; SetPriorityClass(GetCurrentProcess(),initialPriority);
            Check((configure(matrix,refresh,priority)!=0)==accept,"Dialog return value changed.");
            Check(sawConfig && sawCharacters && !failed,"Configuration or character dialog failed.");
            Check(matrix->GetMaxStream()==(accept?173u:137u) && refresh==(accept?61u:41u),"Accept/Cancel did not preserve expected state.");
            if(!accept) Check(priority==initialPriority && GetPriorityClass(GetCurrentProcess())==initialPriority,"Cancel did not restore process priority.");
        }
        about(nullptr); Check(sawInfo && !failed,"About dialog failed.");
        sawInfo=false; hire(nullptr); Check(sawInfo && !failed,"Hire dialog failed.");
        puts("PASS: Native settings/characters/About/Hire dialogs, live preview, validation, dependencies, OK/Cancel and priority restoration.");
    } catch(const std::exception &error) { fprintf(stderr,"FAIL: %s\n",error.what()); result=1; }
    if(hook) UnhookWindowsHookEx(hook);
    SetPriorityClass(GetCurrentProcess(),initialPriority);
    if(matrix) matrix->Release();
    if(factory) factory->Release();
    if(config) FreeLibrary(config);
    if(engine) FreeLibrary(engine);
    CoUninitialize();
    return result;
}
