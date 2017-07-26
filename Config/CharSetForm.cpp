/*!=========================================================================
//
//  Program:   ZMatrix
//  Module:    $RCSfile: CharSetForm.cpp,v $
//  Language:  C/C++
//  Date:      $Date: 2003/05/13 00:49:52 $
//  Version:   $Revision: 1.6 $
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

#include <stdlib.h>
#include <stdio.h>
#include "CharSetForm.h"
#include "ConfigForm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TCharacterSetForm *CharacterSetForm = NULL;

#ifdef UNICODE
    #define CODEPAGE 1200
#else
    #define CODEPAGE CP_ACP
#endif

//---------------------------------------------------------------------------
void TCharacterSetForm::SetFormCharSet(_TCHAR *Chars,int NumChars)
{
#ifdef UNICODE
    WideString TempString;
    ConvertCharSetToWideString(Chars,NumChars,TempString);
#else
    AnsiString TempString;
    ConvertCharSetToAnsiString(Chars,NumChars,TempString);
#endif

    SETTEXTEX TextStruct;
    memset(&TextStruct,0,sizeof(SETTEXTEX));
    TextStruct.flags = ST_DEFAULT;
    TextStruct.codepage = CODEPAGE;

#ifdef UNICODE
    SendMessage(this->CharacterSetEditField->Handle,EM_SETTEXTEX,(WPARAM)&TextStruct,(LPARAM)TempString.c_bstr());
#else
    this->CharacterSetEditField->Text = TempString;
    //SendMessage(this->CharacterSetEditField->Handle,EM_SETTEXTEX,(WPARAM)&TextStruct,(LPARAM)TempString.c_str());
#endif

}
//---------------------------------------------------------------------------
const std::vector<_TCHAR> *TCharacterSetForm::GetFormCharSet(void)
{
    std::vector <BYTE> Buffer;
    GETTEXTLENGTHEX TextLengthStruct;
    memset(&TextLengthStruct,0,sizeof(GETTEXTLENGTHEX));
    TextLengthStruct.flags = GT_DEFAULT;
    TextLengthStruct.codepage = CODEPAGE;

    LRESULT Result = SendMessage(this->CharacterSetEditField->Handle,EM_GETTEXTLENGTHEX,(WPARAM)&TextLengthStruct,(LPARAM)0);

    Buffer.resize(sizeof(_TCHAR)*(Result + 1));

    GETTEXTEX TextStruct;
    memset(&TextStruct,0,sizeof(GETTEXTEX));
    TextStruct.cb = Buffer.size();
    TextStruct.flags = GT_DEFAULT;
    TextStruct.codepage = CODEPAGE;

    SendMessage(this->CharacterSetEditField->Handle,EM_GETTEXTEX,(WPARAM)&TextStruct,(LPARAM)&(Buffer.front()));

    if(Buffer.size()/sizeof(_TCHAR))
    {
#ifdef UNICODE
        WideString TempString = (wchar_t *)&(Buffer.front());
        ConvertWideStringToCharSet(this->CurrentCharSet,TempString);
#else
        AnsiString TempString = this->CharacterSetEditField->Text;
        ConvertAnsiStringToCharSet(this->CurrentCharSet,TempString);
        //AnsiString TempString = (char *)&(Buffer.front());
        //ConvertAnsiStringToCharSet(this->CurrentCharSet,TempString);
#endif
    }
    else
    {
        this->CurrentCharSet.clear();
    }

    return &(this->CurrentCharSet);
}
//---------------------------------------------------------------------------
void TCharacterSetForm::SetFormSpecialStrings(const std::vector<tstring> &SpecialStrings)
{
#ifdef UNICODE
    WideString TempString;
    ConvertSpecialStringsToWideString(SpecialStrings,TempString,EDIT_FIELD_SPECIAL_STRING_DELIMITER);
#else
    AnsiString TempString;
    ConvertSpecialStringsToAnsiString(SpecialStrings,TempString,EDIT_FIELD_SPECIAL_STRING_DELIMITER);
#endif

    SETTEXTEX TextStruct;
    memset(&TextStruct,0,sizeof(SETTEXTEX));
    TextStruct.flags = ST_DEFAULT;
    TextStruct.codepage = CODEPAGE;

#ifdef UNICODE
    SendMessage(this->SpecialStringsEditField->Handle,EM_SETTEXTEX,(WPARAM)&TextStruct,(LPARAM)TempString.c_bstr());
#else
    this->SpecialStringsEditField->Text = TempString;
    //SendMessage(this->CharacterSetEditField->Handle,EM_SETTEXTEX,(WPARAM)&TextStruct,(LPARAM)TempString.c_str());
#endif
}
//---------------------------------------------------------------------------
void TCharacterSetForm::GetFormSpecialStrings(std::vector<tstring> &SpecialStrings)
{
    std::vector <BYTE> Buffer;
    GETTEXTLENGTHEX TextLengthStruct;
    memset(&TextLengthStruct,0,sizeof(GETTEXTLENGTHEX));
    TextLengthStruct.flags = GT_DEFAULT;
    TextLengthStruct.codepage = CODEPAGE;

    LRESULT Result = SendMessage(this->SpecialStringsEditField->Handle,EM_GETTEXTLENGTHEX,(WPARAM)&TextLengthStruct,(LPARAM)0);

    Buffer.resize(sizeof(_TCHAR)*(Result + 1));

    GETTEXTEX TextStruct;
    memset(&TextStruct,0,sizeof(GETTEXTEX));
    TextStruct.cb = Buffer.size();
    TextStruct.flags = GT_DEFAULT;
    TextStruct.codepage = CODEPAGE;

    SendMessage(this->SpecialStringsEditField->Handle,EM_GETTEXTEX,(WPARAM)&TextStruct,(LPARAM)&(Buffer.front()));

    if(Buffer.size()/sizeof(_TCHAR))
    {
#ifdef UNICODE
        WideString TempString = (wchar_t *)&(Buffer.front());
        ConvertWideStringToSpecialStrings(SpecialStrings,TempString,EDIT_FIELD_SPECIAL_STRING_DELIMITER);
#else
        AnsiString TempString = this->SpecialStringsEditField->Text;
        ConvertAnsiStringToSpecialStrings(SpecialStrings,TempString,EDIT_FIELD_SPECIAL_STRING_DELIMITER);
        //AnsiString TempString = (char *)&(Buffer.front());
        //ConvertAnsiStringToCharSet(this->CurrentCharSet,TempString);
#endif
    }
    else
    {
        SpecialStrings.clear();
    }
}
//---------------------------------------------------------------------------
__fastcall TCharacterSetForm::TCharacterSetForm(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TCharacterSetForm::LoadCharacterSetButtonClick(
      TObject *Sender)
{
    if(this->OpenCharSetDialog->Execute())
    {
        FILE *InFile = fopen(this->OpenCharSetDialog->FileName.c_str(),"rt");


        AnsiString InString;
        if(InFile)
        {
            const unsigned int BufferSize = 128;
            char Buffer[BufferSize];
            memset(Buffer,0,BufferSize);

            while(1 == fread(Buffer,BufferSize - 1,1,InFile))
            {
                InString += Buffer;
                memset(Buffer,0,BufferSize);
            }

            InString += Buffer;

            std::vector<_TCHAR> TempCharSet;
            ConvertAnsiStringToCharSet(TempCharSet,InString);
#ifdef UNICODE
            WideString TempString;
            ConvertCharSetToWideString((const _TCHAR *)&(TempCharSet.front()),TempCharSet.size(),TempString);


            SETTEXTEX TextStruct;
            memset(&TextStruct,0,sizeof(SETTEXTEX));
            TextStruct.flags = ST_DEFAULT;
            TextStruct.codepage = CODEPAGE;

            SendMessage(this->CharacterSetEditField->Handle,EM_SETTEXTEX,(WPARAM)&TextStruct,(LPARAM)TempString.c_bstr());

#else
            AnsiString TempString;
            ConvertCharSetToAnsiString((const _TCHAR *)&(TempCharSet.front()),TempCharSet.size(),TempString);

            this->CharacterSetEditField->Text = TempString;
/*
            SETTEXTEX TextStruct;
            memset(&TextStruct,0,sizeof(SETTEXTEX));
            TextStruct.flags = ST_DEFAULT;
            TextStruct.codepage = CODEPAGE;

            SendMessage(this->CharacterSetEditField->Handle,EM_SETTEXTEX,(WPARAM)&TextStruct,(LPARAM)TempString.c_str());
*/
#endif


            fclose(InFile);
        }
        else
        {
            MessageBox(this->Handle,_T("Failed to open selected file location for reading"),_T("Error"),MB_OK|MB_ICONERROR);
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TCharacterSetForm::SaveCharacterSetButtonClick(
      TObject *Sender)
{
    if(this->SaveCharSetDialog->Execute())
    {
        FILE *OutFile = fopen(this->SaveCharSetDialog->FileName.c_str(),"wt");

        if(OutFile)
        {
            std::vector <BYTE> Buffer;
            GETTEXTLENGTHEX TextLengthStruct;
            memset(&TextLengthStruct,0,sizeof(GETTEXTLENGTHEX));
            TextLengthStruct.flags = GT_USECRLF;
            TextLengthStruct.codepage = CODEPAGE;

            LRESULT Result = SendMessage(this->CharacterSetEditField->Handle,EM_GETTEXTLENGTHEX,(WPARAM)&TextLengthStruct,(LPARAM)0);

            Buffer.resize(sizeof(_TCHAR)*(Result + 1));

            GETTEXTEX TextStruct;
            memset(&TextStruct,0,sizeof(GETTEXTEX));
            TextStruct.cb = Buffer.size();
            TextStruct.flags = GT_USECRLF;
            TextStruct.codepage = CODEPAGE;

            SendMessage(this->CharacterSetEditField->Handle,EM_GETTEXTEX,(WPARAM)&TextStruct,(LPARAM)&(Buffer.front()));
/*
            #ifdef UNICODE
            const BYTE BOM[] = {0xFF,0xFE};
            fwrite(BOM,1,2,OutFile);
            #endif

            _ftprintf(OutFile,_T("%s"),&(Buffer.front()));
*/
#ifdef UNICODE
            std::vector<_TCHAR> TempCharSet;
            WideString TempWideString = (const _TCHAR *)&(Buffer.front());
            ConvertWideStringToCharSet(TempCharSet,TempWideString);
#else
            std::vector<_TCHAR> TempCharSet;

            AnsiString TempAnsiString = this->CharacterSetEditField->Text;
            //AnsiString TempAnsiString = (const _TCHAR *)&(Buffer.front());

            ConvertAnsiStringToCharSet(TempCharSet,TempAnsiString);
#endif


            AnsiString OutString;
            ConvertCharSetToAnsiString((const _TCHAR *)&(TempCharSet.front()),TempCharSet.size(),OutString);

            fprintf(OutFile,"%s",OutString.c_str());

            fclose(OutFile);
        }
        else
        {
            MessageBox(this->Handle,_T("Failed to open selected file location for writing"),_T("Error"),MB_OK|MB_ICONERROR);
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TCharacterSetForm::LoadSpecialStringsButtonClick(
      TObject *Sender)
{
    if(this->OpenSpecialStringsDialog->Execute())
    {
        FILE *InFile = fopen(this->OpenSpecialStringsDialog->FileName.c_str(),"rt");


        AnsiString InString;
        if(InFile)
        {
            const unsigned int BufferSize = 128;
            char Buffer[BufferSize];
            memset(Buffer,0,BufferSize);

            while(1 == fread(Buffer,BufferSize - 1,1,InFile))
            {
                InString += Buffer;
                memset(Buffer,0,BufferSize);
            }

            InString += Buffer;

            std::vector<tstring> TempSpecialStrings;
            ConvertAnsiStringToSpecialStrings(TempSpecialStrings,InString,TEXT_FILE_SPECIAL_STRING_DELIMITER);
#ifdef UNICODE
            WideString TempString;
            ConvertSpecialStringsToWideString(TempSpecialStrings,TempString,EDIT_FIELD_SPECIAL_STRING_DELIMITER);


            SETTEXTEX TextStruct;
            memset(&TextStruct,0,sizeof(SETTEXTEX));
            TextStruct.flags = ST_DEFAULT;
            TextStruct.codepage = CODEPAGE;

            SendMessage(this->SpecialStringsEditField->Handle,EM_SETTEXTEX,(WPARAM)&TextStruct,(LPARAM)TempString.c_bstr());

#else
            AnsiString TempString;
            ConvertSpecialStringsToAnsiString(TempSpecialStrings,TempString,EDIT_FIELD_SPECIAL_STRING_DELIMITER);

            this->SpecialStringsEditField->Text = TempString;

#endif


            fclose(InFile);
        }
        else
        {
            MessageBox(this->Handle,_T("Failed to open selected file location for reading"),_T("Error"),MB_OK|MB_ICONERROR);
        }
    }
}
//---------------------------------------------------------------------------

void __fastcall TCharacterSetForm::SaveSpecialStringsButtonClick(
      TObject *Sender)
{
    if(this->SaveSpecialStringsDialog->Execute())
    {
        FILE *OutFile = fopen(this->SaveSpecialStringsDialog->FileName.c_str(),"wt");

        if(OutFile)
        {
            std::vector <BYTE> Buffer;
            GETTEXTLENGTHEX TextLengthStruct;
            memset(&TextLengthStruct,0,sizeof(GETTEXTLENGTHEX));
            TextLengthStruct.flags = GT_USECRLF;
            TextLengthStruct.codepage = CODEPAGE;

            LRESULT Result = SendMessage(this->SpecialStringsEditField->Handle,EM_GETTEXTLENGTHEX,(WPARAM)&TextLengthStruct,(LPARAM)0);

            Buffer.resize(sizeof(_TCHAR)*(Result + 1));

            GETTEXTEX TextStruct;
            memset(&TextStruct,0,sizeof(GETTEXTEX));
            TextStruct.cb = Buffer.size();
            TextStruct.flags = GT_USECRLF;
            TextStruct.codepage = CODEPAGE;

            SendMessage(this->SpecialStringsEditField->Handle,EM_GETTEXTEX,(WPARAM)&TextStruct,(LPARAM)&(Buffer.front()));
/*
            #ifdef UNICODE
            const BYTE BOM[] = {0xFF,0xFE};
            fwrite(BOM,1,2,OutFile);
            #endif

            _ftprintf(OutFile,_T("%s"),&(Buffer.front()));
*/
#ifdef UNICODE
            std::vector<tstring> TempSpecialStrings;
            WideString TempWideString = (const _TCHAR *)&(Buffer.front());
            ConvertWideStringToSpecialStrings(TempSpecialStrings,TempWideString,EDIT_FIELD_SPECIAL_STRING_DELIMITER);
#else
            std::vector<tstring> TempSpecialStrings;

            AnsiString TempAnsiString = this->SpecialStringsEditField->Text;
            //AnsiString TempAnsiString = (const _TCHAR *)&(Buffer.front());

            ConvertAnsiStringToSpecialStrings(TempSpecialStrings,TempAnsiString,EDIT_FIELD_SPECIAL_STRING_DELIMITER);
#endif


            AnsiString OutString;
            ConvertSpecialStringsToAnsiString(TempSpecialStrings,OutString,TEXT_FILE_SPECIAL_STRING_DELIMITER);

            fprintf(OutFile,"%s",OutString.c_str());

            fclose(OutFile);
        }
        else
        {
            MessageBox(this->Handle,_T("Failed to open selected file location for writing"),_T("Error"),MB_OK|MB_ICONERROR);
        }
    }
}
//---------------------------------------------------------------------------

