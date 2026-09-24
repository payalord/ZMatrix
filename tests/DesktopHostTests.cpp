// Build with DesktopHost.cpp and manifest.xml using the x86 Visual Studio tools.
// Default: isolated, test-owned Explorer layouts. --explorer-smoke: briefly create
// a hidden child on the real desktop; no wallpaper, icon or configuration changes.
#include "../DesktopHost.h"
#include <commctrl.h>
#include <tchar.h>
#include <cstdio>
#include <stdexcept>
#include <vector>

static void Check(bool ok, const char* text)
{
    if (!ok) throw std::runtime_error(text);
}

struct Windows
{
    std::vector<HWND> handles;
    ~Windows()
    {
        for (auto i = handles.rbegin(); i != handles.rend(); ++i)
            if (IsWindow(*i)) DestroyWindow(*i);
    }
    HWND Add(LPCTSTR name, HWND parent = NULL, DWORD extended = 0,
        int x = -320, int y = -160, int width = 960, int height = 640)
    {
        HWND window = CreateWindowEx(extended, name, _T("ZMatrix desktop test"),
            parent ? WS_CHILD : WS_POPUP, x, y, width, height,
            parent, NULL, GetModuleHandle(NULL), NULL);
        Check(window != NULL, "Fixture window creation failed.");
        handles.push_back(window);
        return window;
    }
};

static void Register(LPCTSTR name)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = name;
    Check(RegisterClass(&wc) != 0, "Fixture class registration failed.");
}

static void CheckRenderWindow(const DesktopHost& host, Windows& windows, const RECT& bounds)
{
    HWND render = CreateDesktopRenderWindow(host, GetModuleHandle(NULL), _T("ZMatrixHostTest"), bounds);
    Check(render != NULL, "Background child creation failed.");
    windows.handles.push_back(render);
    Check(GetParent(render) == host.parent, "Incorrect background parent.");
    Check(!IsChild(host.iconView, render), "Animation was inserted into the icon layer.");
    Check(!IsWindowEnabled(render), "Background window must not receive desktop input.");
    RECT actual = {};
    GetWindowRect(render, &actual);
    Check(EqualRect(&actual, &bounds) != FALSE, "Negative virtual-screen coordinates were lost.");
    if (host.layered)
    {
        BYTE alpha = 0; DWORD flags = 0;
        Check(GetLayeredWindowAttributes(render, NULL, &alpha, &flags) != FALSE && alpha == 255 && (flags & LWA_ALPHA), "GDI background is not an opaque layered window.");
        Check(GetWindow(render, GW_HWNDPREV) == host.iconView, "Animation is not directly below the icons.");
    }
}

