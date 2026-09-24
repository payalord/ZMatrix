// Pixel-level engine regression test; no desktop drawing or user settings are changed.
// Run from the repository root. Optional argument: the engine DLL before this change.
#include "../zsMatrix/IzsMatrixAppearance.h"
#include "../zsMatrix/IzsMatrixMotion.h"
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

static void CheckAudioMotion(HWND window) {
    Surface background(128,96), normalPixels(128,96), fastPixels(128,96);
    background.Fill(0x193a52);
    auto setup = [&](Engine &engine, unsigned maximum) {
        SelectObject(background.dc,background.previous); engine.matrix->UpdateTarget(window,background.bitmap);
        SelectObject(background.dc,background.bitmap);
        engine.matrix->SetMaxStream(0); engine.matrix->SetMaxStream(maximum);
        engine.matrix->SetSpeedVariance(0); engine.matrix->SetSpecialStringStreamProbability(0);
        engine.matrix->SetMonotonousCleanupEnabled(false); engine.matrix->SetRandomizedCleanupEnabled(false);
        engine.matrix->SetValidCharSet(L"M",1);
        engine.matrix->SetBGMode(bgmodeBitmap);
    };
    for(int mode = 0; mode < 6; ++mode) for(bool special : {false,true}) {
        Engine normal(L".\\zsMatrix.dll"), fast(L".\\zsMatrix.dll");
        setup(normal,1); setup(fast,1);
        normal.matrix->Render(normalPixels.dc); fast.matrix->Render(fastPixels.dc);
        for(auto engine : {normal.matrix,fast.matrix}) {
            engine->SetBlendMode(mode); ApplyGlowEnabled(*engine,true); ApplyBlendStrength(*engine,65);
            engine->SetMonotonousCleanupEnabled(true); engine->SetBackTrace(3);
            engine->SetRandomizedCleanupEnabled(true); engine->SetLeading(4); engine->SetSpacePad(0);
            engine->ClearValidSpecialStringSet(); engine->AddSpecialStringToValidSet(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
            LOGFONTW font = {}; font.lfHeight = -8; wcscpy_s(font.lfFaceName,L"Courier New");
            engine->SetLogFont(font); font.lfHeight = -10; engine->SetSpecialStringLogFont(font);
            auto stream = engine->GetStreams()[0];
            stream->SetStartX(28); stream->SetStartY(-12); stream->SetTickCounter(0); stream->SetNeedsDrawing(false);
            stream->SetSpecialStreamFlag(special); stream->SetSpecialStreamStringIndex(0);
            stream->SetSpecialStreamStringCharIndex(special ? 0 : stream->GetSpecialStreamStringInvalidCharIndex());
        }
        Check(ApplyAudioMotion(*normal.matrix,1,0) && ApplyAudioMotion(*fast.matrix,2,0),"Motion interface unavailable.");
        normalPixels.Fill(0); fastPixels.Fill(0);
        for(int i = 0; i < 8; ++i) normal.matrix->Render(normalPixels.dc);
        for(int i = 0; i < 4; ++i) fast.matrix->Render(fastPixels.dc);
        Check(normalPixels.Read() == fastPixels.Read(),"Accelerated streams skipped characters, special text, blending or cleanup.");
        Check(normal.matrix->GetStreams()[0]->GetStartY() == fast.matrix->GetStreams()[0]->GetStartY(),"Double speed advanced the wrong distance.");
    }
    Engine engine(L".\\zsMatrix.dll"); setup(engine,100);
    // Keep streams above the target so the lifecycle does not affect birth/tick accounting.
    auto liftStreams = [&]() {
        for(unsigned i = 0; i < engine.matrix->GetMaxStream(); ++i)
            if(engine.matrix->GetStreams()[i]->GetStatus()) engine.matrix->GetStreams()[i]->SetStartY(-1000);
    };
    for(double speed : {1.0,1.5,2.0}) for(double spawn : {0.0,0.25,1.0,1.5,2.0}) {
        engine.matrix->SetMaxStream(0); engine.matrix->SetMaxStream(100);
        ApplyAudioMotion(*engine.matrix,1,1); ApplyAudioMotion(*engine.matrix,speed,spawn);
        for(int i = 0; i < 20; ++i) { engine.matrix->Render(normalPixels.dc); liftStreams(); }
        Check(engine.matrix->GetNumStreams() == static_cast<unsigned>(20*spawn),"Birth rate was coupled to speed or lost fractional births.");
    }
    engine.matrix->SetMaxStream(0); engine.matrix->SetMaxStream(7);
    ApplyAudioMotion(*engine.matrix,2,2);
    for(int i = 0; i < 20; ++i) { engine.matrix->Render(normalPixels.dc); liftStreams(); }
    Check(engine.matrix->GetNumStreams() == 7 && engine.matrix->GetMaxStream() == 7,"Audio exceeded or changed Maximum streams.");
    auto stream = engine.matrix->GetStreams()[0];
    stream->SetStartY(-1000); stream->SetTicksToWait(3); stream->SetTickCounter(0);
    ApplyAudioMotion(*engine.matrix,1.5,0);
    for(int i = 0; i < 8; ++i) engine.matrix->Render(normalPixels.dc);
    const int delta = stream->GetStartY()+1000;
    stream->SetStartY(-1000); stream->SetTickCounter(0);
    ApplyAudioMotion(*engine.matrix,1,0);
    for(int i = 0; i < 12; ++i) engine.matrix->Render(normalPixels.dc);
    Check(stream->GetStartY()+1000 == delta && stream->GetTicksToWait() == 3,"Fractional speed lost individual stream timing.");
    // Disabling audio must discard a partial tick, not produce a later extra step.
    ApplyAudioMotion(*engine.matrix,1.5,0); engine.matrix->Render(normalPixels.dc);
    ApplyAudioMotion(*engine.matrix,1,1);
    stream->SetStartY(-1000); stream->SetTicksToWait(0); stream->SetTickCounter(0);
    engine.matrix->Render(normalPixels.dc); const int step = stream->GetStartY()+1000;
    engine.matrix->Render(normalPixels.dc);
    Check(stream->GetStartY()+1000 == 2*step,"Disabled audio retained fractional motion state.");
    const DWORD objects = GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
    for(int i = 0; i < 100; ++i) { ApplyAudioMotion(*engine.matrix,1.75,0.75); engine.matrix->Render(normalPixels.dc); liftStreams(); }
    Check(GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS) == objects,"Motion allocated rendering surfaces per frame.");
}

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
    matrix.SetBGMode(mode==6 ? bgmodeColor : bgmodeBitmap);
    matrix.SetBlendMode(mode==6 ? blendmodeOR : mode);
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
    const DWORD after=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
    const bool arithmetic=matrix.GetBGMode()==bgmodeBitmap && matrix.GetBlendMode()>=blendmodeShading && matrix.GetBlendMode()<=blendmodeMultiply && ReadBlendStrength(matrix)>0;
    Check(after==objects || (arithmetic && after==objects+2),"Rendering allocated unexpected GDI objects.");
    return output.Read();
}
static void CheckArithmeticColors(HWND window) {
    Surface background(128,96), output(128,96);
    Engine mask(L".\\zsMatrix.dll"), engine(L".\\zsMatrix.dll");
    background.Fill(0);
    Configure(*mask.matrix,window,background,6,false,false,0);
    mask.matrix->SetColor(255,255,255,255); mask.matrix->SetFadeColor(255,255,255,255);
    const auto coverage=Frame(*mask.matrix,output,0,false);
    const DWORD wallpapers[]={0,0xffffff,0x804020};
    // Independently calculated RGB fixtures for C=(60,180,100).
    const DWORD expected[3][3]={{0,0x3cb464,0x12351d},{0x3cb464,0xffffff,0x9ec777},{0,0x3cb464,0x1e2d0d}};
    for(int mode=blendmodeShading;mode<=blendmodeMultiply;++mode) for(int wallpaper=0;wallpaper<3;++wallpaper) {
        background.Fill(wallpapers[wallpaper]);
        Configure(*engine.matrix,window,background,mode,false,false,0);
        engine.matrix->SetColor(60,180,100,255); engine.matrix->SetFadeColor(60,180,100,255);
        const auto actual=Frame(*engine.matrix,output,0,false);
        for(size_t pixel=0;pixel<actual.size();++pixel)
            Check(actual[pixel]==(coverage[pixel] ? expected[mode-blendmodeShading][wallpaper] : 0),"Arithmetic blend disagrees with the known RGB fixture.");
    }
    // Large glyphs cross tile boundaries; antialiasing and glow must remain continuous.
    Surface largeBackground(256,256), largeOutput(256,256); largeBackground.Fill(0xffffff);
    Check(SetWindowPos(window,nullptr,0,0,256,256,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE)!=FALSE,"Cannot resize the hidden rendering target.");
    for(BYTE quality:{BYTE(NONANTIALIASED_QUALITY),BYTE(ANTIALIASED_QUALITY),BYTE(CLEARTYPE_QUALITY)}) for(bool glow:{false,true}) {
        Configure(*engine.matrix,window,largeBackground,blendmodeScreen,false,false,0);
        Configure(*mask.matrix,window,largeBackground,6,false,false,0);
        LOGFONTW font={}; font.lfHeight=-180; font.lfQuality=quality; wcscpy_s(font.lfFaceName,L"Courier New");
        for(IzsMatrix *m:{engine.matrix,mask.matrix}) {
            m->SetLogFont(font); m->SetSpecialStringLogFont(font);
            ApplyGlowEnabled(*m,glow);
        }
        mask.matrix->SetColor(255,255,255,255); mask.matrix->SetFadeColor(255,255,255,255);
        const auto white=Frame(*mask.matrix,largeOutput,0,false,115,160);
        Check(white!=std::vector<DWORD>(256*256,0),"Large-glyph fixture did not render.");
        Check(Frame(*engine.matrix,largeOutput,0,false,115,160)==white,"Tiling changed glyph coverage, antialiasing or glow.");
        const DWORD objects=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);
        for(int repeat=0;repeat<8;++repeat) Frame(*engine.matrix,largeOutput,0,false,115,160);
        Check(GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS)==objects,"Repeated rendering leaked GDI objects.");
        engine.matrix->SetBlendMode(blendmodeOR);
        Check(GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS)+2==objects,"Returning to a legacy mode retained the blend workspace.");
    }
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
        for(bool glow:{false,true}) for(DWORD color:{0u,0x1d2f3du}) for(int mode=0;mode<7;++mode) for(bool opaque:{false,true}) for(bool special:{false,true}) {
            Engine engine(L".\\zsMatrix.dll");
            Check(ReadBlendStrength(*engine.matrix)==100,"New engine must default to full strength.");
            Check(!ReadGlowEnabled(*engine.matrix),"New engine must default to glow off.");
            Check(ApplyGlowEnabled(*engine.matrix,glow),"Missing glow interface.");
            Configure(*engine.matrix,window,background,mode,opaque,special,color);
            const auto full=Frame(*engine.matrix,output,color,special);
            Check(full!=std::vector<DWORD>(width*height,color),"Test did not render any characters.");
            Check(full.front()==color && full.back()==color,"First frame exposed wallpaper outside the streams.");
            if(glow) {
                ApplyGlowEnabled(*engine.matrix,false);
                const auto disabled=Frame(*engine.matrix,output,color,special);
                Check(disabled!=full,"Enabling glow did not change the rendered characters.");
                Engine reference(argc>1 && (mode<3 || mode==6) ? argv[1] : L".\\zsMatrix.dll");
                Configure(*reference.matrix,window,background,mode,opaque,special,color);
                Check(Frame(*reference.matrix,output,color,special)==disabled,"Disabling glow did not restore the original renderer.");
                ApplyGlowEnabled(*engine.matrix,true);
            }
            if(argc>1 && !glow && (mode<3 || mode==6)) {
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
            Engine solid(argc>1 && !glow ? argv[1] : L".\\zsMatrix.dll");
            if(glow) ApplyGlowEnabled(*solid.matrix,true);
            Configure(*solid.matrix,window,background,6,opaque,special,color);
            Check(Frame(*solid.matrix,output,color,special)==plain,"Zero strength differs from plain text on the selected background.");
            Engine other(L".\\zsMatrix.dll");
            ApplyGlowEnabled(*other.matrix,glow);
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
                if(mode==6) Check(first==full,"Strength unexpectedly affects solid-background mode.");
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
            Check(copy.matrix->GetBlendMode()==engine.matrix->GetBlendMode(),"Engine copy lost blend mode.");
            Check(ReadGlowEnabled(*copy.matrix)==glow,"Engine copy lost glow state.");
            Check(ReadBlendStrength(*copy.matrix)==50,"Engine copy lost blend strength.");
            copy.matrix->CopyFrom(*copy.matrix);
            Check(ReadBlendStrength(*copy.matrix)==50,"Self-copy damaged the appearance state.");
            Check(ReadGlowEnabled(*copy.matrix)==glow,"Self-copy damaged glow state.");
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
        CheckArithmeticColors(window);
        CheckAudioMotion(window);
        puts("PASS: Legacy rendering, all six blend modes, known RGB fixtures, tiled large glyphs, antialiasing/glow, black/solid startup, strength endpoints/interpolation, idle frames/trails, cleanup, off-screen glyphs, text opacity, special strings, audio, clipping/origin, copies, resize and bounded GDI objects.");
    } catch(const std::exception &error) { fprintf(stderr,"FAIL: %s\n",error.what()); result=1; }
    if(window) DestroyWindow(window);
    CoUninitialize(); return result;
}
