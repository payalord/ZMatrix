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
