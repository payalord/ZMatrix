/*!=========================================================================
//
//  Program:   ZMatrix
//  Module:    $RCSfile: zsMatrix.cpp,v $
//  Language:  C/C++
//  Date:      $Date: 2003/05/23 02:07:12 $
//  Version:   $Revision: 1.22 $
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

#include "zsMatrix.h"
#include <time.h>
#include <math.h>

// {65A36275-972A-4804-A1FA-53DB4E39DE26}
static const GUID CLSID_ZSMATRIXSTREAM = 
{ 0x65a36275, 0x972a, 0x4804, { 0xa1, 0xfa, 0x53, 0xdb, 0x4e, 0x39, 0xde, 0x26 } };

// {95509A7E-CA62-4867-A7C7-DB0EFAF32F7C}
static const GUID IID_IZSMATRIXSTREAM = 
{ 0x95509a7e, 0xca62, 0x4867, { 0xa7, 0xc7, 0xdb, 0xe, 0xfa, 0xf3, 0x2f, 0x7c } };

// {2E393599-F2E3-484a-9B03-D414F6CBA5EB}
static const GUID CLSID_ZSMATRIX = 
{ 0x2e393599, 0xf2e3, 0x484a, { 0x9b, 0x3, 0xd4, 0x14, 0xf6, 0xcb, 0xa5, 0xeb } };

// {CBEAC95F-3125-4603-A0C9-0929BDC60FBC}
static const GUID IID_IZSMATRIX = 
{ 0xcbeac95f, 0x3125, 0x4603, { 0xa0, 0xc9, 0x9, 0x29, 0xbd, 0xc6, 0xf, 0xbc } };

unsigned int g_dwRefCount = 0;

#define PrintError(txt) MessageBox(this->hWnd,_TEXT(txt),_TEXT("Error"),MB_ICONERROR)

#ifndef CLAMP
#define CLAMP(var, min, max) \
if((var) > (max)) (var) = (max); \
if((var) < (min)) (var) = (min);
#endif //CLAMP

#define CommonCleanup() \
if(this->RandomizedCleanupEnabled)\
{\
	static RECT ClearRect;\
	int randomval = (rand()*this->SpacePad/RAND_MAX+this->Leading);\
	this->CalcRectForNthBackChar(this->Streams[i],randomval,ClearRect);\
	Rectangle(hdc,ClearRect.left,\
			  ClearRect.top,\
			  ClearRect.right,\
			  ClearRect.bottom);\
}\
\
if(this->MonotonousCleanupEnabled)\
{\
	static RECT ClearRect;\
	this->CalcRectForNthBackChar(this->Streams[i],this->BackTrace,ClearRect);\
	Rectangle(hdc,ClearRect.left,\
			  ClearRect.top,\
			  ClearRect.right,\
			  ClearRect.bottom);\
}

#define SpecialBitBltCleanup() \
if(this->RandomizedCleanupEnabled)\
{\
	static RECT ClearRect;\
	int randomval = (rand()*this->SpacePad/RAND_MAX+this->Leading);\
	this->CalcRectForNthBackChar(this->Streams[i],randomval,ClearRect);\
	BitBlt(hdc,ClearRect.left,\
			  ClearRect.top,\
			  WIDTH(ClearRect),\
			  HEIGHT(ClearRect),\
			  this->hBGDC,\
			  ClearRect.left,\
			  ClearRect.top,\
			  SRCCOPY);\
}\
\
if(this->MonotonousCleanupEnabled)\
{\
	static RECT ClearRect;\
	this->CalcRectForNthBackChar(this->Streams[i],this->BackTrace,ClearRect);\
	BitBlt(hdc,ClearRect.left,\
			  ClearRect.top,\
			  WIDTH(ClearRect),\
			  HEIGHT(ClearRect),\
			  this->hBGDC,\
			  ClearRect.left,\
			  ClearRect.top,\
			  SRCCOPY);\
}


#define WIDTH(Rect) Rect.right-Rect.left
#define HEIGHT(Rect) Rect.bottom-Rect.top



const unsigned int zsMatrix::DefaultInitialMaxStream = 1000;
const unsigned int zsMatrix::DefaultInitialSpeedVariance = 5;

const bool zsMatrix::DefaultInitialMonotonousCleanupEnabled = false;
const unsigned int zsMatrix::DefaultInitialBackTrace = 40;

const bool zsMatrix::DefaultInitialRandomizedCleanupEnabled = false;
const unsigned int zsMatrix::DefaultInitialLeading = 10;
const unsigned int zsMatrix::DefaultInitialSpacePad = 30;

const BYTE zsMatrix::DefaultInitialColor[4] = {150,255,100,255};
const BYTE zsMatrix::DefaultInitialFadeColor[4] = {50,85,33,128};
const BYTE zsMatrix::DefaultInitialBGColor[4] = {0,0,0,0};

const TBGMode zsMatrix::DefaultInitialBGMode = bgmodeBitmap;

const TBlendMode zsMatrix::DefaultInitialBlendMode = blendmodeXOR;

const LOGFONT zsMatrix::DefaultInitialFont =
{
	12,
	0,
	0,
	0,
	FW_NORMAL,
	0,
	0,
	0,
	DEFAULT_CHARSET,
	OUT_DEFAULT_PRECIS,
	CLIP_DEFAULT_PRECIS,
	DRAFT_QUALITY,
	DEFAULT_PITCH | FF_DONTCARE,
	_TEXT("Matrix Code Font")
};
#ifdef _UNICODE

const _TCHAR zsMatrix::DefaultInitialCharSetStart = 0x3041;
const _TCHAR zsMatrix::DefaultInitialCharSetEnd = 0x30F6;

#else

const _TCHAR zsMatrix::DefaultInitialCharSetStart = (_TCHAR)1;
const _TCHAR zsMatrix::DefaultInitialCharSetEnd = (_TCHAR)254;

#endif


const BYTE zsMatrix::DefaultInitialSpecialStringColor[4] = {255,150,100,255};
const BYTE zsMatrix::DefaultInitialSpecialStringFadeColor[4] = {255,150,100,255};
const BYTE zsMatrix::DefaultInitialSpecialStringBGColor[4] = {0,0,0,0};

const LOGFONT zsMatrix::DefaultInitialSpecialStringFont =
{
	6,
	0,
	0,
	0,
	FW_NORMAL,
	0,
	0,
	0,
	DEFAULT_CHARSET,
	OUT_DEFAULT_PRECIS,
	CLIP_DEFAULT_PRECIS,
	DRAFT_QUALITY,
	DEFAULT_PITCH | FF_DONTCARE,
	_TEXT("Terminal")
};

const basic_string<_TCHAR> zsMatrix::DefaultInitialValidSpecialStrings[] = 
{
	_T("The matrix has you"),
};

const float zsMatrix::DefaultInitialSpecialStringStreamProbability = 0.1f;

const double zsMatrix::DefaultInitialCoeffR1 = 1.0;
const double zsMatrix::DefaultInitialCoeffR0 = 0.0;
const double zsMatrix::DefaultInitialCoeffG1 = 1.0;
const double zsMatrix::DefaultInitialCoeffG0 = 0.0;
const double zsMatrix::DefaultInitialCoeffB1 = 1.0;
const double zsMatrix::DefaultInitialCoeffB0 = 0.0;
const double zsMatrix::DefaultInitialCoeffA1 = 1.0;
const double zsMatrix::DefaultInitialCoeffA0 = 0.0;


void SaveToClipboard(HBITMAP Bitmap)
{
	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_BITMAP,Bitmap);
	CloseClipboard();
}

#include <assert.h>
#define VerifyStreamCount()\
{\
		unsigned int TestCount = 0;\
		for(unsigned int i = 0; i < this->MaxStream; i++)\
		{\
			if(this->Streams[i]->GetStatus())\
				TestCount++;\
		}\
		assert(TestCount == StreamCount);\
}

//===========================================================================
//===========================================================================
zsMatrix::zsMatrix(void)
{
	unsigned int i;

	srand(time(NULL));

	this->RefCount = 0;

	this->hWnd = NULL;
	this->StreamCount = 0;
	this->MaxStream = 0;
	this->BGMode = this->DefaultInitialBGMode;

	this->CoeffR1 = this->DefaultInitialCoeffR1;
	this->CoeffR0 = this->DefaultInitialCoeffR0;
	this->CoeffG1 = this->DefaultInitialCoeffG1;
	this->CoeffG0 = this->DefaultInitialCoeffG0;
	this->CoeffB1 = this->DefaultInitialCoeffB1;
	this->CoeffB0 = this->DefaultInitialCoeffB0;
	this->CoeffA1 = this->DefaultInitialCoeffA1;
	this->CoeffA0 = this->DefaultInitialCoeffA0;

	this->SpecialStringStreamProbability = this->DefaultInitialSpecialStringStreamProbability;

	unsigned int NumDefaultInitialValidSpecialStrings = sizeof(this->DefaultInitialValidSpecialStrings)/sizeof(basic_string<_TCHAR>);
	this->ValidSpecialStrings.resize(NumDefaultInitialValidSpecialStrings);
	for(i = 0; i < NumDefaultInitialValidSpecialStrings; i++)
	{
		this->ValidSpecialStrings[i] = this->DefaultInitialValidSpecialStrings[i];
	}

#ifdef _UNICODE
	this->hFont = CreateFontIndirect(&this->DefaultInitialFont);

	if(this->hFont == NULL)
	{
		this->hFont = (HFONT)GetStockObject(OEM_FIXED_FONT);

		//initialize to a sequence of valid ASCII characters.
		for(i = 1; i < 255; i++)
		{
			this->ValidChars.push_back((_TCHAR)i);
		}
	}
	else
	{
		//initialize to a default sequence of valid characters.
		for(i = this->DefaultInitialCharSetStart; (_TCHAR)i <= this->DefaultInitialCharSetEnd; i++)
		{
			this->ValidChars.push_back((_TCHAR)i);
		}
	}

	this->hSpecialStringFont = CreateFontIndirect(&this->DefaultInitialSpecialStringFont);

	if(this->hSpecialStringFont == NULL)
	{
		this->hSpecialStringFont = (HFONT)GetStockObject(OEM_FIXED_FONT);

	}
	else
	{
	}
#else
	this->hFont = (HFONT)GetStockObject(OEM_FIXED_FONT);

	//initialize to a sequence of valid ASCII characters.
	for(i = this->DefaultInitialCharSetStart; (_TCHAR)i <= this->DefaultInitialCharSetEnd; i++)
	{
		this->ValidChars.push_back((_TCHAR)i);
	}

	this->hSpecialStringFont = (HFONT)GetStockObject(OEM_FIXED_FONT);
#endif

	this->UpdateFontMeasurements();
	this->UpdateSpecialStringFontMeasurements();


	memcpy(this->Color,this->DefaultInitialColor,4);

	this->ColorRef = RGB(this->Color[0],
						 this->Color[1],
						 this->Color[2]);

	memcpy(this->FadeColor,this->DefaultInitialFadeColor,4);

	this->FadeColorRef = RGB(this->FadeColor[0],
							 this->FadeColor[1],
							 this->FadeColor[2]);

	memcpy(this->BGColor,this->DefaultInitialBGColor,4);

	this->BGColorRef = RGB(this->BGColor[0],
						   this->BGColor[1],
						   this->BGColor[2]);

	this->hBGPen = (HPEN)CreatePen(PS_SOLID,0,BGColorRef);
	this->hBGBrush = (HBRUSH)CreateSolidBrush(BGColorRef);

	memcpy(this->SpecialStringColor,this->DefaultInitialSpecialStringColor,4);

	this->SpecialStringColorRef = RGB(this->SpecialStringColor[0],
									  this->SpecialStringColor[1],
									  this->SpecialStringColor[2]);

	memcpy(this->SpecialStringFadeColor,this->DefaultInitialSpecialStringFadeColor,4);

	this->SpecialStringFadeColorRef = RGB(this->SpecialStringFadeColor[0],
										  this->SpecialStringFadeColor[1],
										  this->SpecialStringFadeColor[2]);

	memcpy(this->SpecialStringBGColor,this->DefaultInitialSpecialStringBGColor,4);

	this->SpecialStringBGColorRef = RGB(this->SpecialStringBGColor[0],
										this->SpecialStringBGColor[1],
										this->SpecialStringBGColor[2]);

	this->hSpecialStringBGPen = (HPEN)CreatePen(PS_SOLID,0,SpecialStringBGColorRef);
	this->hSpecialStringBGBrush = (HBRUSH)CreateSolidBrush(SpecialStringBGColorRef);

	this->SetMaxStream(this->DefaultInitialMaxStream);
	this->SpeedVariance = this->DefaultInitialSpeedVariance;

	this->MonotonousCleanupEnabled = this->DefaultInitialMonotonousCleanupEnabled;
	this->BackTrace = this->DefaultInitialBackTrace;

	this->RandomizedCleanupEnabled = this->DefaultInitialRandomizedCleanupEnabled;
	this->Leading = this->DefaultInitialLeading;
	this->SpacePad = this->DefaultInitialSpacePad;


	this->BlendMode = this->DefaultInitialBlendMode;


	this->hBGBitmap = NULL;

	this->hBGDC = NULL;


	this->hTempSpaceDC = NULL;
	this->hTempSpaceBitmap = NULL;


	this->hBackDC = NULL;
	this->hBackBitmap = NULL;


}
//===========================================================================
//===========================================================================
zsMatrix::zsMatrix(HWND Window,HBITMAP BGBitmap)
{
	unsigned int i;

	srand(time(NULL));

	this->RefCount = 0;

	this->hWnd = Window;
	this->StreamCount = 0;
	this->MaxStream = 0;
	this->BGMode = bgmodeBitmap;

	this->CoeffR1 = this->DefaultInitialCoeffR1;
	this->CoeffR0 = this->DefaultInitialCoeffR0;
	this->CoeffG1 = this->DefaultInitialCoeffG1;
	this->CoeffG0 = this->DefaultInitialCoeffG0;
	this->CoeffB1 = this->DefaultInitialCoeffB1;
	this->CoeffB0 = this->DefaultInitialCoeffB0;
	this->CoeffA1 = this->DefaultInitialCoeffA1;
	this->CoeffA0 = this->DefaultInitialCoeffA0;

	this->SpecialStringStreamProbability = this->DefaultInitialSpecialStringStreamProbability;

	unsigned int NumDefaultInitialValidSpecialStrings = sizeof(this->DefaultInitialValidSpecialStrings)/sizeof(basic_string<_TCHAR>);
	this->ValidSpecialStrings.resize(NumDefaultInitialValidSpecialStrings);
	for(i = 0; i < NumDefaultInitialValidSpecialStrings; i++)
	{
		this->ValidSpecialStrings[i] = this->DefaultInitialValidSpecialStrings[i];
	}

#ifdef _UNICODE
	this->hFont = CreateFontIndirect(&this->DefaultInitialFont);

	if(this->hFont == NULL)
	{
		this->hFont = (HFONT)GetStockObject(OEM_FIXED_FONT);

		//initialize to a sequence of valid ASCII characters.
		for(int i = 1; i < 255; i++)
		{
			this->ValidChars.push_back((_TCHAR)i);
		}
	}
	else
	{
		//initialize to a default sequence of valid characters.
		for(i = this->DefaultInitialCharSetStart; (_TCHAR)i <= this->DefaultInitialCharSetEnd; i++)
		{
			this->ValidChars.push_back((_TCHAR)i);
		}
	}

	this->hSpecialStringFont = CreateFontIndirect(&this->DefaultInitialSpecialStringFont);

	if(this->hSpecialStringFont == NULL)
	{
		this->hSpecialStringFont = (HFONT)GetStockObject(OEM_FIXED_FONT);

	}
	else
	{
	}
#else
	this->hFont = (HFONT)GetStockObject(OEM_FIXED_FONT);

	//initialize to a sequence of valid ASCII characters.
	for(i = this->DefaultInitialCharSetStart; (_TCHAR)i <= this->DefaultInitialCharSetEnd; i++)
	{
		this->ValidChars.push_back((_TCHAR)i);
	}

	this->hSpecialStringFont = (HFONT)GetStockObject(OEM_FIXED_FONT);
#endif

	this->UpdateFontMeasurements();
	this->UpdateSpecialStringFontMeasurements();


	memcpy(this->Color,this->DefaultInitialColor,4);

	this->ColorRef = RGB(this->Color[0],
						 this->Color[1],
						 this->Color[2]);

	memcpy(this->FadeColor,this->DefaultInitialFadeColor,4);

	this->FadeColorRef = RGB(this->FadeColor[0],
							 this->FadeColor[1],
							 this->FadeColor[2]);

	memcpy(this->BGColor,this->DefaultInitialBGColor,4);

	this->BGColorRef = RGB(this->BGColor[0],
						   this->BGColor[1],
						   this->BGColor[2]);

	this->hBGPen = (HPEN)CreatePen(PS_SOLID,0,BGColorRef);
	this->hBGBrush = (HBRUSH)CreateSolidBrush(BGColorRef);



	memcpy(this->SpecialStringColor,this->DefaultInitialSpecialStringColor,4);

	this->SpecialStringColorRef = RGB(this->SpecialStringColor[0],
									  this->SpecialStringColor[1],
									  this->SpecialStringColor[2]);

	memcpy(this->SpecialStringFadeColor,this->DefaultInitialSpecialStringFadeColor,4);

	this->SpecialStringFadeColorRef = RGB(this->SpecialStringFadeColor[0],
										  this->SpecialStringFadeColor[1],
										  this->SpecialStringFadeColor[2]);

	memcpy(this->SpecialStringBGColor,this->DefaultInitialSpecialStringBGColor,4);

	this->SpecialStringBGColorRef = RGB(this->SpecialStringBGColor[0],
										this->SpecialStringBGColor[1],
										this->SpecialStringBGColor[2]);

	this->hSpecialStringBGPen = (HPEN)CreatePen(PS_SOLID,0,SpecialStringBGColorRef);
	this->hSpecialStringBGBrush = (HBRUSH)CreateSolidBrush(SpecialStringBGColorRef);



	this->SetMaxStream(this->DefaultInitialMaxStream);
	this->SpeedVariance = this->DefaultInitialSpeedVariance;

	this->MonotonousCleanupEnabled = this->DefaultInitialMonotonousCleanupEnabled;
	this->BackTrace = this->DefaultInitialBackTrace;
	
	this->RandomizedCleanupEnabled = this->DefaultInitialRandomizedCleanupEnabled;
	this->Leading = this->DefaultInitialLeading;
	this->SpacePad = this->DefaultInitialSpacePad;


	this->BlendMode = this->DefaultInitialBlendMode;






	//Setting up of the DCs and Bitmaps

	HDC hdc = GetDC(this->hWnd);
	this->hBGDC = CreateCompatibleDC(hdc);


	BITMAP BitmapHeader;
	GetObject(BGBitmap,sizeof(BitmapHeader),&BitmapHeader);

	this->hBGBitmap = CreateCompatibleBitmap(hdc,BitmapHeader.bmWidth,BitmapHeader.bmHeight);

	this->hTempSpaceDC = CreateCompatibleDC(hdc);
	this->hTempSpaceBitmap = CreateCompatibleBitmap(hdc,BitmapHeader.bmWidth,BitmapHeader.bmHeight);

	this->hBackDC = CreateCompatibleDC(hdc);
	this->hBackBitmap = CreateCompatibleBitmap(hdc,BitmapHeader.bmWidth,BitmapHeader.bmHeight);




	HBITMAP hOldBGBitmap = (HBITMAP) SelectObject(this->hBGDC,this->hBGBitmap);
	HBITMAP hOldTempSpaceBitmap = (HBITMAP) SelectObject(this->hTempSpaceDC,BGBitmap);

	BitBlt(this->hBGDC,0,0,BitmapHeader.bmWidth,BitmapHeader.bmHeight,this->hTempSpaceDC,0,0,SRCCOPY);

	SelectObject(this->hBGDC,hOldBGBitmap);
	SelectObject(this->hTempSpaceDC,hOldTempSpaceBitmap);



	ReleaseDC(hWnd,hdc);

}
//===========================================================================
//===========================================================================
zsMatrix::zsMatrix(const zsMatrix &Other)
:ValidChars(Other.ValidChars),ValidSpecialStrings(Other.ValidSpecialStrings),
MaxStream(Other.MaxStream),SpeedVariance(Other.SpeedVariance),
MonotonousCleanupEnabled(Other.MonotonousCleanupEnabled),BackTrace(Other.BackTrace),
RandomizedCleanupEnabled(Other.RandomizedCleanupEnabled),Leading(Other.Leading),SpacePad(Other.SpacePad),
StreamCount(Other.StreamCount),
ColorRef(Other.ColorRef),FadeColorRef(Other.FadeColorRef),BGColorRef(Other.BGColorRef),
SpecialStringColorRef(Other.SpecialStringColorRef),SpecialStringFadeColorRef(Other.SpecialStringFadeColorRef),SpecialStringBGColorRef(Other.SpecialStringBGColorRef),
SpecialStringTextHeight(Other.SpecialStringTextHeight),SpecialStringTextWidth(Other.SpecialStringTextWidth),
SpecialStringStreamProbability(Other.SpecialStringStreamProbability),
CoeffR1(Other.CoeffR1),CoeffR0(Other.CoeffR0),
CoeffG1(Other.CoeffG1),CoeffG0(Other.CoeffG0),
CoeffB1(Other.CoeffB1),CoeffB0(Other.CoeffB0),
CoeffA1(Other.CoeffA1),CoeffA0(Other.CoeffA0),
textHeight(Other.textHeight),textWidth(Other.textWidth),
hWnd(Other.hWnd),BGMode(Other.BGMode),BlendMode(Other.BlendMode)
{
	this->RefCount = 0;

	this->Streams.resize(Other.GetMaxStream());

	unsigned int i;
	for(i = 0; i < this->Streams.size(); i++)
	{
		this->Streams[i] = new zsMatrixStream(*(Other.Streams[i]));
	}

	//Create local copies of the other item's color and background color
	memcpy(this->Color,Other.Color,sizeof(BYTE)*4);
	memcpy(this->FadeColor,Other.FadeColor,sizeof(BYTE)*4);
	memcpy(this->BGColor,Other.BGColor,sizeof(BYTE)*4);

	//Create local copies of the other item's background brush, pen and font
	LOGFONT OtherLogFont;
	GetObject(Other.hFont,sizeof(LOGFONT),&OtherLogFont);
	this->hFont = CreateFontIndirect(&OtherLogFont);

	this->UpdateFontMeasurements();

	LOGBRUSH OtherLogBGBrush;
	GetObject(Other.hBGBrush,sizeof(LOGBRUSH),&OtherLogBGBrush);
	this->hBGBrush = CreateBrushIndirect(&OtherLogBGBrush);

	LOGPEN OtherLogBGPen;
	GetObject(Other.hBGPen,sizeof(LOGPEN),&OtherLogBGPen);
	this->hBGPen = CreatePenIndirect(&OtherLogBGPen);



	//Create local copies of the other item's color and background color
	memcpy(this->SpecialStringColor,Other.SpecialStringColor,sizeof(BYTE)*4);
	memcpy(this->SpecialStringFadeColor,Other.SpecialStringFadeColor,sizeof(BYTE)*4);
	memcpy(this->SpecialStringBGColor,Other.SpecialStringBGColor,sizeof(BYTE)*4);

	//Create local copies of the other item's background brush, pen and font
	LOGFONT OtherSpecialStringLogFont;
	GetObject(Other.hSpecialStringFont,sizeof(LOGFONT),&OtherSpecialStringLogFont);
	this->hSpecialStringFont = CreateFontIndirect(&OtherSpecialStringLogFont);

	this->UpdateSpecialStringFontMeasurements();

	LOGBRUSH OtherSpecialStringLogBGBrush;
	GetObject(Other.hSpecialStringBGBrush,sizeof(LOGBRUSH),&OtherSpecialStringLogBGBrush);
	this->hSpecialStringBGBrush = CreateBrushIndirect(&OtherSpecialStringLogBGBrush);

	LOGPEN OtherSpecialStringLogBGPen;
	GetObject(Other.hSpecialStringBGPen,sizeof(LOGPEN),&OtherSpecialStringLogBGPen);
	this->hSpecialStringBGPen = CreatePenIndirect(&OtherSpecialStringLogBGPen);





	//Setting up of the DCs and Bitmaps

	HDC hdc = GetDC(this->hWnd);
	this->hBGDC = CreateCompatibleDC(hdc);


	BITMAP BitmapHeader;
	GetObject(Other.hBGBitmap,sizeof(BitmapHeader),&BitmapHeader);

	this->hBGBitmap = CreateCompatibleBitmap(hdc,BitmapHeader.bmWidth,BitmapHeader.bmHeight);

	this->hTempSpaceDC = CreateCompatibleDC(hdc);
	this->hTempSpaceBitmap = CreateCompatibleBitmap(hdc,BitmapHeader.bmWidth,BitmapHeader.bmHeight);

	this->hBackDC = CreateCompatibleDC(hdc);
	this->hBackBitmap = CreateCompatibleBitmap(hdc,BitmapHeader.bmWidth,BitmapHeader.bmHeight);




	HBITMAP hOldBGBitmap = (HBITMAP) SelectObject(this->hBGDC,this->hBGBitmap);
	HBITMAP hOldTempSpaceBitmap = (HBITMAP) SelectObject(this->hTempSpaceDC,Other.hBGBitmap);

	BitBlt(this->hBGDC,0,0,BitmapHeader.bmWidth,BitmapHeader.bmHeight,this->hTempSpaceDC,0,0,SRCCOPY);

	SelectObject(this->hBGDC,hOldBGBitmap);
	SelectObject(this->hTempSpaceDC,hOldTempSpaceBitmap);



	ReleaseDC(hWnd,hdc);



}
//===========================================================================
//===========================================================================
zsMatrix::zsMatrix(const IzsMatrix &Other)
{
	this->RefCount = 0;
	this->hWnd = Other.GethWnd();

	this->SpecialStringStreamProbability = Other.GetSpecialStringStreamProbability();

	this->ValidChars.resize(Other.GetNumCharsInSet());

	unsigned int i;
	const _TCHAR *OtherSet = Other.GetValidCharSet();
	for(i = 0; i < this->ValidChars.size(); i++)
	{
		this->ValidChars[i] = OtherSet[i];
	}

	this->ValidSpecialStrings.resize(Other.GetNumSpecialStringsInSet());

	for(i = 0; i < this->ValidSpecialStrings.size(); i++)
	{
		this->ValidSpecialStrings[i] = Other.GetValidSpecialString(i);
	}

	this->MaxStream = Other.GetMaxStream();
	this->SpeedVariance = Other.GetSpeedVariance();

	this->MonotonousCleanupEnabled = Other.GetMonotonousCleanupEnabled();
	this->BackTrace = Other.GetBackTrace();

	this->RandomizedCleanupEnabled = Other.GetRandomizedCleanupEnabled();
	this->Leading = Other.GetLeading();
	this->SpacePad = Other.GetSpacePad();

	this->Streams.resize(Other.GetMaxStream());
	const IzsMatrixStream **OtherStreams = Other.GetStreams();
	for(i = 0; i < this->Streams.size(); i++)
	{
		this->Streams[i] = this->CreateStream(*(OtherStreams[i]));
	}

	this->StreamCount = Other.GetNumStreams();

	BYTE R,G,B,A;
	Other.GetColor(R,G,B,A);
	this->SetColor(R,G,B,A);

	Other.GetFadeColor(R,G,B,A);
	this->SetFadeColor(R,G,B,A);

	Other.GetBGColor(R,G,B,A);
	this->SetBGColor(R,G,B,A);


	//Create local copies of the other item's background brush, pen and font
	LOGFONT OtherLogFont;
	GetObject(Other.GetFont(),sizeof(LOGFONT),&OtherLogFont);
	this->hFont = CreateFontIndirect(&OtherLogFont);
	this->UpdateFontMeasurements();


	Other.GetSpecialStringColor(R,G,B,A);
	this->SetSpecialStringColor(R,G,B,A);

	Other.GetSpecialStringFadeColor(R,G,B,A);
	this->SetSpecialStringFadeColor(R,G,B,A);

	Other.GetSpecialStringBGColor(R,G,B,A);
	this->SetSpecialStringBGColor(R,G,B,A);

	this->CoeffR1 = Other.GetCoeffR1();
	this->CoeffR0 = Other.GetCoeffR0();
	this->CoeffG1 = Other.GetCoeffG1();
	this->CoeffG0 = Other.GetCoeffG0();
	this->CoeffB1 = Other.GetCoeffB1();
	this->CoeffB0 = Other.GetCoeffB0();
	this->CoeffA1 = Other.GetCoeffA1();
	this->CoeffA0 = Other.GetCoeffA0();


	//Create local copies of the other item's background brush, pen and font
	LOGFONT OtherSpecialStringLogFont;
	GetObject(Other.GetSpecialStringFont(),sizeof(LOGFONT),&OtherSpecialStringLogFont);
	this->hSpecialStringFont = CreateFontIndirect(&OtherSpecialStringLogFont);
	this->UpdateSpecialStringFontMeasurements();




	this->SetBGMode(Other.GetBGMode());

	this->SetBlendMode(Other.GetBlendMode());




	//Setting up of the DCs and Bitmaps

	HDC hdc = GetDC(this->hWnd);
	this->hBGDC = CreateCompatibleDC(hdc);


	BITMAP BitmapHeader;
	GetObject(Other.GetBGBitmap(),sizeof(BitmapHeader),&BitmapHeader);

	this->hBGBitmap = CreateCompatibleBitmap(hdc,BitmapHeader.bmWidth,BitmapHeader.bmHeight);

	this->hTempSpaceDC = CreateCompatibleDC(hdc);
	this->hTempSpaceBitmap = CreateCompatibleBitmap(hdc,BitmapHeader.bmWidth,BitmapHeader.bmHeight);

	this->hBackDC = CreateCompatibleDC(hdc);
	this->hBackBitmap = CreateCompatibleBitmap(hdc,BitmapHeader.bmWidth,BitmapHeader.bmHeight);




	HBITMAP hOldBGBitmap = (HBITMAP) SelectObject(this->hBGDC,this->hBGBitmap);
	HBITMAP hOldTempSpaceBitmap = (HBITMAP) SelectObject(this->hTempSpaceDC,Other.GetBGBitmap());

	BitBlt(this->hBGDC,0,0,BitmapHeader.bmWidth,BitmapHeader.bmHeight,this->hTempSpaceDC,0,0,SRCCOPY);

	SelectObject(this->hBGDC,hOldBGBitmap);
	SelectObject(this->hTempSpaceDC,hOldTempSpaceBitmap);



	ReleaseDC(hWnd,hdc);


}
//===========================================================================
//===========================================================================
zsMatrix::~zsMatrix()
{

	if ((this->hBackDC != NULL) && (0 == DeleteDC(this->hBackDC)))
		PrintError("Failed to delete the back DC");
	else
		this->hBackDC = NULL;
	if ((this->hBackBitmap != NULL) && (0 == DeleteObject(this->hBackBitmap)))
		PrintError("Failed to delete the back bitmap");
	else
		this->hBackBitmap = NULL;
	if ((this->hTempSpaceDC != NULL) && (0 == DeleteDC(this->hTempSpaceDC)))
		PrintError("Failed to delete the temp space DC");
	else
		this->hTempSpaceDC = NULL;
	if ((this->hTempSpaceBitmap != NULL) && (0 == DeleteObject(this->hTempSpaceBitmap)))
		PrintError("Failed to delete the temp space bitmap");
	else
		this->hTempSpaceBitmap = NULL;
	if ((this->hBGDC != NULL) && (0 == DeleteDC(this->hBGDC)))
		PrintError("Failed to delete the BG DC");
	else
		this->hBGDC = NULL;
	if ((this->hBGBitmap != NULL) && (0 == DeleteObject(this->hBGBitmap)))
		PrintError("Failed to delete the BG bitmap");
	else
		this->hBGBitmap = NULL;

	if ((this->hBGBrush != NULL) && (0 == DeleteObject(this->hBGBrush)))
		PrintError("Failed to delete the BG brush");
	else
		this->hBGBrush = NULL;
	if ((this->hBGPen != NULL) && (0 == DeleteObject(this->hBGPen)))
		PrintError("Failed to delete the BG pen");
	else
		this->hBGPen = NULL;
	if ((this->hFont != NULL) && (0 == DeleteObject(this->hFont)))
		PrintError("Failed to delete the font");
	else
		this->hFont = NULL;


	if ((this->hSpecialStringBGBrush != NULL) && (0 == DeleteObject(this->hSpecialStringBGBrush)))
		PrintError("Failed to delete the SpecialString BG brush");
	else
		this->hSpecialStringBGBrush = NULL;
	if ((this->hSpecialStringBGPen != NULL) && (0 == DeleteObject(this->hSpecialStringBGPen)))
		PrintError("Failed to delete the SpecialString BG pen");
	else
		this->hSpecialStringBGPen = NULL;
	if ((this->hSpecialStringFont != NULL) && (0 == DeleteObject(this->hSpecialStringFont)))
		PrintError("Failed to delete the SpecialString font");
	else
		this->hSpecialStringFont = NULL;



	std::vector<IzsMatrixStream *>::iterator Iter = this->Streams.begin();
	for(Iter = this->Streams.begin(); Iter != this->Streams.end(); Iter++)
	{
		delete *Iter;
	}
}
//===========================================================================
//===========================================================================
zsMatrix &zsMatrix::operator=(const IzsMatrix &Other)
{

	if ((this->hBackDC != NULL) && (0 == DeleteDC(this->hBackDC)))
		PrintError("Failed to delete the back DC");
	else
		this->hBackDC = NULL;
	if ((this->hBackBitmap != NULL) && (0 == DeleteObject(this->hBackBitmap)))
		PrintError("Failed to delete the back bitmap");
	else
		this->hBackBitmap = NULL;
	if ((this->hTempSpaceDC != NULL) && (0 == DeleteDC(this->hTempSpaceDC)))
		PrintError("Failed to delete the temp space DC");
	else
		this->hTempSpaceDC = NULL;
	if ((this->hTempSpaceBitmap != NULL) && (0 == DeleteObject(this->hTempSpaceBitmap)))
		PrintError("Failed to delete the temp space bitmap");
	else
		this->hTempSpaceBitmap = NULL;
	if ((this->hBGDC != NULL) && (0 == DeleteDC(this->hBGDC)))
		PrintError("Failed to delete the BG DC");
	else
		this->hBGDC = NULL;
	if ((this->hBGBitmap != NULL) && (0 == DeleteObject(this->hBGBitmap)))
		PrintError("Failed to delete the BG bitmap");
	else
		this->hBGBitmap = NULL;

	if ((this->hBGBrush != NULL) && (0 == DeleteObject(this->hBGBrush)))
		PrintError("Failed to delete the BG brush");
	else
		this->hBGBrush = NULL;
	if ((this->hBGPen != NULL) && (0 == DeleteObject(this->hBGPen)))
		PrintError("Failed to delete the BG pen");
	else
		this->hBGPen = NULL;
	if ((this->hFont != NULL) && (0 == DeleteObject(this->hFont)))
		PrintError("Failed to delete the font");
	else
		this->hFont = NULL;


	if ((this->hSpecialStringBGBrush != NULL) && (0 == DeleteObject(this->hSpecialStringBGBrush)))
		PrintError("Failed to delete the SpecialString BG brush");
	else
		this->hSpecialStringBGBrush = NULL;
	if ((this->hSpecialStringBGPen != NULL) && (0 == DeleteObject(this->hSpecialStringBGPen)))
		PrintError("Failed to delete the SpecialString BG pen");
	else
		this->hSpecialStringBGPen = NULL;
	if ((this->hSpecialStringFont != NULL) && (0 == DeleteObject(this->hSpecialStringFont)))
		PrintError("Failed to delete the SpecialString font");
	else
		this->hSpecialStringFont = NULL;


	this->SethWnd(Other.GethWnd());

	this->SpecialStringStreamProbability = Other.GetSpecialStringStreamProbability();

	this->ValidChars.resize(Other.GetNumCharsInSet());

	unsigned int i;
	const _TCHAR *OtherSet = Other.GetValidCharSet();
	for(i = 0; i < this->ValidChars.size(); i++)
	{
		this->ValidChars[i] = OtherSet[i];
	}

	this->ValidSpecialStrings.resize(Other.GetNumSpecialStringsInSet());

	for(i = 0; i < this->ValidSpecialStrings.size(); i++)
	{
		this->ValidSpecialStrings[i] = Other.GetValidSpecialString(i);
	}

	this->SpeedVariance = Other.GetSpeedVariance();

	this->MonotonousCleanupEnabled = Other.GetMonotonousCleanupEnabled();
	this->BackTrace = Other.GetBackTrace();

	this->RandomizedCleanupEnabled = Other.GetRandomizedCleanupEnabled();
	this->Leading = Other.GetLeading();
	this->SpacePad = Other.GetSpacePad();

	this->SetMaxStream(Other.GetMaxStream());
	const IzsMatrixStream **OtherStreams = Other.GetStreams();
	for(i = 0; i < this->Streams.size(); i++)
	{
		*(this->Streams[i]) = *(OtherStreams[i]);
	}

	this->StreamCount = Other.GetNumStreams();
//VerifyStreamCount();
	BYTE R,G,B,A;
	Other.GetColor(R,G,B,A);
	this->SetColor(R,G,B,A);

	Other.GetFadeColor(R,G,B,A);
	this->SetFadeColor(R,G,B,A);

	Other.GetBGColor(R,G,B,A);
	this->SetBGColor(R,G,B,A);

	//Create local copies of the other item's background brush, pen and font
	this->SetFont(Other.GetFont());

	this->SetBGMode(Other.GetBGMode());

	this->SetBlendMode(Other.GetBlendMode());



	Other.GetSpecialStringColor(R,G,B,A);
	this->SetSpecialStringColor(R,G,B,A);

	Other.GetSpecialStringFadeColor(R,G,B,A);
	this->SetSpecialStringFadeColor(R,G,B,A);

	Other.GetSpecialStringBGColor(R,G,B,A);
	this->SetSpecialStringBGColor(R,G,B,A);

	//Create local copies of the other item's background brush, pen and font
	this->SetSpecialStringFont(Other.GetSpecialStringFont());




	//Setting up of the DCs and Bitmaps

	HDC hdc = GetDC(this->hWnd);
	this->hBGDC = CreateCompatibleDC(hdc);


	BITMAP BitmapHeader;
	GetObject(Other.GetBGBitmap(),sizeof(BitmapHeader),&BitmapHeader);

	this->hBGBitmap = CreateCompatibleBitmap(hdc,BitmapHeader.bmWidth,BitmapHeader.bmHeight);

	this->hTempSpaceDC = CreateCompatibleDC(hdc);
	this->hTempSpaceBitmap = CreateCompatibleBitmap(hdc,BitmapHeader.bmWidth,BitmapHeader.bmHeight);

	this->hBackDC = CreateCompatibleDC(hdc);
	this->hBackBitmap = CreateCompatibleBitmap(hdc,BitmapHeader.bmWidth,BitmapHeader.bmHeight);




	HBITMAP hOldBGBitmap = (HBITMAP) SelectObject(this->hBGDC,this->hBGBitmap);
	HBITMAP hOldTempSpaceBitmap = (HBITMAP) SelectObject(this->hTempSpaceDC,Other.GetBGBitmap());

	BitBlt(this->hBGDC,0,0,BitmapHeader.bmWidth,BitmapHeader.bmHeight,this->hTempSpaceDC,0,0,SRCCOPY);

	SelectObject(this->hBGDC,hOldBGBitmap);
	SelectObject(this->hTempSpaceDC,hOldTempSpaceBitmap);



	ReleaseDC(hWnd,hdc);


	return *this;

}
//===========================================================================
//===========================================================================
int zsMatrix::Render(HDC hdc)
{
	this->CreateDestroyStreams();
	this->UpdateStreams();
	this->DisplayStreams(hdc);
	return true;
}
//===========================================================================
//===========================================================================
void zsMatrix::UpdateTarget(HWND hWnd,HBITMAP BGBitmap)
{

	if ((this->hBackDC != NULL) && (0 == DeleteDC(this->hBackDC)))
		PrintError("Failed to delete the back DC");
	else
		this->hBackDC = NULL;
	if ((this->hBackBitmap != NULL) && (0 == DeleteObject(this->hBackBitmap)))
		PrintError("Failed to delete the back bitmap");
	else
		this->hBackBitmap = NULL;
	if ((this->hTempSpaceDC != NULL) && (0 == DeleteDC(this->hTempSpaceDC)))
		PrintError("Failed to delete the temp space DC");
	else
		this->hTempSpaceDC = NULL;
	if ((this->hTempSpaceBitmap != NULL) && (0 == DeleteObject(this->hTempSpaceBitmap)))
		PrintError("Failed to delete the temp space bitmap");
	else
		this->hTempSpaceBitmap = NULL;
	if ((this->hBGBitmap != NULL) && (0 == DeleteObject(this->hBGBitmap)))
		PrintError("Failed to delete the BG bitmap");
	else
		this->hBGBitmap = NULL;



	this->hWnd = hWnd;


	this->UpdateFontMeasurements();
	this->UpdateSpecialStringFontMeasurements();


	//Setting up of the DCs and Bitmaps

	HDC hdc = GetDC(this->hWnd);
	this->hBGDC = CreateCompatibleDC(hdc);


	BITMAP BitmapHeader;
	GetObject(BGBitmap,sizeof(BitmapHeader),&BitmapHeader);

	this->hBGBitmap = CreateCompatibleBitmap(hdc,BitmapHeader.bmWidth,BitmapHeader.bmHeight);

	this->hTempSpaceDC = CreateCompatibleDC(hdc);
	this->hTempSpaceBitmap = CreateCompatibleBitmap(hdc,BitmapHeader.bmWidth,BitmapHeader.bmHeight);

	this->hBackDC = CreateCompatibleDC(hdc);
	this->hBackBitmap = CreateCompatibleBitmap(hdc,BitmapHeader.bmWidth,BitmapHeader.bmHeight);




	HBITMAP hOldBGBitmap = (HBITMAP) SelectObject(this->hBGDC,this->hBGBitmap);
	HBITMAP hOldTempSpaceBitmap = (HBITMAP) SelectObject(this->hTempSpaceDC,BGBitmap);

	BitBlt(this->hBGDC,0,0,BitmapHeader.bmWidth,BitmapHeader.bmHeight,this->hTempSpaceDC,0,0,SRCCOPY);

	SelectObject(this->hBGDC,hOldBGBitmap);
	SelectObject(this->hTempSpaceDC,hOldTempSpaceBitmap);

	ReleaseDC(hWnd,hdc);



}
//===========================================================================
//===========================================================================
void zsMatrix::SetColor(BYTE R,BYTE G,BYTE B,BYTE A)
{
	Color[0] = R;
	Color[1] = G;
	Color[2] = B;
	Color[3] = A;

	this->ColorRef = RGB(this->Color[0],
						 this->Color[1],
						 this->Color[2]);
}
//===========================================================================
//===========================================================================
void zsMatrix::GetColor(BYTE &R,BYTE &G,BYTE &B,BYTE &A) const 
{
	R = Color[0];
	G = Color[1];
	B = Color[2];
	A = Color[3];
}
//===========================================================================
//===========================================================================
void zsMatrix::SetFadeColor(BYTE R,BYTE G,BYTE B,BYTE A)
{
	FadeColor[0] = R;
	FadeColor[1] = G;
	FadeColor[2] = B;
	FadeColor[3] = A;

	this->FadeColorRef = RGB(this->FadeColor[0],
							 this->FadeColor[1],
							 this->FadeColor[2]);
}
//===========================================================================
//===========================================================================
void zsMatrix::GetFadeColor(BYTE &R,BYTE &G,BYTE &B,BYTE &A) const 
{
	R = FadeColor[0];
	G = FadeColor[1];
	B = FadeColor[2];
	A = FadeColor[3];
}
//===========================================================================
//===========================================================================
void zsMatrix::SetBGColor(BYTE R,BYTE G,BYTE B,BYTE A)
{
	BGColor[0] = R;
	BGColor[1] = G;
	BGColor[2] = B;
	BGColor[3] = A;

	this->BGColorRef = RGB(this->BGColor[0],
						   this->BGColor[1],
						   this->BGColor[2]);



	if ((this->hBGBrush != NULL) && (0 == DeleteObject(this->hBGBrush)))
		PrintError("Failed to delete the BG brush");
	else
		this->hBGBrush = NULL;
	if ((this->hBGPen != NULL) && (0 == DeleteObject(this->hBGPen)))
		PrintError("Failed to delete the BG pen");
	else
		this->hBGPen = NULL;


	this->hBGPen = (HPEN)CreatePen(PS_SOLID,0,BGColorRef);
	this->hBGBrush = (HBRUSH)CreateSolidBrush(BGColorRef);

}
//===========================================================================
//===========================================================================
void zsMatrix::GetBGColor(BYTE &R,BYTE &G,BYTE &B,BYTE &A) const 
{
	R = BGColor[0];
	G = BGColor[1];
	B = BGColor[2];
	A = BGColor[3];
}
//===========================================================================
//===========================================================================
void zsMatrix::SetBGMode(const TBGMode &NewMode)
{
	this->BGMode = (TBGMode)NewMode;
}
//===========================================================================
//===========================================================================
TBGMode zsMatrix::GetBGMode(void) const
{
	return this->BGMode;
}
//===========================================================================
//===========================================================================
void zsMatrix::SetBlendMode(TBlendMode NewBlendMode)
{
	this->BlendMode = NewBlendMode;
}
//===========================================================================
//===========================================================================
TBlendMode zsMatrix::GetBlendMode(void) const 
{
	return this->BlendMode;
}
//===========================================================================
//===========================================================================
void zsMatrix::SetFont(HFONT NewFont)
{

	if ((this->hFont != NULL) && (0 == DeleteObject(this->hFont)))
		PrintError("Failed to delete the font");
	else
		this->hFont = NULL;

	if(NewFont != NULL)
	{
		LOGFONT NewLogFont;
		GetObject(NewFont,sizeof(LOGFONT),&NewLogFont);

		this->hFont = CreateFontIndirect(&NewLogFont);

		this->UpdateFontMeasurements();
	}
}
//===========================================================================
//===========================================================================
void zsMatrix::SetLogFont(const LOGFONT &NewFont)
{

	if ((this->hFont != NULL) && (0 == DeleteObject(this->hFont)))
		PrintError("Failed to delete the font");
	else
		this->hFont = NULL;


	this->hFont = CreateFontIndirect(&NewFont);

	this->UpdateFontMeasurements();
}
//===========================================================================
//===========================================================================
HFONT zsMatrix::GetFont(void) const 
{
	return this->hFont;
}
//===========================================================================
//===========================================================================
void zsMatrix::SetMaxStream(unsigned int NewMax)
{
	if(NewMax < this->MaxStream)
	{
		unsigned int i;
		for(i = NewMax; i < this->MaxStream; i++)
		{
			if(this->Streams[i]->GetStatus())
			{
				this->Streams[i]->SetStatus(false);
				this->StreamCount--;
			}

			this->Streams[i]->Release();
			this->Streams[i] = NULL;
		}

		this->MaxStream = NewMax;
//VerifyStreamCount();
		this->Streams.resize(NewMax);
	}
	else if (NewMax > this->MaxStream)
	{

		this->Streams.resize(NewMax);

		unsigned int i;
		for(i = this->MaxStream; i < NewMax; i++)
		{
			this->Streams[i] = this->CreateStream();
		}

		this->MaxStream = NewMax;
	}

}
//===========================================================================
//===========================================================================
void zsMatrix::SetSpeedVariance(unsigned int NewSpeedVariance)
{
	this->SpeedVariance = NewSpeedVariance;
}
//===========================================================================
//===========================================================================
void zsMatrix::SetMonotonousCleanupEnabled(bool NewMonotonousCleanupEnabled)
{
	this->MonotonousCleanupEnabled = NewMonotonousCleanupEnabled;
}
//===========================================================================
//===========================================================================
void zsMatrix::SetBackTrace(unsigned int NewBT)
{
	this->BackTrace = NewBT;
}
//===========================================================================
//===========================================================================
void zsMatrix::SetRandomizedCleanupEnabled(bool NewRandomizedCleanupEnabled)
{
	this->RandomizedCleanupEnabled = NewRandomizedCleanupEnabled;
}
//===========================================================================
//===========================================================================
void zsMatrix::SetLeading(unsigned int NewLeading)
{
	this->Leading = NewLeading;
}
//===========================================================================
//===========================================================================
void zsMatrix::SetSpacePad(unsigned int NewSP)
{
	this->SpacePad = NewSP;
}
//===========================================================================
//===========================================================================
void zsMatrix::SetSpecialStringStreamProbability(float NewProbability)
{
	if(NewProbability < 0.0) NewProbability = 0.0;
	else if(NewProbability > 1.0) NewProbability = 1.0;

	this->SpecialStringStreamProbability = NewProbability;
}
//===========================================================================
//===========================================================================
float zsMatrix::GetSpecialStringStreamProbability(void) const
{
	return this->SpecialStringStreamProbability;
}
//===========================================================================
//===========================================================================
void zsMatrix::SetSpecialStringColor(BYTE R,BYTE G,BYTE B,BYTE A)
{
	SpecialStringColor[0] = R;
	SpecialStringColor[1] = G;
	SpecialStringColor[2] = B;
	SpecialStringColor[3] = A;

	this->SpecialStringColorRef = RGB(this->SpecialStringColor[0],
									  this->SpecialStringColor[1],
									  this->SpecialStringColor[2]);
}
//===========================================================================
//===========================================================================
void zsMatrix::GetSpecialStringColor(BYTE &R,BYTE &G,BYTE &B,BYTE &A) const
{
	R = SpecialStringColor[0];
	G = SpecialStringColor[1];
	B = SpecialStringColor[2];
	A = SpecialStringColor[3];
}
//===========================================================================
//===========================================================================
void zsMatrix::SetSpecialStringFadeColor(BYTE R,BYTE G,BYTE B,BYTE A)
{
	SpecialStringFadeColor[0] = R;
	SpecialStringFadeColor[1] = G;
	SpecialStringFadeColor[2] = B;
	SpecialStringFadeColor[3] = A;

	this->SpecialStringFadeColorRef = RGB(this->SpecialStringFadeColor[0],
										  this->SpecialStringFadeColor[1],
										  this->SpecialStringFadeColor[2]);
}
//===========================================================================
//===========================================================================
void zsMatrix::GetSpecialStringFadeColor(BYTE &R,BYTE &G,BYTE &B,BYTE &A) const
{
	R = SpecialStringFadeColor[0];
	G = SpecialStringFadeColor[1];
	B = SpecialStringFadeColor[2];
	A = SpecialStringFadeColor[3];
}
//===========================================================================
//===========================================================================
void zsMatrix::SetSpecialStringBGColor(BYTE R,BYTE G,BYTE B,BYTE A)
{
	SpecialStringBGColor[0] = R;
	SpecialStringBGColor[1] = G;
	SpecialStringBGColor[2] = B;
	SpecialStringBGColor[3] = A;

	this->SpecialStringBGColorRef = RGB(this->SpecialStringBGColor[0],
										this->SpecialStringBGColor[1],
										this->SpecialStringBGColor[2]);



	if ((this->hSpecialStringBGBrush != NULL) && (0 == DeleteObject(this->hSpecialStringBGBrush)))
		PrintError("Failed to delete the SpecialString BG brush");
	else
		this->hSpecialStringBGBrush = NULL;
	if ((this->hSpecialStringBGPen != NULL) && (0 == DeleteObject(this->hSpecialStringBGPen)))
		PrintError("Failed to delete the SpecialString BG pen");
	else
		this->hSpecialStringBGPen = NULL;


	this->hSpecialStringBGPen = (HPEN)CreatePen(PS_SOLID,0,SpecialStringBGColorRef);
	this->hSpecialStringBGBrush = (HBRUSH)CreateSolidBrush(SpecialStringBGColorRef);
}
//===========================================================================
//===========================================================================
void zsMatrix::GetSpecialStringBGColor(BYTE &R,BYTE &G,BYTE &B,BYTE &A) const
{
	R = SpecialStringBGColor[0];
	G = SpecialStringBGColor[1];
	B = SpecialStringBGColor[2];
	A = SpecialStringBGColor[3];
}
//===========================================================================
//===========================================================================
void zsMatrix::SetSpecialStringFont(HFONT NewFont)
{
	if ((this->hSpecialStringFont != NULL) && (0 == DeleteObject(this->hSpecialStringFont)))
		PrintError("Failed to delete the SpecialString font");
	else
		this->hSpecialStringFont = NULL;

	if(NewFont != NULL)
	{
		LOGFONT NewLogFont;
		GetObject(NewFont,sizeof(LOGFONT),&NewLogFont);

		this->hSpecialStringFont = CreateFontIndirect(&NewLogFont);

		this->UpdateSpecialStringFontMeasurements();
	}
}
//===========================================================================
//===========================================================================
void zsMatrix::SetSpecialStringLogFont(const LOGFONT &NewFont)
{
	if ((this->hSpecialStringFont != NULL) && (0 == DeleteObject(this->hSpecialStringFont)))
		PrintError("Failed to delete the font");
	else
		this->hSpecialStringFont = NULL;


	this->hSpecialStringFont = CreateFontIndirect(&NewFont);

	this->UpdateSpecialStringFontMeasurements();
}
//===========================================================================
//===========================================================================
HFONT zsMatrix::GetSpecialStringFont(void) const
{
	return this->hSpecialStringFont;
}
//===========================================================================
//===========================================================================
void zsMatrix::AddSpecialStringToValidSet(const _TCHAR *NewString)
{
	this->ValidSpecialStrings.push_back(basic_string<_TCHAR>(NewString));
}
//===========================================================================
//===========================================================================
void zsMatrix::RemoveSpecialStringFromValidSet(const _TCHAR *String)
{
	bool Found = false;
	std::vector< basic_string<_TCHAR> >::iterator Iter = this->ValidSpecialStrings.begin();
	while((Iter != this->ValidSpecialStrings.end()) && !Found)
	{
		if(!Iter->compare(String))
		{
			Found = true;
			this->ValidSpecialStrings.erase(Iter);
		}
		else
			Iter++;
	}
}
//===========================================================================
//===========================================================================
void zsMatrix::ClearValidSpecialStringSet(void)
{
	this->ValidSpecialStrings.clear();
}
//===========================================================================
//===========================================================================
void zsMatrix::SetValidSpecialStringSet(const _TCHAR * const *NewStringSet,unsigned int Size)
{
	this->ValidSpecialStrings.clear();
	this->ValidSpecialStrings.resize(Size);

	unsigned int i;
	for(i = 0; i < Size; i++)
	{
		ValidSpecialStrings[i] = NewStringSet[i];
	}
}
//===========================================================================
//===========================================================================
unsigned int zsMatrix::GetNumSpecialStringsInSet(void) const
{
	return ValidSpecialStrings.size();
}
//===========================================================================
//===========================================================================
const _TCHAR *zsMatrix::GetValidSpecialString(unsigned int Index) const
{
	if(Index >= ValidSpecialStrings.size())
	{
		return NULL;
	}

	return ValidSpecialStrings[Index].c_str();
}
//===========================================================================
//===========================================================================

double zsMatrix::SetCoeffR1(const double &NewR1Coeff)
{
	double RetVal = this->CoeffR1;
	this->CoeffR1 = NewR1Coeff;
	return RetVal;
}
//===========================================================================
//===========================================================================
double zsMatrix::GetCoeffR1(void) const
{
	return this->CoeffR1;
}
//===========================================================================
//===========================================================================
double zsMatrix::SetCoeffR0(const double &NewR0Coeff)
{
	double RetVal = this->CoeffR0;
	this->CoeffR0 = NewR0Coeff;
	return RetVal;
}
//===========================================================================
//===========================================================================
double zsMatrix::GetCoeffR0(void) const
{
	return this->CoeffR0;
}
//===========================================================================
//===========================================================================

double zsMatrix::SetCoeffG1(const double &NewG1Coeff)
{
	double RetVal = this->CoeffG1;
	this->CoeffG1 = NewG1Coeff;
	return RetVal;
}
//===========================================================================
//===========================================================================
double zsMatrix::GetCoeffG1(void) const
{
	return this->CoeffG1;
}
//===========================================================================
//===========================================================================
double zsMatrix::SetCoeffG0(const double &NewG0Coeff)
{
	double RetVal = this->CoeffG0;
	this->CoeffG0 = NewG0Coeff;
	return RetVal;
}
//===========================================================================
//===========================================================================
double zsMatrix::GetCoeffG0(void) const
{
	return this->CoeffG0;
}
//===========================================================================
//===========================================================================

double zsMatrix::SetCoeffB1(const double &NewB1Coeff)
{
	double RetVal = this->CoeffB1;
	this->CoeffB1 = NewB1Coeff;
	return RetVal;
}
//===========================================================================
//===========================================================================
double zsMatrix::GetCoeffB1(void) const
{
	return this->CoeffB1;
}
//===========================================================================
//===========================================================================
double zsMatrix::SetCoeffB0(const double &NewB0Coeff)
{
	double RetVal = this->CoeffB0;
	this->CoeffB0 = NewB0Coeff;
	return RetVal;
}
//===========================================================================
//===========================================================================
double zsMatrix::GetCoeffB0(void) const
{
	return this->CoeffB0;
}
//===========================================================================
//===========================================================================

double zsMatrix::SetCoeffA1(const double &NewA1Coeff)
{
	double RetVal = this->CoeffA1;
	this->CoeffA1 = NewA1Coeff;
	return RetVal;
}
//===========================================================================
//===========================================================================
double zsMatrix::GetCoeffA1(void) const
{
	return this->CoeffA1;
}
//===========================================================================
//===========================================================================
double zsMatrix::SetCoeffA0(const double &NewA0Coeff)
{
	double RetVal = this->CoeffA0;
	this->CoeffA0 = NewA0Coeff;
	return RetVal;
}
//===========================================================================
//===========================================================================
double zsMatrix::GetCoeffA0(void) const
{
	return this->CoeffA0;
}
//===========================================================================
//===========================================================================

void zsMatrix::SethWnd(HWND NewhWnd)
{
	this->hWnd = NewhWnd;

	this->UpdateFontMeasurements();
}
//===========================================================================
//===========================================================================
void zsMatrix::AddCharToValidSet(_TCHAR NewChar)
{
	this->ValidChars.push_back(NewChar);
}
//===========================================================================
//===========================================================================
void zsMatrix::RemoveCharFromValidSet(_TCHAR Char)
{
	bool Found = false;
	std::vector<_TCHAR>::iterator Iter = this->ValidChars.begin();
	while((Iter != this->ValidChars.end()) && !Found)
	{
		if((*Iter) == Char)
		{
			Found = true;
			this->ValidChars.erase(Iter);
		}
		else
			Iter++;
	}
}
//===========================================================================
//===========================================================================
void zsMatrix::ClearValidCharSet(void)
{
	this->ValidChars.clear();
}
//===========================================================================
//===========================================================================
void zsMatrix::SetValidCharSet(const _TCHAR *NewCharSet,unsigned int Size)
{
	this->ValidChars.clear();
	this->ValidChars.resize(Size);

	unsigned int i;
	for(i = 0; i < Size; i++)
	{
		ValidChars[i] = NewCharSet[i];
	}
}
//===========================================================================
//===========================================================================
_TCHAR *zsMatrix::GetValidCharSet(void)
{
	return &(this->ValidChars[0]);
}
//===========================================================================
//===========================================================================
const _TCHAR *zsMatrix::GetValidCharSet(void) const 
{
	return &(this->ValidChars[0]);
}
//===========================================================================
//===========================================================================
unsigned int zsMatrix::GetNumCharsInSet(void) const 
{
	return this->ValidChars.size();
}
//===========================================================================
//===========================================================================
unsigned int zsMatrix::GetNumStreams(void) const
{
	return this->StreamCount;
}
//===========================================================================
//===========================================================================
IzsMatrixStream** zsMatrix::GetStreams(void)
{
	return &(this->Streams[0]);
}
//===========================================================================
//===========================================================================
const IzsMatrixStream** zsMatrix::GetStreams(void) const
{
	return (const IzsMatrixStream **)&(this->Streams[0]);
}
//===========================================================================
//===========================================================================
IzsMatrixStream *zsMatrix::CreateStream(void) const
{
	return new zsMatrixStream();
}
//===========================================================================
//===========================================================================
IzsMatrixStream *zsMatrix::CreateStream(const IzsMatrixStream &Orig) const
{
	return new zsMatrixStream(Orig);
}
//===========================================================================
//===========================================================================
void zsMatrix::SetBGBitmap(HBITMAP NewBitmap)
{
	BITMAP BitmapHeader;
	GetObject(NewBitmap,sizeof(BitmapHeader),&BitmapHeader);

	//Select the new bitmap into the BGDC
	HBITMAP hOldBGBitmap = (HBITMAP)SelectObject(this->hBGDC,this->hBGBitmap);
	//Select the passed bitmap into the TempSpaceDC
	HBITMAP hOldTempSpaceBitmap = (HBITMAP)SelectObject(this->hTempSpaceDC,NewBitmap);

	BitBlt(this->hBGDC,0,0,BitmapHeader.bmWidth,BitmapHeader.bmHeight,this->hTempSpaceDC,0,0,SRCCOPY);

	//Restore the appropriate bitmaps for the TempSpaceDC and the BGDC

	SelectObject(this->hTempSpaceDC,hOldTempSpaceBitmap);

	SelectObject(this->hBGDC,hOldBGBitmap);




}
//===========================================================================
//===========================================================================
void zsMatrix::UpdateSpecialStringFontMeasurements(void)
{
	HDC hdc = GetDC(this->hWnd);
	TEXTMETRIC tm;

	HFONT hPrevFont = (HFONT) SelectObject(hdc,this->hSpecialStringFont);

	GetTextMetrics(hdc,&tm);
	this->SpecialStringTextHeight = tm.tmHeight;
	this->SpecialStringTextWidth = tm.tmMaxCharWidth;

	SelectObject(hdc,hPrevFont);

	ReleaseDC(this->hWnd,hdc);
}
//===========================================================================
//===========================================================================
void zsMatrix::UpdateFontMeasurements(void)
{
	HDC hdc = GetDC(this->hWnd);
	TEXTMETRIC tm;

	HFONT hPrevFont = (HFONT) SelectObject(hdc,this->hFont);

	GetTextMetrics(hdc,&tm);
	this->textHeight = tm.tmHeight;
	this->textWidth = tm.tmMaxCharWidth;

	SelectObject(hdc,hPrevFont);

	ReleaseDC(this->hWnd,hdc);
}
//===========================================================================
//===========================================================================
void zsMatrix::CreateDestroyStreams(void)
{
	unsigned int i;
	int Width,Height;
	RECT ClientRect;
	GetClientRect(this->hWnd,&ClientRect);
	Width = ClientRect.right - ClientRect.left;
	Height = ClientRect.bottom - ClientRect.top;

	unsigned int ColumnWidth = max(this->textWidth,this->SpecialStringTextWidth);
	unsigned int ColumnsInRect = ceil(Width/(float)ColumnWidth);



	int MaxY = Height;

	if(this->MonotonousCleanupEnabled && this->RandomizedCleanupEnabled)
	{
		MaxY += (max(this->BackTrace,(this->Leading + this->SpacePad))*max(this->textHeight,this->SpecialStringTextHeight));
	}
	else if(this->MonotonousCleanupEnabled)
	{
		MaxY += (this->BackTrace*max(this->textHeight,this->SpecialStringTextHeight));
	}
	else if (this->RandomizedCleanupEnabled)
	{
		MaxY += ((this->Leading + this->SpacePad)*max(this->textHeight,this->SpecialStringTextHeight));
	}



	if (this->StreamCount < this->MaxStream)
	{
		//if (rand() < 200000)
		for(i = 0; i < this->MaxStream; i++)
		{
			if (this->Streams[i]->GetStatus() == false)
			{
				this->StreamCount++;
				this->Streams[i]->SetStatus(true);
				this->Streams[i]->SetSpecialStreamFlag(((float)rand()/(RAND_MAX+1)) < this->SpecialStringStreamProbability);

				if(this->ValidSpecialStrings.size())
				{
					this->Streams[i]->SetSpecialStreamStringIndex(rand()%this->ValidSpecialStrings.size());
				}
				else
				{
					this->Streams[i]->SetSpecialStreamStringIndex(0);
				}
				this->Streams[i]->SetSpecialStreamStringCharIndex(zsMatrixStream::GetSpecialStreamStringInvalidCharIndex());
				this->Streams[i]->SetStartY(0);//-textHeight;
				this->Streams[i]->SetStartX((rand()*ColumnsInRect/RAND_MAX)*ColumnWidth);
				this->Streams[i]->SetTicksToWait(rand() %(this->SpeedVariance + 1));
				this->Streams[i]->SetTickCounter(this->Streams[i]->GetTicksToWait());
				break;
			}
		}
	}
	for (i = 0; i < this->MaxStream ; i++)
	{
		if (this->Streams[i]->GetStatus() &&
			(this->Streams[i]->GetStartY() > (int)(MaxY) )
		   )
		{
			this->Streams[i]->SetStatus(false);
			this->StreamCount--;
			this->Streams[i]->SetStartY(0);
		}
	}
}
//===========================================================================
//===========================================================================
void zsMatrix::UpdateStreams(void)
{
	int Width,Height;
	RECT ClientRect;
	GetClientRect(this->hWnd,&ClientRect);
	Width = ClientRect.right - ClientRect.left;
	Height = ClientRect.bottom - ClientRect.top;

	int Rows = Height/this->textHeight;

	for (unsigned int  i=0; i < this->MaxStream; i++)
	{
		if (this->Streams[i]->GetStatus())
		{
			if(this->Streams[i]->GetTickCounter() == 0)
			{
				this->Streams[i]->SetStartY(this->Streams[i]->GetStartY() + this->textHeight);
				this->Streams[i]->SetNeedsDrawing(true);
				this->Streams[i]->SetTickCounter(this->Streams[i]->GetTicksToWait());

				if(this->Streams[i]->GetSpecialStreamFlag())
				{
					if(this->Streams[i]->GetSpecialStreamStringCharIndex() == this->Streams[i]->GetSpecialStreamStringInvalidCharIndex())
					{
						if(!Rows)
						{
							this->Streams[i]->SetSpecialStreamStringCharIndex(0);
						}
						else if(!(rand()%Rows))
						{
							this->Streams[i]->SetSpecialStreamStringCharIndex(0);
						}
					}
					else if(this->ValidSpecialStrings[this->Streams[i]->GetSpecialStreamStringIndex()].length() > this->Streams[i]->GetSpecialStreamStringCharIndex())
					{
						this->Streams[i]->SetSpecialStreamStringCharIndex(this->Streams[i]->GetSpecialStreamStringCharIndex()+1);
						this->Streams[i]->SetStartY(this->Streams[i]->GetStartY() - this->textHeight + this->SpecialStringTextHeight);
					}
					else
					{
						this->Streams[i]->SetSpecialStreamStringCharIndex(this->Streams[i]->GetSpecialStreamStringCharIndex()+1);
					}
				}
			}
			else
			{
				this->Streams[i]->SetTickCounter(this->Streams[i]->GetTickCounter() - 1);
			}
		}
	}
}
//===========================================================================
//===========================================================================
void zsMatrix::DisplayStreams(HDC hdc)
{
	int hdcState = SaveDC(hdc);
	int BGDCState = SaveDC(this->hBGDC);
	int TempSpaceDCState = SaveDC(this->hTempSpaceDC);
	int BackDCState = SaveDC(this->hBackDC);

	UINT Alignment = TA_TOP|TA_CENTER;
	SetTextAlign(hdc,Alignment);
	SetTextAlign(this->hBGDC,Alignment);
	SetTextAlign(this->hTempSpaceDC,Alignment);
	SetTextAlign(this->hBackDC,Alignment);
/*
	int ColorR = this->Color[0];
	int ColorG = this->Color[1];
	int ColorB = this->Color[2];

	float IncR = (float)ColorR/(this->SpeedVariance+1);
	float IncG = (float)ColorG/(this->SpeedVariance+1);
	float IncB = (float)ColorB/(this->SpeedVariance+1);

	int FadeColorR = this->FadeColor[0];
	int FadeColorG = this->FadeColor[1];
	int FadeColorB = this->FadeColor[2];
	
	float FadeIncR = (float)FadeColorR/(this->SpeedVariance+1);
	float FadeIncG = (float)FadeColorG/(this->SpeedVariance+1);
	float FadeIncB = (float)FadeColorB/(this->SpeedVariance+1);
*/
	//unsigned int TextHeight = this->textHeight;
	//unsigned int TextWidth = this->textWidth;

	zsCharDetails BrightCharDetails;
	zsCharDetails DimCharDetails;

	SelectObject(this->hBGDC,this->hBGBitmap);
	SelectObject(this->hTempSpaceDC,this->hTempSpaceBitmap);
	SelectObject(this->hBackDC,this->hBackBitmap);

	if(this->BGMode == bgmodeBitmap)
	{

		//SetROP2(this->hTempSpaceDC,this->ROP);
		SelectObject(this->hTempSpaceDC,this->hFont);
		SelectObject(this->hTempSpaceDC,this->hBGBrush);
		SelectObject(this->hTempSpaceDC,this->hBGPen);

		SelectObject(this->hBackDC,this->hFont);
		SelectObject(this->hBackDC,this->hBGBrush);
		SelectObject(this->hBackDC,this->hBGPen);

		SelectObject(hdc,this->hBGBrush);
		SelectObject(hdc,this->hBGPen);


		SetBkColor(this->hTempSpaceDC,this->BGColorRef);

		SetBkColor(this->hBackDC,this->BGColorRef);


		if(this->BGColor[3] < 128)
		{
			switch(this->BlendMode)
			{
			default:
			case(blendmodeXOR):
				{
					SetBkMode(this->hTempSpaceDC,TRANSPARENT);

					for (unsigned int i=0; i < this->MaxStream; i++)
					{
						if (this->Streams[i]->GetStatus() && this->Streams[i]->GetNeedsDrawing())
						{
							CalcCurrentAndPreceedingCharDetails(this->Streams[i],BrightCharDetails,DimCharDetails);


							//First Operation -- Output a random character in brighter color
							BitBlt(this->hBackDC,BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
								   WIDTH(BrightCharDetails.Rect),HEIGHT(BrightCharDetails.Rect),
								   this->hBGDC,
								   BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
								   SRCCOPY);


							BitBlt(this->hTempSpaceDC,BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
								   WIDTH(BrightCharDetails.Rect),HEIGHT(BrightCharDetails.Rect),
								   this->hBGDC,
								   BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
								   SRCCOPY);


							SelectObject(this->hTempSpaceDC,BrightCharDetails.Font);
							SetTextColor(this->hTempSpaceDC,BrightCharDetails.Color);
							TextOut(this->hTempSpaceDC,BrightCharDetails.TextPoint.x,BrightCharDetails.TextPoint.y,
									&BrightCharDetails.Char,1);


							BitBlt(this->hBackDC,BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
								   WIDTH(BrightCharDetails.Rect),HEIGHT(BrightCharDetails.Rect),
								   this->hTempSpaceDC,
								   BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
								   SRCINVERT);


							BitBlt(hdc,BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
								   WIDTH(BrightCharDetails.Rect),HEIGHT(BrightCharDetails.Rect),
								   this->hBackDC,
								   BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
								   SRCCOPY);


							//Second Operation -- Output another random character in a dimmer color
							BitBlt(this->hBackDC,DimCharDetails.Rect.left,DimCharDetails.Rect.top,
								   WIDTH(DimCharDetails.Rect),HEIGHT(DimCharDetails.Rect),
								   this->hBGDC,
								   DimCharDetails.Rect.left,DimCharDetails.Rect.top,
								   SRCCOPY);


							BitBlt(this->hTempSpaceDC,DimCharDetails.Rect.left,DimCharDetails.Rect.top,
								   WIDTH(DimCharDetails.Rect),HEIGHT(DimCharDetails.Rect),
								   this->hBGDC,
								   DimCharDetails.Rect.left,DimCharDetails.Rect.top,
								   SRCCOPY);


							SelectObject(this->hTempSpaceDC,DimCharDetails.Font);
							SetTextColor(this->hTempSpaceDC,DimCharDetails.Color);
							TextOut(this->hTempSpaceDC,DimCharDetails.TextPoint.x,DimCharDetails.TextPoint.y,
									&DimCharDetails.Char,1);


							BitBlt(this->hBackDC,DimCharDetails.Rect.left,DimCharDetails.Rect.top,
								   WIDTH(DimCharDetails.Rect),HEIGHT(DimCharDetails.Rect),
								   this->hTempSpaceDC,
								   DimCharDetails.Rect.left,DimCharDetails.Rect.top,
								   SRCINVERT);


							BitBlt(hdc,DimCharDetails.Rect.left,DimCharDetails.Rect.top,
								   WIDTH(DimCharDetails.Rect),HEIGHT(DimCharDetails.Rect),
								   this->hBackDC,
								   DimCharDetails.Rect.left,DimCharDetails.Rect.top,
								   SRCCOPY);


							//Third Operation -- Cleanup
							CommonCleanup();
			
							
							this->Streams[i]->SetNeedsDrawing(false);
									
						}
					}
				}
				break;
			case(blendmodeAND):
				{
					SetBkMode(this->hTempSpaceDC,TRANSPARENT);

					for (unsigned int i=0; i < this->MaxStream; i++)
					{
						if (this->Streams[i]->GetStatus() && this->Streams[i]->GetNeedsDrawing())
						{
							CalcCurrentAndPreceedingCharDetails(this->Streams[i],BrightCharDetails,DimCharDetails);


							//First Operation -- Output a random character in brighter color
							BitBlt(this->hBackDC,BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
								   WIDTH(BrightCharDetails.Rect),HEIGHT(BrightCharDetails.Rect),
								   this->hBGDC,
								   BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
								   SRCCOPY);

							Rectangle(this->hTempSpaceDC,
									  BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
									  BrightCharDetails.Rect.right,BrightCharDetails.Rect.bottom);

							SelectObject(this->hTempSpaceDC,BrightCharDetails.Font);
							SetTextColor(this->hTempSpaceDC,BrightCharDetails.Color);
							TextOut(this->hTempSpaceDC,BrightCharDetails.TextPoint.x,BrightCharDetails.TextPoint.y,
									&BrightCharDetails.Char,1);

							BitBlt(this->hBackDC,BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
								   WIDTH(BrightCharDetails.Rect),HEIGHT(BrightCharDetails.Rect),
								   this->hTempSpaceDC,
								   BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
								   SRCAND);


							BitBlt(hdc,BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
								   WIDTH(BrightCharDetails.Rect),HEIGHT(BrightCharDetails.Rect),
								   this->hBackDC,
								   BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
								   SRCCOPY);


							//Second Operation -- Output another random character in a dimmer color
							BitBlt(this->hBackDC,DimCharDetails.Rect.left,DimCharDetails.Rect.top,
								   WIDTH(DimCharDetails.Rect),HEIGHT(DimCharDetails.Rect),
								   this->hBGDC,
								   DimCharDetails.Rect.left,DimCharDetails.Rect.top,
								   SRCCOPY);

							Rectangle(this->hTempSpaceDC,
									  DimCharDetails.Rect.left,DimCharDetails.Rect.top,
									  DimCharDetails.Rect.right,DimCharDetails.Rect.bottom);

							SelectObject(this->hTempSpaceDC,DimCharDetails.Font);
							SetTextColor(this->hTempSpaceDC,DimCharDetails.Color);
							TextOut(this->hTempSpaceDC,DimCharDetails.TextPoint.x,DimCharDetails.TextPoint.y,
									&DimCharDetails.Char,1);

							BitBlt(this->hBackDC,DimCharDetails.Rect.left,DimCharDetails.Rect.top,
								   WIDTH(DimCharDetails.Rect),HEIGHT(DimCharDetails.Rect),
								   this->hTempSpaceDC,
								   DimCharDetails.Rect.left,DimCharDetails.Rect.top,
								   SRCAND);


							BitBlt(hdc,DimCharDetails.Rect.left,DimCharDetails.Rect.top,
								   WIDTH(DimCharDetails.Rect),HEIGHT(DimCharDetails.Rect),
								   this->hBackDC,
								   DimCharDetails.Rect.left,DimCharDetails.Rect.top,
								   SRCCOPY);




							//Third Operation -- Cleanup
							CommonCleanup();
			
							
							this->Streams[i]->SetNeedsDrawing(false);
									
						}
					}
				}
				break;
			case(blendmodeOR):
				{
					SetBkMode(this->hTempSpaceDC,TRANSPARENT);

					for (unsigned int i=0; i < this->MaxStream; i++)
					{
						if (this->Streams[i]->GetStatus() && this->Streams[i]->GetNeedsDrawing())
						{
							CalcCurrentAndPreceedingCharDetails(this->Streams[i],BrightCharDetails,DimCharDetails);


							//First Operation -- Output a random character in brighter color
							BitBlt(this->hBackDC,BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
								   WIDTH(BrightCharDetails.Rect),HEIGHT(BrightCharDetails.Rect),
								   this->hBGDC,
								   BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
								   SRCCOPY);


							Rectangle(this->hTempSpaceDC,
									  BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
									  BrightCharDetails.Rect.right,BrightCharDetails.Rect.bottom);

							SelectObject(this->hTempSpaceDC,BrightCharDetails.Font);
							SetTextColor(this->hTempSpaceDC,BrightCharDetails.Color);
							TextOut(this->hTempSpaceDC,BrightCharDetails.TextPoint.x,BrightCharDetails.TextPoint.y,
									&BrightCharDetails.Char,1);

							BitBlt(this->hBackDC,BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
								   WIDTH(BrightCharDetails.Rect),HEIGHT(BrightCharDetails.Rect),
								   this->hTempSpaceDC,
								   BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
								   SRCPAINT);


							SetTextColor(this->hTempSpaceDC,RGB(255,255,255));
							TextOut(this->hTempSpaceDC,BrightCharDetails.TextPoint.x,BrightCharDetails.TextPoint.y,
									&BrightCharDetails.Char,1);


							BitBlt(this->hBackDC,BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
								   WIDTH(BrightCharDetails.Rect),HEIGHT(BrightCharDetails.Rect),
								   this->hTempSpaceDC,
								   BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
								   SRCAND);


							BitBlt(hdc,BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
								   WIDTH(BrightCharDetails.Rect),HEIGHT(BrightCharDetails.Rect),
								   this->hBackDC,
								   BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
								   SRCCOPY);


							//Second Operation -- Output another random character in a dimmer color
							BitBlt(this->hBackDC,DimCharDetails.Rect.left,DimCharDetails.Rect.top,
								   WIDTH(DimCharDetails.Rect),HEIGHT(DimCharDetails.Rect),
								   this->hBGDC,
								   DimCharDetails.Rect.left,DimCharDetails.Rect.top,
								   SRCCOPY);


							Rectangle(this->hTempSpaceDC,
									  DimCharDetails.Rect.left,DimCharDetails.Rect.top,
									  DimCharDetails.Rect.right,DimCharDetails.Rect.bottom);

							SelectObject(this->hTempSpaceDC,DimCharDetails.Font);
							SetTextColor(this->hTempSpaceDC,DimCharDetails.Color);
							TextOut(this->hTempSpaceDC,DimCharDetails.TextPoint.x,DimCharDetails.TextPoint.y,
									&DimCharDetails.Char,1);

							BitBlt(this->hBackDC,DimCharDetails.Rect.left,DimCharDetails.Rect.top,
								   WIDTH(DimCharDetails.Rect),HEIGHT(DimCharDetails.Rect),
								   this->hTempSpaceDC,
								   DimCharDetails.Rect.left,DimCharDetails.Rect.top,
								   SRCPAINT);


							SetTextColor(this->hTempSpaceDC,RGB(255,255,255));
							TextOut(this->hTempSpaceDC,DimCharDetails.TextPoint.x,DimCharDetails.TextPoint.y,
									&DimCharDetails.Char,1);


							BitBlt(this->hBackDC,DimCharDetails.Rect.left,DimCharDetails.Rect.top,
								   WIDTH(DimCharDetails.Rect),HEIGHT(DimCharDetails.Rect),
								   this->hTempSpaceDC,
								   DimCharDetails.Rect.left,DimCharDetails.Rect.top,
								   SRCAND);


							BitBlt(hdc,DimCharDetails.Rect.left,DimCharDetails.Rect.top,
								   WIDTH(DimCharDetails.Rect),HEIGHT(DimCharDetails.Rect),
								   this->hBackDC,
								   DimCharDetails.Rect.left,DimCharDetails.Rect.top,
								   SRCCOPY);


							//Third Operation -- Cleanup
							CommonCleanup();
			
							
							this->Streams[i]->SetNeedsDrawing(false);
						}
					}
				}
				break;
			}
		}
		else
		{
			SetBkMode(this->hTempSpaceDC,OPAQUE);

			DWORD LastStepROP;
			switch(this->BlendMode)
			{
			case(blendmodeOR):
				{
					LastStepROP=SRCPAINT;
				}
				break;
			case(blendmodeAND):
				{
					LastStepROP=SRCAND;
				}
				break;
			default:
			case(blendmodeXOR):
				{
					LastStepROP=SRCINVERT;
				}
				break;
			}

			for (unsigned int i=0; i < this->MaxStream; i++)
			{
				if (this->Streams[i]->GetStatus() && this->Streams[i]->GetNeedsDrawing())
				{
					CalcCurrentAndPreceedingCharDetails(this->Streams[i],BrightCharDetails,DimCharDetails);


					//First Operation -- Output a random character in brighter color
					BitBlt(this->hBackDC,BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
						   WIDTH(BrightCharDetails.Rect),HEIGHT(BrightCharDetails.Rect),
						   this->hBGDC,
						   BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
						   SRCCOPY);



					Rectangle(this->hTempSpaceDC,BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
							  BrightCharDetails.Rect.right,BrightCharDetails.Rect.bottom);


					SelectObject(this->hTempSpaceDC,BrightCharDetails.Font);
					SetTextColor(this->hTempSpaceDC,BrightCharDetails.Color);
					TextOut(this->hTempSpaceDC,BrightCharDetails.TextPoint.x,BrightCharDetails.TextPoint.y,
							&BrightCharDetails.Char,1);

					BitBlt(this->hBackDC,BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
						   WIDTH(BrightCharDetails.Rect),HEIGHT(BrightCharDetails.Rect),
						   this->hTempSpaceDC,
						   BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
						   LastStepROP);


					BitBlt(hdc,BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
						   WIDTH(BrightCharDetails.Rect),HEIGHT(BrightCharDetails.Rect),
						   this->hBackDC,
						   BrightCharDetails.Rect.left,BrightCharDetails.Rect.top,
						   SRCCOPY);




					//Second Operation -- Output another random character in a dimmer color
					BitBlt(this->hBackDC,DimCharDetails.Rect.left,DimCharDetails.Rect.top,
						   WIDTH(DimCharDetails.Rect),HEIGHT(DimCharDetails.Rect),
						   this->hBGDC,
						   DimCharDetails.Rect.left,DimCharDetails.Rect.top,
						   SRCCOPY);

					Rectangle(this->hTempSpaceDC,DimCharDetails.Rect.left,DimCharDetails.Rect.top,
							  DimCharDetails.Rect.right,DimCharDetails.Rect.bottom);


					SelectObject(this->hTempSpaceDC,DimCharDetails.Font);
					SetTextColor(this->hTempSpaceDC,DimCharDetails.Color);
					TextOut(this->hTempSpaceDC,DimCharDetails.TextPoint.x,DimCharDetails.TextPoint.y,
							&DimCharDetails.Char,1);

					BitBlt(this->hBackDC,DimCharDetails.Rect.left,DimCharDetails.Rect.top,
						   WIDTH(DimCharDetails.Rect),HEIGHT(DimCharDetails.Rect),
						   this->hTempSpaceDC,
						   DimCharDetails.Rect.left,DimCharDetails.Rect.top,
						   LastStepROP);


					BitBlt(hdc,DimCharDetails.Rect.left,DimCharDetails.Rect.top,
						   WIDTH(DimCharDetails.Rect),HEIGHT(DimCharDetails.Rect),
						   this->hBackDC,
						   DimCharDetails.Rect.left,DimCharDetails.Rect.top,
						   SRCCOPY);



	/*
					randomval = rand();
		//SelectObject(this->hBGDC,this->hBGBitmap);
					BitBlt(hdc,x,y-(randomval*this->SpacePad/RAND_MAX+this->Leading)*TextHeight,
							   TextWidth,TextHeight,this->hBGDC,x,y-(randomval*this->SpacePad/RAND_MAX+this->Leading)*TextHeight,SRCCOPY);

					BitBlt(hdc,x,y-(this->BackTrace*TextHeight),
							   TextWidth,TextHeight,this->hBGDC,x,y-(this->BackTrace*TextHeight),SRCCOPY);
	*/
		

					//Third Operation -- Cleanup
					SpecialBitBltCleanup();

					
					this->Streams[i]->SetNeedsDrawing(false);
				}
			}
		}

	}
	else
	{

		//SetROP2(hdc,this->ROP);
		SelectObject(hdc,this->hFont);

		SelectObject(hdc,this->hBGBrush);
		SelectObject(hdc,this->hBGPen);

		SetBkColor(hdc,this->BGColorRef);





		if(this->BGColor[3] < 128)
		{
			SetBkMode(hdc,TRANSPARENT);

			for (unsigned int i=0; i < this->MaxStream; i++)
			{
				if (this->Streams[i]->GetStatus() && this->Streams[i]->GetNeedsDrawing())
				{
					CalcCurrentAndPreceedingCharDetails(this->Streams[i],BrightCharDetails,DimCharDetails);


					//First Operation -- Output a random character in brighter color
					SelectObject(hdc,BrightCharDetails.Font);
					SetTextColor(hdc,BrightCharDetails.Color);
					TextOut(hdc,BrightCharDetails.TextPoint.x,BrightCharDetails.TextPoint.y,&BrightCharDetails.Char,1);


					//Second Operation -- Output another random character in a dimmer color
					SelectObject(hdc,DimCharDetails.Font);
					SetTextColor(hdc,DimCharDetails.Color);
					TextOut(hdc,DimCharDetails.TextPoint.x,DimCharDetails.TextPoint.y,&DimCharDetails.Char,1);





					//Third Operation -- Cleanup
					CommonCleanup();
					
					this->Streams[i]->SetNeedsDrawing(false);

				}
			}
		}
		else
		{
			SetBkMode(hdc,OPAQUE);
			for (unsigned int i=0; i < this->MaxStream; i++)
			{
				if (this->Streams[i]->GetStatus() && this->Streams[i]->GetNeedsDrawing())
				{

					CalcCurrentAndPreceedingCharDetails(this->Streams[i],BrightCharDetails,DimCharDetails);



					//First Operation -- Output a random character in brighter color

					Rectangle(hdc,BrightCharDetails.Rect.left,
							  BrightCharDetails.Rect.top,
							  BrightCharDetails.Rect.right,
							  BrightCharDetails.Rect.bottom);
					SelectObject(hdc,BrightCharDetails.Font);
					SetTextColor(hdc,BrightCharDetails.Color);
					TextOut(hdc,BrightCharDetails.TextPoint.x,BrightCharDetails.TextPoint.y,&BrightCharDetails.Char,1);


					//Second Operation -- Output another random character in a dimmer color


					Rectangle(hdc,DimCharDetails.Rect.left,
							  DimCharDetails.Rect.top,
							  DimCharDetails.Rect.right,
							  DimCharDetails.Rect.bottom);
					SelectObject(hdc,DimCharDetails.Font);
					SetTextColor(hdc,DimCharDetails.Color);
					TextOut(hdc,DimCharDetails.TextPoint.x,DimCharDetails.TextPoint.y,&DimCharDetails.Char,1);





					//Third Operation -- Cleanup
					CommonCleanup();
					
					this->Streams[i]->SetNeedsDrawing(false);

				}
			}
		}



	}
	
	RestoreDC(hdc,hdcState);
	RestoreDC(this->hBGDC,BGDCState);
	RestoreDC(this->hTempSpaceDC,TempSpaceDCState);
	RestoreDC(this->hBackDC,BackDCState);

}
//===========================================================================
//===========================================================================
void zsMatrix::CalcCurrentAndPreceedingCharDetails(const IzsMatrixStream *Stream,zsCharDetails &Current,zsCharDetails &Preceeding) const
{
	DWORD R,G,B;
	unsigned int ColumnWidth = max(this->textWidth,this->SpecialStringTextWidth);
	unsigned int HalfColumnWidth = ColumnWidth/2;

	if(Stream->GetSpecialStreamStringCharIndex() == Stream->GetSpecialStreamStringInvalidCharIndex())
	{
		Current.Char = (this->ValidChars[rand()*(this->ValidChars.size()-1)/RAND_MAX]);
		Current.IsSpecialStringChar = false;
		Preceeding.Char = (this->ValidChars[rand()*(this->ValidChars.size()-1)/RAND_MAX]);
		Preceeding.IsSpecialStringChar = false;
	}
	else if(Stream->GetSpecialStreamStringIndex() >= this->ValidSpecialStrings.size())
	{
		Current.Char = (this->ValidChars[rand()*(this->ValidChars.size()-1)/RAND_MAX]);
		Current.IsSpecialStringChar = false;
		Preceeding.Char = (this->ValidChars[rand()*(this->ValidChars.size()-1)/RAND_MAX]);
		Preceeding.IsSpecialStringChar = false;
	}
	else if(Stream->GetSpecialStreamStringCharIndex() < this->ValidSpecialStrings[Stream->GetSpecialStreamStringIndex()].length())
	{
		Current.Char = this->ValidSpecialStrings[Stream->GetSpecialStreamStringIndex()][Stream->GetSpecialStreamStringCharIndex()];
		Current.IsSpecialStringChar = true;
		if(Stream->GetSpecialStreamStringCharIndex() == 0)
		{
			Preceeding.Char = (this->ValidChars[rand()*(this->ValidChars.size()-1)/RAND_MAX]);
			Preceeding.IsSpecialStringChar = false;
		}
		else
		{
			Preceeding.Char = this->ValidSpecialStrings[Stream->GetSpecialStreamStringIndex()][Stream->GetSpecialStreamStringCharIndex()-1];
			Preceeding.IsSpecialStringChar = true;
		}
	}
	else if(Stream->GetSpecialStreamStringCharIndex() == this->ValidSpecialStrings[Stream->GetSpecialStreamStringIndex()].length())
	{
		Current.Char = (this->ValidChars[rand()*(this->ValidChars.size()-1)/RAND_MAX]);
		Current.IsSpecialStringChar = false;
		Preceeding.Char = this->ValidSpecialStrings[Stream->GetSpecialStreamStringIndex()][Stream->GetSpecialStreamStringCharIndex()-1];
		Preceeding.IsSpecialStringChar = true;
	}
	else
	{
		Current.Char = (this->ValidChars[rand()*(this->ValidChars.size()-1)/RAND_MAX]);
		Current.IsSpecialStringChar = false;
		Preceeding.Char = (this->ValidChars[rand()*(this->ValidChars.size()-1)/RAND_MAX]);
		Preceeding.IsSpecialStringChar = false;
	}

	if(Current.IsSpecialStringChar)
	{
		Current.Font = this->hSpecialStringFont;
		R = this->SpecialStringColor[0]-((Stream->GetTicksToWait()*this->SpecialStringColor[0])/(this->SpeedVariance+1));
		G = this->SpecialStringColor[1]-((Stream->GetTicksToWait()*this->SpecialStringColor[1])/(this->SpeedVariance+1));
		B = this->SpecialStringColor[2]-((Stream->GetTicksToWait()*this->SpecialStringColor[2])/(this->SpeedVariance+1));

		R = R*this->CoeffR1 + this->CoeffR0; CLAMP(R,0,255);
		G = G*this->CoeffG1 + this->CoeffG0; CLAMP(G,0,255);
		B = B*this->CoeffB1 + this->CoeffB0; CLAMP(B,0,255);

		Current.Color = RGB(R,G,B);

		Current.Rect.left = Stream->GetStartX();
		Current.Rect.top = Stream->GetStartY();
		Current.Rect.right = Current.Rect.left + ColumnWidth;
		Current.Rect.bottom = Current.Rect.top + this->SpecialStringTextHeight+1;

		Current.TextPoint.x = Current.Rect.left + HalfColumnWidth;
		Current.TextPoint.y = Current.Rect.top;
	}
	else
	{
		Current.Font = this->hFont;
		R = this->Color[0]-((Stream->GetTicksToWait()*this->Color[0])/(this->SpeedVariance+1));
		G = this->Color[1]-((Stream->GetTicksToWait()*this->Color[1])/(this->SpeedVariance+1));
		B = this->Color[2]-((Stream->GetTicksToWait()*this->Color[2])/(this->SpeedVariance+1));

		R = R*this->CoeffR1 + this->CoeffR0; CLAMP(R,0,255);
		G = G*this->CoeffG1 + this->CoeffG0; CLAMP(G,0,255);
		B = B*this->CoeffB1 + this->CoeffB0; CLAMP(B,0,255);

		Current.Color = RGB(R,G,B);

		Current.Rect.left = Stream->GetStartX();
		Current.Rect.top = Stream->GetStartY();
		Current.Rect.right = Current.Rect.left + ColumnWidth;
		Current.Rect.bottom = Current.Rect.top + this->textHeight+1;

		Current.TextPoint.x = Current.Rect.left + HalfColumnWidth;
		Current.TextPoint.y = Current.Rect.top;
	}


	if(Preceeding.IsSpecialStringChar)
	{
		Preceeding.Font = this->hSpecialStringFont;
		R = this->SpecialStringFadeColor[0]-((Stream->GetTicksToWait()*this->SpecialStringFadeColor[0])/(this->SpeedVariance+1));
		G = this->SpecialStringFadeColor[1]-((Stream->GetTicksToWait()*this->SpecialStringFadeColor[1])/(this->SpeedVariance+1));
		B = this->SpecialStringFadeColor[2]-((Stream->GetTicksToWait()*this->SpecialStringFadeColor[2])/(this->SpeedVariance+1));

		R = R*this->CoeffR1 + this->CoeffR0; CLAMP(R,0,255);
		G = G*this->CoeffG1 + this->CoeffG0; CLAMP(G,0,255);
		B = B*this->CoeffB1 + this->CoeffB0; CLAMP(B,0,255);

		Preceeding.Color = RGB(R,G,B);

		Preceeding.Rect.left = Stream->GetStartX();
		Preceeding.Rect.top = Stream->GetStartY() - this->SpecialStringTextHeight;
		Preceeding.Rect.right = Preceeding.Rect.left + ColumnWidth;
		Preceeding.Rect.bottom = Preceeding.Rect.top + this->SpecialStringTextHeight+1;

		Preceeding.TextPoint.x = Preceeding.Rect.left + HalfColumnWidth;
		Preceeding.TextPoint.y = Preceeding.Rect.top;
	}
	else
	{
		Preceeding.Font = this->hFont;
		R = this->FadeColor[0]-((Stream->GetTicksToWait()*this->FadeColor[0])/(this->SpeedVariance+1));
		G = this->FadeColor[1]-((Stream->GetTicksToWait()*this->FadeColor[1])/(this->SpeedVariance+1));
		B = this->FadeColor[2]-((Stream->GetTicksToWait()*this->FadeColor[2])/(this->SpeedVariance+1));

		R = R*this->CoeffR1 + this->CoeffR0; CLAMP(R,0,255);
		G = G*this->CoeffG1 + this->CoeffG0; CLAMP(G,0,255);
		B = B*this->CoeffB1 + this->CoeffB0; CLAMP(B,0,255);

		Preceeding.Color = RGB(R,G,B);

		Preceeding.Rect.left = Stream->GetStartX();
		Preceeding.Rect.top = Stream->GetStartY() - this->textHeight;
		Preceeding.Rect.right = Preceeding.Rect.left + ColumnWidth;
		Preceeding.Rect.bottom = Preceeding.Rect.top + this->textHeight+1;

		Preceeding.TextPoint.x = Preceeding.Rect.left + HalfColumnWidth;
		Preceeding.TextPoint.y = Preceeding.Rect.top;
	}

}
//===========================================================================
//===========================================================================
void zsMatrix::CalcRectForNthBackChar(const IzsMatrixStream *Stream,unsigned int N,RECT &Rect) const
{
	Rect.left = Stream->GetStartX();
	Rect.right = Rect.left+max(this->textWidth,this->SpecialStringTextWidth)+1;

	if(Stream->GetSpecialStreamStringCharIndex() == Stream->GetSpecialStreamStringInvalidCharIndex())
	{
		Rect.top = Stream->GetStartY() - (N*this->textHeight);
		Rect.bottom = Rect.top + this->textHeight + 1;
	}
	else if(Stream->GetSpecialStreamStringIndex() >= this->ValidSpecialStrings.size())
	{
		Rect.top = Stream->GetStartY() - (N*this->textHeight);
		Rect.bottom = Rect.top + this->textHeight + 1;
	}
	else if(Stream->GetSpecialStreamStringCharIndex() <= this->ValidSpecialStrings[Stream->GetSpecialStreamStringIndex()].length())
	{
		unsigned int NumSpecialStringCharsBack = 0;
		
		if(N <= Stream->GetSpecialStreamStringCharIndex())
		{
			NumSpecialStringCharsBack = N;

			Rect.top = Stream->GetStartY() - NumSpecialStringCharsBack*this->SpecialStringTextHeight;
			Rect.bottom = Rect.top + this->SpecialStringTextHeight + 1;

			return;
		}

		NumSpecialStringCharsBack = Stream->GetSpecialStreamStringCharIndex();
		N -= NumSpecialStringCharsBack;
		unsigned int NumRegularCharsBack = N;

		Rect.top = Stream->GetStartY() - (NumRegularCharsBack*this->textHeight + NumSpecialStringCharsBack*this->SpecialStringTextHeight);
		Rect.bottom = Rect.top + this->textHeight + 1;
	}
	else
	{
		unsigned int NumRegularCharsBack = 0;
		unsigned int CurrentPostSpecialStringIndex = (Stream->GetSpecialStreamStringCharIndex()-this->ValidSpecialStrings[Stream->GetSpecialStreamStringIndex()].length());
		
		if( N <= CurrentPostSpecialStringIndex)
		{
			NumRegularCharsBack = N;

			Rect.top = Stream->GetStartY() - NumRegularCharsBack*this->textHeight;
			Rect.bottom = Rect.top + this->textHeight + 1;

			return;
		}

		NumRegularCharsBack = CurrentPostSpecialStringIndex;
		N -= NumRegularCharsBack;

		unsigned int NumSpecialStringCharsBack = 0;
		
		if( N <= (this->ValidSpecialStrings[Stream->GetSpecialStreamStringIndex()].length()))
		{
			NumSpecialStringCharsBack = N;

			Rect.top = Stream->GetStartY() - (NumRegularCharsBack*this->textHeight + NumSpecialStringCharsBack*this->SpecialStringTextHeight);
			Rect.bottom = Rect.top + this->SpecialStringTextHeight + 1;

			return;
		}
		
		NumSpecialStringCharsBack = (this->ValidSpecialStrings[Stream->GetSpecialStreamStringIndex()].length());
		
		N -= NumSpecialStringCharsBack;

		Rect.top = Stream->GetStartY() - ((NumRegularCharsBack + N)*this->textHeight + NumSpecialStringCharsBack*this->SpecialStringTextHeight);
		Rect.bottom = Rect.top + this->textHeight + 1;
	}
}
//===========================================================================
//===========================================================================
HRESULT zsMatrixFactory::CreatezsMatrix(IzsMatrix **ppNewMatrix,HWND TargetWindow,HBITMAP BGBitmap)
{
	*ppNewMatrix=(IzsMatrix*) new zsMatrix(TargetWindow,BGBitmap);
	return NO_ERROR;
}
//===========================================================================
//===========================================================================
HRESULT zsMatrixFactory::CreatezsMatrix(IzsMatrix **ppNewMatrix,const IzsMatrix &OtherMatrix)
{
	*ppNewMatrix=(IzsMatrix*) new zsMatrix(OtherMatrix);
	return NO_ERROR;
}
//===========================================================================
//===========================================================================
HRESULT zsMatrixFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void** ppObject)
{
	if (pUnkOuter!=NULL)
	{
		return CLASS_E_NOAGGREGATION;
	}

	zsMatrix *pMatrix = new zsMatrix();
	if (FAILED(pMatrix->QueryInterface(riid, ppObject)))
	{
		delete pMatrix;
		*ppObject=NULL;
		return E_NOINTERFACE;
	}
	return NO_ERROR;
}
//===========================================================================
//===========================================================================
HRESULT	zsMatrixFactory::LockServer(BOOL fLock)
{
	if (fLock)
	{
		g_dwRefCount++;
	}
	else
	{
		g_dwRefCount--;
	}
	return NO_ERROR;
}
//===========================================================================
//===========================================================================
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppObject)
{
	if(rclsid == CLSID_ZSMATRIX)
	{
		zsMatrixFactory *pFactory = new zsMatrixFactory();

		if(FAILED(pFactory->QueryInterface(riid,ppObject)))
		{
			delete pFactory;
			*ppObject=NULL;
			return E_INVALIDARG;
		}
	}
	else
	{
		return CLASS_E_CLASSNOTAVAILABLE;
	}
	return NO_ERROR;

}
//===========================================================================
//===========================================================================
HRESULT _stdcall DllCanUnloadNow()
{
	if (g_dwRefCount)
	{
		return S_FALSE;
	}
	else
	{
		return S_OK;
	}
}
//===========================================================================
//===========================================================================
STDAPI DllRegisterServer(void)
{
	HKEY hKeyCLSID, hKeyInproc32;
	DWORD dwDisposition;

	HMODULE hModule=GetModuleHandle(_T("zsMatrix.DLL"));
	if (!hModule)
	{
		return E_UNEXPECTED;
	}

	TCHAR szName[MAX_PATH+1];
	if (GetModuleFileName(hModule, szName, sizeof(szName))==0)
	{
		return E_UNEXPECTED;
	}



	if (RegCreateKeyEx(HKEY_CLASSES_ROOT, 
			_T("CLSID\\{65A36275-972A-4804-A1FA-53DB4E39DE26}"), 
			NULL, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
			&hKeyCLSID, &dwDisposition)!=ERROR_SUCCESS)
	{
		return E_UNEXPECTED;
	}

	if (RegSetValueEx(hKeyCLSID, _T(""), NULL, REG_SZ, (BYTE*) _T("zsMatrixStream"), sizeof(_T("zsMatrixStream")))!=ERROR_SUCCESS)
	{
		RegCloseKey(hKeyCLSID);
		return E_UNEXPECTED;
	}

	if (RegCreateKeyEx(hKeyCLSID, 
			_T("InprocServer32"), 
			NULL, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
			&hKeyInproc32, &dwDisposition)!=ERROR_SUCCESS)
	{
		RegCloseKey(hKeyCLSID);
		return E_UNEXPECTED;
	}

	if (RegSetValueEx(hKeyInproc32, _T(""), NULL, REG_SZ, (BYTE*) szName, sizeof(TCHAR)*(lstrlen(szName)+1))!=ERROR_SUCCESS)
	{
		RegCloseKey(hKeyInproc32);
		RegCloseKey(hKeyCLSID);
		return E_UNEXPECTED;
	}

	RegCloseKey(hKeyInproc32);
	RegCloseKey(hKeyCLSID);






	if (RegCreateKeyEx(HKEY_CLASSES_ROOT, 
			_T("CLSID\\{2E393599-F2E3-484a-9B03-D414F6CBA5EB}"), 
			NULL, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
			&hKeyCLSID, &dwDisposition)!=ERROR_SUCCESS)
	{
		return E_UNEXPECTED;
	}

	if (RegSetValueEx(hKeyCLSID, _T(""), NULL, REG_SZ, (BYTE*) _T("zsMatrix"), sizeof(_T("zsMatrix")))!=ERROR_SUCCESS)
	{
		RegCloseKey(hKeyCLSID);
		return E_UNEXPECTED;
	}

	if (RegCreateKeyEx(hKeyCLSID, 
			_T("InprocServer32"), 
			NULL, _T(""), REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
			&hKeyInproc32, &dwDisposition)!=ERROR_SUCCESS)
	{
		RegCloseKey(hKeyCLSID);
		return E_UNEXPECTED;
	}

	if (RegSetValueEx(hKeyInproc32, _T(""), NULL, REG_SZ, (BYTE*) szName, sizeof(TCHAR)*(lstrlen(szName)+1))!=ERROR_SUCCESS)
	{
		RegCloseKey(hKeyInproc32);
		RegCloseKey(hKeyCLSID);
		return E_UNEXPECTED;
	}

	RegCloseKey(hKeyInproc32);
	RegCloseKey(hKeyCLSID);
	return NOERROR;
}
//===========================================================================
//===========================================================================
STDAPI DllUnregisterServer(void)
{
	if (RegDeleteKey(HKEY_CLASSES_ROOT, 
			_T("CLSID\\{65A36275-972A-4804-A1FA-53DB4E39DE26}\\InprocServer32"))!=ERROR_SUCCESS)
	{
		return E_UNEXPECTED;
	}

	if (RegDeleteKey(HKEY_CLASSES_ROOT, 
			_T("CLSID\\{65A36275-972A-4804-A1FA-53DB4E39DE26}"))!=ERROR_SUCCESS)
	{
		return E_UNEXPECTED;
	}

	if (RegDeleteKey(HKEY_CLASSES_ROOT, 
			_T("CLSID\\{2E393599-F2E3-484a-9B03-D414F6CBA5EB}\\InprocServer32"))!=ERROR_SUCCESS)
	{
		return E_UNEXPECTED;
	}

	if (RegDeleteKey(HKEY_CLASSES_ROOT, 
			_T("CLSID\\{2E393599-F2E3-484a-9B03-D414F6CBA5EB}"))!=ERROR_SUCCESS)
	{
		return E_UNEXPECTED;
	}

	return NOERROR;
}

