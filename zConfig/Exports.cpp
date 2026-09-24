// Keep the five legacy Config.dll entry points and their Unicode/stdcall ABI.
// No C++ runtime objects or exceptions cross the DLL boundary. See LICENSE.TXT.
#include "UI.h"
#include <new>

static_assert(sizeof(wchar_t) == 2 && sizeof(LOGFONTW) == 92, "The engine requires the Unicode Windows ABI.");

extern "C" int __stdcall SaveConfigToFile(IzsMatrix *matrix, unsigned refresh, DWORD priority, const wchar_t *file) {
    if(!matrix || !file) { SetLastError(ERROR_INVALID_PARAMETER); return 0; }
    try { zconfig::Save(file, zconfig::Capture(*matrix, refresh, priority)); return 1; }
    catch(const zconfig::Error &error) { SetLastError(error.code); }
    catch(const std::bad_alloc &) { SetLastError(ERROR_NOT_ENOUGH_MEMORY); }
    catch(...) { SetLastError(ERROR_INVALID_DATA); }
    return 0;
}
extern "C" int __stdcall LoadConfigFromFile(IzsMatrix *matrix, unsigned &refresh, DWORD &priority, const wchar_t *file) {
    if(!matrix || !file) { SetLastError(ERROR_INVALID_PARAMETER); return 0; }
    try {
        const auto loaded = zconfig::Load(file, zconfig::Capture(*matrix, refresh, priority));
        zconfig::Apply(loaded, *matrix, refresh, priority); return 1;
    } catch(const zconfig::Error &error) { SetLastError(error.code); }
    catch(const std::bad_alloc &) { SetLastError(ERROR_NOT_ENOUGH_MEMORY); }
    catch(...) { SetLastError(ERROR_INVALID_DATA); }
    return 0;
}
static int LaunchConfig(IzsMatrix *matrix, unsigned &refresh, DWORD &priority, const audio::HostApi *audioHost) {
    if(!matrix) { SetLastError(ERROR_INVALID_PARAMETER); return 0; }
    // A message loop can reenter exports via the tray menu. Keep a single editor.
    static bool open = false;
    if(open) return 0;
    open = true;
    matrix->AddRef();
    int result = 0;
    try { result = zconfig::Configure(*matrix, refresh, priority, audioHost); }
    catch(const zconfig::Error &error) { zconfig::ShowError(nullptr, error); }
    catch(...) { zconfig::ShowUnexpectedError(nullptr); }
    matrix->Release();
    open = false;
    return result;
}
extern "C" int __stdcall LaunchConfigForm(IzsMatrix *matrix, unsigned &refresh, DWORD &priority) {
    return LaunchConfig(matrix,refresh,priority,nullptr);
}
extern "C" int __stdcall LaunchConfigFormWithAudio(IzsMatrix *matrix, unsigned &refresh, DWORD &priority, const audio::HostApi *host) {
    if(!host || host->size != sizeof(*host) || host->version != 1 || !host->get || !host->preview || !host->commit || !host->status) {
        SetLastError(ERROR_INVALID_PARAMETER); return 0;
    }
    return LaunchConfig(matrix,refresh,priority,host);
}
extern "C" void __stdcall LaunchAboutForm(void *parent) {
    static bool open = false;
    if(open) return;
    open = true;
    try { zconfig::ShowInfo(IsWindow(static_cast<HWND>(parent)) ? static_cast<HWND>(parent) : nullptr, false); }
    catch(const zconfig::Error &error) { zconfig::ShowError(nullptr, error); }
    catch(...) { zconfig::ShowUnexpectedError(nullptr); }
    open = false;
}
extern "C" void __stdcall LaunchHireForm(void *parent) {
    static bool open = false;
    if(open) return;
    open = true;
    try { zconfig::ShowInfo(IsWindow(static_cast<HWND>(parent)) ? static_cast<HWND>(parent) : nullptr, true); }
    catch(const zconfig::Error &error) { zconfig::ShowError(nullptr, error); }
    catch(...) { zconfig::ShowUnexpectedError(nullptr); }
    open = false;
}
