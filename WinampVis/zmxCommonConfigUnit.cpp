/*!=========================================================================
//
//  Program:   ZMatrix
//  Module:    $RCSfile: zmxCommonConfigUnit.cpp,v $
//  Language:  C/C++
//  Date:      $Date: 2003/05/12 07:43:57 $
//  Version:   $Revision: 1.4 $
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

#include "zmxCommonConfigUnit.h"
#include <stdio.h>
#include <windows.h>
#include <tchar.h>
#include <shlobj.h>
#include <IniFiles.hpp>

//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------


#ifndef CLAMP
#define CLAMP(var, min, max) \
if((var) > (max)) (var) = (max); \
if((var) < (min)) (var) = (min);
#endif //CLAMP

#define WRITEFLOAT3(IniFile,Array)\
if(Array != NULL)\
{\
    AnsiString ValueName = #Array;\
    IniFile->WriteString(SectionName,ValueName,AnsiString("{") + AnsiString(Array[0]) + AnsiString(",") + AnsiString(Array[1]) + AnsiString(",") + AnsiString(Array[2]) + AnsiString("}"));\
}

#define READFLOAT3(IniFile,Array)\
if(Array != NULL)\
{\
    AnsiString ValueName = #Array;\
    AnsiString TempStr = IniFile->ReadString(SectionName,ValueName,AnsiString("{") + AnsiString(Array[0]) + AnsiString(",") + AnsiString(Array[1]) + AnsiString(",") + AnsiString(Array[2]) + AnsiString("}"));\
    sscanf(TempStr.c_str(),"{%f,%f,%f}",Array,Array+1,Array+2);\
}

//---------------------------------------------------------------------------
WideString GetConfigFileName(void)
{
    _TCHAR AppDataPath[MAX_PATH];

	if(!SHGetSpecialFolderPath(NULL,AppDataPath,CSIDL_APPDATA,TRUE))
	{
		MessageBox(NULL,_T("Failed to get Application Data folder"),_T("Error"),0);
        return WideString ("./vis_zmx.cfg");
	}

    WideString RetVal(AppDataPath);
    RetVal += _T("/.ZMatrix/");

    if(!DirectoryExists(RetVal))
    {
        CreateDir(RetVal);
    }

    RetVal += _T("vis_zmx.cfg");

    return RetVal;
}
//---------------------------------------------------------------------------
int SaveCommonConfig(WideString ConfigFileName,
                     AnsiString SectionName,
                     float *BaseColorScales,
                     float *BaseColorOffsets,
                     float *PeakColorScales,
                     float *PeakColorOffsets,
                     float *GlobalScale,
                     float *GlobalOffset)
{
    TCustomIniFile *OutFile = new TIniFile(ConfigFileName);

    if(OutFile == NULL)
    {
        return 0;
    }

    AnsiString ValueName;

    WRITEFLOAT3(OutFile,BaseColorScales);
    WRITEFLOAT3(OutFile,BaseColorOffsets);
    WRITEFLOAT3(OutFile,PeakColorScales);
    WRITEFLOAT3(OutFile,PeakColorOffsets);

    if(GlobalScale != NULL)
    {
        ValueName = "GlobalScale";
        OutFile->WriteFloat(SectionName,ValueName,GlobalScale[0]);
    }

    if(GlobalOffset != NULL)
    {
        ValueName = "GlobalOffset";
        OutFile->WriteFloat(SectionName,ValueName,GlobalOffset[0]);
    }

    delete OutFile;

    return 1;
}
//---------------------------------------------------------------------------
int LoadCommonConfig(WideString ConfigFileName,
                     AnsiString SectionName,
                     float *BaseColorScales,
                     float *BaseColorOffsets,
                     float *PeakColorScales,
                     float *PeakColorOffsets,
                     float *GlobalScale,
                     float *GlobalOffset)
{
    TCustomIniFile *OutFile = new TIniFile(ConfigFileName);

    if(OutFile == NULL)
    {
        return 0;
    }

    AnsiString ValueName;

    READFLOAT3(OutFile,BaseColorScales);
    READFLOAT3(OutFile,BaseColorOffsets);
    READFLOAT3(OutFile,PeakColorScales);
    READFLOAT3(OutFile,PeakColorOffsets);

    if(GlobalScale != NULL)
    {
        ValueName = "GlobalScale";
        GlobalScale[0] = OutFile->ReadFloat(SectionName,ValueName,GlobalScale[0]);
    }

    if(GlobalOffset != NULL)
    {
        ValueName = "GlobalOffset";
        GlobalOffset[0] = OutFile->ReadFloat(SectionName,ValueName,GlobalOffset[0]);
    }

    delete OutFile;

    return 1;
}
//---------------------------------------------------------------------------
__fastcall TzmxCommonConfigForm::TzmxCommonConfigForm(HWND ParentWindow,
            float *NewBaseColorScales,
            float *NewBaseColorOffsets,
            float *NewPeakColorScales,
            float *NewPeakColorOffsets,
            float *NewGlobalScale,
            float *NewGlobalOffset)
    : TForm(ParentWindow)
{
    if(NewBaseColorScales)
    {
        memcpy(OrigBaseScales,NewBaseColorScales,3*sizeof(float));
    }
    if(NewBaseColorOffsets)
    {
        memcpy(OrigBaseOffsets,NewBaseColorOffsets,3*sizeof(float));
    }
    if(NewPeakColorScales)
    {
        memcpy(OrigPeakScales,NewPeakColorScales,3*sizeof(float));
    }
    if(NewPeakColorOffsets)
    {
        memcpy(OrigPeakOffsets,NewPeakColorOffsets,3*sizeof(float));
    }
    if(NewGlobalScale)
    {
        OrigGlobalScale = *NewGlobalScale;
    }
    if(NewGlobalOffset)
    {
        OrigGlobalOffset = *NewGlobalOffset;
    }

    this->BaseColorScales = NewBaseColorScales;
    this->BaseColorOffsets = NewBaseColorOffsets;

    this->PeakColorScales = NewPeakColorScales;
    this->PeakColorOffsets = NewPeakColorOffsets;

    this->GlobalScale = NewGlobalScale;
    this->GlobalOffset = NewGlobalOffset;

    this->FillFormBasedOnTargets();
}
//---------------------------------------------------------------------------
void TzmxCommonConfigForm::RestoreOriginals(void)
{
    if(BaseColorScales)
    {
        memcpy(BaseColorScales,OrigBaseScales,3*sizeof(float));
    }
    if(BaseColorOffsets)
    {
        memcpy(BaseColorOffsets,OrigBaseOffsets,3*sizeof(float));
    }
    if(PeakColorScales)
    {
        memcpy(PeakColorScales,OrigPeakScales,3*sizeof(float));
    }
    if(PeakColorOffsets)
    {
        memcpy(PeakColorOffsets,OrigPeakOffsets,3*sizeof(float));
    }
    if(GlobalScale)
    {
        *GlobalScale = OrigGlobalScale;
    }
    if(GlobalOffset)
    {
        *GlobalOffset = OrigGlobalOffset;
    }
}
//---------------------------------------------------------------------------
void TzmxCommonConfigForm::FillFormBasedOnTargets(void)
{
    if(this->BaseColorScales)
    {
        this->BaseColorRedScaleTrackBar->Position = this->BaseColorScales[0]*100;
        this->BaseColorRedScaleWidgetUpDown->Position = this->BaseColorScales[0]*100;

        this->BaseColorGreenScaleTrackBar->Position = this->BaseColorScales[1]*100;
        this->BaseColorGreenScaleWidgetUpDown->Position = this->BaseColorScales[1]*100;

        this->BaseColorBlueScaleTrackBar->Position = this->BaseColorScales[2]*100;
        this->BaseColorBlueScaleWidgetUpDown->Position = this->BaseColorScales[2]*100;
    }
    if(this->BaseColorOffsets)
    {
        int R = BaseColorOffsets[0]; CLAMP(R,0,255);
        int G = BaseColorOffsets[1]; CLAMP(G,0,255);
        int B = BaseColorOffsets[2]; CLAMP(B,0,255);
        TColor NewCol = (TColor)RGB(R,G,B);
        this->BaseColorOffsetColorPanel->Color = (TColor)NewCol;
        this->BaseColorOffsetDialog->Color = (TColor)NewCol;
    }

    if(this->PeakColorScales)
    {
        this->PeakColorRedScaleTrackBar->Position = this->PeakColorScales[0]*100;
        this->PeakColorRedScaleWidgetUpDown->Position = this->PeakColorScales[0]*100;

        this->PeakColorGreenScaleTrackBar->Position = this->PeakColorScales[1]*100;
        this->PeakColorGreenScaleWidgetUpDown->Position = this->PeakColorScales[1]*100;

        this->PeakColorBlueScaleTrackBar->Position = this->PeakColorScales[2]*100;
        this->PeakColorBlueScaleWidgetUpDown->Position = this->PeakColorScales[2]*100;
    }
    if(this->PeakColorOffsets)
    {
        int R = PeakColorOffsets[0]; CLAMP(R,0,255);
        int G = PeakColorOffsets[1]; CLAMP(G,0,255);
        int B = PeakColorOffsets[2]; CLAMP(B,0,255);
        TColor NewCol = (TColor)RGB(R,G,B);
        this->PeakColorOffsetColorPanel->Color = (TColor)NewCol;
        this->PeakColorOffsetDialog->Color = (TColor)NewCol;
    }

    if(this->GlobalScale)
    {
        this->GlobalScaleTrackBar->Position = this->GlobalScale[0]*100;
        this->GlobalScaleWidgetUpDown->Position = this->GlobalScale[0]*100;
    }

    if(this->GlobalOffset)
    {
        this->GlobalOffsetTrackBar->Position = this->GlobalOffset[0]*100;
        this->GlobalOffsetWidgetUpDown->Position = this->GlobalOffset[0]*100;
    }
}
//---------------------------------------------------------------------------
void __fastcall TzmxCommonConfigForm::BaseColorRedScaleTrackBarChange(
      TObject *Sender)
{
    if(this->BaseColorScales) this->BaseColorScales[0] = this->BaseColorRedScaleTrackBar->Position/100.0;
    this->BaseColorRedScaleWidgetUpDown->Position = this->BaseColorRedScaleTrackBar->Position;
}
//---------------------------------------------------------------------------

void __fastcall TzmxCommonConfigForm::BaseColorRedScaleWidgetChange(
      TObject *Sender)
{
    if(this->BaseColorRedScaleWidgetUpDown->Position < this->BaseColorRedScaleTrackBar->Min)
        this->BaseColorRedScaleWidgetUpDown->Position = this->BaseColorRedScaleTrackBar->Min;
    if(this->BaseColorRedScaleWidgetUpDown->Position > this->BaseColorRedScaleTrackBar->Max)
        this->BaseColorRedScaleWidgetUpDown->Position = this->BaseColorRedScaleTrackBar->Max;

    if(this->BaseColorScales) this->BaseColorScales[0] = this->BaseColorRedScaleWidgetUpDown->Position/100.0;
    this->BaseColorRedScaleTrackBar->Position = this->BaseColorRedScaleWidgetUpDown->Position;
}
//---------------------------------------------------------------------------

void __fastcall TzmxCommonConfigForm::BaseColorGreenScaleTrackBarChange(
      TObject *Sender)
{
    if(this->BaseColorScales) this->BaseColorScales[1] = this->BaseColorGreenScaleTrackBar->Position/100.0;
    this->BaseColorGreenScaleWidgetUpDown->Position = this->BaseColorGreenScaleTrackBar->Position;
}
//---------------------------------------------------------------------------

void __fastcall TzmxCommonConfigForm::BaseColorGreenScaleWidgetChange(
      TObject *Sender)
{
    if(this->BaseColorGreenScaleWidgetUpDown->Position < this->BaseColorGreenScaleTrackBar->Min)
        this->BaseColorGreenScaleWidgetUpDown->Position = this->BaseColorGreenScaleTrackBar->Min;
    if(this->BaseColorGreenScaleWidgetUpDown->Position > this->BaseColorGreenScaleTrackBar->Max)
        this->BaseColorGreenScaleWidgetUpDown->Position = this->BaseColorGreenScaleTrackBar->Max;

    if(this->BaseColorScales) this->BaseColorScales[1] = this->BaseColorGreenScaleWidgetUpDown->Position/100.0;
    this->BaseColorGreenScaleTrackBar->Position = this->BaseColorGreenScaleWidgetUpDown->Position;
}
//---------------------------------------------------------------------------

void __fastcall TzmxCommonConfigForm::BaseColorBlueScaleTrackBarChange(
      TObject *Sender)
{
    if(this->BaseColorScales) this->BaseColorScales[2] = this->BaseColorBlueScaleTrackBar->Position/100.0;
    this->BaseColorBlueScaleWidgetUpDown->Position = this->BaseColorBlueScaleTrackBar->Position;
}
//---------------------------------------------------------------------------

void __fastcall TzmxCommonConfigForm::BaseColorBlueScaleWidgetChange(
      TObject *Sender)
{
    if(this->BaseColorBlueScaleWidgetUpDown->Position < this->BaseColorBlueScaleTrackBar->Min)
        this->BaseColorBlueScaleWidgetUpDown->Position = this->BaseColorBlueScaleTrackBar->Min;
    if(this->BaseColorBlueScaleWidgetUpDown->Position > this->BaseColorBlueScaleTrackBar->Max)
        this->BaseColorBlueScaleWidgetUpDown->Position = this->BaseColorBlueScaleTrackBar->Max;

    if(this->BaseColorScales) this->BaseColorScales[2] = this->BaseColorBlueScaleWidgetUpDown->Position/100.0;
    this->BaseColorBlueScaleTrackBar->Position = this->BaseColorBlueScaleWidgetUpDown->Position;
}
//---------------------------------------------------------------------------

void __fastcall TzmxCommonConfigForm::BaseColorOffsetSelectButtonClick(
      TObject *Sender)
{
    if(this->BaseColorOffsetDialog->Execute())
    {
        this->BaseColorOffsetColorPanel->Color = this->BaseColorOffsetDialog->Color;
        BYTE B = (BYTE)((this->BaseColorOffsetDialog->Color>>16) & 0xFF);
        BYTE G = (BYTE)((this->BaseColorOffsetDialog->Color>>8) & 0xFF);
        BYTE R = (BYTE)((this->BaseColorOffsetDialog->Color) & 0xFF);

        if(this->BaseColorOffsets)
        {
            this->BaseColorOffsets[0] = R;
            this->BaseColorOffsets[1] = G;
            this->BaseColorOffsets[2] = B;
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TzmxCommonConfigForm::PeakColorRedScaleTrackBarChange(
      TObject *Sender)
{
    if(this->PeakColorScales) this->PeakColorScales[0] = this->PeakColorRedScaleTrackBar->Position/100.0;
    this->PeakColorRedScaleWidgetUpDown->Position = this->PeakColorRedScaleTrackBar->Position;
}
//---------------------------------------------------------------------------

void __fastcall TzmxCommonConfigForm::PeakColorRedScaleWidgetChange(
      TObject *Sender)
{
    if(this->PeakColorRedScaleWidgetUpDown->Position < this->PeakColorRedScaleTrackBar->Min)
        this->PeakColorRedScaleWidgetUpDown->Position = this->PeakColorRedScaleTrackBar->Min;
    if(this->PeakColorRedScaleWidgetUpDown->Position > this->PeakColorRedScaleTrackBar->Max)
        this->PeakColorRedScaleWidgetUpDown->Position = this->PeakColorRedScaleTrackBar->Max;

    if(this->PeakColorScales) this->PeakColorScales[0] = this->PeakColorRedScaleWidgetUpDown->Position/100.0;
    this->PeakColorRedScaleTrackBar->Position = this->PeakColorRedScaleWidgetUpDown->Position;
}
//---------------------------------------------------------------------------

void __fastcall TzmxCommonConfigForm::PeakColorGreenScaleTrackBarChange(
      TObject *Sender)
{
    if(this->PeakColorScales) this->PeakColorScales[1] = this->PeakColorGreenScaleTrackBar->Position/100.0;
    this->PeakColorGreenScaleWidgetUpDown->Position = this->PeakColorGreenScaleTrackBar->Position;
}
//---------------------------------------------------------------------------

void __fastcall TzmxCommonConfigForm::PeakColorGreenScaleWidgetChange(
      TObject *Sender)
{
    if(this->PeakColorGreenScaleWidgetUpDown->Position < this->PeakColorGreenScaleTrackBar->Min)
        this->PeakColorGreenScaleWidgetUpDown->Position = this->PeakColorGreenScaleTrackBar->Min;
    if(this->PeakColorGreenScaleWidgetUpDown->Position > this->PeakColorGreenScaleTrackBar->Max)
        this->PeakColorGreenScaleWidgetUpDown->Position = this->PeakColorGreenScaleTrackBar->Max;

    if(this->PeakColorScales) this->PeakColorScales[1] = this->PeakColorGreenScaleWidgetUpDown->Position/100.0;
    this->PeakColorGreenScaleTrackBar->Position = this->PeakColorGreenScaleWidgetUpDown->Position;
}
//---------------------------------------------------------------------------

void __fastcall TzmxCommonConfigForm::PeakColorBlueScaleTrackBarChange(
      TObject *Sender)
{
    if(this->PeakColorScales) this->PeakColorScales[2] = this->PeakColorBlueScaleTrackBar->Position/100.0;
    this->PeakColorBlueScaleWidgetUpDown->Position = this->PeakColorBlueScaleTrackBar->Position;
}
//---------------------------------------------------------------------------

void __fastcall TzmxCommonConfigForm::PeakColorBlueScaleWidgetChange(
      TObject *Sender)
{
    if(this->PeakColorBlueScaleWidgetUpDown->Position < this->PeakColorBlueScaleTrackBar->Min)
        this->PeakColorBlueScaleWidgetUpDown->Position = this->PeakColorBlueScaleTrackBar->Min;
    if(this->PeakColorBlueScaleWidgetUpDown->Position > this->PeakColorBlueScaleTrackBar->Max)
        this->PeakColorBlueScaleWidgetUpDown->Position = this->PeakColorBlueScaleTrackBar->Max;

    if(this->PeakColorScales) this->PeakColorScales[2] = this->PeakColorBlueScaleWidgetUpDown->Position/100.0;
    this->PeakColorBlueScaleTrackBar->Position = this->PeakColorBlueScaleWidgetUpDown->Position;
}
//---------------------------------------------------------------------------


void __fastcall TzmxCommonConfigForm::PeakColorOffsetSelectButtonClick(
      TObject *Sender)
{
    if(this->PeakColorOffsetDialog->Execute())
    {
        this->PeakColorOffsetColorPanel->Color = this->PeakColorOffsetDialog->Color;
        BYTE B = (BYTE)((this->PeakColorOffsetDialog->Color>>16) & 0xFF);
        BYTE G = (BYTE)((this->PeakColorOffsetDialog->Color>>8) & 0xFF);
        BYTE R = (BYTE)((this->PeakColorOffsetDialog->Color) & 0xFF);

        if(this->PeakColorOffsets)
        {
            this->PeakColorOffsets[0] = R;
            this->PeakColorOffsets[1] = G;
            this->PeakColorOffsets[2] = B;
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TzmxCommonConfigForm::GlobalScaleTrackBarChange(
      TObject *Sender)
{
    if(this->GlobalScale) this->GlobalScale[0] = this->GlobalScaleTrackBar->Position/100.0;
    this->GlobalScaleWidgetUpDown->Position = this->GlobalScaleTrackBar->Position;
}
//---------------------------------------------------------------------------

void __fastcall TzmxCommonConfigForm::GlobalScaleWidgetChange(
      TObject *Sender)
{
    if(this->GlobalScaleWidgetUpDown->Position < this->GlobalScaleTrackBar->Min)
        this->GlobalScaleWidgetUpDown->Position = this->GlobalScaleTrackBar->Min;
    if(this->GlobalScaleWidgetUpDown->Position > this->GlobalScaleTrackBar->Max)
        this->GlobalScaleWidgetUpDown->Position = this->GlobalScaleTrackBar->Max;

    if(this->GlobalScale) this->GlobalScale[0] = this->GlobalScaleWidgetUpDown->Position/100.0;
    this->GlobalScaleTrackBar->Position = this->GlobalScaleWidgetUpDown->Position;
}
//---------------------------------------------------------------------------

void __fastcall TzmxCommonConfigForm::GlobalOffsetTrackBarChange(
      TObject *Sender)
{
    if(this->GlobalOffset) this->GlobalOffset[0] = this->GlobalOffsetTrackBar->Position/100.0;
    this->GlobalOffsetWidgetUpDown->Position = this->GlobalOffsetTrackBar->Position;
}
//---------------------------------------------------------------------------

void __fastcall TzmxCommonConfigForm::GlobalOffsetWidgetChange(
      TObject *Sender)
{
    if(this->GlobalOffsetWidgetUpDown->Position < this->GlobalOffsetTrackBar->Min)
        this->GlobalOffsetWidgetUpDown->Position = this->GlobalOffsetTrackBar->Min;
    if(this->GlobalOffsetWidgetUpDown->Position > this->GlobalOffsetTrackBar->Max)
        this->GlobalOffsetWidgetUpDown->Position = this->GlobalOffsetTrackBar->Max;

    if(this->GlobalOffset) this->GlobalOffset[0] = this->GlobalOffsetWidgetUpDown->Position/100.0;
    this->GlobalOffsetTrackBar->Position = this->GlobalOffsetWidgetUpDown->Position;
}
//---------------------------------------------------------------------------




