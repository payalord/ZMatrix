// Desktop background window placement for ZMatrix.
#pragma once

#include <windows.h>

struct DesktopHost
{
    HWND progman;
    HWND iconView;
    HWND listView;
    HWND parent;
    HWND insertAfter;
    HWND hiddenWorker;
    bool layered;
};

// Discovery is read-only. The explicit root also allows isolated window tests.
bool FindDesktopHost(HWND progman, DesktopHost& host);
bool InitializeDesktopHost(DesktopHost& host, HWND progman = NULL);
HWND CreateDesktopRenderWindow(const DesktopHost& host, HINSTANCE instance,
    LPCTSTR className, const RECT& screenBounds);
// Returns a hidden, verified child, or NULL; GetLastError reports the failure.
HWND WaitForDesktopRenderWindow(DesktopHost& host, HINSTANCE instance,
    LPCTSTR className, const RECT& screenBounds, DWORD timeoutMs, HWND progman = NULL);
bool IsDesktopHostReady(const DesktopHost& host);
bool IsDesktopRenderWindowReady(const DesktopHost& host, HWND window, HWND previous = NULL);
bool PositionDesktopRenderWindow(const DesktopHost& host, HWND window,
    const RECT& screenBounds, HWND previous = NULL);
void ReleaseDesktopHost(DesktopHost& host);
