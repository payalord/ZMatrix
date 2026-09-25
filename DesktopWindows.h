// Disposable desktop surfaces; animation and control windows have independent lifetimes.
#pragma once
#include "DesktopHost.h"
#include "zsMatrix/IzsMatrixRenderer.h"
#include <vector>

class DesktopWindows {
public:
    static const UINT ChangedMessage = WM_APP + 41;
    static LPCTSTR ClassName();
    static bool Register(HINSTANCE instance);
    void SetController(HWND window) { controller = window; }
    void Adopt(HWND window, const RECT& bounds);
    bool Ensure(DesktopHost& host, HINSTANCE instance, const RECT& canvas, bool multiple);
    // Explicit layouts also allow tests without changing the real desktop.
    bool Rebuild(const DesktopHost& host, HINSTANCE instance, const RECT& canvas,
        const std::vector<RECT>& bounds);
    bool Ready(const DesktopHost& host) const;
    void Invalidate() { dirty = true; }
    void Show(bool visible, COLORREF background);
    bool Render(IzsMatrix& matrix, IzsMatrixRenderer* renderer);
    void Reset();
    size_t Count() const { return windows.size(); }
    HWND Window(size_t index) const { return windows[index].handle; }
private:
    struct WindowEntry { HWND handle; RECT bounds; bool initialized; };
    std::vector<WindowEntry> windows;
    std::vector<MatrixRenderTarget> targets;
    RECT canvas = {};
    HWND controller = NULL;
    bool dirty = true;
    bool Position(const DesktopHost& host);
};
