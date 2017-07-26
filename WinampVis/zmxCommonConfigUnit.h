/*!=========================================================================
//
//  Program:   ZMatrix
//  Module:    $RCSfile: zmxCommonConfigUnit.h,v $
//  Language:  C/C++
//  Date:      $Date: 2003/05/09 06:20:42 $
//  Version:   $Revision: 1.3 $
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

#ifndef zmxCommonConfigUnitH
#define zmxCommonConfigUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TzmxCommonConfigForm : public TForm
{
__published:	// IDE-managed Components
    TGroupBox *BaseColorScaleGroupBox;
    TGroupBox *BaseColorRedScaleGroupBox;
    TTrackBar *BaseColorRedScaleTrackBar;
    TEdit *BaseColorRedScaleWidget;
    TUpDown *BaseColorRedScaleWidgetUpDown;
    TGroupBox *BaseColorGreenScaleGroupBox;
    TTrackBar *BaseColorGreenScaleTrackBar;
    TEdit *BaseColorGreenScaleWidget;
    TUpDown *BaseColorGreenScaleWidgetUpDown;
    TGroupBox *BaseColorBlueScaleGroupBox;
    TTrackBar *BaseColorBlueScaleTrackBar;
    TEdit *BaseColorBlueScaleWidget;
    TUpDown *BaseColorBlueScaleWidgetUpDown;
    TGroupBox *BaseColorOffsetGroupBox;
    TPanel *BaseColorOffsetColorContainerPanel;
    TPanel *BaseColorOffsetColorPanel;
    TButton *BaseColorOffsetSelectButton;
    TPanel *ButtonPanel;
    TButton *OKButton;
    TButton *CancelButton;
    TColorDialog *BaseColorOffsetDialog;
    TGroupBox *PeakColorScaleGroupBox;
    TGroupBox *PeakColorRedScaleGroupBox;
    TTrackBar *PeakColorRedScaleTrackBar;
    TEdit *PeakColorRedScaleWidget;
    TUpDown *PeakColorRedScaleWidgetUpDown;
    TGroupBox *PeakColorGreenScaleGroupBox;
    TTrackBar *PeakColorGreenScaleTrackBar;
    TEdit *PeakColorGreenScaleWidget;
    TUpDown *PeakColorGreenScaleWidgetUpDown;
    TGroupBox *PeakColorBlueScaleGroupBox;
    TTrackBar *PeakColorBlueScaleTrackBar;
    TEdit *PeakColorBlueScaleWidget;
    TUpDown *PeakColorBlueScaleWidgetUpDown;
    TGroupBox *PeakColorOffsetGroupBox;
    TPanel *PeakColorOffsetColorContainerPanel;
    TPanel *PeakColorOffsetColorPanel;
    TButton *PeakColorOffsetSelectButton;
    TColorDialog *PeakColorOffsetDialog;
    TGroupBox *GlobalGroupBox;
    TGroupBox *GlobalScaleGroupBox;
    TTrackBar *GlobalScaleTrackBar;
    TEdit *GlobalScaleWidget;
    TUpDown *GlobalScaleWidgetUpDown;
    TGroupBox *GlobalOffsetGroupBox;
    TTrackBar *GlobalOffsetTrackBar;
    TEdit *GlobalOffsetWidget;
    TUpDown *GlobalOffsetWidgetUpDown;
    void __fastcall BaseColorRedScaleTrackBarChange(TObject *Sender);
    void __fastcall BaseColorRedScaleWidgetChange(TObject *Sender);
    void __fastcall BaseColorGreenScaleTrackBarChange(TObject *Sender);
    void __fastcall BaseColorGreenScaleWidgetChange(TObject *Sender);
    void __fastcall BaseColorBlueScaleTrackBarChange(TObject *Sender);
    void __fastcall BaseColorBlueScaleWidgetChange(TObject *Sender);
    void __fastcall BaseColorOffsetSelectButtonClick(TObject *Sender);
    void __fastcall PeakColorRedScaleTrackBarChange(TObject *Sender);
    void __fastcall PeakColorRedScaleWidgetChange(TObject *Sender);
    void __fastcall PeakColorGreenScaleTrackBarChange(TObject *Sender);
    void __fastcall PeakColorGreenScaleWidgetChange(TObject *Sender);
    void __fastcall PeakColorBlueScaleTrackBarChange(TObject *Sender);
    void __fastcall PeakColorBlueScaleWidgetChange(TObject *Sender);
    void __fastcall PeakColorOffsetSelectButtonClick(TObject *Sender);
    void __fastcall GlobalScaleTrackBarChange(TObject *Sender);
    void __fastcall GlobalScaleWidgetChange(TObject *Sender);
    void __fastcall GlobalOffsetTrackBarChange(TObject *Sender);
    void __fastcall GlobalOffsetWidgetChange(TObject *Sender);
private:	// User declarations
    float *BaseColorScales;
    float *BaseColorOffsets;
    float *PeakColorScales;
    float *PeakColorOffsets;
    float *GlobalScale;
    float *GlobalOffset;

    float OrigBaseScales[3];
    float OrigBaseOffsets[3];
    float OrigPeakScales[3];
    float OrigPeakOffsets[3];
    float OrigGlobalScale;
    float OrigGlobalOffset;

    void FillFormBasedOnTargets(void);
public:		// User declarations
    __fastcall TzmxCommonConfigForm(HWND ParentWindow,
                                    float *NewBaseColorScales = NULL,
                                    float *NewBaseColorOffsets = NULL,
                                    float *NewPeakColorScales = NULL,
                                    float *NewPeakColorOffsets = NULL,
                                    float *NewGlobalScale = NULL,
                                    float *NewGlobalOffset = NULL);

    void RestoreOriginals(void);
};

WideString GetConfigFileName(void);

int SaveCommonConfig(WideString ConfigFileName,
                     AnsiString SectionName,
                     float *BaseColorScales = NULL,
                     float *BaseColorOffsets = NULL,
                     float *PeakColorScales = NULL,
                     float *PeakColorOffsets = NULL,
                     float *GlobalScale = NULL,
                     float *GlobalOffset = NULL);

int LoadCommonConfig(WideString ConfigFileName,
                     AnsiString SectionName,
                     float *BaseColorScales = NULL,
                     float *BaseColorOffsets = NULL,
                     float *PeakColorScales = NULL,
                     float *PeakColorOffsets = NULL,
                     float *GlobalScale = NULL,
                     float *GlobalOffset = NULL);

#endif
