// Transient audio motion controls; the original engine and appearance ABIs stay unchanged.
#pragma once
#include "IzsMatrix.h"

// {31A795F8-3485-49F4-8DC0-25B03B2CB32E}
static const GUID IID_IZSMATRIXMOTION =
{0x31a795f8, 0x3485, 0x49f4, {0x8d, 0xc0, 0x25, 0xb0, 0x3b, 0x2c, 0xb3, 0x2e}};

class IzsMatrixMotion : public IUnknown {
public:
    // Speed: 1..2 times normal. New streams: 0..2 times the normal birth rate.
    // These overrides never change saved speed variance or the maximum stream count.
    virtual void __stdcall SetAudioMotion(double speed, double spawn) = 0;
    virtual double __stdcall GetAudioSpeed() const = 0;
    virtual double __stdcall GetAudioSpawn() const = 0;
};

inline bool ApplyAudioMotion(IzsMatrix &matrix, double speed, double spawn) {
    IzsMatrixMotion *motion = nullptr;
    if(FAILED(matrix.QueryInterface(IID_IZSMATRIXMOTION,reinterpret_cast<void **>(&motion)))) return false;
    motion->SetAudioMotion(speed,spawn);
    motion->Release();
    return true;
}