static void ModernLayout()
{
    Windows windows;
    HWND root = windows.Add(_T("ZMatrixTestProgman"), NULL, WS_EX_NOREDIRECTIONBITMAP);
    HWND icons = windows.Add(_T("SHELLDLL_DefView"), root, WS_EX_LAYERED, 0, 0);
    HWND list = windows.Add(WC_LISTVIEW, icons, 0, 0, 0);
    SetLayeredWindowAttributes(icons, 0, 255, LWA_ALPHA);
    HWND worker = windows.Add(_T("WorkerW"), root, 0, 0, 0);
    SetWindowPos(worker, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    const LONG_PTR iconStyle = GetWindowLongPtr(icons, GWL_STYLE);
    const LRESULT listStyle = SendMessage(list, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
    DesktopHost host = {};
    Check(FindDesktopHost(root, host), "Modern layered desktop was not discovered.");
    Check(host.layered && host.parent == root && host.iconView == icons && host.listView == list, "Incorrect modern desktop selection.");
    RECT bounds = {-320, -160, 640, 480};
    CheckRenderWindow(host, windows, bounds);
    HWND render = windows.handles.back();
    // Wallpaper changes can replace WorkerW. The rendering window belongs to
    // Progman, so it must survive and be repositionable under the same icons.
    DestroyWindow(worker);
    worker = windows.Add(_T("WorkerW"), root, 0, 0, 0);
    Check(IsWindow(render), "Replacing wallpaper WorkerW destroyed the animation.");
    Check(PositionDesktopRenderWindow(host, render, bounds), "Reposition after wallpaper change failed.");
    Check(GetWindow(render, GW_HWNDPREV) == icons, "Icon order changed after wallpaper replacement.");
    bounds = {-240, -120, 400, 360};
    Check(PositionDesktopRenderWindow(host, render, bounds), "Desktop resize failed.");
    RECT actual = {}; GetWindowRect(render, &actual);
    Check(EqualRect(&actual, &bounds) != FALSE, "Resize used screen coordinates as parent coordinates.");
    Check(GetWindowLongPtr(icons, GWL_STYLE) == iconStyle &&
        SendMessage(list, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0) == listStyle,
        "Background placement modified icon styles.");
    std::puts("PASS: modern layered desktop, negative origin, wallpaper replacement and resize.");
}

static void ClassicLayout()
{
    Windows windows;
    HWND root = windows.Add(_T("ZMatrixTestProgman"));
    HWND iconHost = windows.Add(_T("WorkerW"));
    HWND icons = windows.Add(_T("SHELLDLL_DefView"), iconHost, 0, 0, 0);
    windows.Add(WC_LISTVIEW, icons, 0, 0, 0);
    HWND background = windows.Add(_T("WorkerW"));
    HWND unrelated = windows.Add(_T("WorkerW"), NULL, 0, 0, 0, 136, 39);
    ShowWindow(background, SW_SHOWNOACTIVATE);
    ShowWindow(unrelated, SW_SHOWNOACTIVATE);
    SetWindowPos(iconHost, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    SetWindowPos(unrelated, iconHost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    SetWindowPos(background, unrelated, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    SetWindowPos(root, background, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    DesktopHost host = {};
    Check(FindDesktopHost(root, host), "Classic desktop was not discovered.");
    Check(!host.layered && host.parent == background && host.iconView == icons,
        "An unrelated WorkerW was selected as the background.");
    const RECT bounds = {-320, -160, 640, 480};
    CheckRenderWindow(host, windows, bounds);
    ShowWindow(background, SW_HIDE);
    Check(!FindDesktopHost(root, host), "Hidden background was accepted.");
    DestroyWindow(iconHost);
    Check(!FindDesktopHost(root, host), "Desktop without an icon layer was accepted.");
    Check(!FindDesktopHost(NULL, host), "Null root was accepted.");
    std::puts("PASS: classic WorkerW layout, unrelated workers and missing-layer rejection.");
}

static void ExplorerSmoke()
{
    DesktopHost host = {};
    Check(InitializeDesktopHost(host), "Explorer did not provide a background layer.");
    try
    {
        Windows windows;
        const LONG_PTR iconStyle = GetWindowLongPtr(host.iconView, GWL_STYLE);
        const LRESULT listStyle = SendMessage(host.listView, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0);
        RECT bounds = {GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN)};
        bounds.right = bounds.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
        bounds.bottom = bounds.top + GetSystemMetrics(SM_CYVIRTUALSCREEN);
        CheckRenderWindow(host, windows, bounds);
        Check(GetWindowLongPtr(host.iconView, GWL_STYLE) == iconStyle &&
            SendMessage(host.listView, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0) == listStyle,
            "Explorer icon styles changed.");
        std::printf("PASS: Explorer hidden-window smoke; layered=%d, bounds=(%ld,%ld,%ld,%ld), parent=%p, icons=%p.\n",
            host.layered, bounds.left, bounds.top, bounds.right, bounds.bottom, host.parent, host.iconView);
    }
    catch (...) { ReleaseDesktopHost(host); throw; }
    ReleaseDesktopHost(host);
}

int _tmain(int argc, TCHAR** argv)
{
    try
    {
        Register(_T("ZMatrixHostTest"));
        if (argc == 2 && !_tcscmp(argv[1], _T("--explorer-smoke"))) ExplorerSmoke();
        else
        {
            INITCOMMONCONTROLSEX controls = {sizeof(controls), ICC_LISTVIEW_CLASSES};
            InitCommonControlsEx(&controls);
            Register(_T("ZMatrixTestProgman")); Register(_T("SHELLDLL_DefView")); Register(_T("WorkerW"));
            ModernLayout(); ClassicLayout();
        }
        return 0;
    }
    catch (const std::exception& error)
    {
        std::fprintf(stderr, "FAIL: %s (Win32 error %lu)\n", error.what(), GetLastError());
        return 1;
    }
}
