/*!=========================================================================
//
//  Program:   ZMatrix
//  Module:    $RCSfile: CharSetForm.h,v $
//  Language:  C/C++
//  Date:      $Date: 2003/05/05 01:29:24 $
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
#ifndef CharSetFormH
#define CharSetFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <tchar.h>
#include <Dialogs.hpp>
#include <ComCtrls.hpp>
#include <vector>

#include "../globals.h"

//---------------------------------------------------------------------------
class TCharacterSetForm : public TForm
{
__published:	// IDE-managed Components
    TButton *OKButton;
    TButton *CancelButton;
    TOpenDialog *OpenCharSetDialog;
    TSaveDialog *SaveCharSetDialog;
    TGroupBox *CharacterSetGroupBox;
    TRichEdit *CharacterSetEditField;
    TButton *LoadCharacterSetButton;
    TButton *SaveCharacterSetButton;
    TGroupBox *SpecialStringsGroupBox;
    TRichEdit *SpecialStringsEditField;
    TButton *LoadSpecialStringsButton;
    TButton *SaveSpecialStringsButton;
    TSaveDialog *SaveSpecialStringsDialog;
    TOpenDialog *OpenSpecialStringsDialog;
    void __fastcall LoadCharacterSetButtonClick(TObject *Sender);
    void __fastcall SaveCharacterSetButtonClick(TObject *Sender);
    void __fastcall LoadSpecialStringsButtonClick(TObject *Sender);
    void __fastcall SaveSpecialStringsButtonClick(TObject *Sender);
private:	// User declarations
    std::vector<_TCHAR> CurrentCharSet;
public:		// User declarations
    __fastcall TCharacterSetForm(TComponent* Owner);

    void SetFormCharSet(_TCHAR *Chars,int NumChars);
    const std::vector<_TCHAR> *GetFormCharSet(void);

    void SetFormSpecialStrings(const std::vector<tstring> &SpecialStrings);
    void GetFormSpecialStrings(std::vector<tstring> &SpecialStrings);
};
//---------------------------------------------------------------------------
extern PACKAGE TCharacterSetForm *CharacterSetForm;
//---------------------------------------------------------------------------
#endif
