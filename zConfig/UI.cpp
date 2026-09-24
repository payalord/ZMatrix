// Native Win32 dialog support. See LICENSE.TXT.
#include "UI.h"
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>

namespace zconfig {
HINSTANCE Instance = nullptr;
void ShowError(HWND owner, const Error &error) {
    MessageBoxW(owner, error.message.c_str(), L"ZMatrix", MB_OK | MB_ICONERROR);
}
void ShowUnexpectedError(HWND owner) {
    MessageBoxW(owner, L"The operation could not be completed.", L"ZMatrix", MB_OK | MB_ICONERROR);
}
std::wstring WindowText(HWND window) {
    std::vector<wchar_t> text(static_cast<size_t>(GetWindowTextLengthW(window)) + 1);
    GetWindowTextW(window, text.data(), static_cast<int>(text.size()));
    return text.data();
}
std::wstring ModuleFolder() {
    std::vector<wchar_t> file(32768);
    DWORD count = GetModuleFileNameW(Instance, file.data(), static_cast<DWORD>(file.size()));
    Require(count && count < file.size(), L"Cannot locate Config.dll.");
    const std::wstring path(file.data(), count);
    return path.substr(0, path.find_last_of(L'\\') + 1);
}
void Open(HWND owner, const std::wstring &target) {
    Require(reinterpret_cast<INT_PTR>(ShellExecuteW(owner, L"open", target.c_str(), nullptr, nullptr, SW_SHOWNORMAL)) > 32,
            L"Windows could not open the selected document or link.");
}
std::wstring SelectFile(HWND owner, bool save, bool configuration) {
    std::vector<wchar_t> file(32768);
    const std::wstring folder = ModuleFolder();
    OPENFILENAMEW dialog = {sizeof(dialog)};
    dialog.hwndOwner = owner;
    dialog.lpstrFilter = configuration ? L"ZMatrix configuration (*.cfg)\0*.cfg\0All files (*.*)\0*.*\0" :
                                        L"Text files (*.txt)\0*.txt\0All files (*.*)\0*.*\0";
    dialog.lpstrFile = file.data(); dialog.nMaxFile = static_cast<DWORD>(file.size());
    dialog.lpstrInitialDir = folder.c_str(); dialog.lpstrDefExt = configuration ? L"cfg" : L"txt";
    dialog.Flags = OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST |
                   (save ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST);
    if(save ? GetSaveFileNameW(&dialog) : GetOpenFileNameW(&dialog)) return file.data();
    if(CommDlgExtendedError()) throw Error{L"The file dialog could not be opened.", CommDlgExtendedError()};
    return {};
}
void InitDialog(HWND window) {
    SendMessageW(window, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(LoadIconW(Instance, MAKEINTRESOURCEW(IDI_MATRIX))));
    RECT bounds, work;
    GetWindowRect(window, &bounds);
    MONITORINFO monitor = {sizeof(monitor)};
    if(GetMonitorInfoW(MonitorFromWindow(GetWindow(window, GW_OWNER), MONITOR_DEFAULTTOPRIMARY), &monitor)) work = monitor.rcWork;
    else SystemParametersInfoW(SPI_GETWORKAREA, 0, &work, 0);
    SetWindowPos(window, nullptr, work.left + (work.right-work.left-(bounds.right-bounds.left))/2,
        work.top + (work.bottom-work.top-(bounds.bottom-bounds.top))/2, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}
INT_PTR Dialog(int resource, HWND owner, DLGPROC procedure, LPARAM context) {
    INITCOMMONCONTROLSEX controls = {sizeof(controls), ICC_BAR_CLASSES};
    Require(InitCommonControlsEx(&controls) != FALSE, L"Cannot initialize Windows controls.");
    // System32 is available on every supported Windows version, including Windows 7.
    wchar_t system[MAX_PATH];
    Require(GetSystemDirectoryW(system, MAX_PATH) != 0, L"Cannot locate Windows system controls.");
    HMODULE richEdit = LoadLibraryW((std::wstring(system) + L"\\Msftedit.dll").c_str());
    Require(richEdit != nullptr, L"Cannot load the Windows text editor.");
    const INT_PTR result = DialogBoxParamW(Instance, MAKEINTRESOURCEW(resource), owner, procedure, context);
    const DWORD error = GetLastError();
    FreeLibrary(richEdit);
    if(result == -1) throw Error{L"Cannot create the ZMatrix dialog.", error};
    return result;
}
bool HelpCommand(HWND owner, unsigned command) {
    const wchar_t *url = nullptr;
    switch(command) {
    case IDM_HELP: ShowDocument(owner, false); return true;
    case IDM_README: ShowDocument(owner, true); return true;
    case IDM_HOMEPAGE: url = L"https://payalord.github.io/ZMatrix/"; break;
    case IDM_PROJECT: url = L"https://github.com/payalord/ZMatrix"; break;
    case IDM_SUPPORT: case IDM_BUGS: case IDM_FEATURES: case IDM_DISCUSSION:
        url = L"https://github.com/payalord/ZMatrix/issues"; break;
    case IDM_DONATE: url = L"https://zmatrix.sourceforge.net/donate.html"; break;
    case IDM_HIRE: ShowInfo(owner, true); return true;
    case IDM_ABOUT: ShowInfo(owner, false); return true;
    default: return false;
    }
    Open(owner, url); return true;
}
}
