/*!=========================================================================
//
//  Program:   ZMatrix
//  Module:    $RCSfile: ConfigForm.h,v $
//  Language:  C/C++
//  Date:      $Date: 2003/05/12 07:43:57 $
//  Version:   $Revision: 1.19 $
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
#ifndef ConfigFormH
#define ConfigFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include "../zsMatrix/IzsMatrix.h"
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include "CharSetForm.h"
#include <Menus.hpp>
#include <vector>
#include <IniFiles.hpp>

#include "../globals.h"

#define CONFIG_FILE_SPECIAL_STRING_DELIMITER ';'
#define TEXT_FILE_SPECIAL_STRING_DELIMITER '\n'
#define EDIT_FIELD_SPECIAL_STRING_DELIMITER '\n'

//---------------------------------------------------------------------------
class TConfigurationForm : public TForm
{
__published:	// IDE-managed Components
    TGroupBox *TextGroupBox;
    TFontDialog *FontDialog;
    TButton *FontSelectionButton;
    TButton *OKButton;
    TButton *CancelButton;
    TGroupBox *ColorsGroupBox;
    TLabel *ForegroundColorLabel;
    TLabel *BackgroundColorLabel;
    TColorDialog *ForegroundColorDialog;
    TButton *ForegroundColorSelectionButton;
    TColorDialog *BackgroundColorDialog;
    TButton *BackgroundColorSelectionButton;
    TLabel *FontLabel;
    TButton *EditCharSetButton;
    TMainMenu *ConfigMainMenu;
    TMenuItem *File;
    TMenuItem *Save;
    TMenuItem *Load;
    TMenuItem *Seperator;
    TMenuItem *Exit;
    TOpenDialog *ConfigOpenDialog;
    TSaveDialog *ConfigSaveDialog;
    TMenuItem *Help;
    TMenuItem *ReadMe;
    TMenuItem *About;
    TMenuItem *Seperator2;
    TEdit *FontNameWidget;
    TPageControl *ParametersPageControl;
    TTabSheet *MonotonousCleanupParametersTabSheet;
    TTabSheet *RandomizedCleanupParametersTabSheet;
    TPanel *RandomizedCleanupParametersPanel;
    TCheckBox *RandomizedCleanupEnabledCheckBox;
    TGroupBox *LeadingGroupBox;
    TTrackBar *LeadingTrackBar;
    TEdit *LeadingWidget;
    TUpDown *LeadingWidgetUpDown;
    TGroupBox *SpacePadGroupBox;
    TTrackBar *SpacePadTrackBar;
    TEdit *SpacePadWidget;
    TUpDown *SpacePadWidgetUpDown;
    TPanel *MonotonousCleanupParametersPanel;
    TCheckBox *MonotonousCleanupEnabledCheckBox;
    TGroupBox *BackTraceGroupBox;
    TTrackBar *BackTraceTrackBar;
    TEdit *BackTraceWidget;
    TUpDown *BackTraceWidgetUpDown;
    TTabSheet *GeneralParametersTabSheet;
    TPanel *GeneralParametersPanel;
    TGroupBox *MaxStreamGroupBox;
    TTrackBar *MaxStreamTrackBar;
    TEdit *MaxStreamWidget;
    TUpDown *MaxStreamWidgetUpDown;
    TGroupBox *SpeedVarianceGroupBox;
    TTrackBar *SpeedVarianceTrackBar;
    TEdit *SpeedVarianceWidget;
    TUpDown *SpeedVarianceWidgetUpDown;
    TGroupBox *RefreshTimeGroupBox;
    TTrackBar *RefreshTimeTrackBar;
    TEdit *RefreshTimeWidget;
    TUpDown *RefreshTimeWidgetUpDown;
    TMenuItem *Homepage;
    TMenuItem *SupportRequests;
    TMenuItem *BugReports;
    TMenuItem *FeatureRequests;
    TMenuItem *DiscussionForums;
    TMenuItem *ProjectSummaryPage;
    TMenuItem *Donate;
    TPanel *ColorContainerPanel;
    TPanel *LeadColorPanel;
    TPanel *TrailColorPanel;
    TLabel *FadeColorLabel;
    TButton *FadeColorSelectionButton;
    TColorDialog *FadeColorDialog;
    TGroupBox *BackgroundGroupBox;
    TPanel *BGModePanel;
    TLabel *BGModeLabel;
    TRadioButton *BitmapBGModeRadioButton;
    TRadioButton *ColorBGModeRadioButton;
    TPanel *BGOpacityPanel;
    TLabel *BGOpacityLabel;
    TRadioButton *TransparentBGRadioButton;
    TRadioButton *OpaqueBGRadioButton;
    TPanel *BGBlendModePanel;
    TLabel *BGBlendModeLabel;
    TRadioButton *XORBGBlendModeRadioButton;
    TRadioButton *ORBGBlendModeRadioButton;
    TRadioButton *ANDBGBlendModeRadioButton;
    TMenuItem *LoadDefaults;
    TGroupBox *PriorityGroupBox;
    TPanel *PriorityPanel;
    TRadioButton *HighPriorityRadioButton;
    TRadioButton *AboveNormalPriorityRadioButton;
    TRadioButton *NormalPriorityRadioButton;
    TRadioButton *BelowNormalPriorityRadioButton;
    TRadioButton *IdlePriorityRadioButton;
    TMenuItem *N1;
    TMenuItem *N2;
    TMenuItem *LaunchHelp;
    TEdit *SpecialFontNameWidget;
    TLabel *SpecialFontLabel;
    TButton *SpecialFontSelectionButton;
    TGroupBox *SpecialColorsGroupBox;
    TPanel *SpecialColorContainerPanel;
    TPanel *SpecialLeadColorPanel;
    TPanel *SpecialTrailColorPanel;
    TLabel *SpecialColorLabel;
    TButton *SpecialColorSelectionButton;
    TLabel *SpecialFadeColorLabel;
    TButton *SpecialFadeColorSelectionButton;
    TColorDialog *SpecialFadeColorDialog;
    TColorDialog *SpecialForegroundColorDialog;
    TFontDialog *SpecialFontDialog;
    TGroupBox *SpecialStreamProbabilityGroupBox;
    TTrackBar *SpecialStreamProbabilityTrackBar;
    TEdit *SpecialStreamProbabilityWidget;
    TUpDown *SpecialStreamProbabilityWidgetUpDown;
    TMenuItem *HireTheDeveloper;
    TButton *SpecialColorsCopyButton;
    void __fastcall MaxStreamTrackBarChange(TObject *Sender);
    void __fastcall MaxStreamWidgetChange(TObject *Sender);
    void __fastcall FontSelectionButtonClick(TObject *Sender);
    void __fastcall SpecialFontSelectionButtonClick(TObject *Sender);
    void __fastcall ForegroundColorSelectionButtonClick(TObject *Sender);
    void __fastcall FadeColorSelectionButtonClick(TObject *Sender);
    void __fastcall BackgroundColorSelectionButtonClick(TObject *Sender);
    void __fastcall SpecialForegroundColorSelectionButtonClick(TObject *Sender);
    void __fastcall SpecialFadeColorSelectionButtonClick(TObject *Sender);
    void __fastcall ColorBGModeRadioButtonClick(TObject *Sender);
    void __fastcall BitmapBGModeRadioButtonClick(TObject *Sender);
    void __fastcall XORBGBlendModeRadioButtonClick(TObject *Sender);
    void __fastcall ORBGBlendModeRadioButtonClick(TObject *Sender);
    void __fastcall ANDBGBlendModeRadioButtonClick(TObject *Sender);
    void __fastcall TransparentBGRadioButtonClick(TObject *Sender);
    void __fastcall OpaqueBGRadioButtonClick(TObject *Sender);
    void __fastcall SpeedVarianceTrackBarChange(TObject *Sender);
    void __fastcall SpeedVarianceWidgetChange(TObject *Sender);
    void __fastcall LeadingTrackBarChange(TObject *Sender);
    void __fastcall LeadingWidgetChange(TObject *Sender);
    void __fastcall BackTraceTrackBarChange(TObject *Sender);
    void __fastcall BackTraceWidgetChange(TObject *Sender);
    void __fastcall SpacePadTrackBarChange(TObject *Sender);
    void __fastcall SpacePadWidgetChange(TObject *Sender);
    void __fastcall EditCharSetButtonClick(TObject *Sender);
    void __fastcall ExitClick(TObject *Sender);
    void __fastcall SaveClick(TObject *Sender);
    void __fastcall LoadClick(TObject *Sender);
    void __fastcall LoadDefaultsClick(TObject *Sender);
    void __fastcall AboutClick(TObject *Sender);
    void __fastcall ReadMeClick(TObject *Sender);
    void __fastcall MonotonousCleanupEnabledCheckBoxClick(TObject *Sender);
    void __fastcall RandomizedCleanupEnabledCheckBoxClick(TObject *Sender);
    void __fastcall RefreshTimeTrackBarChange(TObject *Sender);
    void __fastcall RefreshTimeWidgetChange(TObject *Sender);
    void __fastcall SpecialStreamProbabilityTrackBarChange(TObject *Sender);
    void __fastcall SpecialStreamProbabilityWidgetChange(TObject *Sender);
    void __fastcall HighPriorityRadioButtonClick(TObject *Sender);
    void __fastcall AboveNormalPriorityRadioButtonClick(TObject *Sender);
    void __fastcall NormalPriorityRadioButtonClick(TObject *Sender);
    void __fastcall BelowNormalPriorityRadioButtonClick(TObject *Sender);
    void __fastcall IdlePriorityRadioButtonClick(TObject *Sender);
    void __fastcall HomepageClick(TObject *Sender);
    void __fastcall ProjectSummaryPageClick(TObject *Sender);
    void __fastcall SupportRequestsClick(TObject *Sender);
    void __fastcall BugReportsClick(TObject *Sender);
    void __fastcall FeatureRequestsClick(TObject *Sender);
    void __fastcall DiscussionForumsClick(TObject *Sender);
    void __fastcall DonateClick(TObject *Sender);
    void __fastcall LaunchHelpClick(TObject *Sender);
    void __fastcall HireTheDeveloperClick(TObject *Sender);
    void __fastcall SpecialColorsCopyButtonClick(TObject *Sender);
private:	// User declarations
    IzsMatrix *TargetMatrix;
    unsigned int *TargetRefreshTime;
    DWORD *TargetPriority;

public:		// User declarations
    __fastcall TConfigurationForm(TComponent* Owner,IzsMatrix *NewTargetMatrix,unsigned int *NewTargetRefreshTime,DWORD *NewTargetPriority);
    __fastcall ~TConfigurationForm(void);

    void SetFormMaxStream(int NewFormMaxStream);
    void SetFormSpeedVariance(int NewFormSpeedVariance);
    void SetFormMonotonousCleanupEnabled(bool Enabled);
    void SetFormBackTrace(int NewFormBackTrace);
    void SetFormRandomizedCleanupEnabled(bool Enabled);
    void SetFormLeading(int NewFormLeading);
    void SetFormSpacePad(int NewFormSpacePad);
    void SetFormFont(HFONT hFont);
    void SetFormSpecialFont(HFONT hFont);
    void SetFormForegroundColor(BYTE R,BYTE G,BYTE B,BYTE A);
    void SetFormFadeColor(BYTE R,BYTE G,BYTE B,BYTE A);
    void SetFormBackgroundColor(BYTE R,BYTE G,BYTE B,BYTE A);
    void SetFormSpecialForegroundColor(BYTE R,BYTE G,BYTE B,BYTE A);
    void SetFormSpecialFadeColor(BYTE R,BYTE G,BYTE B,BYTE A);
    void SetFormBGMode(TBGMode NewBGMode);
    void SetFormBlendingMode(TBlendMode NewBlendMode);
    void SetFormRefreshTime(unsigned int NewFormRefreshTime);
    void SetFormSpecialStreamProbability(float SpecialStreamProbability);
    void SetFormPriority(DWORD NewFormPriority);
    void DisableLeadingConfig(void);
    void EnableLeadingConfig(void);
    void DisableBackTraceConfig(void);
    void EnableBackTraceConfig(void);
    void DisableSpacePadConfig(void);
    void EnableSpacePadConfig(void);
    void DisableBlendingConfig(void);
    void EnableBlendingConfig(void);

    void UpdateFormBasedOnTarget(void);

    void LaunchURL(const _TCHAR *URL);
};


extern "C" __declspec(dllexport) void  __stdcall LaunchAboutForm(void *Parent);

extern "C" __declspec(dllexport) void  __stdcall LaunchHireForm(void *Parent);

extern "C" __declspec(dllexport) int  __stdcall LaunchConfigForm(IzsMatrix *Matrix,unsigned int &RefreshTime,DWORD &Priority);
extern "C" __declspec(dllexport) int  __stdcall SaveConfigToFile(IzsMatrix *Matrix,unsigned int RefreshTime,DWORD Priority,_TCHAR *FileName);
extern "C" __declspec(dllexport) int  __stdcall LoadConfigFromFile(IzsMatrix *Matrix,unsigned int &RefreshTime,DWORD &Priority,_TCHAR *FileName);


int WriteLogFontToConfigFile(TCustomIniFile *OutFile,const AnsiString SectionName,const LOGFONT &LogFont);
int ReadLogFontFromConfigFile(TCustomIniFile *InFile,const AnsiString SectionName,LOGFONT &LogFont);

int ConvertCharSetToAnsiString(const _TCHAR *CharSet,const unsigned int &CharSetSize,AnsiString &OutString);
int ConvertCharSetToWideString(const _TCHAR *CharSet,const unsigned int &CharSetSize,WideString &OutString);
int ConvertAnsiStringToCharSet(std::vector<_TCHAR> &CharSet,const AnsiString &InString);
int ConvertWideStringToCharSet(std::vector<_TCHAR> &CharSet,const WideString &InString);

int ConvertSpecialStringsToAnsiString(const std::vector<tstring> &SpecialStrings,AnsiString &OutString, _TCHAR Delimiter);
int ConvertSpecialStringsToWideString(const std::vector<tstring> &SpecialStrings,WideString &OutString, _TCHAR Delimiter);
int ConvertAnsiStringToSpecialStrings(std::vector<tstring> &SpecialStrings,const AnsiString &InString, _TCHAR Delimiter);
int ConvertWideStringToSpecialStrings(std::vector<tstring> &SpecialStrings,const WideString &InString, _TCHAR Delimiter);


//---------------------------------------------------------------------------
extern PACKAGE TConfigurationForm *ConfigurationForm;
//---------------------------------------------------------------------------
#endif
