/*!=========================================================================
//
//  Program:   ZMatrix
//  Module:    $RCSfile: zsMatrix.h,v $
//  Language:  C/C++
//  Date:      $Date: 2003/05/08 05:24:27 $
//  Version:   $Revision: 1.12 $
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


#ifndef __zsMatrix_h
#define __zsMatrix_h



#include <windows.h>
#include <vector>
#include <tchar.h>
#include "IzsMatrix.h"
//#include "../globals.h"

using namespace std;
extern unsigned int g_dwRefCount;

class zsMatrixStream : public IzsMatrixStream
{
public:
	zsMatrixStream(const IzsMatrixStream &Other)
		:StartX(Other.GetStartX()),StartY(Other.GetStartY()),TickCounter(Other.GetTickCounter()),
		TicksToWait(Other.GetTicksToWait()),Status(Other.GetStatus()),
		SpecialStreamFlag(Other.GetSpecialStreamFlag()),
		SpecialStreamStringIndex(Other.GetSpecialStreamStringIndex()),
		SpecialStreamStringCharIndex(Other.GetSpecialStreamStringCharIndex()),
		NeedsDrawing(Other.GetNeedsDrawing()),
		RefCount(0)
	{
	}

	zsMatrixStream(void)
	{
		this->StartX = 0;
		this->StartY = 0;
		this->TickCounter = 0;
		this->TicksToWait = 0;
		this->Status = false;
		this->SpecialStreamFlag = false;
		this->SpecialStreamStringIndex = 0;
		this->SpecialStreamStringCharIndex = this->GetSpecialStreamStringInvalidCharIndex();
		this->NeedsDrawing = true;
		this->RefCount = 0;
	}

	IzsMatrixStream &__stdcall operator=(const IzsMatrixStream &Other)
	{
		this->StartX = Other.GetStartX();
		this->StartY = Other.GetStartY();
		this->TickCounter = Other.GetTickCounter();
		this->TicksToWait = Other.GetTicksToWait();
		this->Status = Other.GetStatus();
		this->SpecialStreamFlag = Other.GetSpecialStreamFlag();
		this->SpecialStreamStringIndex = Other.GetSpecialStreamStringIndex();
		this->SpecialStreamStringCharIndex = Other.GetSpecialStreamStringCharIndex();
		this->NeedsDrawing = Other.GetNeedsDrawing();

		return *this;
	}

	void __stdcall CopyFrom(const IzsMatrixStream &Other)
	{
		this->operator=(Other);
	}

	void __stdcall SetStartX(int NewStartX){StartX = NewStartX;};
	int __stdcall GetStartX(void) const {return StartX;};

	void __stdcall SetStartY(int NewStartY){StartY = NewStartY;};
	int __stdcall GetStartY(void) const {return StartY;};

	void _stdcall SetTickCounter(unsigned int NewTicks){this->TickCounter = NewTicks;};
	unsigned int _stdcall GetTickCounter(void) const {return TickCounter;};

	void _stdcall SetTicksToWait(unsigned int NewTicks){this->TicksToWait = NewTicks;};
	unsigned int _stdcall GetTicksToWait(void) const {return TicksToWait;};

	void __stdcall SetStatus(bool NewStatus){Status = NewStatus;};
	bool __stdcall GetStatus(void) const {return Status;};

	void __stdcall SetSpecialStreamFlag(bool NewSpecialStreamFlag) {SpecialStreamFlag = NewSpecialStreamFlag;};
	bool __stdcall GetSpecialStreamFlag(void) const {return SpecialStreamFlag;};

	void __stdcall SetSpecialStreamStringIndex(unsigned int NewSpecialStreamStringIndex) {SpecialStreamStringIndex = NewSpecialStreamStringIndex;};
	virtual unsigned int __stdcall GetSpecialStreamStringIndex(void) const {return SpecialStreamStringIndex;};

	virtual void __stdcall SetSpecialStreamStringCharIndex(unsigned int NewSpecialStreamStringCharIndex) {SpecialStreamStringCharIndex = NewSpecialStreamStringCharIndex;};
	virtual unsigned int __stdcall GetSpecialStreamStringCharIndex(void) const {return SpecialStreamStringCharIndex;};

	void __stdcall SetNeedsDrawing(bool NewNeedsDrawing){NeedsDrawing = NewNeedsDrawing;};
	bool __stdcall GetNeedsDrawing(void) const {return NeedsDrawing;};

	HRESULT __stdcall QueryInterface(REFIID riid, void** ppObject)
	{
		if (riid==IID_IUnknown || riid==IID_IZSMATRIXSTREAM)
		{
			*ppObject=(IzsMatrixStream*) this;
		}
		else
		{
			return E_NOINTERFACE;
		}
		AddRef();
		return NO_ERROR;
	};

	ULONG  __stdcall AddRef()
	{
		g_dwRefCount++;
		RefCount++;
		return RefCount;
	};

	ULONG   __stdcall Release()
	{
		if(g_dwRefCount > 0)
			g_dwRefCount--;
		if(RefCount > 0)
			RefCount--;
		if (RefCount==0)
		{
			delete this;
			return 0;
		}
		return RefCount;
	};

protected:
	int StartX;
	int StartY;
	unsigned int TickCounter;
	unsigned int TicksToWait;
	bool Status;
	bool SpecialStreamFlag;
	unsigned int SpecialStreamStringIndex;
	unsigned int SpecialStreamStringCharIndex;
	bool NeedsDrawing;
	unsigned int RefCount;

};

EXPIMP_TEMPLATE template class std::vector<_TCHAR>;
EXPIMP_TEMPLATE template class std::vector<IzsMatrixStream *>;

class zsMatrix : public IzsMatrix
{
public:


	zsMatrix();
	zsMatrix(HWND Window,HBITMAP BGBitmap);
	zsMatrix(const zsMatrix &Other);
	zsMatrix(const IzsMatrix &Other);
	~zsMatrix();

	zsMatrix &operator=(const IzsMatrix &Other);


	void __stdcall CopyFrom(const IzsMatrix &Other)
	{
		this->operator=(Other);
	}

	int __stdcall Render(HDC hdc);


	void __stdcall UpdateTarget(HWND hWnd,HBITMAP BGBITMAP);

	void __stdcall SetColor(BYTE R,BYTE G,BYTE B,BYTE A);
	void __stdcall GetColor(BYTE &R,BYTE &G,BYTE &B,BYTE &A) const ;

	void __stdcall SetFadeColor(BYTE R,BYTE G,BYTE B,BYTE A);
	void __stdcall GetFadeColor(BYTE &R,BYTE &G,BYTE &B,BYTE &A) const ;

	void __stdcall SetBGColor(BYTE R,BYTE G,BYTE B,BYTE A);
	void __stdcall GetBGColor(BYTE &R,BYTE &G,BYTE &B,BYTE &A) const ;

	void __stdcall SetBGMode(const TBGMode &NewMode);
	TBGMode __stdcall GetBGMode(void) const;

	void __stdcall SetBlendMode(TBlendMode NewBlendMode);
	TBlendMode __stdcall GetBlendMode(void) const ;

	void __stdcall SetFont(HFONT NewFont);
	void __stdcall SetLogFont(const LOGFONT &NewFont);
	HFONT __stdcall GetFont(void) const ;

	void __stdcall SetBGBitmap(HBITMAP NewBitmap);
	HBITMAP __stdcall SetBGBitmap_NonCopy(HBITMAP NewBitmap)
	{
		HBITMAP RetVal = this->hBGBitmap;
		this->hBGBitmap = NewBitmap;
		//SelectObject(this->hBGDC,this->hBGBitmap);
		//SelectObject(this->hTempSpaceDC,this->hBGBitmap);
		//return RetVal;
		//SelectObject(this->hBGDC,this->hBGBitmap);
		return RetVal;
	}
	HBITMAP __stdcall GetBGBitmap(void) const 
	{
		return this->hBGBitmap;
	}
	HDC __stdcall GetBGDC(void) const
	{
		return this->hBGDC;
	};


	//------------------------------------------------------------
	//Basic stream rendering parameters
	//------------------------------------------------------------

	void __stdcall SetMaxStream(unsigned int NewMax);
	unsigned int __stdcall GetMaxStream(void) const 
	{return this->MaxStream;};

	void __stdcall SetSpeedVariance(unsigned int NewSpeedVariance);
	unsigned int __stdcall GetSpeedVariance(void) const 
	{return this->SpeedVariance;};

	//------------------------------------------------------------
	//Monotonous cleanup parameter(s)
	//------------------------------------------------------------

	void __stdcall SetMonotonousCleanupEnabled(bool NewEnabled);
	bool __stdcall GetMonotonousCleanupEnabled(void) const
	{return this->MonotonousCleanupEnabled;};

	void __stdcall SetBackTrace(unsigned int NewBT);
	unsigned int __stdcall GetBackTrace(void) const 
	{return this->BackTrace;};

	
	//------------------------------------------------------------
	//Randomized cleanup parameter(s)
	//------------------------------------------------------------

	void __stdcall SetRandomizedCleanupEnabled(bool NewEnabled);
	bool __stdcall GetRandomizedCleanupEnabled(void) const
	{return this->RandomizedCleanupEnabled;};

	void __stdcall SetLeading(unsigned int NewLeading);
	unsigned int __stdcall GetLeading(void) const 
	{return this->Leading;};

	void __stdcall SetSpacePad(unsigned int NewSP);
	unsigned int __stdcall GetSpacePad(void) const 
	{return this->SpacePad;};


	//------------------------------------------------------------
	//Special string parameters(s)
	//------------------------------------------------------------

	void __stdcall SetSpecialStringStreamProbability(float NewProbability);
	float __stdcall GetSpecialStringStreamProbability(void) const;

	void __stdcall SetSpecialStringColor(BYTE R,BYTE G,BYTE B,BYTE A);
	void __stdcall GetSpecialStringColor(BYTE &R,BYTE &G,BYTE &B,BYTE &A) const;

	void __stdcall SetSpecialStringFadeColor(BYTE R,BYTE G,BYTE B,BYTE A);
	void __stdcall GetSpecialStringFadeColor(BYTE &R,BYTE &G,BYTE &B,BYTE &A) const;

	void __stdcall SetSpecialStringBGColor(BYTE R,BYTE G,BYTE B,BYTE A);
	void __stdcall GetSpecialStringBGColor(BYTE &R,BYTE &G,BYTE &B,BYTE &A) const;

	void __stdcall SetSpecialStringFont(HFONT NewFont);
	void __stdcall SetSpecialStringLogFont(const LOGFONT &NewFont);
	HFONT __stdcall GetSpecialStringFont(void) const;

	void __stdcall AddSpecialStringToValidSet(const _TCHAR *NewString);
	void __stdcall RemoveSpecialStringFromValidSet(const _TCHAR *String);
	void __stdcall ClearValidSpecialStringSet(void);
	void __stdcall SetValidSpecialStringSet(const _TCHAR * const *NewStringSet,unsigned int Size);
	const _TCHAR * __stdcall GetValidSpecialString(unsigned int Index) const ;
	unsigned int __stdcall GetNumSpecialStringsInSet(void) const ;

	//------------------------------------------------------------
	//Transform Coefficents
	//------------------------------------------------------------

	double __stdcall SetCoeffR1(const double &NewR1Coeff);
	double __stdcall GetCoeffR1(void) const;
	double __stdcall SetCoeffR0(const double &NewR0Coeff);
	double __stdcall GetCoeffR0(void) const;

	double __stdcall SetCoeffG1(const double &NewG1Coeff);
	double __stdcall GetCoeffG1(void) const;
	double __stdcall SetCoeffG0(const double &NewG0Coeff);
	double __stdcall GetCoeffG0(void) const;

	double __stdcall SetCoeffB1(const double &NewB1Coeff);
	double __stdcall GetCoeffB1(void) const;
	double __stdcall SetCoeffB0(const double &NewB0Coeff);
	double __stdcall GetCoeffB0(void) const;

	double __stdcall SetCoeffA1(const double &NewA1Coeff);
	double __stdcall GetCoeffA1(void) const;
	double __stdcall SetCoeffA0(const double &NewA0Coeff);
	double __stdcall GetCoeffA0(void) const;




	void __stdcall SethWnd(HWND hWnd);
	HWND __stdcall GethWnd(void) const 
	{return this->hWnd;};

	void __stdcall AddCharToValidSet(_TCHAR NewChar);
	void __stdcall RemoveCharFromValidSet(_TCHAR Char);
	void __stdcall ClearValidCharSet(void);
	void __stdcall SetValidCharSet(const _TCHAR *NewCharSet,unsigned int Size);
	_TCHAR *__stdcall GetValidCharSet(void);

	const _TCHAR *__stdcall GetValidCharSet(void) const ;
	unsigned int __stdcall GetNumCharsInSet(void) const ;

	unsigned int __stdcall GetNumStreams(void) const;
	IzsMatrixStream** __stdcall GetStreams(void);
	const IzsMatrixStream** __stdcall GetStreams(void) const;

	IzsMatrixStream *__stdcall CreateStream(void) const;
	IzsMatrixStream *__stdcall CreateStream(const IzsMatrixStream &Orig) const;

	HRESULT __stdcall QueryInterface(REFIID riid, void** ppObject)
	{
		if (riid==IID_IUnknown || riid==IID_IZSMATRIX)
		{
			*ppObject=(IzsMatrix*) this;
		}
		else
		{
			return E_NOINTERFACE;
		}
		AddRef();
		return NO_ERROR;
	};

	ULONG  __stdcall AddRef()
	{
		g_dwRefCount++;
		RefCount++;
		return RefCount;
	};

	ULONG   __stdcall Release()
	{
		if(g_dwRefCount > 0)
			g_dwRefCount--;
		if(RefCount > 0)
			RefCount--;
		if (RefCount==0)
		{
			delete this;
			return 0;
		}
		return RefCount;
	};

	vector<_TCHAR> ValidChars;
private:

	unsigned int MaxStream;		// max number of streams to use
	unsigned int SpeedVariance;	// What is the maximum number of cycles to randomly wait for

	bool MonotonousCleanupEnabled;
	unsigned int BackTrace;		// number of characters behind to erase the trail

	bool RandomizedCleanupEnabled;
	unsigned int Leading;		// minumum number of characters to display before erasing begins
	unsigned int SpacePad;		// number of characters to randomly delete from +/- SPACEPAD



	vector<IzsMatrixStream *> Streams;
	unsigned int StreamCount;

	// the color in which to render the streams
	BYTE Color[4];
	COLORREF ColorRef;

	BYTE FadeColor[4];
	COLORREF FadeColorRef;

	BYTE BGColor[4];
	COLORREF BGColorRef;

	// hfont to diplay on screen
	HFONT hFont;
	//these vary with the font currently assigned

	//used to calculate the offsets needed to draw the next character below or across in a new stream
	unsigned int textHeight;
	unsigned int textWidth;

	HBRUSH hBGBrush;
	HPEN hBGPen;
	HBITMAP hBGBitmap;
	HDC hBGDC;

	HDC hTempSpaceDC;
	HBITMAP hTempSpaceBitmap;

	HDC hBackDC;
	HBITMAP hBackBitmap;

	HWND hWnd;

	TBGMode BGMode;

	//The raster operation mode used when drawing
	TBlendMode BlendMode;



	float SpecialStringStreamProbability;

	// the color in which to render the special strings
	BYTE SpecialStringColor[4];
	COLORREF SpecialStringColorRef;

	BYTE SpecialStringFadeColor[4];
	COLORREF SpecialStringFadeColorRef;

	BYTE SpecialStringBGColor[4];
	COLORREF SpecialStringBGColorRef;
	HBRUSH hSpecialStringBGBrush;
	HPEN hSpecialStringBGPen;

	// hfont to diplay on screen for special strings
	HFONT hSpecialStringFont;

	vector< basic_string<_TCHAR> > ValidSpecialStrings;

	//used to calculate the offsets needed to draw the next character below or across in a new stream
	unsigned int SpecialStringTextHeight;
	unsigned int SpecialStringTextWidth;

	void UpdateSpecialStringFontMeasurements(void);


	double CoeffR1;
	double CoeffR0;
	double CoeffG1;
	double CoeffG0;
	double CoeffB1;
	double CoeffB0;
	double CoeffA1;
	double CoeffA0;



	void UpdateFontMeasurements(void);

	void CreateDestroyStreams(void);
	void UpdateStreams(void);
	void DisplayStreams(HDC hdc);


	struct zsCharDetails
	{
		_TCHAR Char;
		HFONT Font;
		COLORREF Color;
		RECT Rect;
		POINT TextPoint;
		bool IsSpecialStringChar;
	};
	void CalcCurrentAndPreceedingCharDetails(const IzsMatrixStream *Stream,zsCharDetails &Current,zsCharDetails &Preceeding) const;

	void CalcRectForNthBackChar(const IzsMatrixStream *Stream,unsigned int N,RECT &Rect) const;

	void PrintDebug(_TCHAR *Text)
	{
#ifdef _MSC_VER
		OutputDebugString(Text);
		OutputDebugString(_TEXT("\n"));
#endif
		MessageBox(this->hWnd,Text,_TEXT("Debug"),0);
	}
	
	unsigned int RefCount;


	//Default Initial Values for parameters
	static const unsigned int DefaultInitialMaxStream;
	static const unsigned int DefaultInitialSpeedVariance;

	static const bool DefaultInitialMonotonousCleanupEnabled;
	static const unsigned int DefaultInitialBackTrace;

	static const bool DefaultInitialRandomizedCleanupEnabled;
	static const unsigned int DefaultInitialLeading;
	static const unsigned int DefaultInitialSpacePad;

	static const BYTE DefaultInitialColor[4];
	static const BYTE DefaultInitialFadeColor[4];
	static const BYTE DefaultInitialBGColor[4];

	static const TBGMode DefaultInitialBGMode;

	static const TBlendMode DefaultInitialBlendMode;

	static const LOGFONT DefaultInitialFont;

	static const _TCHAR DefaultInitialCharSetStart;
	static const _TCHAR DefaultInitialCharSetEnd;


	static const BYTE DefaultInitialSpecialStringColor[4];
	static const BYTE DefaultInitialSpecialStringFadeColor[4];
	static const BYTE DefaultInitialSpecialStringBGColor[4];

	static const LOGFONT DefaultInitialSpecialStringFont;

	static const basic_string<_TCHAR> DefaultInitialValidSpecialStrings[];

	static const float DefaultInitialSpecialStringStreamProbability;


	static const double DefaultInitialCoeffR1;
	static const double DefaultInitialCoeffR0;
	static const double DefaultInitialCoeffG1;
	static const double DefaultInitialCoeffG0;
	static const double DefaultInitialCoeffB1;
	static const double DefaultInitialCoeffB0;
	static const double DefaultInitialCoeffA1;
	static const double DefaultInitialCoeffA0;
};

class zsMatrixFactory : public IzsMatrixFactory
{
public:
	zsMatrixFactory(void)
	{
		RefCount = 0;
	};

	HRESULT __stdcall CreatezsMatrix(IzsMatrix **ppNewMatrix,HWND TargetWindow,HBITMAP BGBitmap);
	HRESULT __stdcall CreatezsMatrix(IzsMatrix **ppNewMatrix,const IzsMatrix &OtherMatrix);

	HRESULT __stdcall QueryInterface(REFIID riid, void** ppObject)
	{
		if (riid==IID_IUnknown || riid==IID_IClassFactory)
		{
			*ppObject=(IzsMatrixFactory*) this;
		}
		else
		{
			return E_NOINTERFACE;
		}
		AddRef();
		return NO_ERROR;
	};

	ULONG  __stdcall AddRef()
	{
		g_dwRefCount++;
		RefCount++;
		return RefCount;
	};

	ULONG   __stdcall Release()
	{
		if(g_dwRefCount > 0)
			g_dwRefCount--;
		if(RefCount > 0)
			RefCount--;
		if (RefCount==0)
		{
			delete this;
			return 0;
		}
		return RefCount;
	};

	HRESULT __stdcall CreateInstance(IUnknown *pUnkOuter, REFIID riid, void** ppObject);
	HRESULT	__stdcall LockServer(BOOL fLock);

protected:
	unsigned int RefCount;
};

#endif