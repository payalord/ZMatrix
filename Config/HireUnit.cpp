//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "HireUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "SHDocVw_OCX"
#pragma resource "*.dfm"
THireForm *HireForm;
//---------------------------------------------------------------------------
__fastcall THireForm::THireForm(TComponent* Owner)
    : TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall THireForm::FormShow(TObject *Sender)
{
    CommentsBrowser->Navigate(L"res://Config.dll/hire.html");
}
//---------------------------------------------------------------------------
