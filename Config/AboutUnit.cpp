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
#include <vector>
#include <stdio.h>
#include "../globals.h"
#include "ConfigForm.h"
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
    // Use the wide Windows API directly; a VCL file name must not pass through ANSI.
    const WideString FileName = Application->ExeName;
    DWORD UnusedHandle = 0;
    const DWORD Size = GetFileVersionInfoSizeW(FileName.c_bstr(), &UnusedHandle);
    std::vector<BYTE> VersionData(Size);
    if(Size != 0 && GetFileVersionInfoW(FileName.c_bstr(), 0, Size, &VersionData[0]))
    {
        VS_FIXEDFILEINFO *Version = NULL;
        unsigned int VersionSize = 0;
        if(VerQueryValueW(&VersionData[0], L"\\", (void **)&Version, &VersionSize) &&
           VersionSize >= sizeof(*Version) && Version->dwSignature == 0xFEEF04BD)
        {
            char Buffer[128];
            snprintf(Buffer, sizeof(Buffer), "Version %u.%u.%u.%u",
                HIWORD(Version->dwFileVersionMS), LOWORD(Version->dwFileVersionMS),
                HIWORD(Version->dwFileVersionLS), LOWORD(Version->dwFileVersionLS));
            VersionLabel->Caption = Buffer;
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

