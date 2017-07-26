/*!=========================================================================
//
//  Program:   ZMatrix
//  Module:    $RCSfile: ConfigForm.cpp,v $
//  Language:  C/C++
//  Date:      $Date: 2003/05/12 07:43:56 $
//  Version:   $Revision: 1.21 $
//
//  Copyright (c) 2001-2002 Z. Shaker
//  All rights reserved.
//  See License.txt for details.
//
//  This file is part of ZMatrix.
//
//  ZMatrix is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  ZMatrix is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with ZMatrix; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//=========================================================================*/

//---------------------------------------------------------------------------
#define NO_WIN32_LEAN_AND_MEAN
#include <vcl.h>
#pragma hdrstop
#include <tchar.h>
#include <SysUtils.hpp>
#include <stdio.h>
#include <htmlhelp.h>

#include "ConfigForm.h"
#include "AboutUnit.h"
#include "HireUnit.h"


//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TConfigurationForm *ConfigurationForm = NULL;

#ifndef min
#define min(a,b) (a)<(b)? (a):(b)
#endif

#ifndef CLAMP
#define CLAMP(var,min,max)\
if((var) > (max)) (var) = (max);\
if((var) < (min)) (var) = (min);
#endif

#define LONG_MAX_PATH 2048

//---------------------------------------------------------------------------
extern "C" void  __stdcall LaunchAboutForm(void *Parent)
{
    static bool AlreadyLaunched = false;

    if(!AlreadyLaunched)
    {
        AlreadyLaunched = true;

        if(AboutForm == NULL)
        {
            AboutForm = new TAboutForm((TComponent *)Parent);
        }

        AboutForm->ShowModal();

        if(AboutForm != NULL)
        {
            delete AboutForm;
            AboutForm = NULL;
        }

        AlreadyLaunched = false;
    }
}
//---------------------------------------------------------------------------
extern "C" void  __stdcall LaunchHireForm(void *Parent)
{
    static bool AlreadyLaunched = false;

    if(!AlreadyLaunched)
    {
        AlreadyLaunched = true;

        if(HireForm == NULL)
        {
            HireForm = new THireForm((TComponent *)Parent);
        }

        HireForm->ShowModal();

        if(HireForm != NULL)
        {
            delete HireForm;
            HireForm = NULL;
        }

        AlreadyLaunched = false;
    }
}
//---------------------------------------------------------------------------
extern "C" int __stdcall LaunchConfigForm(IzsMatrix *Matrix,unsigned int &RefreshTime,DWORD &Priority)
{
    //zsMatrix Matrix(NULL,NULL);
    int returnValue;

    ConfigurationForm = new TConfigurationForm(NULL,Matrix,&RefreshTime,&Priority);

    returnValue = ConfigurationForm->ShowModal();

    delete ConfigurationForm;

    ConfigurationForm = NULL;

    if(returnValue == mrOk)
        return true;

    return false;
}
//---------------------------------------------------------------------------
int  __stdcall SaveConfigToFile(IzsMatrix *Matrix,unsigned int RefreshTime,DWORD Priority,_TCHAR *FileName)
{
    if((Matrix == NULL) || (FileName == NULL))
    {
        return 0;
    }
    AnsiString SectionName;
    AnsiString ValueName;

    TIniFile *OutFile = new TIniFile(ExpandFileName(AnsiString(FileName)));
    //TIniFile *OutFile = new TIniFile(AnsiString(FileName));

    SectionName = "General";

    ValueName = "ConfigFileFormatVersion";
    OutFile->WriteString(SectionName,ValueName,"1.0");

    ValueName = "MaxStream";
    OutFile->WriteInteger(SectionName,ValueName,Matrix->GetMaxStream());
    ValueName = "SpeedVariance";
    OutFile->WriteInteger(SectionName,ValueName,Matrix->GetSpeedVariance());
    ValueName = "MonotonousCleanupEnabled";
    OutFile->WriteBool(SectionName,ValueName,Matrix->GetMonotonousCleanupEnabled());
    ValueName = "BackTrace";
    OutFile->WriteInteger(SectionName,ValueName,Matrix->GetBackTrace());
    ValueName = "RandomizedCleanupEnabled";
    OutFile->WriteBool(SectionName,ValueName,Matrix->GetRandomizedCleanupEnabled());
    ValueName = "Leading";
    OutFile->WriteInteger(SectionName,ValueName,Matrix->GetLeading());
    ValueName = "SpacePad";
    OutFile->WriteInteger(SectionName,ValueName,Matrix->GetSpacePad());

    ValueName = "RefreshTime";
    OutFile->WriteInteger(SectionName,ValueName,RefreshTime);

    ValueName = "PriorityClass";
    AnsiString PriorityName;
    switch(Priority)
    {
    case(ABOVE_NORMAL_PRIORITY_CLASS): PriorityName = "ABOVE_NORMAL_PRIORITY_CLASS"; break;
    case(BELOW_NORMAL_PRIORITY_CLASS): PriorityName = "BELOW_NORMAL_PRIORITY_CLASS"; break;
    case(HIGH_PRIORITY_CLASS): PriorityName = "HIGH_PRIORITY_CLASS"; break;
    case(IDLE_PRIORITY_CLASS): PriorityName = "IDLE_PRIORITY_CLASS"; break;
    case(NORMAL_PRIORITY_CLASS): PriorityName = "NORMAL_PRIORITY_CLASS"; break;
    default: PriorityName = "IDLE_PRIORITY_CLASS"; break;
    }
    OutFile->WriteString(SectionName,ValueName,PriorityName);

    ValueName = "SpecialStringStreamProbability";
    OutFile->WriteFloat(SectionName,ValueName,Matrix->GetSpecialStringStreamProbability());

    SectionName = "Text";

    LOGFONT LogFont;
    GetObject(Matrix->GetFont(),sizeof(LOGFONT),&LogFont);

    WriteLogFontToConfigFile(OutFile,SectionName,LogFont);

    ValueName = "CharSet";
    AnsiString CharSet;
    ConvertCharSetToAnsiString(Matrix->GetValidCharSet(),Matrix->GetNumCharsInSet(),CharSet);

    OutFile->WriteString(SectionName,ValueName,CharSet);


    SectionName = "SpecialText";

    GetObject(Matrix->GetSpecialStringFont(),sizeof(LOGFONT),&LogFont);

    WriteLogFontToConfigFile(OutFile,SectionName,LogFont);

    ValueName = "Strings";
    AnsiString SpecialStrings;
    vector<tstring> SpecialStringVec;
    SpecialStringVec.resize(Matrix->GetNumSpecialStringsInSet());
    unsigned int i = 0;
    for(vector<tstring>::iterator iter = SpecialStringVec.begin();
        iter != SpecialStringVec.end();
        ++iter, ++i)
    {
        iter->assign(Matrix->GetValidSpecialString(i));
    }
    ConvertSpecialStringsToAnsiString(SpecialStringVec,SpecialStrings,CONFIG_FILE_SPECIAL_STRING_DELIMITER);

    OutFile->WriteString(SectionName,ValueName,SpecialStrings);




    SectionName = "Colors";

    ValueName = "FGColor";
    BYTE R,G,B,A;
    Matrix->GetColor(R,G,B,A);
    AnsiString ForegroundColor = "{" + AnsiString(R) + "," + AnsiString(G) + "," + AnsiString(B) + "," + AnsiString(A) + "}";

    OutFile->WriteString(SectionName,ValueName,ForegroundColor);

    ValueName = "FadeColor";

    Matrix->GetFadeColor(R,G,B,A);
    AnsiString FadeColor = "{" + AnsiString(R) + "," + AnsiString(G) + "," + AnsiString(B) + "," + AnsiString(A) + "}";

    OutFile->WriteString(SectionName,ValueName,FadeColor);

    ValueName = "BGColor";
    Matrix->GetBGColor(R,G,B,A);
    AnsiString BackgroundColor = "{" + AnsiString(R) + "," + AnsiString(G) + "," + AnsiString(B) + "," + AnsiString(A) + "}";

    OutFile->WriteString(SectionName,ValueName,BackgroundColor);




    ValueName = "SpecialStringFGColor";
    Matrix->GetSpecialStringColor(R,G,B,A);
    AnsiString SpecialStringForegroundColor = "{" + AnsiString(R) + "," + AnsiString(G) + "," + AnsiString(B) + "," + AnsiString(A) + "}";

    OutFile->WriteString(SectionName,ValueName,SpecialStringForegroundColor);

    ValueName = "SpecialStringFadeColor";

    Matrix->GetSpecialStringFadeColor(R,G,B,A);
    AnsiString SpecialStringFadeColor = "{" + AnsiString(R) + "," + AnsiString(G) + "," + AnsiString(B) + "," + AnsiString(A) + "}";

    OutFile->WriteString(SectionName,ValueName,SpecialStringFadeColor);

    ValueName = "SpecialStringBGColor";
    Matrix->GetSpecialStringBGColor(R,G,B,A);
    AnsiString SpecialStringBackgroundColor = "{" + AnsiString(R) + "," + AnsiString(G) + "," + AnsiString(B) + "," + AnsiString(A) + "}";

    OutFile->WriteString(SectionName,ValueName,SpecialStringBackgroundColor);








    ValueName = "BGMode";
    TBGMode BGMode = Matrix->GetBGMode();
    AnsiString BGModeString;

    switch(BGMode)
    {
    case(bgmodeColor):
    {
        BGModeString = "bgmodeColor";
    }
    break;
    default:
    case(bgmodeBitmap):
    {
        BGModeString = "bgmodeBitmap";
    }
    break;
    }
    OutFile->WriteString(SectionName,ValueName,BGModeString);


    ValueName = "BlendMode";
    TBlendMode BlendMode = Matrix->GetBlendMode();
    AnsiString BlendModeString;

    switch(BlendMode)
    {
    case(blendmodeOR):
    {
        BlendModeString = "blendmodeOR";
    }
    break;
    case(blendmodeAND):
    {
        BlendModeString = "blendmodeAND";
    }
    break;
    default:
    case(blendmodeXOR):
    {
        BlendModeString = "blendmodeXOR";
    }
    break;
    }
    OutFile->WriteString(SectionName,ValueName,BlendModeString);


    delete OutFile;
    return 1;
}
//---------------------------------------------------------------------------
int  __stdcall LoadConfigFromFile(IzsMatrix *Matrix,unsigned int &RefreshTime,DWORD &Priority,_TCHAR *FileName)
{
    if((Matrix == NULL) || (FileName == NULL))
    {
        return 0;
    }
    AnsiString SectionName;
    AnsiString ValueName;

    TMemIniFile *InFile = new TMemIniFile(ExpandFileName(AnsiString(FileName)));
    //TMemIniFile *InFile = new TMemIniFile(AnsiString(FileName));

    SectionName = "General";

    ValueName = "MaxStream";
    Matrix->SetMaxStream(InFile->ReadInteger(SectionName,ValueName,Matrix->GetMaxStream()));
    ValueName = "SpeedVariance";
    Matrix->SetSpeedVariance(InFile->ReadInteger(SectionName,ValueName,Matrix->GetSpeedVariance()));
    ValueName = "MonotonousCleanupEnabled";
    Matrix->SetMonotonousCleanupEnabled(InFile->ReadBool(SectionName,ValueName,Matrix->GetMonotonousCleanupEnabled()));
    ValueName = "BackTrace";
    Matrix->SetBackTrace(InFile->ReadInteger(SectionName,ValueName,Matrix->GetBackTrace()));
    ValueName = "RandomizedCleanupEnabled";
    Matrix->SetRandomizedCleanupEnabled(InFile->ReadBool(SectionName,ValueName,Matrix->GetRandomizedCleanupEnabled()));
    ValueName = "Leading";
    Matrix->SetLeading(InFile->ReadInteger(SectionName,ValueName,Matrix->GetLeading()));
    ValueName = "SpacePad";
    Matrix->SetSpacePad(InFile->ReadInteger(SectionName,ValueName,Matrix->GetSpacePad()));

    ValueName = "RefreshTime";
    RefreshTime = InFile->ReadInteger(SectionName,ValueName,RefreshTime);

    ValueName = "PriorityClass";
    AnsiString PriorityName = InFile->ReadString(SectionName,ValueName,"IDLE_PRIORITY_CLASS");
    if(PriorityName == "ABOVE_NORMAL_PRIORITY_CLASS")
    {
        Priority = ABOVE_NORMAL_PRIORITY_CLASS;
    }
    else if(PriorityName == "BELOW_NORMAL_PRIORITY_CLASS")
    {
        Priority = BELOW_NORMAL_PRIORITY_CLASS;
    }
    else if(PriorityName == "HIGH_PRIORITY_CLASS")
    {
        Priority = HIGH_PRIORITY_CLASS;
    }
    else if(PriorityName == "IDLE_PRIORITY_CLASS")
    {
        Priority = IDLE_PRIORITY_CLASS;
    }
    else if(PriorityName == "NORMAL_PRIORITY_CLASS")
    {
        Priority = NORMAL_PRIORITY_CLASS;
    }
    else
    {
        Priority = IDLE_PRIORITY_CLASS;
    }

    ValueName = "SpecialStringStreamProbability";
    Matrix->SetSpecialStringStreamProbability(InFile->ReadFloat(SectionName,ValueName,Matrix->GetSpecialStringStreamProbability()));

    SectionName = "Text";

    LOGFONT LogFont;
    ReadLogFontFromConfigFile(InFile,SectionName,LogFont);

    Matrix->SetLogFont(LogFont);


    ValueName = "CharSet";
    AnsiString CharSetInString = InFile->ReadString(SectionName,ValueName,"*");
    std::vector<_TCHAR> CharSet;
    ConvertAnsiStringToCharSet(CharSet,CharSetInString);

    if(CharSet.empty())
    {
        Matrix->ClearValidCharSet();
    }
    else
    {
        const _TCHAR *CharSetPointer = &(CharSet.front());
        unsigned int CharSetSize = CharSet.size();
        Matrix->SetValidCharSet(CharSetPointer,CharSetSize);
    }

    SectionName = "SpecialText";

    ReadLogFontFromConfigFile(InFile,SectionName,LogFont);

    Matrix->SetSpecialStringLogFont(LogFont);


    ValueName = "Strings";
    AnsiString SpecialStringsInString = InFile->ReadString(SectionName,ValueName,"The matrix has you");
    std::vector<tstring> SpecialStringVec;
    ConvertAnsiStringToSpecialStrings(SpecialStringVec,SpecialStringsInString,CONFIG_FILE_SPECIAL_STRING_DELIMITER);

    if(SpecialStringVec.empty())
    {
        Matrix->ClearValidSpecialStringSet();
    }
    else
    {
        unsigned int SpecialStringArraySize = SpecialStringVec.size();
        const _TCHAR * /*const*/ *SpecialStringArray = new const _TCHAR *[SpecialStringArraySize];

        for(unsigned int i = 0; i < SpecialStringArraySize; i++)
        {
            SpecialStringArray[i] = SpecialStringVec[i].c_str();
        }

        Matrix->SetValidSpecialStringSet(SpecialStringArray,SpecialStringArraySize);

        delete[] SpecialStringArray;
    }

    SectionName = "Colors";

    ValueName = "FGColor";
    int IntR = 150,IntG = 255,IntB = 100,IntA = 255;
    AnsiString ForegroundColorString = InFile->ReadString(SectionName,ValueName,"{150,255,100,255}");
    sscanf(ForegroundColorString.c_str(),"{%d,%d,%d,%d}",&IntR,&IntG,&IntB,&IntA);
    CLAMP(IntR,0,255);CLAMP(IntG,0,255);CLAMP(IntB,0,255);CLAMP(IntA,0,255);
    Matrix->SetColor((BYTE)IntR,(BYTE)IntG,(BYTE)IntB,(BYTE)IntA);

    ValueName = "FadeColor";
    IntR = 0,IntG = 0,IntB = 0,IntA = 0;
    AnsiString FadeColorString = InFile->ReadString(SectionName,ValueName,"{50,85,33,128}");
    sscanf(FadeColorString.c_str(),"{%d,%d,%d,%d}",&IntR,&IntG,&IntB,&IntA);
    CLAMP(IntR,0,255);CLAMP(IntG,0,255);CLAMP(IntB,0,255);CLAMP(IntA,0,255);
    Matrix->SetFadeColor((BYTE)IntR,(BYTE)IntG,(BYTE)IntB,(BYTE)IntA);

    ValueName = "BGColor";
    IntR = 0,IntG = 0,IntB = 0,IntA = 0;
    AnsiString BackgroundColorString = InFile->ReadString(SectionName,ValueName,"{0,0,0,0}");
    sscanf(BackgroundColorString.c_str(),"{%d,%d,%d,%d}",&IntR,&IntG,&IntB,&IntA);
    CLAMP(IntR,0,255);CLAMP(IntG,0,255);CLAMP(IntB,0,255);CLAMP(IntA,0,255);
    Matrix->SetBGColor((BYTE)IntR,(BYTE)IntG,(BYTE)IntB,(BYTE)IntA);


    ValueName = "SpecialStringFGColor";
    IntR = 150,IntG = 255,IntB = 100,IntA = 255;
    AnsiString SpecialStringForegroundColorString = InFile->ReadString(SectionName,ValueName,"{150,255,100,255}");
    sscanf(SpecialStringForegroundColorString.c_str(),"{%d,%d,%d,%d}",&IntR,&IntG,&IntB,&IntA);
    CLAMP(IntR,0,255);CLAMP(IntG,0,255);CLAMP(IntB,0,255);CLAMP(IntA,0,255);
    Matrix->SetSpecialStringColor((BYTE)IntR,(BYTE)IntG,(BYTE)IntB,(BYTE)IntA);

    ValueName = "SpecialStringFadeColor";
    IntR = 0,IntG = 0,IntB = 0,IntA = 0;
    AnsiString SpecialStringFadeColorString = InFile->ReadString(SectionName,ValueName,"{50,85,33,128}");
    sscanf(SpecialStringFadeColorString.c_str(),"{%d,%d,%d,%d}",&IntR,&IntG,&IntB,&IntA);
    CLAMP(IntR,0,255);CLAMP(IntG,0,255);CLAMP(IntB,0,255);CLAMP(IntA,0,255);
    Matrix->SetSpecialStringFadeColor((BYTE)IntR,(BYTE)IntG,(BYTE)IntB,(BYTE)IntA);

    ValueName = "SpecialStringBGColor";
    IntR = 0,IntG = 0,IntB = 0,IntA = 0;
    AnsiString SpecialStringBackgroundColorString = InFile->ReadString(SectionName,ValueName,"{0,0,0,0}");
    sscanf(SpecialStringBackgroundColorString.c_str(),"{%d,%d,%d,%d}",&IntR,&IntG,&IntB,&IntA);
    CLAMP(IntR,0,255);CLAMP(IntG,0,255);CLAMP(IntB,0,255);CLAMP(IntA,0,255);
    Matrix->SetSpecialStringBGColor((BYTE)IntR,(BYTE)IntG,(BYTE)IntB,(BYTE)IntA);


    ValueName = "BGMode";
    TBGMode BGMode;
    AnsiString BGModeString = InFile->ReadString(SectionName,ValueName,"bgmodeBitmap");

    if(BGModeString == "bgmodeColor")
    {
        BGMode = bgmodeColor;
    }
    else
    {
        BGMode = bgmodeBitmap;
    }

    Matrix->SetBGMode(BGMode);

    ValueName = "BlendMode";
    TBlendMode BlendMode;
    AnsiString BlendModeString = InFile->ReadString(SectionName,ValueName,"blendmodeXOR");

    if(BlendModeString == "blendmodeOR")
    {
        BlendMode = blendmodeOR;
    }
    else if(BlendModeString == "blendmodeAND")
    {
        BlendMode = blendmodeAND;
    }
    else
    {
        BlendMode = blendmodeXOR;
    }

    Matrix->SetBlendMode(BlendMode);

    delete InFile;
    return 1;
}
//---------------------------------------------------------------------------
int WriteLogFontToConfigFile(TCustomIniFile *OutFile,const AnsiString SectionName,const LOGFONT &LogFont)
{
    if(OutFile == NULL)
    {
        return 0;
    }

    AnsiString ValueName;

    ValueName = "FontHeight";
    OutFile->WriteInteger(SectionName,ValueName,LogFont.lfHeight);
    ValueName = "FontWidth";
    OutFile->WriteInteger(SectionName,ValueName,LogFont.lfWidth);
    ValueName = "FontEscapement";
    OutFile->WriteInteger(SectionName,ValueName,LogFont.lfEscapement);
    ValueName = "FontOrientation";
    OutFile->WriteInteger(SectionName,ValueName,LogFont.lfOrientation);
    ValueName = "FontWeight";
    OutFile->WriteInteger(SectionName,ValueName,LogFont.lfWeight);
    ValueName = "FontItalic";
    OutFile->WriteBool(SectionName,ValueName,(LogFont.lfItalic == TRUE));
    ValueName = "FontUnderline";
    OutFile->WriteBool(SectionName,ValueName,(LogFont.lfUnderline == TRUE));
    ValueName = "FontStrikeOut";
    OutFile->WriteBool(SectionName,ValueName,(LogFont.lfStrikeOut == TRUE));

    ValueName = "FontCharSet";
    AnsiString CharSetName;
    switch(LogFont.lfCharSet)
    {
    case(ANSI_CHARSET): CharSetName = "ANSI_CHARSET"; break;
    case(BALTIC_CHARSET): CharSetName = "BALTIC_CHARSET"; break;
    case(CHINESEBIG5_CHARSET): CharSetName = "CHINESEBIG5_CHARSET"; break;
    case(EASTEUROPE_CHARSET): CharSetName = "EASTEUROPE_CHARSET"; break;
    case(GB2312_CHARSET): CharSetName = "GB2312_CHARSET"; break;
    case(GREEK_CHARSET): CharSetName = "GREEK_CHARSET"; break;
    case(HANGUL_CHARSET): CharSetName = "HANGUL_CHARSET"; break;
    case(MAC_CHARSET): CharSetName = "MAC_CHARSET"; break;
    case(OEM_CHARSET): CharSetName = "OEM_CHARSET"; break;
    case(RUSSIAN_CHARSET): CharSetName = "RUSSIAN_CHARSET"; break;
    case(SHIFTJIS_CHARSET): CharSetName = "SHIFTJIS_CHARSET"; break;
    case(SYMBOL_CHARSET): CharSetName = "SYMBOL_CHARSET"; break;
    case(TURKISH_CHARSET): CharSetName = "TURKISH_CHARSET"; break;
    case(JOHAB_CHARSET): CharSetName = "JOHAB_CHARSET"; break;
    case(HEBREW_CHARSET): CharSetName = "HEBREW_CHARSET"; break;
    case(ARABIC_CHARSET): CharSetName = "ARABIC_CHARSET"; break;
    case(THAI_CHARSET): CharSetName = "THAI_CHARSET"; break;
    default: CharSetName = "DEFAULT_CHARSET"; break;
    }

    OutFile->WriteString(SectionName,ValueName,CharSetName);

    ValueName = "FontOutPrecision";
    AnsiString OutPrecisionName;
    switch(LogFont.lfOutPrecision)
    {
    case(OUT_DEVICE_PRECIS): OutPrecisionName = "OUT_DEVICE_PRECIS"; break;
    case(OUT_OUTLINE_PRECIS): OutPrecisionName = "OUT_OUTLINE_PRECIS"; break;
    case(OUT_RASTER_PRECIS): OutPrecisionName = "OUT_RASTER_PRECIS"; break;
    case(OUT_STRING_PRECIS): OutPrecisionName = "OUT_STRING_PRECIS"; break;
    case(OUT_STROKE_PRECIS): OutPrecisionName = "OUT_STROKE_PRECIS"; break;
    case(OUT_TT_ONLY_PRECIS): OutPrecisionName = "OUT_TT_ONLY_PRECIS"; break;
    case(OUT_TT_PRECIS): OutPrecisionName = "OUT_TT_PRECIS"; break;
    default: OutPrecisionName = "OUT_DEFAULT_PRECIS"; break;
    }

    OutFile->WriteString(SectionName,ValueName,OutPrecisionName);

    ValueName = "FontClipPrecision";
    AnsiString ClipPrecisionName;
    switch(LogFont.lfClipPrecision)
    {
    case(CLIP_STROKE_PRECIS): ClipPrecisionName = "CLIP_STROKE_PRECIS"; break;
    case(CLIP_EMBEDDED): ClipPrecisionName = "CLIP_EMBEDDED"; break;
    case(CLIP_LH_ANGLES): ClipPrecisionName = "CLIP_LH_ANGLES"; break;
    default: ClipPrecisionName = "CLIP_DEFAULT_PRECIS"; break;
    }

    OutFile->WriteString(SectionName,ValueName,ClipPrecisionName);

    ValueName = "FontQuality";
    AnsiString QualityName;
    switch(LogFont.lfQuality)
    {
    case(DRAFT_QUALITY): QualityName = "DRAFT_QUALITY"; break;
    case(PROOF_QUALITY): QualityName = "PROOF_QUALITY"; break;
    default: QualityName = "DEFAULT_QUALITY"; break;
    }

    OutFile->WriteString(SectionName,ValueName,QualityName);

    ValueName = "FontPitch";
    AnsiString PitchName;

    if(LogFont.lfPitchAndFamily & FIXED_PITCH)
    {
        PitchName = "FIXED_PITCH";
    }
    else if(LogFont.lfPitchAndFamily & VARIABLE_PITCH)
    {
        PitchName = "VARIABLE_PITCH";
    }
    else
    {
        PitchName = "DEFAULT_PITCH";
    }

    OutFile->WriteString(SectionName,ValueName,PitchName);

    ValueName = "FontFamily";
    AnsiString FamilyName;

    if(LogFont.lfPitchAndFamily & FF_DECORATIVE)
    {
        FamilyName = "FF_DECORATIVE";
    }
    else if(LogFont.lfPitchAndFamily & FF_MODERN)
    {
        FamilyName = "FF_MODERN";
    }
    else if(LogFont.lfPitchAndFamily & FF_ROMAN)
    {
        FamilyName = "FF_ROMAN";
    }
    else if(LogFont.lfPitchAndFamily & FF_SCRIPT)
    {
        FamilyName = "FF_SCRIPT";
    }
    else if(LogFont.lfPitchAndFamily & FF_SWISS)
    {
        FamilyName = "FF_SWISS";
    }
    else
    {
        FamilyName = "FF_DONTCARE";
    }

    OutFile->WriteString(SectionName,ValueName,FamilyName);

    ValueName = "FontName";
    OutFile->WriteString(SectionName,ValueName,AnsiString(LogFont.lfFaceName));

    return 1;
}
//---------------------------------------------------------------------------
int ReadLogFontFromConfigFile(TCustomIniFile *InFile,const AnsiString SectionName,LOGFONT &LogFont)
{
    if(InFile == NULL)
    {
        return 0;
    }

    AnsiString ValueName;

    LogFont.lfHeight = 12;
    LogFont.lfWidth = 0;
    LogFont.lfEscapement = 0;
    LogFont.lfOrientation = 0;
    LogFont.lfWeight = 400;
    LogFont.lfItalic = FALSE;
    LogFont.lfUnderline = FALSE;
    LogFont.lfStrikeOut = FALSE;

    ValueName = "FontHeight";
    LogFont.lfHeight = InFile->ReadInteger(SectionName,ValueName,LogFont.lfHeight);
    ValueName = "FontWidth";
    LogFont.lfWidth = InFile->ReadInteger(SectionName,ValueName,LogFont.lfWidth);
    ValueName = "FontEscapement";
    LogFont.lfEscapement = InFile->ReadInteger(SectionName,ValueName,LogFont.lfEscapement);
    ValueName = "FontOrientation";
    LogFont.lfOrientation = InFile->ReadInteger(SectionName,ValueName,LogFont.lfOrientation);
    ValueName = "FontWeight";
    LogFont.lfWeight = InFile->ReadInteger(SectionName,ValueName,LogFont.lfWeight);
    ValueName = "FontItalic";
    LogFont.lfItalic = InFile->ReadBool(SectionName,ValueName,(LogFont.lfItalic == TRUE));
    ValueName = "FontUnderline";
    LogFont.lfUnderline = InFile->ReadBool(SectionName,ValueName,(LogFont.lfUnderline == TRUE));
    ValueName = "FontStrikeOut";
    LogFont.lfStrikeOut = InFile->ReadBool(SectionName,ValueName,(LogFont.lfStrikeOut == TRUE));

    ValueName = "FontCharSet";
    AnsiString CharSetName = InFile->ReadString(SectionName,ValueName,"DEFAULT_CHARSET");
    if(CharSetName == "ANSI_CHARSET")
    {
        LogFont.lfCharSet = ANSI_CHARSET;
    }
    else if(CharSetName == "BALTIC_CHARSET")
    {
        LogFont.lfCharSet = BALTIC_CHARSET;
    }
    else if(CharSetName == "CHINESEBIG5_CHARSET")
    {
        LogFont.lfCharSet = CHINESEBIG5_CHARSET;
    }
    else if(CharSetName == "EASTEUROPE_CHARSET")
    {
        LogFont.lfCharSet = EASTEUROPE_CHARSET;
    }
    else if(CharSetName == "GB2312_CHARSET")
    {
        LogFont.lfCharSet = GB2312_CHARSET;
    }
    else if(CharSetName == "GREEK_CHARSET")
    {
        LogFont.lfCharSet = GREEK_CHARSET;
    }
    else if(CharSetName == "HANGUL_CHARSET")
    {
        LogFont.lfCharSet = HANGUL_CHARSET;
    }
    else if(CharSetName == "MAC_CHARSET")
    {
        LogFont.lfCharSet = MAC_CHARSET;
    }
    else if(CharSetName == "OEM_CHARSET")
    {
        LogFont.lfCharSet = OEM_CHARSET;
    }
    else if(CharSetName == "RUSSIAN_CHARSET")
    {
        LogFont.lfCharSet = RUSSIAN_CHARSET;
    }
    else if(CharSetName == "SHIFTJIS_CHARSET")
    {
        LogFont.lfCharSet = SHIFTJIS_CHARSET;
    }
    else if(CharSetName == "SYMBOL_CHARSET")
    {
        LogFont.lfCharSet = SYMBOL_CHARSET;
    }
    else if(CharSetName == "TURKISH_CHARSET")
    {
        LogFont.lfCharSet = TURKISH_CHARSET;
    }
    else if(CharSetName == "JOHAB_CHARSET")
    {
        LogFont.lfCharSet = JOHAB_CHARSET;
    }
    else if(CharSetName == "HEBREW_CHARSET")
    {
        LogFont.lfCharSet = HEBREW_CHARSET;
    }
    else if(CharSetName == "ARABIC_CHARSET")
    {
        LogFont.lfCharSet = ARABIC_CHARSET;
    }
    else if(CharSetName == "THAI_CHARSET")
    {
        LogFont.lfCharSet = THAI_CHARSET;
    }
    else
    {
        LogFont.lfCharSet = DEFAULT_CHARSET;
    }

    ValueName = "FontOutPrecision";
    AnsiString OutPrecisionName = InFile->ReadString(SectionName,ValueName,"OUT_DEFAULT_PRECIS");
    if(OutPrecisionName == "OUT_DEVICE_PRECIS")
    {
        LogFont.lfOutPrecision = OUT_DEVICE_PRECIS;
    }
    else if(OutPrecisionName == "OUT_OUTLINE_PRECIS")
    {
        LogFont.lfOutPrecision = OUT_OUTLINE_PRECIS;
    }
    else if(OutPrecisionName == "OUT_RASTER_PRECIS")
    {
        LogFont.lfOutPrecision = OUT_RASTER_PRECIS;
    }
    else if(OutPrecisionName == "OUT_STRING_PRECIS")
    {
        LogFont.lfOutPrecision = OUT_STRING_PRECIS;
    }
    else if(OutPrecisionName == "OUT_STROKE_PRECIS")
    {
        LogFont.lfOutPrecision = OUT_STROKE_PRECIS;
    }
    else if(OutPrecisionName == "OUT_TT_ONLY_PRECIS")
    {
        LogFont.lfOutPrecision = OUT_TT_ONLY_PRECIS;
    }
    else if(OutPrecisionName == "OUT_TT_PRECIS")
    {
        LogFont.lfOutPrecision = OUT_TT_PRECIS;
    }
    else
    {
        LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
    }
    

    ValueName = "FontClipPrecision";
    AnsiString ClipPrecisionName = InFile->ReadString(SectionName,ValueName,"CLIP_DEFAULT_PRECIS");
    if(ClipPrecisionName == "CLIP_STROKE_PRECIS")
    {
        LogFont.lfClipPrecision = CLIP_STROKE_PRECIS;
    }
    else if(ClipPrecisionName == "CLIP_EMBEDDED")
    {
        LogFont.lfClipPrecision = CLIP_EMBEDDED;
    }
    else if(ClipPrecisionName == "CLIP_LH_ANGLES")
    {
        LogFont.lfClipPrecision = CLIP_LH_ANGLES;
    }
    else
    {
        LogFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    }

    ValueName = "FontQuality";
    AnsiString QualityName = InFile->ReadString(SectionName,ValueName,"DEFAULT_QUALITY");
    if(QualityName == "DRAFT_QUALITY")
    {
        LogFont.lfQuality = DRAFT_QUALITY;
    }
    else if(QualityName == "PROOF_QUALITY")
    {
        LogFont.lfQuality = PROOF_QUALITY;
    }
    else
    {
        LogFont.lfQuality = DEFAULT_QUALITY;
    }

    ValueName = "FontPitch";
    AnsiString PitchName = InFile->ReadString(SectionName,ValueName,"DEFAULT_PITCH");

    if(PitchName == "FIXED_PITCH")
    {
        LogFont.lfPitchAndFamily = FIXED_PITCH;
    }
    else if (PitchName == "VARIABLE_PITCH")
    {
        LogFont.lfPitchAndFamily = VARIABLE_PITCH;
    }
    else
    {
        LogFont.lfPitchAndFamily = DEFAULT_PITCH;
    }

    ValueName = "FontFamily";
    AnsiString FamilyName = InFile->ReadString(SectionName,ValueName,"FF_DONTCARE");

    if(FamilyName == "FF_DECORATIVE")
    {
        LogFont.lfPitchAndFamily |= FF_DECORATIVE;
    }
    else if(FamilyName == "FF_MODERN")
    {
        LogFont.lfPitchAndFamily |= FF_MODERN;
    }
    else if(FamilyName == "FF_ROMAN")
    {
        LogFont.lfPitchAndFamily |= FF_ROMAN;
    }
    else if(FamilyName == "FF_SCRIPT")
    {
        LogFont.lfPitchAndFamily |= FF_SCRIPT;
    }
    else if(FamilyName == "FF_SWISS")
    {
        LogFont.lfPitchAndFamily |= FF_SWISS;
    }
    else
    {
        LogFont.lfPitchAndFamily |= FF_DONTCARE;
    }

    ValueName = "FontName";
    AnsiString FontNameString = InFile->ReadString(SectionName,ValueName,"Terminal");
    #ifdef _UNICODE
    snwprintf(LogFont.lfFaceName,LF_FACESIZE - 1,_TEXT("%S"),FontNameString.c_str());
    #else
    snprintf(LogFont.lfFaceName,LF_FACESIZE - 1,"%s",FontNameString.c_str());
    #endif
    LogFont.lfFaceName[LF_FACESIZE - 1] = (_TCHAR)'\0';

    return 1;
}
//---------------------------------------------------------------------------
int ConvertCharSetToAnsiString(const _TCHAR *CharSet,const unsigned int &CharSetSize,AnsiString &OutString)
{
    OutString.SetLength(0);
    static char HexBuff[16];
    unsigned int i;
    bool NeedsTrailingCleanup = false;
    for(i = 0; i < CharSetSize; i++)
    {
        if((CharSet[i] == '\t') || (CharSet[i] == ' ') || (CharSet[i] == '\r') ||
            (CharSet[i] == '\f') || (CharSet[i] == '\n') || (CharSet[i] == '\0')
            )
        {
        }
        else if(CharSet[i] == ',')
        {
            OutString+= AnsiString("\\,, ");
            NeedsTrailingCleanup = true;
        }
        else if(CharSet[i] == '\\')
        {
            OutString+= AnsiString("\\\\, ");
            NeedsTrailingCleanup = true;
        }
        else if(CharSet[i] < 256)
        {
            if(_istprint(CharSet[i]))
            {
                OutString+= AnsiString((char)CharSet[i]) + AnsiString(", ");
                NeedsTrailingCleanup = true;
            }
        }
        else
        {
            if(_istprint(CharSet[i]))
            {
                sprintf(HexBuff,"\\0x%X, ",(unsigned int)CharSet[i]);
                OutString+= AnsiString(HexBuff);
                NeedsTrailingCleanup = true;
            }
        }
    }

    if(NeedsTrailingCleanup)
    {
        int DeleteStartIndex = OutString.Length() + 1;
        while((DeleteStartIndex > 1) &&
        ((OutString[DeleteStartIndex - 1] == ' ') || (OutString[DeleteStartIndex - 1] == ',')))
        {
            DeleteStartIndex--;
        }
        OutString.Delete(DeleteStartIndex,OutString.Length());
    }

    return 1;
}
//---------------------------------------------------------------------------
int ConvertCharSetToWideString(const _TCHAR *CharSet,const unsigned int &CharSetSize,WideString &OutString)
{
    OutString.SetLength(0);
    unsigned int i;
    bool NeedsTrailingCleanup = false;
    for(i = 0; i < CharSetSize; i++)
    {
        if((CharSet[i] == '\t') || (CharSet[i] == ' ') || (CharSet[i] == '\r') ||
            (CharSet[i] == '\f') || (CharSet[i] == '\n') || (CharSet[i] == '\0')
            )
        {
        }
        else if(CharSet[i] == ',')
        {
            OutString+= WideString("\\,, ");
            NeedsTrailingCleanup = true;
        }
        else if(CharSet[i] == '\\')
        {
            OutString+= WideString("\\\\, ");
            NeedsTrailingCleanup = true;
        }
        else
        {
            if(_istprint(CharSet[i]))
            {
                OutString+= WideString((wchar_t)CharSet[i]) + WideString(", ");
                NeedsTrailingCleanup = true;
            }
        }
    }

    if(NeedsTrailingCleanup)
    {
        int DeleteStartIndex = OutString.Length() + 1;
        while((DeleteStartIndex > 1) &&
        ((OutString[DeleteStartIndex - 1] == ' ') || (OutString[DeleteStartIndex - 1] == ',')))
        {
            DeleteStartIndex--;
        }
        OutString.Delete(DeleteStartIndex,OutString.Length());
    }

    return 1;
}
//---------------------------------------------------------------------------
int  ConvertAnsiStringToCharSet(std::vector<_TCHAR> &CharSet,const AnsiString &InString)
{
    int i = 0;
    int j;
    char TempBuff[16];
    char TempChar;
    int TextLength = InString.Length();
    int TempInt;
    CharSet.clear();
    char *MemoCString = InString.c_str();

    if(MemoCString == NULL)
    {
        return 1;
    }

    while(MemoCString[i] != '\0')
    {
        TempChar = MemoCString[i];
        if((TempChar == ' ') || (TempChar == '\t') || (TempChar == '\n') ||
           (TempChar == ',') || (TempChar == '\r') || (TempChar == '\f') )
        {
            i++;
        }
        else if(TempChar == '\\')
        {
            //if at least 3 more chars after the backslash, and the two immediately following are 0 and x (any case)
            if((TextLength > (i + 3)) && (MemoCString[i+1] == '0') &&
                ((MemoCString[i+2] == 'x') || (MemoCString[i+2] == 'X'))
               )
            {
                //officially over \0x , so we can update i
                i+=3;

                j = 0;
                while((j < 4) && (MemoCString[i+j] != ' ') &&
                                 (MemoCString[i+j] != '\t') &&
                                 (MemoCString[i+j] != '\r') &&
                                 (MemoCString[i+j] != '\f') &&
                                 (MemoCString[i+j] != '\n') &&
                                 (MemoCString[i+j] != ',') &&
                      ( ((MemoCString[i+j] >= '0') && (MemoCString[i+j] <= '9')) ||
                        ((MemoCString[i+j] >= 'A') && (MemoCString[i+j] <= 'F')) ||
                        ((MemoCString[i+j] >= 'a') && (MemoCString[i+j] <= 'f'))
                      )
                      )
                {
                    TempBuff[j] = MemoCString[i+j];
                    j++;
                }
                TempBuff[j] = '\0';

                i+= j;

                if(sscanf(TempBuff,"%x",&TempInt))
                {
                    if(_istprint((_TCHAR)TempInt))
                    {
                        CharSet.push_back((_TCHAR)TempInt);
                    }
                }

            }
            //if at least 1 more char after the backslash, and the immediately following is backslash
            else if((TextLength > (i + 1)) && (MemoCString[i+1] == '\\'))
            {
                CharSet.push_back(_TEXT('\\'));
                i+=2;
            }
            //if at least 1 more char after the backslash, and the immediately following is comma
            else if((TextLength > (i + 1)) && (MemoCString[i+1] == ','))
            {
                CharSet.push_back(_TEXT(','));
                i+=2;
            }
            else
            {
                i++;
            }
        }
        else
        {
            if(_istprint((_TCHAR)TempChar))
            {
                CharSet.push_back((_TCHAR)TempChar);
            }
            i++;
        }
    }

    return CharSet.size();
}
//---------------------------------------------------------------------------
int  ConvertWideStringToCharSet(std::vector<_TCHAR> &CharSet,const WideString &InString)
{
    int i = 0;
    int j;
    wchar_t TempBuff[16];
    wchar_t TempChar;
    int TextLength = InString.Length();
    int TempInt;
    CharSet.clear();
    BSTR MemoCString = InString.c_bstr();

    if(MemoCString == NULL)
    {
        return 1;
    }

    while(MemoCString[i] != '\0')
    {
        TempChar = MemoCString[i];
        if((TempChar == ' ') || (TempChar == '\t') || (TempChar == '\n') ||
           (TempChar == ',') || (TempChar == '\r') || (TempChar == '\f') )
        {
            i++;
        }
        else if(TempChar == '\\')
        {
            //if at least 3 more chars after the backslash, and the two immediately following are 0 and x (any case)
            if((TextLength > (i + 3)) && (MemoCString[i+1] == '0') &&
                ((MemoCString[i+2] == 'x') || (MemoCString[i+2] == 'X'))
               )
            {
                //officially over \0x , so we can update i
                i+=3;

                j = 0;
                while((j < 4) && (MemoCString[i+j] != ' ') &&
                                 (MemoCString[i+j] != '\t') &&
                                 (MemoCString[i+j] != '\r') &&
                                 (MemoCString[i+j] != '\f') &&
                                 (MemoCString[i+j] != '\n') &&
                                 (MemoCString[i+j] != ',') &&
                      ( ((MemoCString[i+j] >= '0') && (MemoCString[i+j] <= '9')) ||
                        ((MemoCString[i+j] >= 'A') && (MemoCString[i+j] <= 'F')) ||
                        ((MemoCString[i+j] >= 'a') && (MemoCString[i+j] <= 'f'))
                      )
                      )
                {
                    TempBuff[j] = MemoCString[i+j];
                    j++;
                }
                TempBuff[j] = '\0';

                i+= j;

                if(swscanf(TempBuff,L"%x",&TempInt))
                {
                    if(_istprint((_TCHAR)TempInt))
                    {
                        CharSet.push_back((_TCHAR)TempInt);
                    }
                }

            }
            //if at least 1 more char after the backslash, and the immediately following is backslash
            else if((TextLength > (i + 1)) && (MemoCString[i+1] == '\\'))
            {
                CharSet.push_back(_TEXT('\\'));
                i+=2;
            }
            //if at least 1 more char after the backslash, and the immediately following is comma
            else if((TextLength > (i + 1)) && (MemoCString[i+1] == ','))
            {
                CharSet.push_back(_TEXT(','));
                i+=2;
            }
            else
            {
                i++;
            }
        }
        else
        {
            if(_istprint((_TCHAR)TempChar))
            {
                CharSet.push_back((_TCHAR)TempChar);
            }
            i++;
        }
    }

    return CharSet.size();
}
//---------------------------------------------------------------------------
int ConvertSpecialStringsToAnsiString(const std::vector<tstring> &SpecialStrings,AnsiString &OutString, _TCHAR Delimiter)
{
    OutString.SetLength(0);
    static char HexBuff[16];
    unsigned int i, j;
    AnsiString DelimiterReplacement;

    if(Delimiter == '\n')
    {
        DelimiterReplacement = "\\n";
    }
    else
    {
        DelimiterReplacement = "\\";
        DelimiterReplacement += (char)Delimiter;
    }

    for(i = 0; i < SpecialStrings.size(); i++)
    {
        for(j = 0; j < SpecialStrings[i].length(); j++)
        {
            _TCHAR CurrentChar = SpecialStrings[i][j];

            if((CurrentChar == '\0')
                )
            {
            }
            else if(CurrentChar == Delimiter)
            {
                OutString+= DelimiterReplacement;
            }
            else if(CurrentChar == '\\')
            {
                OutString+= AnsiString("\\\\");
            }
            else if(CurrentChar < 256)
            {
                if(_istprint(CurrentChar))
                {
                    OutString+= AnsiString((char)CurrentChar);
                }
            }
            else
            {
                if(_istprint(CurrentChar))
                {
                    sprintf(HexBuff,"\\0x%X",(unsigned int)CurrentChar);
                    OutString+= AnsiString(HexBuff);
                }
            }
        }

        if(i < (SpecialStrings.size() - 1))
        {
            OutString+= (char)Delimiter;
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
int ConvertSpecialStringsToWideString(const std::vector<tstring> &SpecialStrings,WideString &OutString, _TCHAR Delimiter)
{
    OutString.SetLength(0);
    unsigned int i, j;
    AnsiString DelimiterReplacement;

    if(Delimiter == '\n')
    {
        DelimiterReplacement = "\\n";
    }
    else
    {
        DelimiterReplacement = "\\";
        DelimiterReplacement += Delimiter;
    }

    for(i = 0; i < SpecialStrings.size(); i++)
    {
        for(j = 0; j < SpecialStrings[i].length(); j++)
        {
            _TCHAR CurrentChar = SpecialStrings[i][j];

            if((CurrentChar == '\0')
                )
            {
            }
            else if(CurrentChar == Delimiter)
            {
                OutString+= DelimiterReplacement;
            }
            else if(CurrentChar == '\\')
            {
                OutString+= AnsiString("\\\\");
            }
            else
            {
                if(_istprint(CurrentChar))
                {
                    OutString+= AnsiString((char)CurrentChar);
                }
            }
        }

        if(i < (SpecialStrings.size() - 1))
        {
            OutString+= WideString(Delimiter);
        }
    }

    return 1;
}
//---------------------------------------------------------------------------
int  ConvertAnsiStringToSpecialStrings(std::vector<tstring> &SpecialStrings,const AnsiString &InString, _TCHAR Delimiter)
{
    int i = 0;
    int j;
    char TempBuff[16];
    char TempChar;
    int TextLength = InString.Length();
    int TempInt;
    SpecialStrings.clear();
    tstring CurrentString;
    char *MemoCString = InString.c_str();

    if(MemoCString == NULL)
    {
        return 1;
    }

    while(MemoCString[i] != '\0')
    {
        TempChar = MemoCString[i];

        if(TempChar == Delimiter)
        {
            if(CurrentString.length() > 0)
            {
                SpecialStrings.push_back(CurrentString);
                CurrentString.resize(0);
            }
            i++;
        }
        else if(TempChar == '\\')
        {
            if((TextLength > (i + 1)) && (MemoCString[i+1] == Delimiter))
            {
                CurrentString.append(1,(_TCHAR)MemoCString[i+1]);
                i+=2;
            }
            //if at least 3 more chars after the backslash, and the two immediately following are 0 and x (any case)
            else
            if((TextLength > (i + 3)) && (MemoCString[i+1] == '0') &&
                ((MemoCString[i+2] == 'x') || (MemoCString[i+2] == 'X'))
               )
            {
                //officially over \0x , so we can update i
                i+=3;

                j = 0;
                while((j < 4) && (MemoCString[i+j] != Delimiter) &&
                      ( ((MemoCString[i+j] >= '0') && (MemoCString[i+j] <= '9')) ||
                        ((MemoCString[i+j] >= 'A') && (MemoCString[i+j] <= 'F')) ||
                        ((MemoCString[i+j] >= 'a') && (MemoCString[i+j] <= 'f'))
                      )
                      )
                {
                    TempBuff[j] = MemoCString[i+j];
                    j++;
                }
                TempBuff[j] = '\0';

                i+= j;

                if(sscanf(TempBuff,"%x",&TempInt))
                {
                    if(((_TCHAR)TempInt) == Delimiter)
                    {
                        if(CurrentString.length() > 0)
                        {
                            SpecialStrings.push_back(CurrentString);
                            CurrentString.resize(0);
                        }
                    }
                    else if(_istprint((_TCHAR)TempInt))
                    {
                        CurrentString.append(1,(_TCHAR)TempInt);
                    }
                }

            }
            else
            {
                i++;
            }
        }
        else
        {
            if(_istprint((_TCHAR)TempChar))
            {
                CurrentString.append(1,(_TCHAR)TempChar);
            }
            i++;
        }
    }

    if(CurrentString.length() > 0)
    {
        SpecialStrings.push_back(CurrentString);
        CurrentString.resize(0);
    }

    return 1;
}
//---------------------------------------------------------------------------
int  ConvertWideStringToSpecialStrings(std::vector<tstring> &SpecialStrings,const WideString &InString, _TCHAR Delimiter)
{
    int i = 0;
    int j;
    wchar_t TempBuff[16];
    wchar_t TempChar;
    int TextLength = InString.Length();
    int TempInt;
    SpecialStrings.clear();
    tstring CurrentString;
    BSTR MemoCString = InString.c_bstr();

    if(MemoCString == NULL)
    {
        return 1;
    }

    while(MemoCString[i] != '\0')
    {
        TempChar = MemoCString[i];

        if(TempChar == Delimiter)
        {
            if(CurrentString.length() > 0)
            {
                SpecialStrings.push_back(CurrentString);
                CurrentString.resize(0);
            }
            i++;
        }
        else if(TempChar == '\\')
        {
            if((TextLength > (i + 1)) && (MemoCString[i+1] == Delimiter))
            {
                CurrentString.append(1,(_TCHAR)MemoCString[i+1]);
                i+=2;
            }
            //if at least 3 more chars after the backslash, and the two immediately following are 0 and x (any case)
            else
            if((TextLength > (i + 3)) && (MemoCString[i+1] == '0') &&
                ((MemoCString[i+2] == 'x') || (MemoCString[i+2] == 'X'))
               )
            {
                //officially over \0x , so we can update i
                i+=3;

                j = 0;
                while((j < 4) && (MemoCString[i+j] != Delimiter) &&
                      ( ((MemoCString[i+j] >= '0') && (MemoCString[i+j] <= '9')) ||
                        ((MemoCString[i+j] >= 'A') && (MemoCString[i+j] <= 'F')) ||
                        ((MemoCString[i+j] >= 'a') && (MemoCString[i+j] <= 'f'))
                      )
                      )
                {
                    TempBuff[j] = MemoCString[i+j];
                    j++;
                }
                TempBuff[j] = '\0';

                i+= j;

                if(swscanf(TempBuff,L"%x",&TempInt))
                {
                    if(((_TCHAR)TempInt) == Delimiter)
                    {
                        if(CurrentString.length() > 0)
                        {
                            SpecialStrings.push_back(CurrentString);
                            CurrentString.resize(0);
                        }
                    }
                    else if(_istprint((_TCHAR)TempInt))
                    {
                        CurrentString.append(1,(_TCHAR)TempInt);
                    }
                }

            }
            else
            {
                i++;
            }
        }
        else
        {
            if(_istprint((_TCHAR)TempChar))
            {
                CurrentString.append(1,(_TCHAR)TempChar);
            }
            i++;
        }
    }

    if(CurrentString.length() > 0)
    {
        SpecialStrings.push_back(CurrentString);
        CurrentString.resize(0);
    }

    return 1;
}
//---------------------------------------------------------------------------
void TConfigurationForm::SetFormMaxStream(int NewFormMaxStream)
{
    CLAMP(NewFormMaxStream,this->MaxStreamTrackBar->Min,this->MaxStreamTrackBar->Max);

    this->MaxStreamTrackBar->Position = NewFormMaxStream;
    this->MaxStreamWidgetUpDown->Position = NewFormMaxStream;
}
//---------------------------------------------------------------------------
void TConfigurationForm::SetFormSpeedVariance(int NewFormSpeedVariance)
{
    CLAMP(NewFormSpeedVariance,this->SpeedVarianceTrackBar->Min,this->SpeedVarianceTrackBar->Max);

    this->SpeedVarianceTrackBar->Position = NewFormSpeedVariance;
    this->SpeedVarianceWidgetUpDown->Position = NewFormSpeedVariance;
}
//---------------------------------------------------------------------------
void TConfigurationForm::SetFormMonotonousCleanupEnabled(bool Enabled)
{
    this->MonotonousCleanupEnabledCheckBox->Checked = Enabled;
    if(Enabled)
    {
        this->EnableBackTraceConfig();
    }
    else
    {
        this->DisableBackTraceConfig();
    }
}
//---------------------------------------------------------------------------
void TConfigurationForm::SetFormBackTrace(int NewFormBackTrace)
{
    CLAMP(NewFormBackTrace,this->BackTraceTrackBar->Min,this->BackTraceTrackBar->Max);

    this->BackTraceTrackBar->Position = NewFormBackTrace;
    this->BackTraceWidgetUpDown->Position = NewFormBackTrace;
}
//---------------------------------------------------------------------------
void TConfigurationForm::SetFormRandomizedCleanupEnabled(bool Enabled)
{
    this->RandomizedCleanupEnabledCheckBox->Checked = Enabled;
    if(Enabled)
    {
        this->EnableLeadingConfig();
        this->EnableSpacePadConfig();
    }
    else
    {
        this->DisableLeadingConfig();
        this->DisableSpacePadConfig();
    }
}
//---------------------------------------------------------------------------
void TConfigurationForm::SetFormLeading(int NewFormLeading)
{
    CLAMP(NewFormLeading,this->LeadingTrackBar->Min,this->LeadingTrackBar->Max);

    this->LeadingTrackBar->Position = NewFormLeading;
    this->LeadingWidgetUpDown->Position = NewFormLeading;
}
//---------------------------------------------------------------------------
void TConfigurationForm::SetFormSpacePad(int NewFormSpacePad)
{
    CLAMP(NewFormSpacePad,this->SpacePadTrackBar->Min,this->SpacePadTrackBar->Max);

    this->SpacePadTrackBar->Position = NewFormSpacePad;
    this->SpacePadWidgetUpDown->Position = NewFormSpacePad;
}
//---------------------------------------------------------------------------
void TConfigurationForm::SetFormFont(HFONT hFont)
{
    if(hFont != NULL)
    {
        int TempSize;

        TFont *TargetFont = this->FontDialog->Font;
        LOGFONT NewLogFont;
        GetObject(hFont,sizeof(LOGFONT),&NewLogFont);

        TargetFont->Name = NewLogFont.lfFaceName;
        TargetFont->Charset = NewLogFont.lfCharSet;

        TempSize = -NewLogFont.lfHeight * 72 / TargetFont->PixelsPerInch;
        if(TempSize)
            TargetFont->Size = TempSize;
        else
            TargetFont->Size = 1;


        if(NewLogFont.lfPitchAndFamily & FIXED_PITCH)
        {
            TargetFont->Pitch = fpFixed;
        }
        else if(NewLogFont.lfPitchAndFamily & VARIABLE_PITCH)
        {
            TargetFont->Pitch = fpVariable;
        }
        else
        {
            TargetFont->Pitch = fpDefault;
        }

        if(NewLogFont.lfWeight >= FW_BOLD) TargetFont->Style = TargetFont->Style << fsBold;
        else TargetFont->Style = TargetFont->Style >> fsBold;

        if(NewLogFont.lfItalic) TargetFont->Style = TargetFont->Style << fsItalic;
        else TargetFont->Style = TargetFont->Style >> fsItalic;

        if(NewLogFont.lfUnderline) TargetFont->Style = TargetFont->Style << fsUnderline;
        else TargetFont->Style = TargetFont->Style >> fsUnderline;

        if(NewLogFont.lfStrikeOut) TargetFont->Style = TargetFont->Style << fsStrikeOut;
        else TargetFont->Style = TargetFont->Style >> fsStrikeOut;

        this->FontNameWidget->Text = NewLogFont.lfFaceName;
    }
}
//---------------------------------------------------------------------------
void TConfigurationForm::SetFormSpecialFont(HFONT hFont)
{
    if(hFont != NULL)
    {
        int TempSize;

        TFont *TargetFont = this->SpecialFontDialog->Font;
        LOGFONT NewLogFont;
        GetObject(hFont,sizeof(LOGFONT),&NewLogFont);

        TargetFont->Name = NewLogFont.lfFaceName;
        TargetFont->Charset = NewLogFont.lfCharSet;

        TempSize = -NewLogFont.lfHeight * 72 / TargetFont->PixelsPerInch;
        if(TempSize)
            TargetFont->Size = TempSize;
        else
            TargetFont->Size = 1;


        if(NewLogFont.lfPitchAndFamily & FIXED_PITCH)
        {
            TargetFont->Pitch = fpFixed;
        }
        else if(NewLogFont.lfPitchAndFamily & VARIABLE_PITCH)
        {
            TargetFont->Pitch = fpVariable;
        }
        else
        {
            TargetFont->Pitch = fpDefault;
        }

        if(NewLogFont.lfWeight >= FW_BOLD) TargetFont->Style = TargetFont->Style << fsBold;
        else TargetFont->Style = TargetFont->Style >> fsBold;

        if(NewLogFont.lfItalic) TargetFont->Style = TargetFont->Style << fsItalic;
        else TargetFont->Style = TargetFont->Style >> fsItalic;

        if(NewLogFont.lfUnderline) TargetFont->Style = TargetFont->Style << fsUnderline;
        else TargetFont->Style = TargetFont->Style >> fsUnderline;

        if(NewLogFont.lfStrikeOut) TargetFont->Style = TargetFont->Style << fsStrikeOut;
        else TargetFont->Style = TargetFont->Style >> fsStrikeOut;

        this->SpecialFontNameWidget->Text = NewLogFont.lfFaceName;
    }
}
//---------------------------------------------------------------------------
void TConfigurationForm::SetFormForegroundColor(BYTE R,BYTE G,BYTE B,BYTE A)
{
    TColor NewCol = (TColor)RGB(R,G,B);
    this->LeadColorPanel->Font->Color = (TColor)NewCol;
    this->ForegroundColorDialog->Color = (TColor)NewCol;
}
//---------------------------------------------------------------------------
void TConfigurationForm::SetFormFadeColor(BYTE R,BYTE G,BYTE B,BYTE A)
{
    TColor NewFadeCol = (TColor)RGB(R,G,B);
    this->TrailColorPanel->Font->Color = (TColor)NewFadeCol;
    this->FadeColorDialog->Color = (TColor)NewFadeCol;
}
//---------------------------------------------------------------------------
void TConfigurationForm::SetFormBackgroundColor(BYTE R,BYTE G,BYTE B,BYTE A)
{
    TColor NewBGCol = (TColor)RGB(R,G,B);
    this->LeadColorPanel->Color = (TColor)NewBGCol;
    this->TrailColorPanel->Color = (TColor)NewBGCol;
    this->SpecialLeadColorPanel->Color = (TColor)NewBGCol;
    this->SpecialTrailColorPanel->Color = (TColor)NewBGCol;
    this->BackgroundColorDialog->Color = (TColor)NewBGCol;

    if(A < 128)
    {
        this->TransparentBGRadioButton->Checked = true;
    }
    else
    {
        this->OpaqueBGRadioButton->Checked = true;
    }
}
//---------------------------------------------------------------------------
void TConfigurationForm::SetFormSpecialForegroundColor(BYTE R,BYTE G,BYTE B,BYTE A)
{
    TColor NewCol = (TColor)RGB(R,G,B);
    this->SpecialLeadColorPanel->Font->Color = (TColor)NewCol;
    this->SpecialForegroundColorDialog->Color = (TColor)NewCol;
}
//---------------------------------------------------------------------------
void TConfigurationForm::SetFormSpecialFadeColor(BYTE R,BYTE G,BYTE B,BYTE A)
{
    TColor NewFadeCol = (TColor)RGB(R,G,B);
    this->SpecialTrailColorPanel->Font->Color = (TColor)NewFadeCol;
    this->SpecialFadeColorDialog->Color = (TColor)NewFadeCol;
}
//---------------------------------------------------------------------------
void TConfigurationForm::SetFormBGMode(TBGMode NewBGMode)
{
    switch(NewBGMode)
    {
    case(bgmodeBitmap):
    {
        this->BitmapBGModeRadioButton->Checked = true;

        this->EnableBlendingConfig();
    }
    break;
    case(bgmodeColor):
    {
        this->ColorBGModeRadioButton->Checked = true;

        this->DisableBlendingConfig();
    }
    break;
    }
}
//---------------------------------------------------------------------------
void TConfigurationForm::SetFormBlendingMode(TBlendMode NewBlendMode)
{
    switch(NewBlendMode)
    {
    case(blendmodeXOR):
    {
        this->XORBGBlendModeRadioButton->Checked = true;
    }
    break;
    case(blendmodeOR):
    {
        this->ORBGBlendModeRadioButton->Checked = true;
    }
    break;
    case(blendmodeAND):
    {
        this->ANDBGBlendModeRadioButton->Checked = true;
    }
    break;
    }
}
//---------------------------------------------------------------------------
void TConfigurationForm::DisableLeadingConfig(void)
{
    this->LeadingGroupBox->Enabled = false;
    this->LeadingTrackBar->Enabled = false;
    this->LeadingWidget->Enabled = false;
    this->LeadingWidgetUpDown->Enabled = false;
}
//---------------------------------------------------------------------------
void TConfigurationForm::EnableLeadingConfig(void)
{
    this->LeadingGroupBox->Enabled = true;
    this->LeadingTrackBar->Enabled = true;
    this->LeadingWidget->Enabled = true;
    this->LeadingWidgetUpDown->Enabled = true;
}
//---------------------------------------------------------------------------
void TConfigurationForm::DisableBackTraceConfig(void)
{
    this->BackTraceGroupBox->Enabled = false;
    this->BackTraceTrackBar->Enabled = false;
    this->BackTraceWidget->Enabled = false;
    this->BackTraceWidgetUpDown->Enabled = false;
}
//---------------------------------------------------------------------------
void TConfigurationForm::EnableBackTraceConfig(void)
{
    this->BackTraceGroupBox->Enabled = true;
    this->BackTraceTrackBar->Enabled = true;
    this->BackTraceWidget->Enabled = true;
    this->BackTraceWidgetUpDown->Enabled = true;
}
//---------------------------------------------------------------------------
void TConfigurationForm::DisableSpacePadConfig(void)
{
    this->SpacePadGroupBox->Enabled = false;
    this->SpacePadTrackBar->Enabled = false;
    this->SpacePadWidget->Enabled = false;
    this->SpacePadWidgetUpDown->Enabled = false;
}
//---------------------------------------------------------------------------
void TConfigurationForm::EnableSpacePadConfig(void)
{
    this->SpacePadGroupBox->Enabled = true;
    this->SpacePadTrackBar->Enabled = true;
    this->SpacePadWidget->Enabled = true;
    this->SpacePadWidgetUpDown->Enabled = true;
}
//---------------------------------------------------------------------------
void TConfigurationForm::SetFormRefreshTime(unsigned int NewFormRefreshTime)
{
    CLAMP(NewFormRefreshTime,((unsigned int)this->RefreshTimeTrackBar->Min),((unsigned int)this->RefreshTimeTrackBar->Max));

    this->RefreshTimeTrackBar->Position = NewFormRefreshTime;
    this->RefreshTimeWidgetUpDown->Position = NewFormRefreshTime;
}
//---------------------------------------------------------------------------
void TConfigurationForm::SetFormSpecialStreamProbability(float NewSpecialStreamProbability)
{
    unsigned int NewProbabilityAsInteger = (unsigned int)(NewSpecialStreamProbability*100);
    CLAMP(NewProbabilityAsInteger,((unsigned int)this->SpecialStreamProbabilityTrackBar->Min),((unsigned int)this->SpecialStreamProbabilityTrackBar->Max));

    this->SpecialStreamProbabilityTrackBar->Position = NewProbabilityAsInteger;
    this->SpecialStreamProbabilityWidgetUpDown->Position = NewProbabilityAsInteger;
}
//---------------------------------------------------------------------------
void TConfigurationForm::SetFormPriority(DWORD NewFormPriority)
{
    switch(NewFormPriority)
    {
    case(HIGH_PRIORITY_CLASS):
        {
            this->HighPriorityRadioButton->Checked = true;
        }
        break;
    case(ABOVE_NORMAL_PRIORITY_CLASS):
        {
            this->AboveNormalPriorityRadioButton->Checked = true;
        }
        break;
    case(NORMAL_PRIORITY_CLASS):
        {
            this->NormalPriorityRadioButton->Checked = true;
        }
        break;
    case(BELOW_NORMAL_PRIORITY_CLASS):
        {
            this->BelowNormalPriorityRadioButton->Checked = true;
        }
        break;
    default:
    case(IDLE_PRIORITY_CLASS):
        {
            this->IdlePriorityRadioButton->Checked = true;
        }
        break;
    }
}
//---------------------------------------------------------------------------
void TConfigurationForm::DisableBlendingConfig(void)
{
    this->BGBlendModePanel->Enabled = false;
    this->BGBlendModeLabel->Enabled = false;
    this->XORBGBlendModeRadioButton->Enabled = false;
    this->ORBGBlendModeRadioButton->Enabled = false;
    this->ANDBGBlendModeRadioButton->Enabled = false;
}
//---------------------------------------------------------------------------
void TConfigurationForm::EnableBlendingConfig(void)
{
    this->BGBlendModePanel->Enabled = true;
    this->BGBlendModeLabel->Enabled = true;
    this->XORBGBlendModeRadioButton->Enabled = true;
    this->ORBGBlendModeRadioButton->Enabled = true;
    this->ANDBGBlendModeRadioButton->Enabled = true;
}
//---------------------------------------------------------------------------
void TConfigurationForm::UpdateFormBasedOnTarget(void)
{
    this->SetFormMaxStream(this->TargetMatrix->GetMaxStream());
    this->SetFormSpeedVariance(this->TargetMatrix->GetSpeedVariance());
    this->SetFormBackTrace(this->TargetMatrix->GetBackTrace());
    this->SetFormMonotonousCleanupEnabled(this->TargetMatrix->GetMonotonousCleanupEnabled());
    this->SetFormLeading(this->TargetMatrix->GetLeading());
    this->SetFormSpacePad(this->TargetMatrix->GetSpacePad());
    this->SetFormRandomizedCleanupEnabled(this->TargetMatrix->GetRandomizedCleanupEnabled());

    this->SetFormFont(this->TargetMatrix->GetFont());
    this->SetFormSpecialFont(this->TargetMatrix->GetSpecialStringFont());

    BYTE R,G,B,A;
    this->TargetMatrix->GetColor(R,G,B,A);
    this->SetFormForegroundColor(R,G,B,A);
    this->TargetMatrix->GetFadeColor(R,G,B,A);
    this->SetFormFadeColor(R,G,B,A);
    this->TargetMatrix->GetBGColor(R,G,B,A);
    this->SetFormBackgroundColor(R,G,B,A);

    this->TargetMatrix->GetSpecialStringColor(R,G,B,A);
    this->SetFormSpecialForegroundColor(R,G,B,A);
    this->TargetMatrix->GetSpecialStringFadeColor(R,G,B,A);
    this->SetFormSpecialFadeColor(R,G,B,A);

    this->SetFormBGMode((TBGMode)this->TargetMatrix->GetBGMode());

    this->SetFormBlendingMode((TBlendMode)this->TargetMatrix->GetBlendMode());

    this->SetFormRefreshTime(*(this->TargetRefreshTime));

    this->SetFormSpecialStreamProbability(this->TargetMatrix->GetSpecialStringStreamProbability());

    this->SetFormPriority(*(this->TargetPriority));
}
//---------------------------------------------------------------------------
void TConfigurationForm::LaunchURL(const _TCHAR *URL)
{
    ShellExecute(this->Handle,_TEXT("open"),URL, NULL, NULL, SW_SHOWNORMAL );
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//--------------------------CONSTRUCTOR--------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
__fastcall TConfigurationForm::TConfigurationForm(TComponent* Owner,IzsMatrix *NewTargetMatrix,unsigned int *NewTargetRefreshTime,DWORD *NewTargetPriority)
:TForm(Owner)
{
    if(CharacterSetForm == NULL)
    {
        CharacterSetForm = new TCharacterSetForm(this);
    }

    this->TargetMatrix = NewTargetMatrix;
    this->TargetRefreshTime = NewTargetRefreshTime;
    this->TargetPriority = NewTargetPriority;

    this->UpdateFormBasedOnTarget();

}
//---------------------------------------------------------------------------
__fastcall TConfigurationForm::~TConfigurationForm(void)
{
    if(CharacterSetForm != NULL)
    {
        delete CharacterSetForm;
        CharacterSetForm = NULL;
    }

}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::MaxStreamTrackBarChange(
      TObject *Sender)
{
    TargetMatrix->SetMaxStream(this->MaxStreamTrackBar->Position);
    this->MaxStreamWidgetUpDown->Position = this->MaxStreamTrackBar->Position;
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::MaxStreamWidgetChange(
      TObject *Sender)
{
    if(this->MaxStreamWidgetUpDown->Position < this->MaxStreamTrackBar->Min)
        this->MaxStreamWidgetUpDown->Position = this->MaxStreamTrackBar->Min;
    if(this->MaxStreamWidgetUpDown->Position > this->MaxStreamTrackBar->Max)
        this->MaxStreamWidgetUpDown->Position = this->MaxStreamTrackBar->Max;
    //CLAMP(this->MaxStreamWidgetUpDown->Position,this->MaxStreamTrackBar->Min,this->MaxStreamTrackBar->Max);

    TargetMatrix->SetMaxStream(this->MaxStreamWidgetUpDown->Position);
    this->MaxStreamTrackBar->Position = this->MaxStreamWidgetUpDown->Position;
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::FontSelectionButtonClick(
      TObject *Sender)
{
    if(this->FontDialog->Execute())
    {
        this->FontNameWidget->Text = this->FontDialog->Font->Name;
        this->TargetMatrix->SetFont(this->FontDialog->Font->Handle);
    }
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::SpecialFontSelectionButtonClick(
      TObject *Sender)
{
    if(this->SpecialFontDialog->Execute())
    {
        this->SpecialFontNameWidget->Text = this->SpecialFontDialog->Font->Name;
        this->TargetMatrix->SetSpecialStringFont(this->SpecialFontDialog->Font->Handle);
    }
}
//---------------------------------------------------------------------------


void __fastcall TConfigurationForm::ForegroundColorSelectionButtonClick(
      TObject *Sender)
{
    if(this->ForegroundColorDialog->Execute())
    {
        this->LeadColorPanel->Font->Color = this->ForegroundColorDialog->Color;
        BYTE DummyR,DummyG,DummyB,A;
        this->TargetMatrix->GetColor(DummyR,DummyG,DummyB,A);
        BYTE B = (BYTE)((this->ForegroundColorDialog->Color>>16) & 0xFF);
        BYTE G = (BYTE)((this->ForegroundColorDialog->Color>>8) & 0xFF);
        BYTE R = (BYTE)((this->ForegroundColorDialog->Color) & 0xFF);
        this->TargetMatrix->SetColor(R,G,B,A);
    }
}
//--------------------------------------------------------------------------- 

void __fastcall TConfigurationForm::FadeColorSelectionButtonClick(
      TObject *Sender)
{
    if(this->FadeColorDialog->Execute())
    {
        this->TrailColorPanel->Font->Color = this->FadeColorDialog->Color;
        BYTE DummyR,DummyG,DummyB,A;
        this->TargetMatrix->GetFadeColor(DummyR,DummyG,DummyB,A);
        BYTE B = (BYTE)((this->FadeColorDialog->Color>>16) & 0xFF);
        BYTE G = (BYTE)((this->FadeColorDialog->Color>>8) & 0xFF);
        BYTE R = (BYTE)((this->FadeColorDialog->Color) & 0xFF);
        this->TargetMatrix->SetFadeColor(R,G,B,A);
    }
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::BackgroundColorSelectionButtonClick(
      TObject *Sender)
{
    if(this->BackgroundColorDialog->Execute())
    {
        this->LeadColorPanel->Color = this->BackgroundColorDialog->Color;
        this->TrailColorPanel->Color = this->BackgroundColorDialog->Color;
        this->SpecialLeadColorPanel->Color = this->BackgroundColorDialog->Color;
        this->SpecialTrailColorPanel->Color = this->BackgroundColorDialog->Color;
        BYTE DummyR,DummyG,DummyB,A;
        this->TargetMatrix->GetBGColor(DummyR,DummyG,DummyB,A);
        BYTE B = (BYTE)((this->BackgroundColorDialog->Color>>16) & 0xFF);
        BYTE G = (BYTE)((this->BackgroundColorDialog->Color>>8) & 0xFF);
        BYTE R = (BYTE)((this->BackgroundColorDialog->Color) & 0xFF);
        this->TargetMatrix->SetBGColor(R,G,B,A);
        this->TargetMatrix->SetSpecialStringBGColor(R,G,B,A);
    }
}
//---------------------------------------------------------------------------


void __fastcall TConfigurationForm::SpecialForegroundColorSelectionButtonClick(
      TObject *Sender)
{
    if(this->SpecialForegroundColorDialog->Execute())
    {
        this->SpecialLeadColorPanel->Font->Color = this->SpecialForegroundColorDialog->Color;
        BYTE DummyR,DummyG,DummyB,A;
        this->TargetMatrix->GetSpecialStringColor(DummyR,DummyG,DummyB,A);
        BYTE B = (BYTE)((this->SpecialForegroundColorDialog->Color>>16) & 0xFF);
        BYTE G = (BYTE)((this->SpecialForegroundColorDialog->Color>>8) & 0xFF);
        BYTE R = (BYTE)((this->SpecialForegroundColorDialog->Color) & 0xFF);
        this->TargetMatrix->SetSpecialStringColor(R,G,B,A);
    }
}
//--------------------------------------------------------------------------- 

void __fastcall TConfigurationForm::SpecialFadeColorSelectionButtonClick(
      TObject *Sender)
{
    if(this->SpecialFadeColorDialog->Execute())
    {
        this->SpecialTrailColorPanel->Font->Color = this->SpecialFadeColorDialog->Color;
        BYTE DummyR,DummyG,DummyB,A;
        this->TargetMatrix->GetSpecialStringFadeColor(DummyR,DummyG,DummyB,A);
        BYTE B = (BYTE)((this->SpecialFadeColorDialog->Color>>16) & 0xFF);
        BYTE G = (BYTE)((this->SpecialFadeColorDialog->Color>>8) & 0xFF);
        BYTE R = (BYTE)((this->SpecialFadeColorDialog->Color) & 0xFF);
        this->TargetMatrix->SetSpecialStringFadeColor(R,G,B,A);
    }
}
//---------------------------------------------------------------------------


void __fastcall TConfigurationForm::ColorBGModeRadioButtonClick(
      TObject *Sender)
{
    this->TargetMatrix->SetBGMode(bgmodeColor);

    this->DisableBlendingConfig();
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::BitmapBGModeRadioButtonClick(
      TObject *Sender)
{
    this->TargetMatrix->SetBGMode(bgmodeBitmap);

    this->EnableBlendingConfig();
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::XORBGBlendModeRadioButtonClick(
      TObject *Sender)
{
    this->TargetMatrix->SetBlendMode(blendmodeXOR);
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::ORBGBlendModeRadioButtonClick(
      TObject *Sender)
{
    this->TargetMatrix->SetBlendMode(blendmodeOR);
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::ANDBGBlendModeRadioButtonClick(
      TObject *Sender)
{
    this->TargetMatrix->SetBlendMode(blendmodeAND);
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::TransparentBGRadioButtonClick(
      TObject *Sender)
{
    BYTE R,G,B,A;
    this->TargetMatrix->GetBGColor(R,G,B,A);
    this->TargetMatrix->SetBGColor(R,G,B,0);
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::OpaqueBGRadioButtonClick(
      TObject *Sender)
{
    BYTE R,G,B,A;
    this->TargetMatrix->GetBGColor(R,G,B,A);
    this->TargetMatrix->SetBGColor(R,G,B,255);
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::SpeedVarianceTrackBarChange(
      TObject *Sender)
{
    TargetMatrix->SetSpeedVariance(this->SpeedVarianceTrackBar->Position);
    this->SpeedVarianceWidgetUpDown->Position = this->SpeedVarianceTrackBar->Position;
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::SpeedVarianceWidgetChange(
      TObject *Sender)
{
    if(this->SpeedVarianceWidgetUpDown->Position < this->SpeedVarianceTrackBar->Min)
        this->SpeedVarianceWidgetUpDown->Position = this->SpeedVarianceTrackBar->Min;
    if(this->SpeedVarianceWidgetUpDown->Position > this->SpeedVarianceTrackBar->Max)
        this->SpeedVarianceWidgetUpDown->Position = this->SpeedVarianceTrackBar->Max;
    //CLAMP(this->SpeedVarianceWidgetUpDown->Position,this->SpeedVarianceTrackBar->Min,this->SpeedVarianceTrackBar->Max);

    TargetMatrix->SetSpeedVariance(this->SpeedVarianceWidgetUpDown->Position);
    this->SpeedVarianceTrackBar->Position = this->SpeedVarianceWidgetUpDown->Position;
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::LeadingTrackBarChange(TObject *Sender)
{
    TargetMatrix->SetLeading(this->LeadingTrackBar->Position);
    this->LeadingWidgetUpDown->Position = this->LeadingTrackBar->Position;
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::LeadingWidgetChange(TObject *Sender)
{
    if(this->LeadingWidgetUpDown->Position < this->LeadingTrackBar->Min)
        this->LeadingWidgetUpDown->Position = this->LeadingTrackBar->Min;
    if(this->LeadingWidgetUpDown->Position > this->LeadingTrackBar->Max)
        this->LeadingWidgetUpDown->Position = this->LeadingTrackBar->Max;
    //CLAMP(this->LeadingWidgetUpDown->Position,this->LeadingTrackBar->Min,this->LeadingTrackBar->Max);

    TargetMatrix->SetLeading(this->LeadingWidgetUpDown->Position);
    this->LeadingTrackBar->Position = this->LeadingWidgetUpDown->Position;
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::BackTraceTrackBarChange(
      TObject *Sender)
{
    TargetMatrix->SetBackTrace(this->BackTraceTrackBar->Position);
    this->BackTraceWidgetUpDown->Position = this->BackTraceTrackBar->Position;
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::BackTraceWidgetChange(
      TObject *Sender)
{
    if(this->BackTraceWidgetUpDown->Position < this->BackTraceTrackBar->Min)
        this->BackTraceWidgetUpDown->Position = this->BackTraceTrackBar->Min;
    if(this->BackTraceWidgetUpDown->Position > this->BackTraceTrackBar->Max)
        this->BackTraceWidgetUpDown->Position = this->BackTraceTrackBar->Max;
    //CLAMP(this->BackTraceWidgetUpDown->Position,this->BackTraceTrackBar->Min,this->BackTraceTrackBar->Max);

    TargetMatrix->SetBackTrace(this->BackTraceWidgetUpDown->Position);
    this->BackTraceTrackBar->Position = this->BackTraceWidgetUpDown->Position;
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::SpacePadTrackBarChange(TObject *Sender)
{
    TargetMatrix->SetSpacePad(this->SpacePadTrackBar->Position);
    this->SpacePadWidgetUpDown->Position = this->SpacePadTrackBar->Position;
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::SpacePadWidgetChange(TObject *Sender)
{
    if(this->SpacePadWidgetUpDown->Position < this->SpacePadTrackBar->Min)
        this->SpacePadWidgetUpDown->Position = this->SpacePadTrackBar->Min;
    if(this->SpacePadWidgetUpDown->Position > this->SpacePadTrackBar->Max)
        this->SpacePadWidgetUpDown->Position = this->SpacePadTrackBar->Max;
    //CLAMP(this->SpacePadWidgetUpDown->Position,this->SpacePadTrackBar->Min,this->SpacePadTrackBar->Max);

    TargetMatrix->SetSpacePad(this->SpacePadWidgetUpDown->Position);
    this->SpacePadTrackBar->Position = this->SpacePadWidgetUpDown->Position;
}
//---------------------------------------------------------------------------


void __fastcall TConfigurationForm::EditCharSetButtonClick(TObject *Sender)
{
    CharacterSetForm->SetFormCharSet(this->TargetMatrix->GetValidCharSet(),
                                     this->TargetMatrix->GetNumCharsInSet());

    vector<tstring> SpecialStringVec;
    SpecialStringVec.resize(this->TargetMatrix->GetNumSpecialStringsInSet());
    unsigned int i = 0;
    for(vector<tstring>::iterator iter = SpecialStringVec.begin();
        iter != SpecialStringVec.end();
        ++iter, ++i)
    {
        iter->assign(this->TargetMatrix->GetValidSpecialString(i));
    }

    CharacterSetForm->SetFormSpecialStrings(SpecialStringVec);

    if(mrOk == CharacterSetForm->ShowModal())
    {
        const vector<_TCHAR> *CharSet = CharacterSetForm->GetFormCharSet();

        if(CharSet->empty())
        {
            this->TargetMatrix->ClearValidCharSet();
        }
        else
        {
            const _TCHAR *CharSetPointer = &(CharSet->front());
            this->TargetMatrix->SetValidCharSet(CharSetPointer,CharSet->size());
        }


        CharacterSetForm->GetFormSpecialStrings(SpecialStringVec);

        if(SpecialStringVec.empty())
        {
            this->TargetMatrix->ClearValidSpecialStringSet();
        }
        else
        {
            unsigned int SpecialStringArraySize = SpecialStringVec.size();
            const _TCHAR * /*const*/ *SpecialStringArray = new const _TCHAR *[SpecialStringArraySize];

            for(i = 0; i < SpecialStringArraySize; i++)
            {
                SpecialStringArray[i] = SpecialStringVec[i].c_str();
            }

            this->TargetMatrix->SetValidSpecialStringSet(SpecialStringArray,SpecialStringArraySize);

            delete[] SpecialStringArray;
        }


    }
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::ExitClick(TObject *Sender)
{
    this->ModalResult = mrCancel;
}
//---------------------------------------------------------------------------
void __fastcall TConfigurationForm::SaveClick(TObject *Sender)
{
    if(this->ConfigSaveDialog->Execute())
    {
        _TCHAR TargetFile[LONG_MAX_PATH];
        #ifdef _UNICODE
        snwprintf(TargetFile,LONG_MAX_PATH - 1,_TEXT("%S"),this->ConfigSaveDialog->FileName.c_str());
        #else
        snprintf(TargetFile,LONG_MAX_PATH - 1,"%s",this->ConfigSaveDialog->FileName.c_str());
        #endif

        SaveConfigToFile(this->TargetMatrix,*(this->TargetRefreshTime),GetPriorityClass(GetCurrentProcess()),TargetFile);
    }
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::LoadClick(TObject *Sender)
{
    if(this->ConfigOpenDialog->Execute())
    {
        _TCHAR TargetFile[LONG_MAX_PATH];
        #ifdef _UNICODE
        snwprintf(TargetFile,LONG_MAX_PATH - 1,_TEXT("%S"),this->ConfigOpenDialog->FileName.c_str());
        #else
        snprintf(TargetFile,LONG_MAX_PATH - 1,"%s",this->ConfigOpenDialog->FileName.c_str());
        #endif

        DWORD PriorityClass = IDLE_PRIORITY_CLASS;
        LoadConfigFromFile(this->TargetMatrix,*(this->TargetRefreshTime),PriorityClass,TargetFile);

        SetPriorityClass(GetCurrentProcess(),PriorityClass);

        SetTimer(this->TargetMatrix->GethWnd(),REFRESH_TIMER_ID,*(this->TargetRefreshTime),NULL);

        this->UpdateFormBasedOnTarget();
    }
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::LoadDefaultsClick(TObject *Sender)
{
    _TCHAR TargetFile[] = _T("./default.cfg");
    DWORD PriorityClass = IDLE_PRIORITY_CLASS;
    LoadConfigFromFile(this->TargetMatrix,*(this->TargetRefreshTime),PriorityClass,TargetFile);

    SetPriorityClass(GetCurrentProcess(),PriorityClass);

    SetTimer(this->TargetMatrix->GethWnd(),REFRESH_TIMER_ID,*(this->TargetRefreshTime),NULL);

    this->UpdateFormBasedOnTarget();
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::AboutClick(TObject *Sender)
{
    LaunchAboutForm(this);
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::ReadMeClick(TObject *Sender)
{
    ShellExecute(this->Handle,_TEXT("open"),_TEXT("readme.txt"), NULL, NULL, SW_SHOWNORMAL );
    //WinExec("notepad.exe readme.txt",SW_SHOW);
/*    STARTUPINFO StartupInfo;
    memset(&StartupInfo,0,sizeof(StartupInfo));
    StartupInfo.cb = sizeof(StartupInfo);

    PROCESS_INFORMATION ProcessInfo;

    CreateProcess(NULL,_TEXT("start readme.txt"),NULL,NULL,FALSE,0,NULL,NULL,&StartupInfo,&ProcessInfo);
*/
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::MonotonousCleanupEnabledCheckBoxClick(
      TObject *Sender)
{
    this->SetFormMonotonousCleanupEnabled(this->MonotonousCleanupEnabledCheckBox->Checked);
    this->TargetMatrix->SetMonotonousCleanupEnabled(this->MonotonousCleanupEnabledCheckBox->Checked);
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::RandomizedCleanupEnabledCheckBoxClick(
      TObject *Sender)
{
    this->SetFormRandomizedCleanupEnabled(this->RandomizedCleanupEnabledCheckBox->Checked);
    this->TargetMatrix->SetRandomizedCleanupEnabled(this->RandomizedCleanupEnabledCheckBox->Checked);
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::RefreshTimeTrackBarChange(
      TObject *Sender)
{
    *TargetRefreshTime = this->RefreshTimeTrackBar->Position;
    SetTimer(this->TargetMatrix->GethWnd(),REFRESH_TIMER_ID,*TargetRefreshTime,0);

    this->RefreshTimeWidgetUpDown->Position = this->RefreshTimeTrackBar->Position;
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::RefreshTimeWidgetChange(
      TObject *Sender)
{
    if(this->RefreshTimeWidgetUpDown->Position < this->RefreshTimeTrackBar->Min)
        this->RefreshTimeWidgetUpDown->Position = this->RefreshTimeTrackBar->Min;
    if(this->RefreshTimeWidgetUpDown->Position > this->RefreshTimeTrackBar->Max)
        this->RefreshTimeWidgetUpDown->Position = this->RefreshTimeTrackBar->Max;
    //CLAMP(this->RefreshTimeWidgetUpDown->Position,this->RefreshTimeTrackBar->Min,this->RefreshTimeTrackBar->Max);

    *TargetRefreshTime = this->RefreshTimeWidgetUpDown->Position;
    SetTimer(this->TargetMatrix->GethWnd(),REFRESH_TIMER_ID,*TargetRefreshTime,0);

    this->RefreshTimeTrackBar->Position = this->RefreshTimeWidgetUpDown->Position;
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::SpecialStreamProbabilityTrackBarChange(
      TObject *Sender)
{
    TargetMatrix->SetSpecialStringStreamProbability(this->SpecialStreamProbabilityTrackBar->Position/100.0);
    this->SpecialStreamProbabilityWidgetUpDown->Position = this->SpecialStreamProbabilityTrackBar->Position;
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::SpecialStreamProbabilityWidgetChange(
      TObject *Sender)
{
    if(this->SpecialStreamProbabilityWidgetUpDown->Position < this->SpecialStreamProbabilityTrackBar->Min)
        this->SpecialStreamProbabilityWidgetUpDown->Position = this->SpecialStreamProbabilityTrackBar->Min;
    if(this->SpecialStreamProbabilityWidgetUpDown->Position > this->SpecialStreamProbabilityTrackBar->Max)
        this->SpecialStreamProbabilityWidgetUpDown->Position = this->SpecialStreamProbabilityTrackBar->Max;
    //CLAMP(this->SpeedVarianceWidgetUpDown->Position,this->SpeedVarianceTrackBar->Min,this->SpeedVarianceTrackBar->Max);

    TargetMatrix->SetSpecialStringStreamProbability(this->SpecialStreamProbabilityWidgetUpDown->Position/100.0);
    this->SpecialStreamProbabilityTrackBar->Position = this->SpecialStreamProbabilityWidgetUpDown->Position;
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::HighPriorityRadioButtonClick(
      TObject *Sender)
{
    *(this->TargetPriority) = HIGH_PRIORITY_CLASS;
    SetPriorityClass(GetCurrentProcess(),*(this->TargetPriority));
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::AboveNormalPriorityRadioButtonClick(
      TObject *Sender)
{
    *(this->TargetPriority) = ABOVE_NORMAL_PRIORITY_CLASS;
    SetPriorityClass(GetCurrentProcess(),*(this->TargetPriority));
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::NormalPriorityRadioButtonClick(
      TObject *Sender)
{
    *(this->TargetPriority) = NORMAL_PRIORITY_CLASS;
    SetPriorityClass(GetCurrentProcess(),*(this->TargetPriority));
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::BelowNormalPriorityRadioButtonClick(
      TObject *Sender)
{
    *(this->TargetPriority) = BELOW_NORMAL_PRIORITY_CLASS;
    SetPriorityClass(GetCurrentProcess(),*(this->TargetPriority));
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::IdlePriorityRadioButtonClick(
      TObject *Sender)
{
    *(this->TargetPriority) = IDLE_PRIORITY_CLASS;
    SetPriorityClass(GetCurrentProcess(),*(this->TargetPriority));
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::HomepageClick(TObject *Sender)
{
    this->LaunchURL(HOMEPAGE_URL);    
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::ProjectSummaryPageClick(
      TObject *Sender)
{
    this->LaunchURL(PROJECTSUMMARYPAGE_URL);    
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::SupportRequestsClick(TObject *Sender)
{
    this->LaunchURL(SUPPORTREQUESTS_URL);    
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::BugReportsClick(TObject *Sender)
{
    this->LaunchURL(BUGREPORTS_URL);    
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::FeatureRequestsClick(TObject *Sender)
{
    this->LaunchURL(FEATUREREQUESTS_URL);    
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::DiscussionForumsClick(TObject *Sender)
{
    this->LaunchURL(DISCUSSIONFORUMS_URL);    
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::DonateClick(TObject *Sender)
{
    this->LaunchURL(DONATE_URL);
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::LaunchHelpClick(TObject *Sender)
{
    ShellExecute(this->Handle,_TEXT("open"),_TEXT("ZMatrixHelp.chm"), NULL, NULL, SW_SHOWNORMAL );
}
//---------------------------------------------------------------------------


void __fastcall TConfigurationForm::HireTheDeveloperClick(TObject *Sender)
{
    LaunchHireForm(this);
}
//---------------------------------------------------------------------------

void __fastcall TConfigurationForm::SpecialColorsCopyButtonClick(
      TObject *Sender)
{
    BYTE DummyR,DummyG,DummyB,A;
    BYTE R,G,B;

    this->TargetMatrix->GetColor(R,G,B,A);
    this->SpecialForegroundColorDialog->Color = (TColor)RGB(R,G,B);

    this->TargetMatrix->GetFadeColor(R,G,B,A);
    this->SpecialFadeColorDialog->Color = (TColor)RGB(R,G,B);


    this->SpecialLeadColorPanel->Font->Color = this->SpecialForegroundColorDialog->Color;
    this->TargetMatrix->GetSpecialStringColor(DummyR,DummyG,DummyB,A);
    B = (BYTE)((this->SpecialForegroundColorDialog->Color>>16) & 0xFF);
    G = (BYTE)((this->SpecialForegroundColorDialog->Color>>8) & 0xFF);
    R = (BYTE)((this->SpecialForegroundColorDialog->Color) & 0xFF);
    this->TargetMatrix->SetSpecialStringColor(R,G,B,A);

    this->SpecialTrailColorPanel->Font->Color = this->SpecialFadeColorDialog->Color;
    this->TargetMatrix->GetSpecialStringFadeColor(DummyR,DummyG,DummyB,A);
    B = (BYTE)((this->SpecialFadeColorDialog->Color>>16) & 0xFF);
    G = (BYTE)((this->SpecialFadeColorDialog->Color>>8) & 0xFF);
    R = (BYTE)((this->SpecialFadeColorDialog->Color) & 0xFF);
    this->TargetMatrix->SetSpecialStringFadeColor(R,G,B,A);
}
//---------------------------------------------------------------------------

