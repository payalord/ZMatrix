// Pixel-level engine regression test; no desktop drawing or user settings are changed.
// Run from the repository root. Optional argument: the engine DLL before this change.
#include "../zsMatrix/IzsMatrixAppearance.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <vector>

static void Check(bool ok, const char *message) { if(!ok) throw std::runtime_error(message); }
static const GUID MatrixClass = {0x2e393599,0xf2e3,0x484a,{0x9b,0x03,0xd4,0x14,0xf6,0xcb,0xa5,0xeb}};
static const GUID MatrixInterface = {0xcbeac95f,0x3125,0x4603,{0xa0,0xc9,0x09,0x29,0xbd,0xc6,0x0f,0xbc}};

struct Surface {
    int width, height;
    HDC dc;
    HBITMAP bitmap;
    HGDIOBJ previous;
    DWORD *pixels = nullptr;
    Surface(int w,int h) : width(w),height(h) {
        dc=CreateCompatibleDC(nullptr);
        BITMAPINFO info={}; info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
        info.bmiHeader.biWidth=w; info.bmiHeader.biHeight=-h;
        info.bmiHeader.biPlanes=1; info.bmiHeader.biBitCount=32; info.bmiHeader.biCompression=BI_RGB;
        bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,reinterpret_cast<void **>(&pixels),nullptr,0);
        Check(dc && bitmap,"Cannot create test surface.");
        previous=SelectObject(dc,bitmap);
    }
    ~Surface() { DeleteDC(dc); DeleteObject(bitmap); }
    void Fill(DWORD color) { GdiFlush(); std::fill(pixels,pixels+width*height,color); }
    std::vector<DWORD> Read() const {
        GdiFlush(); std::vector<DWORD> result(pixels,pixels+width*height);
        for(auto &pixel:result) pixel&=0xffffff;
        return result;
    }
};
struct Engine {
    HMODULE module;
    IzsMatrix *matrix=nullptr;
    explicit Engine(const wchar_t *path) : module(LoadLibraryW(path)) {
        Check(module!=nullptr,"Cannot load engine DLL.");
        using Factory=HRESULT(STDAPICALLTYPE *)(REFCLSID,REFIID,void **);
        auto create=reinterpret_cast<Factory>(GetProcAddress(module,"DllGetClassObject"));
        IClassFactory *factory=nullptr;
        Check(create && SUCCEEDED(create(MatrixClass,IID_IClassFactory,reinterpret_cast<void **>(&factory))),"Cannot get engine factory.");
        const HRESULT result=factory->CreateInstance(nullptr,MatrixInterface,reinterpret_cast<void **>(&matrix));
        factory->Release(); Check(SUCCEEDED(result),"Cannot create test engine.");
    }
    ~Engine() { if(matrix) matrix->Release(); if(module) FreeLibrary(module); }
};

