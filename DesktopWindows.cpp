#include "DesktopWindows.h"
#include <tchar.h>

namespace {
    LRESULT CALLBACK RenderWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
    {
        // GDI retains the trails in the window surface. Erasing on exposure
        // would destroy them; new surfaces are explicitly cleared once.
        if (message == WM_ERASEBKGND) return 1;
        if (message == 0x02E0 /* WM_DPICHANGED */ || message == WM_NCDESTROY)
            if (HWND controller = reinterpret_cast<HWND>(GetWindowLongPtr(window, GWLP_USERDATA)))
                PostMessage(controller, DesktopWindows::ChangedMessage, message, 0);
        return DefWindowProc(window, message, wParam, lParam);
    }

    bool OwnedWindow(HWND window)
    {
        TCHAR name[64] = {};
        return GetWindowThreadProcessId(window, NULL) == GetCurrentThreadId() &&
            GetClassName(window, name, ARRAYSIZE(name)) && !_tcscmp(name, DesktopWindows::ClassName());
    }

    BOOL CALLBACK CollectMonitor(HMONITOR, HDC, LPRECT bounds, LPARAM parameter)
    {
        reinterpret_cast<std::vector<RECT>*>(parameter)->push_back(*bounds);
        return TRUE;
    }
}

LPCTSTR DesktopWindows::ClassName() { return _T("ZMatrixDesktopSurface"); }

bool DesktopWindows::Register(HINSTANCE instance)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = RenderWindowProc;
    wc.hInstance = instance;
    wc.lpszClassName = ClassName();
    return RegisterClass(&wc) != 0 || GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

void DesktopWindows::Adopt(HWND window, const RECT& bounds)
{
    Reset();
    windows.push_back({window, bounds, false});
    SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(controller));
    canvas = bounds;
}

void DesktopWindows::Reset()
{
    for (const auto& window : windows)
        if (OwnedWindow(window.handle))
        {
            SetWindowLongPtr(window.handle, GWLP_USERDATA, 0);
            DestroyWindow(window.handle);
        }
    windows.clear();
    targets.clear();
    dirty = true;
}

bool DesktopWindows::Ready(const DesktopHost& host) const
{
    if (windows.empty()) return false;
    HWND previous = host.insertAfter;
    for (const auto& window : windows)
    {
        if (!OwnedWindow(window.handle) || !IsDesktopRenderWindowReady(host, window.handle, previous)) return false;
        RECT actual = {};
        if (!GetWindowRect(window.handle, &actual) || !EqualRect(&actual, &window.bounds)) return false;
        if (host.layered) previous = window.handle;
    }
    return true;
}

bool DesktopWindows::Position(const DesktopHost& host)
{
    HWND previous = host.insertAfter;
    for (const auto& window : windows)
    {
        if (!OwnedWindow(window.handle) || !PositionDesktopRenderWindow(host, window.handle, window.bounds, previous)) return false;
        if (host.layered) previous = window.handle;
    }
    return Ready(host);
}

bool DesktopWindows::Rebuild(const DesktopHost& host, HINSTANCE instance, const RECT& bounds,
    const std::vector<RECT>& monitorBounds)
{
    Reset();
    canvas = bounds;
    if (IsRectEmpty(&canvas) || monitorBounds.empty()) return false;
    for (const RECT& monitor : monitorBounds)
    {
        HWND window = CreateDesktopRenderWindow(host, instance, ClassName(), monitor);
        if (!window) { Reset(); return false; }
        windows.push_back({window, monitor, false});
        SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(controller));
    }
    if (!Position(host)) { Reset(); return false; }
    targets.resize(windows.size());
    dirty = false;
    return true;
}

bool DesktopWindows::Ensure(DesktopHost& host, HINSTANCE instance, const RECT& bounds, bool multiple)
{
    if (!EqualRect(&canvas, &bounds)) dirty = true;
    if (!dirty && (Ready(host) || Position(host))) return true;
    if (!IsDesktopHostReady(host))
    {
        Reset();
        ReleaseDesktopHost(host);
        if (!InitializeDesktopHost(host)) return false;
    }
    std::vector<RECT> monitors;
    // Keep the classic Explorer/Windows 7 path as a single background window.
    if (multiple && host.layered)
    {
        if (!EnumDisplayMonitors(NULL, NULL, CollectMonitor, reinterpret_cast<LPARAM>(&monitors))) return false;
    }
    else monitors.push_back(bounds);
    return Rebuild(host, instance, bounds, monitors);
}

void DesktopWindows::Show(bool visible, COLORREF background)
{
    for (auto& window : windows)
    {
        if (!OwnedWindow(window.handle)) continue;
        if (!visible) { ShowWindow(window.handle, SW_HIDE); continue; }
        if (!IsWindowVisible(window.handle)) ShowWindow(window.handle, SW_SHOWNOACTIVATE);
        if (!window.initialized)
        {
            HDC dc = GetDC(window.handle);
            if (!dc) continue;
            RECT area; GetClientRect(window.handle, &area);
            SetDCBrushColor(dc, background);
            window.initialized = FillRect(dc, &area, static_cast<HBRUSH>(GetStockObject(DC_BRUSH))) != 0;
            ReleaseDC(window.handle, dc);
        }
    }
}

bool DesktopWindows::Render(IzsMatrix& matrix, IzsMatrixRenderer* renderer)
{
    if (targets.size() != windows.size() || targets.empty()) return false;
    size_t acquired = 0;
    for (; acquired < windows.size(); ++acquired)
    {
        if (!OwnedWindow(windows[acquired].handle) || !windows[acquired].initialized) break;
        auto& target = targets[acquired];
        target.bounds = windows[acquired].bounds;
        OffsetRect(&target.bounds, -canvas.left, -canvas.top);
        target.dc = GetDC(windows[acquired].handle);
        if (!target.dc) break;
        if (!SetViewportOrgEx(target.dc, -target.bounds.left, -target.bounds.top, NULL))
        {
            ReleaseDC(windows[acquired].handle, target.dc);
            break;
        }
    }
    bool result = false;
    if (acquired == windows.size())
        result = renderer ? renderer->RenderTargets(targets.data(), static_cast<unsigned>(targets.size())) != 0 :
            targets.size() == 1 && matrix.Render(targets[0].dc) != 0;
    while (acquired)
    {
        --acquired;
        ReleaseDC(windows[acquired].handle, targets[acquired].dc);
        targets[acquired].dc = NULL;
    }
    return result;
}
