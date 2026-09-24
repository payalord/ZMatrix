// ZMatrix configuration dialog with live preview and apply/cancel handling. See LICENSE.TXT.
#include "UI.h"
#include <commctrl.h>
#include <commdlg.h>
#include <dlgs.h>
#include <algorithm>
#include <cmath>

namespace zconfig {
static const DWORD Priorities[] = {IDLE_PRIORITY_CLASS, BELOW_NORMAL_PRIORITY_CLASS, NORMAL_PRIORITY_CLASS, ABOVE_NORMAL_PRIORITY_CLASS, HIGH_PRIORITY_CLASS};
struct NumberControl { int id; unsigned minimum, maximum; };
static const NumberControl Numbers[] = {
    {IDC_MAX_STREAM,0,2000}, {IDC_SPEED,0,50}, {IDC_REFRESH,1,500}, {IDC_PROBABILITY,0,100},
    {IDC_BACKTRACE,0,100}, {IDC_LEADING,0,100}, {IDC_SPACE,0,100}, {IDC_BLEND_STRENGTH,0,100}
};
struct Editor {
    IzsMatrix &matrix;
    unsigned &refresh;
    DWORD &priority;
    Settings settings;
    const audio::HostApi *audioHost;
    bool audioEdited = false;
    bool updating = false;
    COLORREF customColors[16] = {};
    Editor(IzsMatrix &m, unsigned &r, DWORD &p, const audio::HostApi *a) : matrix(m), refresh(r), priority(p), settings(Capture(m,r,p)), audioHost(a) {}
    unsigned Number(int id) const {
        switch(id) {
        case IDC_MAX_STREAM: return settings.maxStream;
        case IDC_SPEED: return settings.speedVariance;
        case IDC_REFRESH: return settings.refresh;
        case IDC_PROBABILITY: return static_cast<unsigned>(std::round(settings.probability * 100));
        case IDC_BACKTRACE: return settings.backTrace;
        case IDC_LEADING: return settings.leading;
        case IDC_BLEND_STRENGTH: return settings.blendStrength;
        default: return settings.spacePad;
        }
    }
    void Number(int id, unsigned value) {
        switch(id) {
        case IDC_MAX_STREAM: settings.maxStream = value; break;
        case IDC_SPEED: settings.speedVariance = value; break;
        case IDC_REFRESH: settings.refresh = value; break;
        case IDC_PROBABILITY: settings.probability = value / 100.0f; break;
        case IDC_BACKTRACE: settings.backTrace = value; break;
        case IDC_LEADING: settings.leading = value; break;
        case IDC_SPACE: settings.spacePad = value; break;
        case IDC_BLEND_STRENGTH: settings.blendStrength = value; break;
        }
    }
    Color *ColorFor(int id) {
        switch(id) {
        case IDC_FOREGROUND: return &settings.foreground;
        case IDC_FADE: return &settings.fade;
        case IDC_BACKGROUND: return &settings.background;
        case IDC_SPECIAL_FOREGROUND: return &settings.specialForeground;
        case IDC_SPECIAL_FADE: return &settings.specialFade;
        default: return nullptr;
        }
    }
    void Preview() {
        const auto oldRefresh = refresh;
        const auto oldPriority = priority;
        Apply(settings, matrix, refresh, priority);
        if(oldRefresh != refresh && IsWindow(matrix.GethWnd())) SetTimer(matrix.GethWnd(), 400, refresh, nullptr);
        if(oldPriority != priority) Require(SetPriorityClass(GetCurrentProcess(), priority) != FALSE, L"Windows could not change the process priority.");
    }
    void EnableControls(HWND window) const {
        for(int id : {IDC_BACKTRACE, IDC_BACKTRACE+SLIDER_OFFSET}) EnableWindow(GetDlgItem(window, id), settings.monotonous);
        for(int id : {IDC_LEADING, IDC_LEADING+SLIDER_OFFSET, IDC_SPACE, IDC_SPACE+SLIDER_OFFSET}) EnableWindow(GetDlgItem(window, id), settings.randomized);
        EnableWindow(GetDlgItem(window, IDC_BLEND), settings.backgroundMode == bgmodeBitmap);
        IzsMatrixAppearance *appearance = nullptr;
        const bool supported = SUCCEEDED(matrix.QueryInterface(IID_IZSMATRIXAPPEARANCE, reinterpret_cast<void **>(&appearance))) && settings.backgroundMode == bgmodeBitmap;
        if(appearance) appearance->Release();
        EnableWindow(GetDlgItem(window, IDC_BLEND_STRENGTH), supported);
        EnableWindow(GetDlgItem(window, IDC_BLEND_STRENGTH+SLIDER_OFFSET), supported);
    }
    void Populate(HWND window) {
        updating = true;
        for(const auto &n : Numbers) {
            SetDlgItemInt(window, n.id, Number(n.id), FALSE);
            SendDlgItemMessageW(window, n.id+SLIDER_OFFSET, TBM_SETPOS, TRUE, Number(n.id));
        }
        SetDlgItemTextW(window, IDC_FONT_NAME, settings.font.lfFaceName);
        SetDlgItemTextW(window, IDC_SPECIAL_FONT_NAME, settings.specialFont.lfFaceName);
        CheckDlgButton(window, IDC_MONOTONOUS, settings.monotonous ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(window, IDC_RANDOMIZED, settings.randomized ? BST_CHECKED : BST_UNCHECKED);
        SendDlgItemMessageW(window, IDC_BG_MODE, CB_SETCURSEL, settings.backgroundMode == bgmodeColor ? 1 : 0, 0);
        SendDlgItemMessageW(window, IDC_BLEND, CB_SETCURSEL, settings.blendMode, 0);
        SendDlgItemMessageW(window, IDC_TEXT_BACKGROUND, CB_SETCURSEL, settings.background.a >= 128 ? 1 : 0, 0);
        for(unsigned i = 0; i < _countof(Priorities); ++i)
            if(Priorities[i] == settings.priority) SendDlgItemMessageW(window, IDC_PRIORITY, CB_SETCURSEL, i, 0);
        EnableControls(window);
        InvalidateRect(window, nullptr, TRUE);
        updating = false;
    }
    bool ReadNumber(HWND window, const NumberControl &n, bool validate) {
        BOOL valid = FALSE;
        const auto value = GetDlgItemInt(window, n.id, &valid, FALSE);
        // Existing CFG values may exceed a slider's range. Keep them until edited.
        if(!valid || value < n.minimum || (value > n.maximum && value != Number(n.id))) {
            if(validate) {
                SetFocus(GetDlgItem(window, n.id));
                SendDlgItemMessageW(window, n.id, EM_SETSEL, 0, -1);
                throw Error{L"Enter a number from " + std::to_wstring(n.minimum) + L" to " + std::to_wstring(n.maximum) + L".", ERROR_INVALID_DATA};
            }
            return false;
        }
        if(value != Number(n.id)) {
            Number(n.id, value);
            if(n.id == IDC_BLEND_STRENGTH) ApplyBlendStrength(matrix, value);
            else Preview();
        }
        SendDlgItemMessageW(window, n.id+SLIDER_OFFSET, TBM_SETPOS, TRUE, value);
        return true;
    }
    void Validate(HWND window) { for(const auto &n : Numbers) ReadNumber(window, n, true); }
};
static void AddOptions(HWND window, int id, std::initializer_list<const wchar_t *> options) {
    for(const auto text : options) SendDlgItemMessageW(window, id, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text));
}
static UINT_PTR CALLBACK FontOptions(HWND window, UINT message, WPARAM, LPARAM) {
    if(message == WM_INITDIALOG) {
        // Keep underline/strikeout controls. Colors are edited in the main form.
        ShowWindow(GetDlgItem(window, cmb4), SW_HIDE);
        ShowWindow(GetDlgItem(window, stc4), SW_HIDE);
    }
    return 0;
}
static void DrawColor(const DRAWITEMSTRUCT &draw, Color color) {
    HBRUSH brush = CreateSolidBrush(RGB(color.r, color.g, color.b));
    FillRect(draw.hDC, &draw.rcItem, brush); DeleteObject(brush);
    RECT bounds = draw.rcItem;
    DrawEdge(draw.hDC, &bounds, draw.itemState & ODS_SELECTED ? EDGE_SUNKEN : EDGE_RAISED, BF_RECT);
    SetBkMode(draw.hDC, TRANSPARENT);
    SetTextColor(draw.hDC, color.r * 299 + color.g * 587 + color.b * 114 > 128000 ? RGB(0,0,0) : RGB(255,255,255));
    const auto text = WindowText(draw.hwndItem);
    DrawTextW(draw.hDC, text.c_str(), -1, &bounds, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    if(draw.itemState & ODS_FOCUS) { InflateRect(&bounds, -3, -3); DrawFocusRect(draw.hDC, &bounds); }
}
static void DrawPreview(const DRAWITEMSTRUCT &draw, const Settings &s) {
    HBRUSH brush = CreateSolidBrush(RGB(s.background.r,s.background.g,s.background.b));
    FillRect(draw.hDC, &draw.rcItem, brush); DeleteObject(brush);
    SetBkMode(draw.hDC, TRANSPARENT);
    const Color colors[] = {s.foreground, s.fade, s.specialForeground, s.specialFade};
    const int width = (draw.rcItem.right-draw.rcItem.left)/4;
    for(int i = 0; i < 4; ++i) {
        // Fit the sample to its panel; the selected font itself is unchanged.
        LOGFONTW font = i < 2 ? s.font : s.specialFont;
        font.lfHeight = -(draw.rcItem.bottom-draw.rcItem.top-4); font.lfWidth = 0;
        HFONT handle = CreateFontIndirectW(&font);
        HGDIOBJ previous = SelectObject(draw.hDC, handle);
        SetTextColor(draw.hDC, RGB(colors[i].r,colors[i].g,colors[i].b));
        RECT bounds = draw.rcItem; bounds.left += i*width; bounds.right = bounds.left+width;
        DrawTextW(draw.hDC, L"Aa", -1, &bounds, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(draw.hDC, previous); DeleteObject(handle);
    }
}
static INT_PTR CALLBACK ConfigProcedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    auto context = reinterpret_cast<Editor *>(GetWindowLongPtrW(window, DWLP_USER));
    try {
        if(message == WM_INITDIALOG) {
            context = reinterpret_cast<Editor *>(lparam);
            SetWindowLongPtrW(window, DWLP_USER, lparam);
            InitDialog(window);
            EnableWindow(GetDlgItem(window,IDC_AUDIO),context->audioHost != nullptr);
            for(const auto &n : Numbers) {
                SendDlgItemMessageW(window, n.id+SLIDER_OFFSET, TBM_SETRANGEMIN, FALSE, n.minimum);
                SendDlgItemMessageW(window, n.id+SLIDER_OFFSET, TBM_SETRANGEMAX, FALSE, n.maximum);
                SendDlgItemMessageW(window, n.id, EM_SETLIMITTEXT, 10, 0);
            }
            AddOptions(window, IDC_BG_MODE, {L"Desktop bitmap", L"Solid color"});
            AddOptions(window, IDC_BLEND, {L"XOR", L"AND", L"OR"});
            AddOptions(window, IDC_TEXT_BACKGROUND, {L"Transparent", L"Opaque"});
            AddOptions(window, IDC_PRIORITY, {L"Idle (recommended)", L"Below normal", L"Normal", L"Above normal", L"High"});
            context->Populate(window); return TRUE;
        }
        if(!context) return FALSE;
        if(message == WM_CLOSE) { EndDialog(window, IDCANCEL); return TRUE; }
        if(message == WM_DRAWITEM) {
            auto &draw = *reinterpret_cast<DRAWITEMSTRUCT *>(lparam);
            if(draw.CtlID == IDC_PREVIEW) { DrawPreview(draw, context->settings); return TRUE; }
            if(auto color = context->ColorFor(draw.CtlID)) { DrawColor(draw, *color); return TRUE; }
        }
        if(context->updating) return FALSE;
        if(message == WM_HSCROLL && lparam) {
            const int id = GetDlgCtrlID(reinterpret_cast<HWND>(lparam))-SLIDER_OFFSET;
            for(const auto &n : Numbers) if(n.id == id) {
                const auto value = static_cast<unsigned>(SendMessageW(reinterpret_cast<HWND>(lparam), TBM_GETPOS, 0, 0));
                SetDlgItemInt(window, id, value, FALSE); return TRUE;
            }
        }
        if(message != WM_COMMAND) return FALSE;
        const unsigned id = LOWORD(wparam), code = HIWORD(wparam);
        if(code == EN_CHANGE) {
            for(const auto &n : Numbers) if(n.id == id) { context->ReadNumber(window, n, false); return TRUE; }
        }
        if(id == IDCANCEL || id == IDM_EXIT) { EndDialog(window, IDCANCEL); return TRUE; }
        if(id == IDOK) {
            context->Validate(window);
            if(context->audioHost && context->audioEdited) {
                audio::Settings audioSettings;
                context->audioHost->get(context->audioHost->context,&audioSettings);
                const DWORD error = context->audioHost->commit(context->audioHost->context,&audioSettings);
                if(error) throw Error{L"The audio settings could not be saved.",error};
            }
            EndDialog(window, IDOK); return TRUE;
        }
        if(id == IDC_AUDIO && context->audioHost) {
            if(EditAudio(window,*context->audioHost)) context->audioEdited = true;
            return TRUE;
        }
        if(id == IDM_SAVE) {
            context->Validate(window);
            const auto file = SelectFile(window, true, true);
            if(!file.empty()) Save(file, context->settings);
            return TRUE;
        }
        if(id == IDM_LOAD || id == IDM_DEFAULTS) {
            const auto file = id == IDM_DEFAULTS ? ModuleFolder() + L"default.cfg" : SelectFile(window, false, true);
            if(!file.empty()) { context->settings = Load(file, context->settings); context->Preview(); context->Populate(window); }
            return TRUE;
        }
        if(HelpCommand(window, id)) return TRUE;
        auto &s = context->settings;
        if(code == CBN_SELCHANGE) {
            const auto selection = SendDlgItemMessageW(window, id, CB_GETCURSEL, 0, 0);
            if(selection == CB_ERR) return TRUE;
            switch(id) {
            case IDC_BG_MODE: s.backgroundMode = static_cast<TBGMode>(selection); break;
            case IDC_BLEND: s.blendMode = static_cast<TBlendMode>(selection); break;
            case IDC_TEXT_BACKGROUND: s.background.a = selection ? 255 : 0; break;
            case IDC_PRIORITY: s.priority = Priorities[selection]; break;
            default: return FALSE;
            }
            context->Preview(); context->EnableControls(window); return TRUE;
        }
        if(code != BN_CLICKED) return FALSE;
        if(auto color = context->ColorFor(id)) {
            CHOOSECOLORW choose = {sizeof(choose)};
            choose.hwndOwner = window; choose.rgbResult = RGB(color->r, color->g, color->b);
            choose.lpCustColors = context->customColors; choose.Flags = CC_FULLOPEN | CC_RGBINIT;
            if(!ChooseColorW(&choose)) return TRUE;
            color->r = GetRValue(choose.rgbResult); color->g = GetGValue(choose.rgbResult); color->b = GetBValue(choose.rgbResult);
            if(id == IDC_BACKGROUND) s.specialBackground = s.background;
        } else switch(id) {
        case IDC_FONT: case IDC_SPECIAL_FONT: {
            LOGFONTW font = id == IDC_FONT ? s.font : s.specialFont;
            CHOOSEFONTW choose = {sizeof(choose)};
            choose.hwndOwner = window; choose.lpLogFont = &font;
            choose.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_EFFECTS | CF_ENABLEHOOK;
            choose.lpfnHook = FontOptions;
            if(!ChooseFontW(&choose)) return TRUE;
            (id == IDC_FONT ? s.font : s.specialFont) = font;
            SetDlgItemTextW(window, id == IDC_FONT ? IDC_FONT_NAME : IDC_SPECIAL_FONT_NAME, font.lfFaceName);
            break;
        }
        case IDC_CHARACTERS: if(!EditCharacters(window, s)) return TRUE; break;
        case IDC_COPY_COLORS: {
            const BYTE leadAlpha = s.specialForeground.a, trailAlpha = s.specialFade.a;
            s.specialForeground = s.foreground; s.specialFade = s.fade;
            s.specialForeground.a = leadAlpha; s.specialFade.a = trailAlpha;
            break;
        }
        case IDC_MONOTONOUS: s.monotonous = IsDlgButtonChecked(window, id) == BST_CHECKED; break;
        case IDC_RANDOMIZED: s.randomized = IsDlgButtonChecked(window, id) == BST_CHECKED; break;
        default: return FALSE;
        }
        context->Preview(); context->EnableControls(window);
        for(int control : {IDC_FOREGROUND,IDC_FADE,IDC_BACKGROUND,IDC_SPECIAL_FOREGROUND,IDC_SPECIAL_FADE,IDC_PREVIEW})
            InvalidateRect(GetDlgItem(window, control), nullptr, TRUE);
        return TRUE;
    } catch(const Error &error) { ShowError(window, error); }
    catch(...) { ShowUnexpectedError(window); }
    if(message == WM_INITDIALOG) EndDialog(window, IDCANCEL);
    return TRUE;
}
int Configure(IzsMatrix &matrix, unsigned &refresh, DWORD &priority, const audio::HostApi *audioHost) {
    Editor context(matrix, refresh, priority, audioHost);
    audio::Settings originalAudio = {};
    if(audioHost) audioHost->get(audioHost->context,&originalAudio);
    const Settings original = context.settings;
    const DWORD originalProcessPriority = GetPriorityClass(GetCurrentProcess());
    INT_PTR result = IDCANCEL;
    try { result = Dialog(IDD_CONFIG, nullptr, ConfigProcedure, reinterpret_cast<LPARAM>(&context)); }
    catch(...) {
        if(audioHost) audioHost->preview(audioHost->context,&originalAudio);
        Apply(original, matrix, refresh, priority);
        if(IsWindow(matrix.GethWnd())) SetTimer(matrix.GethWnd(), 400, refresh, nullptr);
        SetPriorityClass(GetCurrentProcess(), originalProcessPriority);
        throw;
    }
    if(result != IDOK) {
        if(audioHost) audioHost->preview(audioHost->context,&originalAudio);
        Apply(original, matrix, refresh, priority);
        if(IsWindow(matrix.GethWnd())) SetTimer(matrix.GethWnd(), 400, refresh, nullptr);
        SetPriorityClass(GetCurrentProcess(), originalProcessPriority);
    }
    return result == IDOK;
}
}
