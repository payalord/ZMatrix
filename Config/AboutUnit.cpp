/*!=========================================================================
//
//  Program:   ZMatrix
//  Module:    $RCSfile: AboutUnit.cpp,v $
//  Language:  C/C++
//  Date:      $Date: 2003/01/28 07:36:38 $
//  Version:   $Revision: 1.5 $
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
#include <vcl.h>
#pragma hdrstop

#include <mmsystem.h>
#include "AboutUnit.h"
#include "resource.h"
#include <tchar.h>
#include <string>
#include <stdio.h>
#include "../globals.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "SHDocVw_OCX"
#pragma resource "*.dfm"
TAboutForm *AboutForm = NULL;
extern HINSTANCE ghInst;
//---------------------------------------------------------------------------
__fastcall TAboutForm::TAboutForm(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TAboutForm::FormShow(TObject *Sender)
{

    void* VersionData = NULL;
    try
    {
        DWORD dwResult;
        bool bResult;
        unsigned int uiResult;
        DWORD EmptyHandle;
        char* Build = NULL;
        std::basic_string<TCHAR> TempString;

        TempString.resize(Application->ExeName.Length() + 1);
#ifdef UNICODE
        Application->ExeName.WideChar(&(TempString[0]),TempString.size());
#else
        TempString = Application->ExeName.c_str();
#endif
        dwResult = GetFileVersionInfoSize(&(TempString[0]),&EmptyHandle);

        VersionData = new BYTE [dwResult];

        bResult = GetFileVersionInfo(&(TempString[0]), 0,dwResult, VersionData);


        if(bResult)
        {
            char TempBuffer[128];
            DWORD *pdwLang;
            unsigned int LangSize;

            VerQueryValue(VersionData,
                          TEXT("\\VarFileInfo\\Translation"),
                          (void **)&pdwLang,
                          &LangSize);


            TCHAR szSubBlock[512];   // Version language and section definition
            _stprintf(szSubBlock, _T("\\StringFileInfo\\%04hX%04hX\\FileVersion"),
                      LOWORD(*pdwLang), HIWORD(*pdwLang));

            VerQueryValue(VersionData,
                          szSubBlock, (void**)&Build,
                          &uiResult);
#ifdef UNICODE
            snprintf(TempBuffer,128,"Version %ls (UNICODE)",Build);
#else
            snprintf(TempBuffer,128,"Version %s (ASCII)",Build);
#endif

            VersionLabel->Caption = TempBuffer;
        }
    }
    __finally
    {
        if(VersionData != NULL)
        {
            delete VersionData;
        }
    }


    PlaySound(MAKEINTRESOURCE(ID_HAPPYWAV),ghInst,SND_RESOURCE | SND_ASYNC );

    CommentsBrowser->Navigate(L"res://Config.dll/comments.html");

}
//---------------------------------------------------------------------------
void __fastcall TAboutForm::DonateButtonClick(TObject *Sender)
{
    this->LaunchURL(DONATE_URL);    
}
//---------------------------------------------------------------------------
void TAboutForm::LaunchURL(const _TCHAR *URL)
{
    ShellExecute(this->Handle,_TEXT("open"),URL, NULL, NULL, SW_SHOWNORMAL );
}
//---------------------------------------------------------------------------

