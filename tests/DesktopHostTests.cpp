// Build with DesktopHost.cpp and DesktopWindows.cpp; embed manifest.xml using the x86 VS tools.
// Default: isolated, test-owned Explorer layouts. --explorer-smoke: briefly create
// a hidden child on the real desktop; no wallpaper, icon or configuration changes.
#include "../DesktopHost.h"
#include "../DesktopWindows.h"
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
    Check(!(GetWindowLongPtr(render, GWL_STYLE) & WS_VISIBLE), "Background was shown before startup completed.");
    Check(IsDesktopRenderWindowReady(host, render), "Created background did not pass validation.");
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
    SetWindowPos(render, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    Check(!IsDesktopRenderWindowReady(host, render), "Rendering above icons was allowed.");
    Check(PositionDesktopRenderWindow(host, render, bounds) && IsDesktopRenderWindowReady(host, render),
        "Correct icon order was not restored.");
    SetLayeredWindowAttributes(render, 0, 128, LWA_ALPHA);
    Check(!IsDesktopRenderWindowReady(host, render), "Unexpected desktop transparency was accepted.");
    SetLayeredWindowAttributes(render, 0, 255, LWA_ALPHA);
    SetWindowLongPtr(icons, GWL_EXSTYLE, 0);
    Check(!IsDesktopRenderWindowReady(host, render), "An unavailable icon layer was accepted.");
    Check(!PositionDesktopRenderWindow(host, render, bounds), "An invalid desktop was repositioned.");
    SetWindowLongPtr(icons, GWL_EXSTYLE, WS_EX_LAYERED);
    SetLayeredWindowAttributes(icons, 0, 255, LWA_ALPHA);
    Check(IsDesktopRenderWindowReady(host, render), "Restored icon layer was not accepted.");
    DestroyWindow(list);
    Check(!IsDesktopRenderWindowReady(host, render), "Lost icon list was accepted.");
    Check(!CreateDesktopRenderWindow(host, GetModuleHandle(NULL), _T("ZMatrixHostTest"), bounds),
        "A stale desktop host created a rendering window.");
    std::puts("PASS: modern layered desktop, negative origin, wallpaper replacement and resize.");
}

