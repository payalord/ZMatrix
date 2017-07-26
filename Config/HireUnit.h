//---------------------------------------------------------------------------

#ifndef HireUnitH
#define HireUnitH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "SHDocVw_OCX.h"
#include <ExtCtrls.hpp>
#include <Graphics.hpp>
#include <OleCtrls.hpp>
//---------------------------------------------------------------------------
class THireForm : public TForm
{
__published:	// IDE-managed Components
    TCppWebBrowser *CommentsBrowser;
    TImage *CodeImage;
    TButton *OKButton;
    void __fastcall FormShow(TObject *Sender);
private:	// User declarations
public:		// User declarations
    __fastcall THireForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE THireForm *HireForm;
//---------------------------------------------------------------------------
#endif
