#pragma once
#include "Settings.h"
#include "resource.h"
#include "../Audio/AudioSettings.h"

namespace zconfig {
extern HINSTANCE Instance;
void ShowError(HWND owner, const Error &error);
void ShowUnexpectedError(HWND owner);
std::wstring WindowText(HWND window);
std::wstring ModuleFolder();
void Open(HWND owner, const std::wstring &target);
std::wstring SelectFile(HWND owner, bool save, bool configuration);
void InitDialog(HWND window);
INT_PTR Dialog(int resource, HWND owner, DLGPROC procedure, LPARAM context);
int Configure(IzsMatrix &matrix, unsigned &refresh, DWORD &priority, const audio::HostApi *audioHost = nullptr);
bool EditAudio(HWND owner, const audio::HostApi &host);
bool EditCharacters(HWND owner, Settings &settings);
void ShowInfo(HWND owner, bool hire);
void ShowDocument(HWND owner, bool readme);
bool HelpCommand(HWND owner, unsigned command);
}
