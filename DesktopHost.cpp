// Desktop background window placement for ZMatrix.
#include "DesktopHost.h"
#include <tchar.h>

namespace
{
    const UINT CreateWorkerMessage = 0x052C;

    bool HasClass(HWND window, LPCTSTR expected)
    {
        TCHAR name[64] = {};
        return GetClassName(window, name, ARRAYSIZE(name)) && !_tcscmp(name, expected);
    }

    bool SameProcess(HWND first, HWND second)
    {
        DWORD firstId = 0, secondId = 0;
        GetWindowThreadProcessId(first, &firstId);
        GetWindowThreadProcessId(second, &secondId);
        return firstId && firstId == secondId;
    }

    BOOL CALLBACK FindIconHost(HWND window, LPARAM parameter)
    {
        DesktopHost& host = *reinterpret_cast<DesktopHost*>(parameter);
        if (SameProcess(window, host.progman) && HasClass(window, _T("WorkerW")))
        {
            HWND view = FindWindowEx(window, NULL, _T("SHELLDLL_DefView"), NULL);
            if (view)
            {
                host.iconView = view;
                return FALSE;
            }
        }
        return TRUE;
    }

    bool IsWindows7()
    {
        OSVERSIONINFOEX version = {};
        version.dwOSVersionInfoSize = sizeof(version);
        version.dwMajorVersion = 6;
        version.dwMinorVersion = 1;
        DWORDLONG mask = VerSetConditionMask(0, VER_MAJORVERSION, VER_EQUAL);
        mask = VerSetConditionMask(mask, VER_MINORVERSION, VER_EQUAL);
        return VerifyVersionInfo(&version, VER_MAJORVERSION | VER_MINORVERSION, mask) != FALSE;
    }
}

bool FindDesktopHost(HWND progman, DesktopHost& host)
{
    ZeroMemory(&host, sizeof(host));
    if (!IsWindow(progman))
        return false;

    host.progman = progman;
    host.iconView = FindWindowEx(progman, NULL, _T("SHELLDLL_DefView"), NULL);
    if (!host.iconView)
        EnumWindows(FindIconHost, reinterpret_cast<LPARAM>(&host));
    if (!host.iconView)
        return false;

    host.listView = FindWindowEx(host.iconView, NULL, _T("SysListView32"), NULL);
    HWND iconHost = GetParent(host.iconView);

    // Modern Explorer composites the icons as a layered child of Progman.
    // A separate opaque layered child below DefView keeps GDI out of that layer.
    if (iconHost == progman &&
        (GetWindowLongPtr(progman, GWL_EXSTYLE) & WS_EX_NOREDIRECTIONBITMAP) &&
        (GetWindowLongPtr(host.iconView, GWL_EXSTYLE) & WS_EX_LAYERED))
    {
        host.parent = progman;
        host.insertAfter = host.iconView;
        host.layered = true;
        return true;
    }

    // Older Explorer separates the icons and wallpaper into top-level windows.
    // Do not accept the many unrelated helper windows also named WorkerW.
    for (HWND candidate = GetWindow(iconHost, GW_HWNDNEXT); candidate;
         candidate = GetWindow(candidate, GW_HWNDNEXT))
    {
        if (HasClass(candidate, _T("WorkerW")) && SameProcess(candidate, progman) &&
            IsWindowVisible(candidate) &&
            !FindWindowEx(candidate, NULL, _T("SHELLDLL_DefView"), NULL))
        {
            RECT iconBounds = {}, workerBounds = {};
            if (GetWindowRect(iconHost, &iconBounds) && GetWindowRect(candidate, &workerBounds) &&
                workerBounds.left <= iconBounds.left && workerBounds.top <= iconBounds.top &&
                workerBounds.right >= iconBounds.right && workerBounds.bottom >= iconBounds.bottom)
            {
                host.parent = candidate;
                host.insertAfter = HWND_BOTTOM;
                return true;
            }
        }
        if (candidate == progman)
            break;
    }
    return false;
}

bool InitializeDesktopHost(DesktopHost& host)
{
    HWND progman = FindWindow(_T("Progman"), NULL);
    ZeroMemory(&host, sizeof(host));
    if (!progman)
        return false;

    // Explorer's background separation message is undocumented. Inspect the
    // resulting hierarchy, and fail safely if the shell does not provide it.
    const bool modern = (GetWindowLongPtr(progman, GWL_EXSTYLE) & WS_EX_NOREDIRECTIONBITMAP) != 0;
    DWORD_PTR result = 0;
    if (!SendMessageTimeout(progman, CreateWorkerMessage, modern ? 0xD : 0,
        modern ? 1 : 0, SMTO_ABORTIFHUNG | SMTO_BLOCK, 1000, &result))
        return false;
    if (!FindDesktopHost(progman, host))
        return false;

    // Preserve the Windows 7 Aero workaround without hiding a shell window on
    // every animation frame. Restore only the window we actually hid on exit.
    if (!host.layered && IsWindows7() && GetParent(host.iconView) != progman)
    {
        host.hiddenWorker = host.parent;
        ShowWindow(host.hiddenWorker, SW_HIDE);
        host.parent = progman;
        host.insertAfter = HWND_BOTTOM;
    }
    return true;
}

bool PositionDesktopRenderWindow(const DesktopHost& host, HWND window, const RECT& screenBounds)
{
    if (!IsWindow(host.parent) || !IsWindow(host.iconView) || GetParent(window) != host.parent)
        return false;
    POINT origin = { screenBounds.left, screenBounds.top };
    if (!ScreenToClient(host.parent, &origin))
        return false;
    return SetWindowPos(window, host.insertAfter, origin.x, origin.y,
        screenBounds.right - screenBounds.left, screenBounds.bottom - screenBounds.top,
        SWP_NOACTIVATE) != FALSE;
}

HWND CreateDesktopRenderWindow(const DesktopHost& host, HINSTANCE instance,
    LPCTSTR className, const RECT& screenBounds)
{
    const DWORD extendedStyle = WS_EX_TRANSPARENT | WS_EX_NOACTIVATE |
        (host.layered ? WS_EX_LAYERED : 0);
    HWND window = CreateWindowEx(extendedStyle, className, _T("Matrix Code"),
        WS_CHILD | WS_DISABLED | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
        0, 0, 0, 0, host.parent, NULL, instance, NULL);
    if (!window)
        return NULL;
    if ((host.layered && !SetLayeredWindowAttributes(window, 0, 255, LWA_ALPHA)) ||
        !PositionDesktopRenderWindow(host, window, screenBounds))
    {
        // Avoid invoking the application's normal shutdown on a partial startup.
        SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(DefWindowProc));
        DestroyWindow(window);
        return NULL;
    }
    return window;
}

void ReleaseDesktopHost(DesktopHost& host)
{
    if (IsWindow(host.hiddenWorker) && SameProcess(host.hiddenWorker, host.progman) &&
        HasClass(host.hiddenWorker, _T("WorkerW")))
        ShowWindow(host.hiddenWorker, SW_SHOWNOACTIVATE);
    ZeroMemory(&host, sizeof(host));
}
