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
#include "../zsMatrix/IzsMatrixAppearance.h"
#include "../zConfig/resource.h"
#include "../Audio/AudioSettings.h"

static void Check(bool ok, const char *text) { if(!ok) throw std::runtime_error(text); }
static IzsMatrix *matrix;
static HHOOK hook;
static bool accepted, failed, sawConfig, sawCharacters, sawInfo;
static std::wstring captureFolder;
static unsigned refresh;
static DWORD priority;
static bool fontDialog, colorDialog;
static bool testingAudio;
static int audioCase, audioCommits;
static audio::Settings audioCurrent, audioSaved;
static void __stdcall GetAudio(void *, audio::Settings *out) { *out = audioCurrent; }
static DWORD __stdcall PreviewAudio(void *, const audio::Settings *value) { audioCurrent = *value; return ERROR_SUCCESS; }
static DWORD __stdcall CommitAudio(void *, const audio::Settings *value) {
    ++audioCommits;
    if(audioCase == 3) return ERROR_ACCESS_DENIED;
    audioSaved = *value; return ERROR_SUCCESS;
}
static void __stdcall AudioStatus(void *, audio::Status *out) { *out = {audioCurrent.enabled ? audio::Capturing : audio::Disabled,S_OK,0.25}; }
static const audio::HostApi audioHost = {sizeof(audio::HostApi),1,nullptr,GetAudio,PreviewAudio,CommitAudio,AudioStatus};

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
            if(testingAudio) {
                Check(IsWindowEnabled(GetDlgItem(window,IDC_AUDIO)) != FALSE,"Audio editor button unavailable.");
                Click(window,IDC_AUDIO);
                Check((audioCurrent.enabled != FALSE) == (audioCase != 0),"Audio child Cancel failed to restore its snapshot.");
                Capture(window,L"config-audio");
                Click(window,audioCase == 1 ? IDCANCEL : IDOK);
                if(audioCase == 3) {
                    Check(IsWindow(window) && audioCommits == 1,"Save failure closed the parent dialog.");
                    Click(window,IDCANCEL);
                }
                return;
            }
            Check(!IsWindowEnabled(GetDlgItem(window,IDC_AUDIO)),"Legacy ABI unexpectedly enabled the audio editor.");
            sawConfig = true;
            Capture(window,L"config");
            Check(GetDlgItemInt(window,IDC_MAX_STREAM,nullptr,FALSE)==137,"Initial stream value not shown.");
            SetDlgItemInt(window,IDC_MAX_STREAM,173,FALSE);
            SetDlgItemInt(window,IDC_REFRESH,61,FALSE);
            SetDlgItemInt(window,IDC_PROBABILITY,27,FALSE);
            Check(matrix->GetMaxStream()==173 && refresh==61,"Live numerical preview failed.");
            Check(matrix->GetSpecialStringStreamProbability()==0.27f,"Probability preview failed.");
            Check(GetDlgItemInt(window,IDC_BLEND_STRENGTH,nullptr,FALSE)==100,"Initial blend strength not shown.");
            SetDlgItemInt(window,IDC_BLEND_STRENGTH,25,FALSE);
            Check(ReadBlendStrength(*matrix)==25,"Blend strength live preview failed.");
            SetDlgItemInt(window,IDC_BLEND_STRENGTH,101,FALSE);
            Check(ReadBlendStrength(*matrix)==25,"Out-of-range blend strength was applied.");
            const HWND slider=GetDlgItem(window,IDC_BLEND_STRENGTH+SLIDER_OFFSET);
            SendMessageW(slider,TBM_SETPOS,TRUE,35);
            SendMessageW(window,WM_HSCROLL,MAKEWPARAM(TB_THUMBPOSITION,35),reinterpret_cast<LPARAM>(slider));
            Check(ReadBlendStrength(*matrix)==35 && GetDlgItemInt(window,IDC_BLEND_STRENGTH,nullptr,FALSE)==35,"Blend slider and numeric edit differ.");
            Capture(window,L"config-blend");
            // Invalid edits must not modify the engine or underflow unsigned values.
            SetDlgItemTextW(window,IDC_REFRESH,L"0");
            Check(refresh==61,"Invalid refresh time was applied.");
            SetDlgItemInt(window,IDC_REFRESH,61,FALSE);
            Select(window,IDC_BG_MODE,1);
            Check(matrix->GetBGMode()==bgmodeColor && !IsWindowEnabled(GetDlgItem(window,IDC_BLEND)),"Background mode dependencies failed.");
            Check(!IsWindowEnabled(slider) && !IsWindowEnabled(GetDlgItem(window,IDC_BLEND_STRENGTH)),"Bitmap blend strength remained enabled in solid-color mode.");
            Select(window,IDC_BG_MODE,0);
            Check(IsWindowEnabled(slider) && IsWindowEnabled(GetDlgItem(window,IDC_BLEND_STRENGTH)) && ReadBlendStrength(*matrix)==35,"Returning to bitmap mode lost blend strength.");
            Select(window,IDC_BG_MODE,1);
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
        } else if(GetDlgItem(window,IDC_AUDIO_ENABLED)) {
            Check(testingAudio,"Unexpected audio editor.");
            Click(window,IDC_AUDIO_ENABLED);
            Check(audioCurrent.enabled != FALSE,"Audio enable preview failed.");
            SetDlgItemTextW(window,IDC_AUDIO_NUMBER,L"37.5");
            Check(audioCurrent.profiles[audio::WaveformVariation].baseScale[0] == 0.375,"Fractional audio percentage lost.");
            SetDlgItemTextW(window,IDC_AUDIO_NUMBER,L"NaN");
            Check(audioCurrent.profiles[audio::WaveformVariation].baseScale[0] == 0.375,"Invalid audio edit applied.");
            SetDlgItemTextW(window,IDC_AUDIO_NUMBER,L"37.5");
            Select(window,IDC_AUDIO_MODE,audio::SpectralCentroid);
            SetDlgItemTextW(window,IDC_AUDIO_NUMBER+6,L"650");
            SetDlgItemTextW(window,IDC_AUDIO_NUMBER+7,L"-12.5");
            Check(audioCurrent.profiles[audio::SpectralCentroid].globalScale == 6.5 && audioCurrent.profiles[audio::SpectralCentroid].globalOffset == -0.125,"Global audio mapping preview failed.");
            Select(window,IDC_AUDIO_MODE,audio::WaveformVariation);
            Check(audioCurrent.profiles[audio::WaveformVariation].baseScale[0] == 0.375 && audioCurrent.profiles[audio::WaveformVariation].globalScale == 3,"Switching effects mixed their settings.");
            Select(window,IDC_AUDIO_MODE,audio::SpectralCentroid);
            Capture(window,L"audio");
            Click(window,audioCase == 0 ? IDCANCEL : IDOK);
        } else if(testingAudio && audioCase == 3) {
            bool expected = false;
            EnumChildWindows(window,[](HWND child,LPARAM context)->BOOL {
                wchar_t text[256]; GetWindowTextW(child,text,_countof(text));
                if(wcscmp(text,L"The audio settings could not be saved.") == 0) *reinterpret_cast<bool *>(context) = true;
                return TRUE;
            },reinterpret_cast<LPARAM>(&expected));
            Check(expected,"Unexpected error during audio save-failure test.");
            PostMessageW(window,WM_CLOSE,0,0);
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
        typedef int(__stdcall *ConfigureWithAudio)(IzsMatrix *,unsigned &,DWORD &,const audio::HostApi *);
        auto configureWithAudio=reinterpret_cast<ConfigureWithAudio>(GetProcAddress(config,"LaunchConfigFormWithAudio"));
        auto about=reinterpret_cast<Info>(GetProcAddress(config,"LaunchAboutForm"));
        auto hire=reinterpret_cast<Info>(GetProcAddress(config,"LaunchHireForm"));
        Check(configure && configureWithAudio && about && hire,"Missing dialog exports.");
        hook=SetWindowsHookExW(WH_CBT,Observe,nullptr,GetCurrentThreadId());
        Check(hook!=nullptr,"Cannot observe test windows.");
        for(bool accept : {false,true}) {
            accepted=accept; sawConfig=false; sawCharacters=false;
            matrix->SetMaxStream(137); matrix->SetValidCharSet(L"01",2);
            matrix->SetBGMode(bgmodeBitmap); matrix->SetSpecialStringStreamProbability(0.1f);
            Check(ApplyBlendStrength(*matrix,100),"Missing appearance interface.");
            refresh=41; priority=initialPriority; SetPriorityClass(GetCurrentProcess(),initialPriority);
            Check((configure(matrix,refresh,priority)!=0)==accept,"Dialog return value changed.");
            Check(sawConfig && sawCharacters && !failed,"Configuration or character dialog failed.");
            Check(matrix->GetMaxStream()==(accept?173u:137u) && refresh==(accept?61u:41u),"Accept/Cancel did not preserve expected state.");
            Check(ReadBlendStrength(*matrix)==(accept?35u:100u),"Accept/Cancel did not preserve blend strength.");
            if(!accept) Check(priority==initialPriority && GetPriorityClass(GetCurrentProcess())==initialPriority,"Cancel did not restore process priority.");
        }
        testingAudio = true;
        for(audioCase = 0; audioCase < 4; ++audioCase) {
            audioCurrent = audioSaved = audio::Defaults(); audioCommits = 0;
            const bool shouldAccept = audioCase == 0 || audioCase == 2;
            Check((configureWithAudio(matrix,refresh,priority,&audioHost) != 0) == shouldAccept && !failed,"Audio dialog integration failed.");
            Check((audioCurrent.enabled != FALSE) == (audioCase == 2),"Parent Cancel failed to restore the original audio settings.");
            Check((audioSaved.enabled != FALSE) == (audioCase == 2),"Audio preview or Cancel unexpectedly persisted settings.");
            Check(audioCommits == (audioCase >= 2 ? 1 : 0),"Audio persisted before the parent accepted.");
        }
        testingAudio = false;
        about(nullptr); Check(sawInfo && !failed,"About dialog failed.");
        sawInfo=false; hire(nullptr); Check(sawInfo && !failed,"Hire dialog failed.");
        puts("PASS: Native settings/characters/audio/About/Hire dialogs, preview, validation, nested OK/Cancel, save failure and priority restoration.");
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
