// Character set and special-string editor. See LICENSE.TXT.
#include "UI.h"
#include <richedit.h>

namespace zconfig {
struct CharacterEditor { Settings &settings; };
static std::wstring Lines(const std::wstring &text) {
    // Rich Edit internally uses CR; plain Windows edit controls use CRLF.
    std::wstring result;
    for(size_t i = 0; i < text.size(); ++i) {
        if(text[i] == L'\r') { result += L'\n'; if(i+1 < text.size() && text[i+1] == L'\n') ++i; }
        else result += text[i];
    }
    return result;
}
static INT_PTR CALLBACK CharacterProcedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    auto context = reinterpret_cast<CharacterEditor *>(GetWindowLongPtrW(window, DWLP_USER));
    try {
        if(message == WM_INITDIALOG) {
            context = reinterpret_cast<CharacterEditor *>(lparam);
            SetWindowLongPtrW(window, DWLP_USER, lparam);
            InitDialog(window);
            for(int id : {IDC_CHAR_TEXT, IDC_STRING_TEXT}) {
                SendDlgItemMessageW(window, id, EM_SETTEXTMODE, TM_PLAINTEXT, 0);
                SendDlgItemMessageW(window, id, EM_EXLIMITTEXT, 0, 1024 * 1024);
            }
            SetDlgItemTextW(window, IDC_CHAR_TEXT, FormatCharacters(context->settings.characters, false).c_str());
            SetDlgItemTextW(window, IDC_STRING_TEXT, FormatStrings(context->settings.strings, L'\n', false).c_str());
            return TRUE;
        }
        if(!context) return FALSE;
        if(message == WM_CLOSE) { EndDialog(window, IDCANCEL); return TRUE; }
        if(message != WM_COMMAND) return FALSE;
        const unsigned id = LOWORD(wparam);
        if(id == IDCANCEL) { EndDialog(window, IDCANCEL); return TRUE; }
        if(id == IDOK) {
            auto characters = ParseCharacters(WindowText(GetDlgItem(window, IDC_CHAR_TEXT)));
            auto strings = ParseStrings(Lines(WindowText(GetDlgItem(window, IDC_STRING_TEXT))), L'\n');
            context->settings.characters = std::move(characters);
            context->settings.strings = std::move(strings);
            EndDialog(window, IDOK); return TRUE;
        }
        const bool characters = id == IDC_LOAD_CHAR || id == IDC_SAVE_CHAR;
        const bool save = id == IDC_SAVE_CHAR || id == IDC_SAVE_STRINGS;
        if(!characters && id != IDC_LOAD_STRINGS && id != IDC_SAVE_STRINGS) return FALSE;
        const auto file = SelectFile(window, save, false);
        if(file.empty()) return TRUE;
        const HWND edit = GetDlgItem(window, characters ? IDC_CHAR_TEXT : IDC_STRING_TEXT);
        if(save) {
            const auto text = Lines(WindowText(edit));
            WriteText(file, characters ? FormatCharacters(ParseCharacters(text), true) : FormatStrings(ParseStrings(text, L'\n'), L'\n', true));
        } else {
            const auto text = Lines(ReadText(file));
            SetWindowTextW(edit, (characters ? FormatCharacters(ParseCharacters(text), false) : FormatStrings(ParseStrings(text, L'\n'), L'\n', false)).c_str());
        }
        return TRUE;
    } catch(const Error &error) { ShowError(window, error); }
    catch(...) { ShowUnexpectedError(window); }
    if(message == WM_INITDIALOG) EndDialog(window, IDCANCEL);
    return TRUE;
}
bool EditCharacters(HWND owner, Settings &settings) {
    CharacterEditor context{settings};
    return Dialog(IDD_CHARACTERS, owner, CharacterProcedure, reinterpret_cast<LPARAM>(&context)) == IDOK;
}
}
