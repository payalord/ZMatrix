// Author information using Windows Rich Edit, without a VCL/ActiveX browser.
// The original HTML, images and sound are retained as resources. See LICENSE.TXT.
#include "UI.h"
#include <commctrl.h>
#include <richedit.h>
#include <mmsystem.h>
#include <algorithm>

namespace zconfig {
struct Info { bool hire; HBITMAP bitmap = nullptr; ~Info() { if(bitmap) DeleteObject(bitmap); } };
static std::wstring AuthorText(bool hire) {
    const HRSRC resource = FindResourceW(Instance, hire ? L"HIRE.HTML" : L"COMMENTS.HTML", RT_HTML);
    Require(resource != nullptr, L"The author information is missing.");
    const char *data = static_cast<const char *>(LockResource(LoadResource(Instance, resource)));
    Require(data != nullptr, L"Cannot read the author information.");
    std::string html(data, SizeofResource(Instance, resource));
    // These two bundled documents have a small, fixed HTML vocabulary. Convert
    // their body to readable text, retaining every hyperlink's target.
    const auto body = html.find("<body>");
    Require(body != std::string::npos, L"Invalid author information.");
    std::string text, link;
    auto newline = [&]() { if(!text.empty() && text.back() != '\n') text += '\n'; };
    for(size_t i = body+6; i < html.size();) {
        if(html[i] == '<') {
            const auto end = html.find('>', i);
            if(end == std::string::npos) break;
            const auto tag = html.substr(i+1, end-i-1);
            if(tag == "/body") break;
            if(tag.compare(0, 2, "a ") == 0) {
                const auto start = tag.find("href=\"");
                if(start != std::string::npos) {
                    const auto finish = tag.find('"', start+6);
                    link = tag.substr(start+6, finish-start-6);
                }
            } else if(tag == "/a" && !link.empty()) { text += " (" + link + ")"; link.clear(); }
            else if(tag == "br" || tag == "/tr" || tag == "/th" || tag == "/td") newline();
            else if(tag == "hr" || tag == "/p") { newline(); text += '\n'; }
            i = end+1;
        } else {
            const char c = html[i++];
            if(c == '\r' || c == '\n' || c == '\t' || c == ' ') {
                if(!text.empty() && text.back() != '\n' && text.back() != ' ') text += ' ';
            } else text += c;
        }
    }
    const std::pair<const char *, const char *> entities[] = {{"&nbsp;"," "},{"&amp;","&"},{"&lt;","<"},{"&gt;",">"},{"&quot;","\""}};
    for(const auto &entity : entities) {
        const std::string from = entity.first;
        size_t offset = 0;
        while((offset = text.find(from, offset)) != std::string::npos) { text.replace(offset, from.size(), entity.second); ++offset; }
    }
    const int length = MultiByteToWideChar(1252, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
    std::wstring wide(length, 0);
    if(length) MultiByteToWideChar(1252, 0, text.data(), static_cast<int>(text.size()), &wide[0], length);
    return wide;
}
static std::wstring Version() {
    wchar_t file[32768];
    if(!GetModuleFileNameW(nullptr, file, _countof(file))) return L"ZMatrix";
    DWORD unused = 0;
    const DWORD size = GetFileVersionInfoSizeW(file, &unused);
    std::vector<BYTE> bytes(size);
    VS_FIXEDFILEINFO *info = nullptr;
    UINT count = 0;
    if(size && GetFileVersionInfoW(file, 0, size, bytes.data()) &&
       VerQueryValueW(bytes.data(), L"\\", reinterpret_cast<void **>(&info), &count) &&
       count >= sizeof(*info) && info->dwSignature == 0xfeef04bd) {
        wchar_t result[128];
        swprintf_s(result, L"ZMatrix %u.%u.%u.%u", HIWORD(info->dwFileVersionMS), LOWORD(info->dwFileVersionMS), HIWORD(info->dwFileVersionLS), LOWORD(info->dwFileVersionLS));
        return result;
    }
    return L"ZMatrix";
}
static void DrawImage(const DRAWITEMSTRUCT &draw, HBITMAP bitmap) {
    FillRect(draw.hDC, &draw.rcItem, GetSysColorBrush(COLOR_3DFACE));
    BITMAP dimensions = {};
    if(!bitmap || !GetObjectW(bitmap, sizeof(dimensions), &dimensions)) return;
    const double scale = std::min(static_cast<double>(draw.rcItem.right-draw.rcItem.left)/dimensions.bmWidth,
                                  static_cast<double>(draw.rcItem.bottom-draw.rcItem.top)/dimensions.bmHeight);
    const int width = static_cast<int>(dimensions.bmWidth*scale), height = static_cast<int>(dimensions.bmHeight*scale);
    HDC source = CreateCompatibleDC(draw.hDC);
    HGDIOBJ previous = SelectObject(source, bitmap);
    SetStretchBltMode(draw.hDC, HALFTONE); SetBrushOrgEx(draw.hDC, 0, 0, nullptr);
    StretchBlt(draw.hDC, draw.rcItem.left+(draw.rcItem.right-draw.rcItem.left-width)/2,
        draw.rcItem.top+(draw.rcItem.bottom-draw.rcItem.top-height)/2, width, height,
        source, 0, 0, dimensions.bmWidth, dimensions.bmHeight, SRCCOPY);
    SelectObject(source, previous); DeleteDC(source);
}
static INT_PTR CALLBACK InfoProcedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    auto context = reinterpret_cast<Info *>(GetWindowLongPtrW(window, DWLP_USER));
    try {
        if(message == WM_INITDIALOG) {
            context = reinterpret_cast<Info *>(lparam);
            SetWindowLongPtrW(window, DWLP_USER, lparam);
            InitDialog(window);
            SetWindowTextW(window, context->hire ? L"Hire the original author" : L"About ZMatrix");
            SetDlgItemTextW(window, IDC_INFO_TITLE, (context->hire ?
                L"Original author's developer profile\r\nZ. Shaker (Happy Dude)\r\nContact information from the original project." :
                Version() + L"\r\nOriginal author: Z. Shaker (Happy Dude)\r\nWindows port: Payalord").c_str());
            HWND edit = GetDlgItem(window, IDC_INFO_TEXT);
            SendMessageW(edit, EM_SETEVENTMASK, 0, ENM_LINK);
            SendMessageW(edit, EM_AUTOURLDETECT, TRUE, 0);
            SetWindowTextW(edit, AuthorText(context->hire).c_str());
            SendMessageW(edit, EM_SETSEL, 0, 0);
            SendMessageW(edit, EM_SCROLL, SB_TOP, 0);
            context->bitmap = LoadBitmapW(Instance, MAKEINTRESOURCEW(context->hire ? IDB_HIRE : IDB_AUTHOR));
            if(!context->hire) PlaySoundW(MAKEINTRESOURCEW(102), Instance, SND_RESOURCE | SND_ASYNC | SND_NODEFAULT);
            SetFocus(GetDlgItem(window, IDOK));
            return FALSE;
        }
        if(!context) return FALSE;
        if(message == WM_DRAWITEM && reinterpret_cast<DRAWITEMSTRUCT *>(lparam)->CtlID == IDC_INFO_IMAGE) {
            DrawImage(*reinterpret_cast<DRAWITEMSTRUCT *>(lparam), context->bitmap); return TRUE;
        }
        if(message == WM_NOTIFY) {
            const auto link = reinterpret_cast<ENLINK *>(lparam);
            if(link->nmhdr.idFrom == IDC_INFO_TEXT && link->nmhdr.code == EN_LINK && link->msg == WM_LBUTTONUP) {
                std::vector<wchar_t> text(static_cast<size_t>(link->chrg.cpMax-link->chrg.cpMin)+1);
                TEXTRANGEW range = {link->chrg, text.data()};
                SendMessageW(link->nmhdr.hwndFrom, EM_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&range));
                Open(window, text.data()); return TRUE;
            }
        }
        if(message == WM_COMMAND) {
            switch(LOWORD(wparam)) {
            case IDOK: case IDCANCEL: EndDialog(window, IDOK); return TRUE;
            case IDC_DONATE: HelpCommand(window, IDM_DONATE); return TRUE;
            case IDC_HOMEPAGE: HelpCommand(window, IDM_HOMEPAGE); return TRUE;
            }
        }
        if(message == WM_CLOSE) { EndDialog(window, IDOK); return TRUE; }
        if(message == WM_DESTROY && !context->hire) PlaySoundW(nullptr, nullptr, 0);
    } catch(const Error &error) { ShowError(window, error); }
    catch(...) { ShowUnexpectedError(window); }
    if(message == WM_INITDIALOG) EndDialog(window, IDCANCEL);
    return FALSE;
}
void ShowInfo(HWND owner, bool hire) {
    Info context{hire};
    Dialog(IDD_INFO, owner, InfoProcedure, reinterpret_cast<LPARAM>(&context));
}
}