static void MultipleWindows()
{
    Windows fixture;
    HWND root=fixture.Add(_T("ZMatrixTestProgman"),NULL,WS_EX_NOREDIRECTIONBITMAP);
    HWND icons=fixture.Add(_T("SHELLDLL_DefView"),root,WS_EX_LAYERED,0,0);
    fixture.Add(WC_LISTVIEW,icons,0,0,0);
    SetLayeredWindowAttributes(icons,0,255,LWA_ALPHA);
    DesktopHost host={};Check(FindDesktopHost(root,host),"Multi-window host missing.");
    Check(DesktopWindows::Register(GetModuleHandle(NULL)),"Surface class failed.");
    DesktopWindows surfaces;
    const RECT canvas={-320,-160,640,480};
    const std::vector<RECT> monitors={{-320,-160,0,160},{0,-80,640,480}};
    const DWORD objects=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
    const DWORD userObjects=GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS);
    for(int iteration=0;iteration<30;++iteration)
    {
        Check(surfaces.Rebuild(host,GetModuleHandle(NULL),canvas,monitors),"Monitor windows failed to build.");
        Check(surfaces.Count()==2 && surfaces.Ready(host),"Monitor group validation failed.");
        for(size_t i=0;i<surfaces.Count();++i)
        {
            RECT actual={};GetWindowRect(surfaces.Window(i),&actual);
            Check(EqualRect(&actual,&monitors[i])!=FALSE,"Monitor window bounds changed.");
            Check(!(GetWindowLongPtr(surfaces.Window(i),GWL_STYLE)&WS_VISIBLE),"Rebuilt windows were shown before validation.");
        }
        SetWindowPos(surfaces.Window(1),HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
        Check(!surfaces.Ready(host),"A monitor window above icons was accepted.");
        Check(surfaces.Ensure(host,GetModuleHandle(NULL),canvas,true) && surfaces.Ready(host),"Monitor group order was not repaired.");
        surfaces.Reset();
        Check(GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS)==objects,"Repeated monitor-window rebuilding leaked GDI objects.");
        Check(GetGuiResources(GetCurrentProcess(),GR_USEROBJECTS)==userObjects,"Repeated rebuilding leaked window handles.");
    }
    Check(surfaces.Rebuild(host,GetModuleHandle(NULL),canvas,{canvas}) && surfaces.Count()==1,"Single-monitor transition failed.");
    Check(surfaces.Rebuild(host,GetModuleHandle(NULL),canvas,monitors),"Two-monitor transition failed.");
    DestroyWindow(root);
    Check(!surfaces.Ready(host),"Destroyed Explorer host was accepted.");
    surfaces.Reset();
    Check(!surfaces.Rebuild(host,GetModuleHandle(NULL),canvas,monitors),"Stale host created surfaces.");
    std::puts("PASS: monitor group geometry/order, one/two monitor transitions, lost host and repeated resource cleanup.");
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
    HWND render = windows.handles.back();
    // Move the hidden fixture icon host behind the background explicitly.
    // HWND_TOP can be constrained by foreground activation rules.
    SetWindowPos(iconHost, background, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    Check(!IsDesktopRenderWindowReady(host, render), "Classic background above icons was accepted.");
    SetWindowPos(iconHost, unrelated, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    SetWindowPos(background, iconHost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    Check(IsDesktopRenderWindowReady(host, render), "Restored classic background was rejected.");
    ShowWindow(background, SW_HIDE);
    Check(!IsDesktopRenderWindowReady(host, render), "Unavailable classic background was accepted.");
    Check(!FindDesktopHost(root, host), "Hidden background was accepted.");
    DestroyWindow(iconHost);
    Check(!FindDesktopHost(root, host), "Desktop without an icon layer was accepted.");
    Check(!FindDesktopHost(NULL, host), "Null root was accepted.");
    std::puts("PASS: classic WorkerW layout, unrelated workers and missing-layer rejection.");
}

struct DelayedIcons
{
    HWND view = NULL;
    HWND list = NULL;
};

static void CALLBACK CreateDelayedIcons(HWND root, UINT, UINT_PTR timer, DWORD)
{
    KillTimer(root, timer);
    DelayedIcons& icons = *reinterpret_cast<DelayedIcons*>(GetWindowLongPtr(root, GWLP_USERDATA));
    icons.view = CreateWindowEx(WS_EX_LAYERED, _T("SHELLDLL_DefView"), _T(""), WS_CHILD,
        0, 0, 960, 640, root, NULL, GetModuleHandle(NULL), NULL);
    if (!icons.view) return;
    SetLayeredWindowAttributes(icons.view, 0, 255, LWA_ALPHA);
    icons.list = CreateWindowEx(0, WC_LISTVIEW, _T(""), WS_CHILD,
        0, 0, 960, 640, icons.view, NULL, GetModuleHandle(NULL), NULL);
}

static void DelayedStartup()
{
    Windows windows;
    HWND root = windows.Add(_T("ZMatrixTestProgman"), NULL, WS_EX_NOREDIRECTIONBITMAP);
    const RECT bounds = {-320, -160, 640, 480};
    DesktopHost host = {};
    Check(!CreateDesktopRenderWindow(host, GetModuleHandle(NULL), _T("ZMatrixHostTest"), bounds),
        "Missing desktop fell back to a top-level rendering window.");
    const ULONGLONG start = GetTickCount64();
    HWND render = WaitForDesktopRenderWindow(host, GetModuleHandle(NULL), _T("ZMatrixHostTest"), bounds, 150, root);
    Check(!render && GetLastError() == ERROR_TIMEOUT, "Missing desktop did not time out safely.");
    Check(GetTickCount64() - start >= 150 && !host.parent && !GetWindow(root, GW_CHILD),
        "Timed-out startup left a desktop host or rendering window.");
    PostQuitMessage(7);
    render = WaitForDesktopRenderWindow(host, GetModuleHandle(NULL), _T("ZMatrixHostTest"), bounds, 1000, root);
    Check(!render && GetLastError() == ERROR_CANCELLED, "Shutdown did not cancel startup.");
    MSG message = {};
    Check(PeekMessage(&message, NULL, WM_QUIT, WM_QUIT, PM_REMOVE) && message.wParam == 7,
        "Startup consumed the application quit request.");
    DelayedIcons icons;
    SetWindowLongPtr(root, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&icons));
    Check(SetTimer(root, 1, 100, CreateDelayedIcons) != 0, "Delayed desktop timer creation failed.");
    render = WaitForDesktopRenderWindow(host, GetModuleHandle(NULL), _T("ZMatrixHostTest"), bounds, 3000, root);
    Check(icons.view && icons.list && render, "Startup did not retry after the desktop became ready.");
    windows.handles.push_back(render);
    Check(IsDesktopRenderWindowReady(host, render) && !(GetWindowLongPtr(render, GWL_STYLE) & WS_VISIBLE),
        "Delayed startup returned an unsafe or visible rendering window.");
    ReleaseDesktopHost(host);
    std::puts("PASS: safe startup timeout, cancellation and delayed desktop readiness.");
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
            ModernLayout(); MultipleWindows(); ClassicLayout(); DelayedStartup();
        }
        return 0;
    }
    catch (const std::exception& error)
    {
        std::fprintf(stderr, "FAIL: %s (Win32 error %lu)\n", error.what(), GetLastError());
        return 1;
    }
}
