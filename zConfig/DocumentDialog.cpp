// Offline viewer for the bundled Markdown documents. See LICENSE.TXT.
#include "UI.h"
#include <commctrl.h>
#include <richedit.h>
#include <algorithm>
#include <cstdio>
#include <memory>
#include <sstream>

namespace zconfig {
namespace {
struct Source { const wchar_t *title, *file; };
const Source Sources[] = {
    {L"User guide", L"docs\\USER_GUIDE.md"}, {L"Readme", L"README.md"},
    {L"Building", L"BUILDING.md"}, {L"Module guide", L"docs\\DEVELOPMENT.md"},
    {L"Build scripts", L"docs\\SCRIPTS.md"}, {L"Original readme (historical)", L"ORIGINALREADME.md"},
    {L"Font notice", L"Matrix Code Font ReadMe.txt"}, {L"License", L"LICENSE.TXT"}
};
struct Heading { LONG start, end; unsigned level; std::wstring title; };
struct Link { LONG start, end; std::wstring target; };
struct Document {
    unsigned source;
    std::wstring text;
    std::vector<Heading> headings;
    std::vector<Link> links;
};
std::wstring ReadDocument(const wchar_t *file) {
    FILE *raw = nullptr;
    _wfopen_s(&raw, (ModuleFolder() + file).c_str(), L"rb");
    Require(raw != nullptr, L"The documentation file is missing or cannot be opened. Repair the ZMatrix installation.");
    std::unique_ptr<FILE, decltype(&fclose)> input(raw, fclose);
    Require(_fseeki64(raw, 0, SEEK_END) == 0, L"Cannot read the documentation file.");
    const auto size = _ftelli64(raw);
    Require(size > 0 && size <= 1024 * 1024, L"The documentation file is empty or too large.");
    rewind(raw);
    std::string bytes(static_cast<size_t>(size), '\0');
    Require(fread(&bytes[0], 1, bytes.size(), raw) == bytes.size(), L"Cannot read the documentation file.");
    const int offset = bytes.compare(0, 3, "\xef\xbb\xbf") == 0 ? 3 : 0;
    const int count = static_cast<int>(bytes.size()) - offset;
    const int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, bytes.data()+offset, count, nullptr, 0);
    Require(length > 0, L"The documentation must contain valid UTF-8 text.");
    std::wstring text(length, L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, bytes.data()+offset, count, &text[0], length);
    return text;
}
void Inline(Document &doc, const std::wstring &line) {
    // The bundled prose uses inline code, escaped punctuation and simple links.
    bool code = false;
    for(size_t i = 0; i < line.size(); ++i) {
        if(line[i] == L'`') { code = !code; continue; }
        if(!code && line[i] == L'\\' && i+1 < line.size() &&
            std::wstring(L"\\`*_{}[]()#+-.!>").find(line[i+1]) != std::wstring::npos) {
            doc.text += line[++i]; continue;
        }
        if(!code && line[i] == L'[') {
            const auto labelEnd = line.find(L"](", i+1);
            const auto targetEnd = labelEnd == std::wstring::npos ? labelEnd : line.find(L')', labelEnd+2);
            if(targetEnd != std::wstring::npos) {
                const LONG start = static_cast<LONG>(doc.text.size());
                doc.text += line.substr(i+1, labelEnd-i-1);
                doc.links.push_back({start, static_cast<LONG>(doc.text.size()), line.substr(labelEnd+2, targetEnd-labelEnd-2)});
                i = targetEnd; continue;
            }
        }
        if(!code && (line.compare(i, 8, L"https://") == 0 || line.compare(i, 7, L"http://") == 0)) {
            auto end = line.find_first_of(L" \t<>)", i);
            if(end == std::wstring::npos) end = line.size();
            while(end > i && (line[end-1] == L'.' || line[end-1] == L',')) --end;
            const LONG start = static_cast<LONG>(doc.text.size());
            const auto target = line.substr(i, end-i);
            doc.text += target;
            doc.links.push_back({start, static_cast<LONG>(doc.text.size()), target});
            i = end-1; continue;
        }
        doc.text += line[i];
    }
}
Document Parse(unsigned source) {
    Document doc{source};
    const auto input = ReadDocument(Sources[source].file);
    const std::wstring file = Sources[source].file;
    if(file.size() < 3 || file.substr(file.size()-3) != L".md") {
        for(wchar_t ch : input) if(ch != L'\r') doc.text += ch == L'\n' ? L'\r' : ch;
        return doc;
    }
    std::wistringstream stream(input);
    std::wstring line;
    bool code = false, paragraph = false;
    while(std::getline(stream, line)) {
        if(!line.empty() && line.back() == L'\r') line.pop_back();
        if(line.compare(0, 3, L"```") == 0) { code = !code; paragraph = false; continue; }
        if(code) { doc.text += line + L'\r'; continue; }
        if(line.empty()) { doc.text += L'\r'; paragraph = false; continue; }
        unsigned level = 0;
        while(level < line.size() && line[level] == L'#') ++level;
        if(level && level <= 6 && level < line.size() && line[level] == L' ') {
            const LONG start = static_cast<LONG>(doc.text.size());
            Inline(doc, line.substr(level+1));
            doc.headings.push_back({start, static_cast<LONG>(doc.text.size()), level, line.substr(level+1)});
            doc.text += L'\r'; paragraph = false;
        } else {
            const bool item = line.compare(0, 2, L"- ") == 0 || line.front() == L'|';
            if(paragraph && !item && !doc.text.empty()) { doc.text.back() = L' '; }
            const auto first = line.find_first_not_of(L' ');
            Inline(doc, first == std::wstring::npos ? L"" : line.substr(first));
            doc.text += L'\r'; paragraph = true;
        }
    }
    return doc;
}
void Load(HWND window, Document &doc, unsigned source) {
    Document next = Parse(source); // Keep the old document usable if loading fails.
    const HWND edit = GetDlgItem(window, IDC_DOC_TEXT), sections = GetDlgItem(window, IDC_DOC_SECTION);
    SendMessageW(edit, WM_SETREDRAW, FALSE, 0);
    SendMessageW(edit, EM_EXLIMITTEXT, 0, 1024 * 1024);
    SetWindowTextW(edit, next.text.c_str());
    SendMessageW(edit, EM_SETSEL, 0, -1);
    CHARFORMAT2W format = {sizeof(format)};
    format.dwMask = CFM_FACE | CFM_SIZE | CFM_BOLD | CFM_LINK;
    format.yHeight = 200;
    wcscpy_s(format.szFaceName, L"Segoe UI");
    SendMessageW(edit, EM_SETCHARFORMAT, SCF_SELECTION, reinterpret_cast<LPARAM>(&format));
    // Automatic URL detection would clear explicit links whose labels are prose.
    SendMessageW(edit, EM_AUTOURLDETECT, FALSE, 0);
    SendMessageW(edit, EM_SETEVENTMASK, 0, ENM_LINK);
    SendMessageW(sections, CB_RESETCONTENT, 0, 0);
    for(const auto &heading : next.headings) {
        SendMessageW(sections, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(heading.title.c_str()));
        SendMessageW(edit, EM_SETSEL, heading.start, heading.end);
        format.dwMask = CFM_BOLD | CFM_SIZE; format.dwEffects = CFE_BOLD;
        format.yHeight = heading.level == 1 ? 320 : heading.level == 2 ? 260 : 220;
        SendMessageW(edit, EM_SETCHARFORMAT, SCF_SELECTION, reinterpret_cast<LPARAM>(&format));
    }
    for(const auto &link : next.links) {
        SendMessageW(edit, EM_SETSEL, link.start, link.end);
        format.dwMask = CFM_LINK; format.dwEffects = CFE_LINK;
        SendMessageW(edit, EM_SETCHARFORMAT, SCF_SELECTION, reinterpret_cast<LPARAM>(&format));
    }
    SendMessageW(sections, CB_SETCURSEL, next.headings.empty() ? -1 : 0, 0);
    EnableWindow(sections, !next.headings.empty());
    SendMessageW(edit, EM_SETSEL, 0, 0);
    SendMessageW(edit, EM_SCROLL, SB_TOP, 0);
    SendMessageW(edit, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(edit, nullptr, TRUE);
    SendDlgItemMessageW(window, IDC_DOC_SOURCE, CB_SETCURSEL, source, 0);
    SetDlgItemTextW(window, IDC_DOC_STATUS, L"");
    SetWindowTextW(window, (std::wstring(L"ZMatrix - ") + Sources[source].title).c_str());
    doc = std::move(next);
}
void Follow(HWND window, Document &doc, std::wstring target) {
    if(target.compare(0, 8, L"https://") == 0 || target.compare(0, 7, L"http://") == 0) {
        Open(window, target); return;
    }
    // Local links can open only bundled documents, never arbitrary executables.
    size_t space;
    while((space = target.find(L"%20")) != std::wstring::npos) target.replace(space, 3, L" ");
    const std::wstring current = Sources[doc.source].file;
    const auto slash = current.find_last_of(L'\\');
    const auto path = FullPath(ModuleFolder() + (slash == std::wstring::npos ? L"" : current.substr(0, slash+1)) + target);
    for(unsigned i = 0; i < _countof(Sources); ++i)
        if(_wcsicmp(path.c_str(), FullPath(ModuleFolder()+Sources[i].file).c_str()) == 0) { Load(window, doc, i); return; }
    SetDlgItemTextW(window, IDC_DOC_STATUS, L"This link is not an installed document.");
}
void FindNext(HWND window) {
    const auto term = WindowText(GetDlgItem(window, IDC_DOC_FIND));
    if(term.empty()) { SetFocus(GetDlgItem(window, IDC_DOC_FIND)); return; }
    const HWND edit = GetDlgItem(window, IDC_DOC_TEXT);
    CHARRANGE selection;
    SendMessageW(edit, EM_EXGETSEL, 0, reinterpret_cast<LPARAM>(&selection));
    FINDTEXTEXW find = {{selection.cpMax, -1}, const_cast<wchar_t *>(term.c_str()), {}};
    LRESULT found = SendMessageW(edit, EM_FINDTEXTEXW, FR_DOWN, reinterpret_cast<LPARAM>(&find));
    if(found < 0 && selection.cpMax > 0) {
        find.chrg = {0, selection.cpMax};
        found = SendMessageW(edit, EM_FINDTEXTEXW, FR_DOWN, reinterpret_cast<LPARAM>(&find));
    }
    SetDlgItemTextW(window, IDC_DOC_STATUS, found < 0 ? L"No matches." : L"");
    if(found >= 0) {
        SendMessageW(edit, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&find.chrgText));
        SendMessageW(edit, EM_SCROLLCARET, 0, 0);
    }
}
void Layout(HWND window) {
    RECT client; GetClientRect(window, &client);
    RECT units = {0, 0, 8, 22}; MapDialogRect(window, &units);
    const int gap = units.right, row = units.bottom, width = client.right;
    auto place = [&](int id, int x, int y, int w, int h) { MoveWindow(GetDlgItem(window, id), x, y, std::max(1,w), std::max(1,h), TRUE); };
    place(IDC_DOC_SOURCE, gap, gap, width/3-gap, row*8);
    place(IDC_DOC_SECTION, width/3+gap, gap, width*2/3-2*gap, row*12);
    place(IDC_DOC_FIND, gap, gap+row, width-12*gap, row-gap/2);
    place(IDC_DOC_NEXT, width-10*gap, gap+row, 9*gap, row-gap/2);
    place(IDC_DOC_TEXT, gap, gap+2*row, width-2*gap, client.bottom-3*gap-3*row);
    place(IDC_DOC_STATUS, gap, client.bottom-gap-row, width-12*gap, row);
    place(IDOK, width-10*gap, client.bottom-gap-row, 9*gap, row-gap/2);
}
INT_PTR CALLBACK Procedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    auto doc = reinterpret_cast<Document *>(GetWindowLongPtrW(window, DWLP_USER));
    try {
        if(message == WM_INITDIALOG) {
            doc = reinterpret_cast<Document *>(lparam);
            SetWindowLongPtrW(window, DWLP_USER, lparam);
            InitDialog(window);
            for(const auto &source : Sources)
                SendDlgItemMessageW(window, IDC_DOC_SOURCE, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(source.title));
            SendDlgItemMessageW(window, IDC_DOC_FIND, EM_SETCUEBANNER, TRUE, reinterpret_cast<LPARAM>(L"Find in this document"));
            Load(window, *doc, doc->source); Layout(window); return TRUE;
        }
        if(!doc) return FALSE;
        if(message == WM_SIZE && wparam != SIZE_MINIMIZED) { Layout(window); return TRUE; }
        if(message == WM_GETMINMAXINFO) {
            RECT minimum = {0, 0, 340, 220}; MapDialogRect(window, &minimum);
            auto sizes = reinterpret_cast<MINMAXINFO *>(lparam);
            sizes->ptMinTrackSize = {minimum.right, minimum.bottom}; return TRUE;
        }
        if(message == WM_COMMAND) {
            switch(LOWORD(wparam)) {
            case IDOK:
                if(GetFocus() == GetDlgItem(window, IDC_DOC_FIND)) { FindNext(window); return TRUE; }
                EndDialog(window, IDOK); return TRUE;
            case IDCANCEL: EndDialog(window, IDOK); return TRUE;
            case IDC_DOC_NEXT: FindNext(window); return TRUE;
            case IDC_DOC_SOURCE:
                if(HIWORD(wparam) == CBN_SELCHANGE) {
                    const auto index = SendDlgItemMessageW(window, IDC_DOC_SOURCE, CB_GETCURSEL, 0, 0);
                    if(index >= 0 && index < _countof(Sources)) Load(window, *doc, static_cast<unsigned>(index));
                }
                return TRUE;
            case IDC_DOC_SECTION:
                if(HIWORD(wparam) == CBN_SELCHANGE) {
                    const auto index = SendDlgItemMessageW(window, IDC_DOC_SECTION, CB_GETCURSEL, 0, 0);
                    if(index >= 0 && static_cast<size_t>(index) < doc->headings.size()) {
                        const auto &heading = doc->headings[index];
                        SendDlgItemMessageW(window, IDC_DOC_TEXT, EM_SETSEL, heading.start, heading.end);
                        SendDlgItemMessageW(window, IDC_DOC_TEXT, EM_SCROLLCARET, 0, 0);
                    }
                }
                return TRUE;
            }
        }
        if(message == WM_NOTIFY) {
            const auto link = reinterpret_cast<ENLINK *>(lparam);
            if(link->nmhdr.idFrom == IDC_DOC_TEXT && link->nmhdr.code == EN_LINK && link->msg == WM_LBUTTONUP) {
                for(const auto &known : doc->links)
                    if(link->chrg.cpMin >= known.start && link->chrg.cpMax <= known.end) {
                        const auto target = known.target; Follow(window, *doc, target); return TRUE;
                    }
                Follow(window, *doc, doc->text.substr(link->chrg.cpMin, link->chrg.cpMax-link->chrg.cpMin));
                return TRUE;
            }
        }
        if(message == WM_CLOSE) { EndDialog(window, IDOK); return TRUE; }
    } catch(const Error &error) {
        ShowError(window, error);
        if(doc) SendDlgItemMessageW(window, IDC_DOC_SOURCE, CB_SETCURSEL, doc->source, 0);
    } catch(...) {
        ShowUnexpectedError(window);
        if(doc) SendDlgItemMessageW(window, IDC_DOC_SOURCE, CB_SETCURSEL, doc->source, 0);
    }
    if(message == WM_INITDIALOG) EndDialog(window, IDCANCEL);
    return FALSE;
}
}
void ShowDocument(HWND owner, bool readme) {
    static bool open = false;
    if(open) return;
    open = true;
    try {
        Document doc{readme ? 1u : 0u};
        Dialog(IDD_DOCUMENT, owner, Procedure, reinterpret_cast<LPARAM>(&doc));
    } catch(...) { open = false; throw; }
    open = false;
}
}
