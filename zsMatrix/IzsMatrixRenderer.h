// Optional multi-target rendering; the original IzsMatrix vtable is unchanged.
#pragma once
#include "IzsMatrix.h"

static const GUID IID_IZSMATRIXRENDERER =
{0x3e631b28, 0x9a6d, 0x4d48, {0x93, 0xaf, 0x51, 0x3e, 0x23, 0x8d, 0x2a, 0xb7}};

struct MatrixRenderTarget {
    HDC dc;
    // Bounds in the engine's canvas coordinates. The caller maps those
    // coordinates to the target DC with its viewport origin.
    RECT bounds;
};

class IzsMatrixRenderer : public IUnknown {
public:
    // Advances the animation once, then routes each draw to the intersecting
    // targets. DC state is restored; no DC or target pointer is retained.
    virtual int __stdcall RenderTargets(const MatrixRenderTarget* targets, unsigned count) = 0;
};
