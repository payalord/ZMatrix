// Optional appearance controls. The original IzsMatrix interface remains unchanged.
// See LICENSE.TXT.
#pragma once
#include "IzsMatrix.h"

// {83600EA2-486C-4545-B7CC-868F34789FB5}
static const GUID IID_IZSMATRIXAPPEARANCE =
{0x83600ea2, 0x486c, 0x4545, {0xb7, 0xcc, 0x86, 0x8f, 0x34, 0x78, 0x9f, 0xb5}};

class IzsMatrixAppearance : public IUnknown {
public:
    // Bitmap mode: 0 = plain character colors, 100 = legacy wallpaper blending.
    // Applies to newly drawn characters; existing trails retain their pixels.
    virtual void __stdcall SetBlendStrength(unsigned percent) = 0;
    virtual unsigned __stdcall GetBlendStrength() const = 0;
};

inline unsigned ReadBlendStrength(const IzsMatrix &matrix) {
    IzsMatrixAppearance *appearance = nullptr;
    if(FAILED(const_cast<IzsMatrix &>(matrix).QueryInterface(
        IID_IZSMATRIXAPPEARANCE, reinterpret_cast<void **>(&appearance)))) return 100;
    const unsigned percent = appearance->GetBlendStrength();
    appearance->Release();
    return percent;
}

inline bool ApplyBlendStrength(IzsMatrix &matrix, unsigned percent) {
    IzsMatrixAppearance *appearance = nullptr;
    if(FAILED(matrix.QueryInterface(IID_IZSMATRIXAPPEARANCE,
        reinterpret_cast<void **>(&appearance)))) return false;
    appearance->SetBlendStrength(percent);
    appearance->Release();
    return true;
}

// A separate interface keeps the existing appearance vtable compatible.
// {A3EB0DCF-DDB4-4FAD-91EF-6BFC74762A01}
static const GUID IID_IZSMATRIXGLOW =
{0xa3eb0dcf, 0xddb4, 0x4fad, {0x91, 0xef, 0x6b, 0xfc, 0x74, 0x76, 0x2a, 0x01}};

class IzsMatrixGlow : public IUnknown {
public:
    virtual void __stdcall SetGlowEnabled(BOOL enabled) = 0;
    virtual BOOL __stdcall GetGlowEnabled() const = 0;
};

inline bool ReadGlowEnabled(const IzsMatrix &matrix) {
    IzsMatrixGlow *glow = nullptr;
    if(FAILED(const_cast<IzsMatrix &>(matrix).QueryInterface(
        IID_IZSMATRIXGLOW, reinterpret_cast<void **>(&glow)))) return false;
    const bool enabled = glow->GetGlowEnabled() != FALSE;
    glow->Release();
    return enabled;
}

inline bool ApplyGlowEnabled(IzsMatrix &matrix, bool enabled) {
    IzsMatrixGlow *glow = nullptr;
    if(FAILED(matrix.QueryInterface(IID_IZSMATRIXGLOW, reinterpret_cast<void **>(&glow)))) return false;
    glow->SetGlowEnabled(enabled);
    glow->Release();
    return true;
}
