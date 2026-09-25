#include "UI.h"
#include "../Audio/AudioCapture.h"
#include <commctrl.h>
#include <commdlg.h>
#include <shlobj.h>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace zconfig {
struct AudioEditor {
    const audio::HostApi &host;
    audio::Settings settings;
    bool updating = false;
    COLORREF customColors[16] = {};
    std::vector<audio::Device> devices;
    double &ReactionValue(int index) {
        double *values[] = {&settings.sensitivity,&settings.smoothing,&settings.brightnessStrength,&settings.speedStrength,&settings.spawnStrength};
        return *values[index];
    }
    BOOL &Switch(int index) {
        BOOL *values[] = {&settings.brightnessEnabled,&settings.speedEnabled,&settings.spawnEnabled,&settings.colorEnabled};
        return *values[index];
    }
    double &Value(int index) {
        auto &m = settings.profiles[settings.mode];
        if(index < 3) return m.baseScale[index];
        if(index < 6) return m.peakScale[index-3];
        return index == 6 ? m.globalScale : m.globalOffset;
    }
    void Preview() {
        const DWORD error = host.preview(host.context,&settings);
        if(error) throw Error{L"The audio settings could not be applied.",error};
    }
    void Populate(HWND window) {
        updating = true;
        CheckDlgButton(window,IDC_AUDIO_ENABLED,settings.enabled ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(window,IDC_AUDIO_SILENCE,settings.returnOnSilence ? BST_CHECKED : BST_UNCHECKED);
        SetDlgItemInt(window,IDC_AUDIO_SILENCE_DELAY,settings.silenceDelaySeconds,FALSE);
        EnableWindow(GetDlgItem(window,IDC_AUDIO_SILENCE_DELAY),settings.returnOnSilence);
        EnableWindow(GetDlgItem(window,IDC_AUDIO_SILENCE_LABEL),settings.returnOnSilence);
        SendDlgItemMessageW(window,IDC_AUDIO_MODE,CB_SETCURSEL,settings.mode,0);
        SendDlgItemMessageW(window,IDC_AUDIO_SOURCE,CB_SETCURSEL,settings.responseSource,0);
        for(int i = 0; i < 4; ++i) CheckDlgButton(window,IDC_AUDIO_BRIGHTNESS+i,Switch(i) ? BST_CHECKED : BST_UNCHECKED);
        for(int i = 0; i < 5; ++i) {
            std::wostringstream text; text.imbue(std::locale::classic()); text << std::setprecision(17) << ReactionValue(i)*100;
            const int id = IDC_AUDIO_REACTION_NUMBER+i;
            SetDlgItemTextW(window,id,text.str().c_str());
            SendDlgItemMessageW(window,id+SLIDER_OFFSET,TBM_SETPOS,TRUE,static_cast<LPARAM>(std::lround(ReactionValue(i)*100)));
            const BOOL enabled = i < 2 || Switch(i-2);
            EnableWindow(GetDlgItem(window,id),enabled); EnableWindow(GetDlgItem(window,id+SLIDER_OFFSET),enabled);
        }
        SetDlgItemTextW(window,IDC_AUDIO_SOURCE_DESCRIPTION,settings.responseSource == audio::AudioLevel ?
            L"Follows the loudness of all sound. Controls brightness, speed and new streams." :
            L"Follows low-frequency energy (bass). Controls brightness, speed and new streams.");
        for(int i = 0; i < 8; ++i) {
            // Preserve imported fractional percentages until the user edits them.
            std::wostringstream text; text.imbue(std::locale::classic()); text << std::setprecision(17) << Value(i)*100;
            SetDlgItemTextW(window,IDC_AUDIO_NUMBER+i,text.str().c_str());
            SendDlgItemMessageW(window,IDC_AUDIO_NUMBER+i+SLIDER_OFFSET,TBM_SETPOS,TRUE,static_cast<LPARAM>(std::lround(Value(i)*100)));
            EnableWindow(GetDlgItem(window,IDC_AUDIO_NUMBER+i),settings.colorEnabled);
            EnableWindow(GetDlgItem(window,IDC_AUDIO_NUMBER+i+SLIDER_OFFSET),settings.colorEnabled);
        }
        for(int id : {IDC_AUDIO_MODE,IDC_AUDIO_BASE_COLOR,IDC_AUDIO_PEAK_COLOR,IDC_AUDIO_DESCRIPTION,IDC_AUDIO_RESET})
            EnableWindow(GetDlgItem(window,id),settings.colorEnabled);
        const auto &m = settings.profiles[settings.mode];
        for(int i = 0; i < 2; ++i) {
            const double *rgb = i ? m.peakOffset : m.baseOffset;
            wchar_t label[80]; swprintf_s(label,L"RGB %.0f, %.0f, %.0f...",rgb[0],rgb[1],rgb[2]);
            SetDlgItemTextW(window,i ? IDC_AUDIO_PEAK_COLOR : IDC_AUDIO_BASE_COLOR,label);
        }
        SetDlgItemTextW(window,IDC_AUDIO_DESCRIPTION,settings.mode == audio::WaveformVariation ?
            L"Follows waveform changes: stronger sound and higher frequencies increase the response." :
            L"Spectral centroid reacts to the balance of frequencies: higher frequencies increase the response.");
        updating = false;
    }
    void Devices(HWND window) {
        std::vector<audio::Device> available;
        const HRESULT hr = audio::PlaybackDevices(available);
        devices = {{L"",L"System default output"}};
        devices.insert(devices.end(),available.begin(),available.end());
        size_t selected = 0;
        for(size_t i = 0; i < devices.size(); ++i) if(devices[i].id == settings.deviceId) selected = i;
        if(settings.deviceId[0] && !selected) {
            selected = devices.size(); devices.push_back({settings.deviceId,L"Saved output (currently unavailable)"});
        }
        SendDlgItemMessageW(window,IDC_AUDIO_DEVICE,CB_RESETCONTENT,0,0);
        for(const auto &device : devices) SendDlgItemMessageW(window,IDC_AUDIO_DEVICE,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(device.name.c_str()));
        SendDlgItemMessageW(window,IDC_AUDIO_DEVICE,CB_SETCURSEL,selected,0);
        if(FAILED(hr)) SetDlgItemTextW(window,IDC_AUDIO_STATUS,L"Playback devices could not be listed. You can retry with Refresh.");
    }
    bool ReadNumber(HWND window, int i, bool validate, bool reaction = false) {
        const int id = (reaction ? IDC_AUDIO_REACTION_NUMBER : IDC_AUDIO_NUMBER)+i;
        const auto text = WindowText(GetDlgItem(window,id));
        std::wistringstream in(text); in.imbue(std::locale::classic());
        double value = 0;
        const int low = !reaction && i == 7 ? -500 : 0;
        const int high = reaction ? (i == 0 ? 2000 : 100) : i == 7 ? 500 : i == 6 ? 2000 : 1000;
        bool valid = bool(in >> value);
        in >> std::ws;
        valid = valid && in.eof() && std::isfinite(value) && value >= low && value <= high;
        if(!valid) {
            if(validate) {
                SetFocus(GetDlgItem(window,id));
                throw Error{L"Enter a percentage from " + std::to_wstring(low) + L" to " + std::to_wstring(high) + L" (use a decimal point).",ERROR_INVALID_DATA};
            }
            return false;
        }
        double &setting = reaction ? ReactionValue(i) : Value(i);
        const double previous = setting;
        if(value != previous*100) setting = value/100;
        try { Preview(); } catch(...) { setting = previous; throw; }
        SendDlgItemMessageW(window,id+SLIDER_OFFSET,TBM_SETPOS,TRUE,static_cast<LPARAM>(std::lround(value)));
        return true;
    }
    void ReadSilenceDelay(HWND window, bool validate) {
        std::wistringstream in(WindowText(GetDlgItem(window,IDC_AUDIO_SILENCE_DELAY)));
        in.imbue(std::locale::classic());
        int value = 0;
        const bool parsed = bool(in >> value); in >> std::ws;
        if(!parsed || !in.eof() || value < 1 || value > 60) {
            if(validate) {
                SetFocus(GetDlgItem(window,IDC_AUDIO_SILENCE_DELAY));
                throw Error{L"Enter a whole number of seconds from 1 to 60.",ERROR_INVALID_DATA};
            }
            return;
        }
        const UINT previous = settings.silenceDelaySeconds;
        settings.silenceDelaySeconds = static_cast<UINT>(value);
        try { Preview(); } catch(...) { settings.silenceDelaySeconds = previous; throw; }
    }
    void Validate(HWND window) {
        for(int i = 0; i < 8; ++i) ReadNumber(window,i,true);
        for(int i = 0; i < 5; ++i) ReadNumber(window,i,true,true);
        ReadSilenceDelay(window,true);
    }
    void Status(HWND window) {
        audio::Status status = {}; host.status(host.context,&status);
        std::wstring text;
        if(status.error != S_OK && status.state == audio::Disabled) text = L"Saved audio settings could not be read; defaults are in use.";
        else if(status.state == audio::Disabled) text = settings.enabled ?
            L"No active influence. Enable an effect and set its strength above zero." : L"Disabled. Ordinary ZMatrix appearance and motion are in use.";
        else if(status.state == audio::Starting) text = L"Opening the playback output...";
        else if(status.state == audio::Capturing && status.waitingForSound)
            text = L"Waiting for sound. Ordinary appearance is active.";
        else if(status.state == audio::Capturing) {
            text = L"Capturing output audio.";
            wchar_t value[80];
            if((settings.brightnessEnabled && settings.brightnessStrength > 0) || (settings.speedEnabled && settings.speedStrength > 0) ||
               (settings.spawnEnabled && settings.spawnStrength > 0)) {
                swprintf_s(value,L" %ls response: %.1f%%.",settings.responseSource == audio::BassEnergy ? L"Bass" : L"Level",100*status.response);
                text += value;
            }
            if(settings.colorEnabled) {
                swprintf_s(value,L" Color response: %.1f%%.",100*std::clamp(status.descriptor*settings.profiles[settings.mode].globalScale+
                    settings.profiles[settings.mode].globalOffset,0.0,1.0));
                text += value;
            }
        } else {
            wchar_t value[144]; swprintf_s(value,L"Output unavailable (0x%08lX). Ordinary appearance and motion are in use; retrying...",static_cast<unsigned long>(status.error));
            text = value;
        }
        SetDlgItemTextW(window,IDC_AUDIO_STATUS,text.c_str());
    }
};
static void ImportLegacyWinampSettings(HWND window, AudioEditor &editor) {
    wchar_t file[MAX_PATH] = L"vis_zmx.cfg", folder[MAX_PATH] = {};
    std::wstring initial;
    if(SUCCEEDED(SHGetFolderPathW(nullptr,CSIDL_APPDATA,nullptr,SHGFP_TYPE_CURRENT,folder))) initial = std::wstring(folder)+L"\\.ZMatrix";
    OPENFILENAMEW choose = {sizeof(choose)};
    choose.hwndOwner = window; choose.lpstrFile = file; choose.nMaxFile = _countof(file);
    choose.lpstrFilter = L"Legacy Winamp settings (vis_zmx.cfg)\0*.cfg\0All files\0*.*\0";
    choose.lpstrTitle = L"Import legacy Winamp settings"; choose.lpstrInitialDir = initial.c_str();
    choose.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    if(!GetOpenFileNameW(&choose)) return;
    auto imported = editor.settings;
    const DWORD error = audio::Load(file,imported,true);
    if(error) throw Error{L"This file does not contain valid legacy Winamp settings. RGB lists must use decimal points, and all values must stay within the supported ranges.",error};
    imported.colorEnabled = TRUE;
    const auto previous = editor.settings;
    editor.settings = imported;
    try { editor.Preview(); } catch(...) { editor.settings = previous; throw; }
    editor.Populate(window);
}
static INT_PTR CALLBACK AudioProcedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    auto context = reinterpret_cast<AudioEditor *>(GetWindowLongPtrW(window,DWLP_USER));
    try {
        if(message == WM_INITDIALOG) {
            context = reinterpret_cast<AudioEditor *>(lparam);
            SetWindowLongPtrW(window,DWLP_USER,lparam); InitDialog(window);
            SendDlgItemMessageW(window,IDC_AUDIO_SILENCE_DELAY,EM_SETLIMITTEXT,3,0);
            for(const auto label : {L"Waveform variation",L"Spectral centroid"})
                SendDlgItemMessageW(window,IDC_AUDIO_MODE,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(label));
            for(const auto label : {L"Audio level",L"Bass energy"})
                SendDlgItemMessageW(window,IDC_AUDIO_SOURCE,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(label));
            for(int i = 0; i < 5; ++i) {
                SendDlgItemMessageW(window,IDC_AUDIO_REACTION_NUMBER+i+SLIDER_OFFSET,TBM_SETRANGEMAX,FALSE,i == 0 ? 2000 : 100);
                SendDlgItemMessageW(window,IDC_AUDIO_REACTION_NUMBER+i,EM_SETLIMITTEXT,32,0);
            }
            for(int i = 0; i < 8; ++i) {
                SendDlgItemMessageW(window,IDC_AUDIO_NUMBER+i+SLIDER_OFFSET,TBM_SETRANGEMIN,FALSE,i == 7 ? -500 : 0);
                // Refresh thumb geometry even when the initial position stays at zero.
                SendDlgItemMessageW(window,IDC_AUDIO_NUMBER+i+SLIDER_OFFSET,TBM_SETRANGEMAX,TRUE,i == 7 ? 500 : i == 6 ? 2000 : 1000);
                SendDlgItemMessageW(window,IDC_AUDIO_NUMBER+i,EM_SETLIMITTEXT,32,0);
            }
            context->Devices(window); context->Populate(window); context->Status(window);
            SetTimer(window,1,500,nullptr); return TRUE;
        }
        if(!context) return FALSE;
        if(message == WM_TIMER && wparam == 1) { context->Status(window); return TRUE; }
        if(message == WM_CLOSE) { EndDialog(window,IDCANCEL); return TRUE; }
        if(message == WM_DESTROY) { KillTimer(window,1); return TRUE; }
        if(context->updating) return FALSE;
        if(message == WM_HSCROLL && lparam) {
            const int id = GetDlgCtrlID(reinterpret_cast<HWND>(lparam))-SLIDER_OFFSET;
            if((id >= IDC_AUDIO_NUMBER && id < IDC_AUDIO_NUMBER+8) ||
               (id >= IDC_AUDIO_REACTION_NUMBER && id < IDC_AUDIO_REACTION_NUMBER+5)) {
                SetDlgItemInt(window,id,static_cast<UINT>(SendMessageW(reinterpret_cast<HWND>(lparam),TBM_GETPOS,0,0)),TRUE);
                return TRUE;
            }
        }
        if(message != WM_COMMAND) return FALSE;
        const int id = LOWORD(wparam), code = HIWORD(wparam);
        if(id == IDCANCEL) { EndDialog(window,IDCANCEL); return TRUE; }
        if(id == IDOK) { context->Validate(window); EndDialog(window,IDOK); return TRUE; }
        if(code == EN_CHANGE && id == IDC_AUDIO_SILENCE_DELAY) { context->ReadSilenceDelay(window,false); return TRUE; }
        if(code == EN_CHANGE && id >= IDC_AUDIO_NUMBER && id < IDC_AUDIO_NUMBER+8) { context->ReadNumber(window,id-IDC_AUDIO_NUMBER,false); return TRUE; }
        if(code == EN_CHANGE && id >= IDC_AUDIO_REACTION_NUMBER && id < IDC_AUDIO_REACTION_NUMBER+5) {
            context->ReadNumber(window,id-IDC_AUDIO_REACTION_NUMBER,false,true); return TRUE;
        }
        const auto previous = context->settings;
        if(code == CBN_SELCHANGE && id == IDC_AUDIO_MODE) {
            try { context->Validate(window); }
            catch(...) { SendDlgItemMessageW(window,id,CB_SETCURSEL,context->settings.mode,0); throw; }
            context->settings.mode = static_cast<UINT>(SendDlgItemMessageW(window,id,CB_GETCURSEL,0,0));
        } else if(code == CBN_SELCHANGE && id == IDC_AUDIO_SOURCE) {
            context->settings.responseSource = static_cast<UINT>(SendDlgItemMessageW(window,id,CB_GETCURSEL,0,0));
        } else if(code == BN_CLICKED && id >= IDC_AUDIO_BRIGHTNESS && id <= IDC_AUDIO_COLOR) {
            context->Switch(id-IDC_AUDIO_BRIGHTNESS) = IsDlgButtonChecked(window,id) == BST_CHECKED;
        } else if(code == CBN_SELCHANGE && id == IDC_AUDIO_DEVICE) {
            const auto selected = SendDlgItemMessageW(window,id,CB_GETCURSEL,0,0);
            if(selected < 0 || static_cast<size_t>(selected) >= context->devices.size()) return TRUE;
            const auto &device = context->devices[static_cast<size_t>(selected)];
            if(device.id.size() >= _countof(context->settings.deviceId)) throw Error{L"The output device ID is too long.",ERROR_INVALID_DATA};
            wcscpy_s(context->settings.deviceId,device.id.c_str());
        } else if(code == BN_CLICKED && id == IDC_AUDIO_ENABLED) {
            context->settings.enabled = IsDlgButtonChecked(window,id) == BST_CHECKED;
        } else if(code == BN_CLICKED && id == IDC_AUDIO_SILENCE) {
            context->settings.returnOnSilence = IsDlgButtonChecked(window,id) == BST_CHECKED;
        } else if(code == BN_CLICKED && id == IDC_AUDIO_RESET) {
            context->settings.profiles[context->settings.mode] = audio::Defaults().profiles[context->settings.mode];
        } else if(code == BN_CLICKED && id == IDC_AUDIO_REFRESH) { context->Devices(window); return TRUE; }
        else if(code == BN_CLICKED && id == IDC_AUDIO_IMPORT) { ImportLegacyWinampSettings(window,*context); return TRUE; }
        else if(code == BN_CLICKED && (id == IDC_AUDIO_BASE_COLOR || id == IDC_AUDIO_PEAK_COLOR)) {
            auto &mapping = context->settings.profiles[context->settings.mode];
            double *rgb = id == IDC_AUDIO_BASE_COLOR ? mapping.baseOffset : mapping.peakOffset;
            CHOOSECOLORW choose = {sizeof(choose)};
            choose.hwndOwner = window; choose.lpCustColors = context->customColors; choose.Flags = CC_FULLOPEN | CC_RGBINIT;
            choose.rgbResult = RGB(static_cast<BYTE>(rgb[0]),static_cast<BYTE>(rgb[1]),static_cast<BYTE>(rgb[2]));
            if(!ChooseColorW(&choose)) return TRUE;
            rgb[0] = GetRValue(choose.rgbResult); rgb[1] = GetGValue(choose.rgbResult); rgb[2] = GetBValue(choose.rgbResult);
        } else return FALSE;
        try { context->Preview(); }
        catch(...) { context->settings = previous; context->Populate(window); context->Devices(window); throw; }
        context->Populate(window); context->Status(window); return TRUE;
    } catch(const Error &error) { ShowError(window,error); }
    catch(...) { ShowUnexpectedError(window); }
    if(message == WM_INITDIALOG) EndDialog(window,IDCANCEL);
    return TRUE;
}
bool EditAudio(HWND owner, const audio::HostApi &host) {
    AudioEditor editor{host}; host.get(host.context,&editor.settings);
    const auto original = editor.settings;
    INT_PTR result = IDCANCEL;
    try { result = Dialog(IDD_AUDIO,owner,AudioProcedure,reinterpret_cast<LPARAM>(&editor)); }
    catch(...) { host.preview(host.context,&original); throw; }
    if(result != IDOK) host.preview(host.context,&original);
    return result == IDOK;
}
}