static void Arm(IzsMatrix &matrix,bool special,int x=28,int y=42) {
    auto stream=matrix.GetStreams()[0];
    stream->SetStatus(true); stream->SetStartX(x); stream->SetStartY(y);
    stream->SetTicksToWait(0); stream->SetTickCounter(1); stream->SetNeedsDrawing(true);
    stream->SetSpecialStreamFlag(false); stream->SetSpecialStreamStringIndex(0);
    stream->SetSpecialStreamStringCharIndex(special ? 1 : stream->GetSpecialStreamStringInvalidCharIndex());
}
static void Configure(IzsMatrix &matrix,HWND window,Surface &background,int mode,bool opaque,bool special,DWORD color) {
    // UpdateTarget copies the bitmap, which must not be selected into another DC.
    SelectObject(background.dc,background.previous);
    matrix.UpdateTarget(window,background.bitmap);
    SelectObject(background.dc,background.bitmap);
    matrix.SetBGMode(mode==3 ? bgmodeColor : bgmodeBitmap);
    matrix.SetBlendMode(mode==3 ? blendmodeOR : mode);
    matrix.SetBGColor((color>>16)&255,(color>>8)&255,color&255,opaque ? 255 : 0);
    matrix.SetColor(187,255,187,255); matrix.SetFadeColor(0,128,0,255);
    matrix.SetSpecialStringColor(251,255,251,255); matrix.SetSpecialStringFadeColor(208,244,208,128);
    LOGFONTW font={}; font.lfHeight=-18; font.lfQuality=NONANTIALIASED_QUALITY;
    wcscpy_s(font.lfFaceName,L"Courier New"); matrix.SetLogFont(font); matrix.SetSpecialStringLogFont(font);
    matrix.SetValidCharSet(L"M",1); matrix.ClearValidSpecialStringSet(); matrix.AddSpecialStringToValidSet(L"MM");
    matrix.SetSpeedVariance(0); matrix.SetMonotonousCleanupEnabled(false); matrix.SetRandomizedCleanupEnabled(false);
    matrix.SetMaxStream(1);
    Arm(matrix,special);
    if(special) {
        matrix.SetCoeffR1(0.5); matrix.SetCoeffG1(1.25); matrix.SetCoeffB1(0.75);
        matrix.SetCoeffR0(23.5); matrix.SetCoeffG0(10.25); matrix.SetCoeffB0(41.75);
    }
}
static void CheckMix(const std::vector<DWORD> &actual,const std::vector<DWORD> &full,
                     const std::vector<DWORD> &plain,unsigned percent) {
    for(size_t i=0;i<actual.size();++i) for(unsigned shift:{0u,8u,16u}) {
        const int a=(actual[i]>>shift)&255, f=(full[i]>>shift)&255, b=(plain[i]>>shift)&255;
        const double expected=(percent*f+(100-percent)*b)/100.0;
        Check(std::abs(a-expected)<=2,"Blend strength does not interpolate plain and wallpaper-based character colors.");
    }
}
static std::vector<DWORD> Frame(IzsMatrix &matrix,Surface &output,DWORD color,bool special,int x=28,int y=42) {
    output.Fill(color); Arm(matrix,special,x,y);
    const DWORD objects=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
    Check(matrix.Render(output.dc)!=0,"Rendering failed.");
    Check(GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS)==objects,"Rendering allocated persistent GDI objects.");
    return output.Read();
}
int wmain(int argc,wchar_t **argv) {
    CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
    const int width=128,height=96;
    HWND window=CreateWindowExW(0,L"STATIC",L"ZMatrix rendering test",WS_POPUP,0,0,width,height,nullptr,nullptr,nullptr,nullptr);
    int result=0;
    try {
        Check(window!=nullptr,"Cannot create hidden test window.");
        Surface background(width,height),alternate(width,height),output(width,height);
        for(int y=0;y<height;++y) for(int x=0;x<width;++x)
            background.pixels[y*width+x]=RGB((x*3+y)%256,(y*5+x)%256,(x+y*2)%256);
        alternate.Fill(0xc83675);
        for(DWORD color:{0u,0x1d2f3du}) for(int mode=0;mode<4;++mode) for(bool opaque:{false,true}) for(bool special:{false,true}) {
            Engine engine(L".\\zsMatrix.dll");
            Check(ReadBlendStrength(*engine.matrix)==100,"New engine must default to full strength.");
            Configure(*engine.matrix,window,background,mode,opaque,special,color);
            const auto full=Frame(*engine.matrix,output,color,special);
            Check(full!=std::vector<DWORD>(width*height,color),"Test did not render any characters.");
            Check(full.front()==color && full.back()==color,"First frame exposed wallpaper outside the streams.");
            if(argc>1) {
                Engine previous(argv[1]);
                Configure(*previous.matrix,window,background,mode,opaque,special,color);
                Check(Frame(*previous.matrix,output,color,special)==full,"100% changed the original rendering from a solid/black start.");
                Surface history(width,height);
                history.Fill(color); output.Fill(color);
                previous.matrix->SetMonotonousCleanupEnabled(true); previous.matrix->SetBackTrace(3);
                engine.matrix->SetMonotonousCleanupEnabled(true); engine.matrix->SetBackTrace(3);
                for(int y=14;y<height+28;y+=14) {
                    Arm(*previous.matrix,special,28,y); Arm(*engine.matrix,special,28,y);
                    previous.matrix->Render(history.dc); engine.matrix->Render(output.dc);
                    Check(history.Read()==output.Read(),"100% changed progressive drawing, trails or cleanup.");
                }
                engine.matrix->SetMonotonousCleanupEnabled(false);
            }
            Check(ApplyBlendStrength(*engine.matrix,0),"Missing appearance interface.");
            const auto plain=Frame(*engine.matrix,output,color,special);
            Engine solid(argc>1 ? argv[1] : L".\\zsMatrix.dll");
            Configure(*solid.matrix,window,background,3,opaque,special,color);
            Check(Frame(*solid.matrix,output,color,special)==plain,"Zero strength differs from plain text on the selected background.");
            Engine other(L".\\zsMatrix.dll");
            Configure(*other.matrix,window,alternate,mode,opaque,special,color);
            ApplyBlendStrength(*other.matrix,0);
            Check(Frame(*other.matrix,output,color,special)==plain,"Wallpaper affects zero-strength characters.");
            for(unsigned percent:{0u,25u,50u,75u,100u}) {
                ApplyBlendStrength(*engine.matrix,percent);
                const auto first=Frame(*engine.matrix,output,color,special);
                CheckMix(first,full,plain,percent);
                Arm(*engine.matrix,special);
                Check(engine.matrix->Render(output.dc)!=0 && output.Read()==first,"Repeated character drawing faded or accumulated colors.");
                // With no dirty characters, a strength change must leave existing trails untouched.
                engine.matrix->GetStreams()[0]->SetNeedsDrawing(false);
                engine.matrix->GetStreams()[0]->SetTickCounter(1);
                ApplyBlendStrength(*engine.matrix,100-percent);
                Check(engine.matrix->Render(output.dc)!=0 && output.Read()==first,"An idle frame repainted the screen or existing trails.");
                if(mode==3) Check(first==full,"Strength unexpectedly affects solid-background mode.");
            }
            ApplyBlendStrength(*engine.matrix,50);
            const auto mixed=Frame(*engine.matrix,output,color,special);
            Arm(*engine.matrix,special);
            output.Fill(0x123456);
            const int saved=SaveDC(output.dc);
            IntersectClipRect(output.dc,20,15,100,75); SetViewportOrgEx(output.dc,7,-3,nullptr);
            Check(engine.matrix->Render(output.dc)!=0,"Translated/clipped rendering failed.");
            RestoreDC(output.dc,saved);
            const auto clipped=output.Read();
            for(int y=0;y<height;++y) for(int x=0;x<width;++x) {
                if(x<20 || x>=100 || y<15 || y>=75) Check(clipped[y*width+x]==0x123456,"Rendering overwrote excluded desktop regions.");
                else if(mixed[(y+3)*width+x-7]!=color)
                    Check(clipped[y*width+x]==mixed[(y+3)*width+x-7],"Character colors changed with target clipping/origin.");
            }
            Engine copy(L".\\zsMatrix.dll"); copy.matrix->CopyFrom(*engine.matrix);
            Check(ReadBlendStrength(*copy.matrix)==50,"Engine copy lost blend strength.");
            copy.matrix->CopyFrom(*copy.matrix);
            Check(ReadBlendStrength(*copy.matrix)==50,"Self-copy damaged the appearance state.");
            ApplyBlendStrength(*copy.matrix,200);
            Check(ReadBlendStrength(*copy.matrix)==100,"Engine accepted strength above 100%.");
            // Cleanup and off-screen glyphs must obey the same endpoint/interpolation rules.
            engine.matrix->SetMonotonousCleanupEnabled(true); engine.matrix->SetBackTrace(3);
            engine.matrix->SetRandomizedCleanupEnabled(true); engine.matrix->SetLeading(4); engine.matrix->SetSpacePad(0);
            for(POINT point:std::vector<POINT>{{28,70},{-4,8},{width-4,height-2}}) {
                ApplyBlendStrength(*engine.matrix,100);
                const auto edgeFull=Frame(*engine.matrix,output,color,special,point.x,point.y);
                ApplyBlendStrength(*engine.matrix,0);
                const auto edgePlain=Frame(*engine.matrix,output,color,special,point.x,point.y);
                ApplyBlendStrength(*engine.matrix,50);
                CheckMix(Frame(*engine.matrix,output,color,special,point.x,point.y),edgeFull,edgePlain,50);
            }
            Surface resized(64,48); resized.Fill(0x335577);
            Configure(*engine.matrix,window,resized,mode,opaque,special,color);
            ApplyBlendStrength(*engine.matrix,0);
            const auto resizedPlain=Frame(*engine.matrix,resized,color,special);
            Check(resizedPlain.front()==color && resizedPlain.back()==color,"Resize caused full-surface painting.");
        }
        puts("PASS: Original black/solid startup at 100%, plain colors at 0%, per-character interpolation, unchanged idle frames/trails, cleanup, off-screen glyphs, XOR/AND/OR, text opacity, special strings, audio, clipping/origin, copies, resize and no added GDI objects.");
    } catch(const std::exception &error) { fprintf(stderr,"FAIL: %s\n",error.what()); result=1; }
    if(window) DestroyWindow(window);
    CoUninitialize(); return result;
}
