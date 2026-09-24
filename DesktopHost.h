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
bool InitializeDesktopHost(DesktopHost& host);
HWND CreateDesktopRenderWindow(const DesktopHost& host, HINSTANCE instance,
    LPCTSTR className, const RECT& screenBounds);
bool PositionDesktopRenderWindow(const DesktopHost& host, HWND window,
    const RECT& screenBounds);
void ReleaseDesktopHost(DesktopHost& host);
