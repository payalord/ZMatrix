// Run from the repository root after an x86 build. Uses test-owned help windows;
// does not launch the animation, register COM or change user settings.
// Optional argument: an existing directory for a BMP of the help window.
#define NOMINMAX
#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>
#include "../zConfig/resource.h"

static void Check(bool ok, const char *message) { if(!ok) throw std::runtime_error(message); }
static bool failed, sawDocument;
static unsigned initialDocument;
static std::wstring captureFolder;

static std::wstring Text(HWND control) {
    std::vector<wchar_t> text(GetWindowTextLengthW(control)+1);
    GetWindowTextW(control,text.data(),static_cast<int>(text.size()));
    return text.data();
}
static void Select(HWND window, int id, unsigned value) {
    SendDlgItemMessageW(window,id,CB_SETCURSEL,value,0);
    SendMessageW(window,WM_COMMAND,MAKEWPARAM(id,CBN_SELCHANGE),reinterpret_cast<LPARAM>(GetDlgItem(window,id)));
}
static void Capture(HWND window) {
    if(captureFolder.empty()) return;
    UpdateWindow(window);
    RECT rect; GetWindowRect(window,&rect);
    const int width=rect.right-rect.left, height=rect.bottom-rect.top;
    HDC screen=GetDC(window), memory=CreateCompatibleDC(screen);
    BITMAPINFO info={}; info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth=width; info.bmiHeader.biHeight=-height;
    info.bmiHeader.biPlanes=1; info.bmiHeader.biBitCount=32; info.bmiHeader.biCompression=BI_RGB;
    void *pixels=nullptr;
    HBITMAP bitmap=CreateDIBSection(screen,&info,DIB_RGB_COLORS,&pixels,nullptr,0);
    HGDIOBJ previous=SelectObject(memory,bitmap);
    SendMessageW(window,WM_PRINT,reinterpret_cast<WPARAM>(memory),PRF_CLIENT|PRF_NONCLIENT|PRF_CHILDREN|PRF_ERASEBKGND);
    GdiFlush();
    BITMAPFILEHEADER header={}; header.bfType=0x4d42;
    header.bfOffBits=sizeof(header)+sizeof(info.bmiHeader); header.bfSize=header.bfOffBits+width*height*4;
    FILE *file=nullptr; _wfopen_s(&file,(captureFolder+L"\\help.bmp").c_str(),L"wb");
    if(file) { fwrite(&header,sizeof(header),1,file); fwrite(&info.bmiHeader,sizeof(info.bmiHeader),1,file); fwrite(pixels,1,width*height*4,file); fclose(file); }
    SelectObject(memory,previous); DeleteObject(bitmap); DeleteDC(memory); ReleaseDC(window,screen);
}
static void CALLBACK Exercise(HWND window, UINT, UINT_PTR timer, DWORD) {
    KillTimer(window,timer);
    try {
        HWND edit=GetDlgItem(window,IDC_DOC_TEXT);
        Check(edit!=nullptr,"Documentation opened an error dialog instead of the viewer.");
        sawDocument=true;
        Check((GetWindowLongPtrW(edit,GWL_STYLE)&ES_READONLY)!=0,"Help is not read-only.");
        Check(SendDlgItemMessageW(window,IDC_DOC_SOURCE,CB_GETCURSEL,0,0)==static_cast<LRESULT>(initialDocument),"Wrong initial document.");
        Select(window,IDC_DOC_SOURCE,0);
        Check(Text(edit).find(L"Blend strength (%)")!=std::wstring::npos,"User guide is empty or outdated.");
        Check(Text(edit).find(L"%APPDATA%\\.ZMatrix")!=std::wstring::npos,"Markdown damaged a configuration path.");
        Check(Text(edit).find(L"\\0x0041")!=std::wstring::npos,"Markdown damaged a character escape.");
        Check(SendDlgItemMessageW(window,IDC_DOC_SECTION,CB_GETCOUNT,0,0)>15,"Missing guide sections.");
        SetDlgItemTextW(window,IDC_DOC_FIND,L"Blend strength (%)");
        SendMessageW(window,WM_COMMAND,IDC_DOC_NEXT,0);
        CHARRANGE selection; SendMessageW(edit,EM_EXGETSEL,0,reinterpret_cast<LPARAM>(&selection));
        Check(selection.cpMax>selection.cpMin,"Search did not select its match.");
        std::vector<wchar_t> selected(selection.cpMax-selection.cpMin+1);
        TEXTRANGEW range={selection,selected.data()};
        SendMessageW(edit,EM_GETTEXTRANGE,0,reinterpret_cast<LPARAM>(&range));
        Check(std::wstring(selected.data())==L"Blend strength (%)","Search selected the wrong text.");
        SetDlgItemTextW(window,IDC_DOC_FIND,L"no-such-zmatrix-help-phrase");
        SendMessageW(window,WM_COMMAND,IDC_DOC_NEXT,0);
        Check(Text(GetDlgItem(window,IDC_DOC_STATUS))==L"No matches.","Missing-search result was not reported.");
        Select(window,IDC_DOC_SECTION,3);
        SendMessageW(edit,EM_EXGETSEL,0,reinterpret_cast<LPARAM>(&selection));
        Check(selection.cpMax>selection.cpMin,"Section selection did not navigate.");
        Select(window,IDC_DOC_SOURCE,1);
        Check(SendDlgItemMessageW(window,IDC_DOC_SOURCE,CB_GETCURSEL,0,0)==1,"Readme selection was not retained.");
        // Follow the local user-guide hyperlink without a browser/file association.
        FINDTEXTEXW find={{0,-1},const_cast<wchar_t *>(L"user guide"),{}};
        Check(SendMessageW(edit,EM_FINDTEXTEXW,FR_DOWN,reinterpret_cast<LPARAM>(&find))>=0,"Readme guide link is missing.");
        SendMessageW(edit,EM_EXSETSEL,0,reinterpret_cast<LPARAM>(&find.chrgText));
        CHARFORMAT2W format={sizeof(format)};
        SendMessageW(edit,EM_GETCHARFORMAT,SCF_SELECTION,reinterpret_cast<LPARAM>(&format));
        Check((format.dwEffects&CFE_LINK)!=0,"Local Markdown link is not clickable.");
        ENLINK link={}; link.nmhdr={edit,IDC_DOC_TEXT,EN_LINK}; link.msg=WM_LBUTTONUP; link.chrg=find.chrgText;
        SendMessageW(window,WM_NOTIFY,IDC_DOC_TEXT,reinterpret_cast<LPARAM>(&link));
        Check(SendDlgItemMessageW(window,IDC_DOC_SOURCE,CB_GETCURSEL,0,0)==0,"Local Markdown link left the offline viewer.");
        for(unsigned i=0;i<8;++i) {
            Select(window,IDC_DOC_SOURCE,i);
            Check(Text(edit).size()>100,"A bundled document is missing or blank.");
        }
        Check(!IsWindowEnabled(GetDlgItem(window,IDC_DOC_SECTION)),"Plain-text license has invalid sections.");
        Select(window,IDC_DOC_SOURCE,0);
        RECT before,after; GetWindowRect(edit,&before);
        RECT bounds; GetWindowRect(window,&bounds);
        SetWindowPos(window,nullptr,0,0,bounds.right-bounds.left+80,bounds.bottom-bounds.top+40,SWP_NOMOVE|SWP_NOZORDER);
        GetWindowRect(edit,&after);
        Check(after.right-after.left>before.right-before.left,"Document text did not resize.");
        SetDlgItemTextW(window,IDC_DOC_FIND,L"");
        SendMessageW(edit,EM_SETSEL,0,0);
        Capture(window);
        PostMessageW(window,WM_CLOSE,0,0);
    } catch(const std::exception &error) {
        failed=true; std::fprintf(stderr,"FAIL: %s\n",error.what());
        PostMessageW(window,WM_COMMAND,IDOK,0);
    }
}
static LRESULT CALLBACK Observe(int code, WPARAM wparam, LPARAM lparam) {
    if(code==HCBT_ACTIVATE) {
        HWND window=reinterpret_cast<HWND>(wparam);
        wchar_t name[32]; GetClassNameW(window,name,32);
        if(wcscmp(name,L"#32770")==0 && !GetPropW(window,L"DocumentationTest")) {
            SetPropW(window,L"DocumentationTest",reinterpret_cast<HANDLE>(1));
            SetTimer(window,1,100,Exercise);
        }
    }
    return CallNextHookEx(nullptr,code,wparam,lparam);
}
struct ProcessWindows { DWORD pid; HWND document=nullptr; bool animation=false; };
static BOOL CALLBACK FindProcessWindow(HWND window, LPARAM parameter) {
    auto &state=*reinterpret_cast<ProcessWindows *>(parameter);
    DWORD pid; GetWindowThreadProcessId(window,&pid);
    if(pid!=state.pid) return TRUE;
    if(GetDlgItem(window,IDC_DOC_TEXT)) state.document=window;
    wchar_t name[80]; GetClassNameW(window,name,80);
    if(wcscmp(name,L"ZMatrix")==0 || wcscmp(name,L"ZMatrixListenerClass")==0) state.animation=true;
    return TRUE;
}
static void CommandLine(const wchar_t *option) {
    wchar_t path[32768]; Check(GetFullPathNameW(L"matrix.exe",_countof(path),path,nullptr)!=0,"Cannot locate matrix.exe.");
    std::wstring command=L"\""+std::wstring(path)+L"\" "+option;
    wchar_t temp[MAX_PATH]; GetTempPathW(MAX_PATH,temp);
    STARTUPINFOW startup={sizeof(startup)}; startup.dwFlags=STARTF_USESHOWWINDOW; startup.wShowWindow=SW_HIDE;
    PROCESS_INFORMATION process={};
    Check(CreateProcessW(path,&command[0],nullptr,nullptr,FALSE,0,nullptr,temp,&startup,&process)!=FALSE,"Cannot launch documentation command.");
    ProcessWindows state{process.dwProcessId};
    const ULONGLONG deadline=GetTickCount64()+10000;
    do {
        EnumWindows(FindProcessWindow,reinterpret_cast<LPARAM>(&state));
        if(state.document || WaitForSingleObject(process.hProcess,0)==WAIT_OBJECT_0) break;
        Sleep(20);
    } while(GetTickCount64()<deadline);
    if(state.document) PostMessageW(state.document,WM_CLOSE,0,0);
    const bool exited=WaitForSingleObject(process.hProcess,3000)==WAIT_OBJECT_0;
    DWORD code=1; GetExitCodeProcess(process.hProcess,&code);
    // Stop only the test-owned child if a regression prevents it from closing.
    if(!exited) TerminateProcess(process.hProcess,1);
    CloseHandle(process.hThread); CloseHandle(process.hProcess);
    Check(state.document && !state.animation && exited && code==0,"Help command did not open/close independently of desktop startup.");
}
int wmain(int argc, wchar_t **argv) {
    if(argc>1) captureFolder=argv[1];
    CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
    HMODULE config=nullptr; HHOOK hook=nullptr; int result=0;
    try {
        config=LoadLibraryW(L".\\Config.dll"); Check(config!=nullptr,"Cannot load Config.dll.");
        typedef void(__stdcall *Launcher)(void *,BOOL);
        const auto launch=reinterpret_cast<Launcher>(GetProcAddress(config,"LaunchDocumentation"));
        Check(launch!=nullptr,"Missing documentation export.");
        hook=SetWindowsHookExW(WH_CBT,Observe,nullptr,GetCurrentThreadId()); Check(hook!=nullptr,"Cannot observe the test dialog.");
        for(initialDocument=0;initialDocument<2;++initialDocument) {
            sawDocument=false; launch(nullptr,initialDocument!=0);
            Check(sawDocument && !failed,"Documentation dialog verification failed.");
        }
        UnhookWindowsHookEx(hook); hook=nullptr;
        CommandLine(L"/help"); CommandLine(L"/readme");
        std::puts("PASS: offline documents, Unicode paths/escapes, links, search, sections, resize, read-only text, and standalone /help and /readme from another working directory.");
    } catch(const std::exception &error) { std::fprintf(stderr,"FAIL: %s\n",error.what()); result=1; }
    if(hook) UnhookWindowsHookEx(hook);
    if(config) FreeLibrary(config);
    CoUninitialize(); return result;
}
