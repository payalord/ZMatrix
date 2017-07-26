/*!=========================================================================
//
//  Program:   ZMatrix
//  Module:    $RCSfile: IzsMatrix.h,v $
//  Language:  C/C++
//  Date:      $Date: 2003/05/06 05:56:12 $
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

#ifndef __IzsMatrix_h
#define __IzsMatrix_h



#include <windows.h>
#include <ole2.h>
#include <vector>
#include <tchar.h>

#ifdef USE_MEM_MANAGER
#include "../mmgr.h"
#endif

#ifdef ZSMATRIX_EXPORTS
	#ifndef IMPEXP
		#define IMPEXP __declspec(dllexport)
	#endif
	#ifndef EXPIMP_TEMPLATE
		#define EXPIMP_TEMPLATE
	#endif
#else
	#ifdef _MSC_VER
		#pragma warning( disable : 4231 )
		#ifndef IMPEXP
			#define IMPEXP __declspec(dllimport)
		#endif
		#ifndef EXPIMP_TEMPLATE
			#define EXPIMP_TEMPLATE extern
		#endif
	#else
//Non MSVC compilers can't link to the dll (stupid lib formats), so they must have their own version.
		#ifndef IMPEXP
			#define IMPEXP
		#endif
		#ifndef EXPIMP_TEMPLATE
			#define EXPIMP_TEMPLATE
		#endif
	#endif
#endif


// {65A36275-972A-4804-A1FA-53DB4E39DE26}
extern "C" IMPEXP const GUID CLSID_ZSMATRIXSTREAM;
//{ 0x65a36275, 0x972a, 0x4804, { 0xa1, 0xfa, 0x53, 0xdb, 0x4e, 0x39, 0xde, 0x26 } };

// {95509A7E-CA62-4867-A7C7-DB0EFAF32F7C}
extern "C" IMPEXP const GUID IID_IZSMATRIXSTREAM;
//{ 0x95509a7e, 0xca62, 0x4867, { 0xa7, 0xc7, 0xdb, 0xe, 0xfa, 0xf3, 0x2f, 0x7c } };

// {2E393599-F2E3-484a-9B03-D414F6CBA5EB}
extern "C" IMPEXP const GUID CLSID_ZSMATRIX;
//{ 0x2e393599, 0xf2e3, 0x484a, { 0x9b, 0x3, 0xd4, 0x14, 0xf6, 0xcb, 0xa5, 0xeb } };

// {CBEAC95F-3125-4603-A0C9-0929BDC60FBC}
extern "C" IMPEXP const GUID IID_IZSMATRIX;
//{ 0xcbeac95f, 0x3125, 0x4603, { 0xa0, 0xc9, 0x9, 0x29, 0xbd, 0xc6, 0xf, 0xbc } };



class IzsMatrixStream: public IUnknown
{
public:
	virtual IzsMatrixStream &__stdcall operator=(const IzsMatrixStream &Other) = 0;
	virtual void __stdcall CopyFrom(const IzsMatrixStream &Other) = 0;

	virtual void __stdcall SetStartX(int NewStartX) = 0;
	virtual int __stdcall GetStartX(void) const = 0;

	virtual void __stdcall SetStartY(int NewStartY) = 0;
	virtual int __stdcall GetStartY(void) const = 0;

	virtual void __stdcall SetTickCounter(unsigned int NewTicks) = 0;
	virtual unsigned int __stdcall GetTickCounter(void) const = 0;

	virtual void __stdcall SetTicksToWait(unsigned int NewTicksToWait) = 0;
	virtual unsigned int __stdcall GetTicksToWait(void) const = 0;

	virtual void __stdcall SetStatus(bool NewStatus) = 0;
	virtual bool __stdcall GetStatus(void) const = 0;

	virtual void __stdcall SetSpecialStreamFlag(bool NewSpecialStreamFlag) = 0;
	virtual bool __stdcall GetSpecialStreamFlag(void) const = 0;

	virtual void __stdcall SetSpecialStreamStringIndex(unsigned int NewSpecialStreamStringIndex) = 0;
	virtual unsigned int __stdcall GetSpecialStreamStringIndex(void) const = 0;

	virtual void __stdcall SetSpecialStreamStringCharIndex(unsigned int NewSpecialStreamStringCharIndex) = 0;
	virtual unsigned int __stdcall GetSpecialStreamStringCharIndex(void) const = 0;

	static unsigned int __stdcall GetSpecialStreamStringInvalidCharIndex(void) { return 0xFFFFFFFF;};

	virtual void __stdcall SetNeedsDrawing(bool NewNeedsDrawing) = 0;
	virtual bool __stdcall GetNeedsDrawing(void) const = 0;
};


typedef int TBGMode;
typedef int TBlendMode;
enum __TBGMode{bgmodeBitmap=0,bgmodeColor=1};
enum __TBlendMode{blendmodeXOR=0,blendmodeAND=1,blendmodeOR=2};

class IzsMatrix: public IUnknown
{
public:

	virtual void __stdcall CopyFrom(const IzsMatrix &Other) = 0;

	virtual int __stdcall Render(HDC hdc) = 0;

	virtual void __stdcall UpdateTarget(HWND hWnd,HBITMAP BGBITMAP) = 0;

	virtual void __stdcall SetColor(BYTE R,BYTE G,BYTE B,BYTE A) = 0;
	virtual void __stdcall GetColor(BYTE &R,BYTE &G,BYTE &B,BYTE &A) const = 0;

	virtual void __stdcall SetFadeColor(BYTE R,BYTE G,BYTE B,BYTE A) = 0;
	virtual void __stdcall GetFadeColor(BYTE &R,BYTE &G,BYTE &B,BYTE &A) const = 0;

	virtual void __stdcall SetBGColor(BYTE R,BYTE G,BYTE B,BYTE A) = 0;
	virtual void __stdcall GetBGColor(BYTE &R,BYTE &G,BYTE &B,BYTE &A) const = 0;

	virtual void __stdcall SetBGMode(const TBGMode &NewMode) = 0;
	virtual TBGMode __stdcall GetBGMode(void) const = 0;

	virtual void __stdcall SetBlendMode(TBlendMode NewBlendMode) = 0;
	virtual TBlendMode __stdcall GetBlendMode(void) const = 0;

	virtual void __stdcall SetFont(HFONT NewFont) = 0;
	virtual void __stdcall SetLogFont(const LOGFONT &NewFont) = 0;
	virtual HFONT __stdcall GetFont(void) const = 0;

	virtual void __stdcall SetBGBitmap(HBITMAP NewBitmap) = 0;
	virtual HBITMAP __stdcall SetBGBitmap_NonCopy(HBITMAP NewBitmap) = 0;
	virtual HBITMAP __stdcall GetBGBitmap(void) const = 0;

	virtual HDC __stdcall GetBGDC(void) const = 0;

	//------------------------------------------------------------
	//Basic stream rendering parameters
	//------------------------------------------------------------

	virtual void __stdcall SetMaxStream(unsigned int NewMax) = 0;
	virtual unsigned int __stdcall GetMaxStream(void) const = 0;

	virtual void __stdcall SetSpeedVariance(unsigned int NewSpeedVariance) = 0;
	virtual unsigned int __stdcall GetSpeedVariance(void) const = 0;

	//------------------------------------------------------------
	//Monotonous cleanup parameter(s)
	//------------------------------------------------------------

	virtual void __stdcall SetMonotonousCleanupEnabled(bool NewEnabled) = 0;
	virtual bool __stdcall GetMonotonousCleanupEnabled(void) const = 0;

	virtual void __stdcall SetBackTrace(unsigned int NewBT) = 0;
	virtual unsigned int __stdcall GetBackTrace(void) const = 0;

	//------------------------------------------------------------
	//Randomized cleanup parameter(s)
	//------------------------------------------------------------

	virtual void __stdcall SetRandomizedCleanupEnabled(bool NewEnabled) = 0;
	virtual bool __stdcall GetRandomizedCleanupEnabled(void) const = 0;

	virtual void __stdcall SetLeading(unsigned int NewLeading) = 0;
	virtual unsigned int __stdcall GetLeading(void) const = 0;

	virtual void __stdcall SetSpacePad(unsigned int NewSP) = 0;
	virtual unsigned int __stdcall GetSpacePad(void) const = 0;


	//------------------------------------------------------------
	//Special string parameters(s)
	//------------------------------------------------------------

	virtual void __stdcall SetSpecialStringStreamProbability(float NewProbability) = 0;
	virtual float __stdcall GetSpecialStringStreamProbability(void) const = 0;

	virtual void __stdcall SetSpecialStringColor(BYTE R,BYTE G,BYTE B,BYTE A) = 0;
	virtual void __stdcall GetSpecialStringColor(BYTE &R,BYTE &G,BYTE &B,BYTE &A) const = 0;

	virtual void __stdcall SetSpecialStringFadeColor(BYTE R,BYTE G,BYTE B,BYTE A) = 0;
	virtual void __stdcall GetSpecialStringFadeColor(BYTE &R,BYTE &G,BYTE &B,BYTE &A) const = 0;

	virtual void __stdcall SetSpecialStringBGColor(BYTE R,BYTE G,BYTE B,BYTE A) = 0;
	virtual void __stdcall GetSpecialStringBGColor(BYTE &R,BYTE &G,BYTE &B,BYTE &A) const = 0;

	virtual void __stdcall SetSpecialStringFont(HFONT NewFont) = 0;
	virtual void __stdcall SetSpecialStringLogFont(const LOGFONT &NewFont) = 0;
	virtual HFONT __stdcall GetSpecialStringFont(void) const = 0;

	virtual void __stdcall AddSpecialStringToValidSet(const _TCHAR *NewString) = 0;
	virtual void __stdcall RemoveSpecialStringFromValidSet(const _TCHAR *String) = 0;
	virtual void __stdcall ClearValidSpecialStringSet(void) = 0;
	virtual void __stdcall SetValidSpecialStringSet(const _TCHAR * const *NewStringSet,unsigned int Size) = 0;
	virtual const _TCHAR * __stdcall GetValidSpecialString(unsigned int Index) const = 0;
	virtual unsigned int __stdcall GetNumSpecialStringsInSet(void) const = 0;

	//------------------------------------------------------------
	//Transform Coefficents
	//------------------------------------------------------------

	virtual double __stdcall SetCoeffR1(const double &NewR1Coeff) = 0;
	virtual double __stdcall GetCoeffR1(void) const = 0;
	virtual double __stdcall SetCoeffR0(const double &NewR0Coeff) = 0;
	virtual double __stdcall GetCoeffR0(void) const = 0;

	virtual double __stdcall SetCoeffG1(const double &NewG1Coeff) = 0;
	virtual double __stdcall GetCoeffG1(void) const = 0;
	virtual double __stdcall SetCoeffG0(const double &NewG0Coeff) = 0;
	virtual double __stdcall GetCoeffG0(void) const = 0;

	virtual double __stdcall SetCoeffB1(const double &NewB1Coeff) = 0;
	virtual double __stdcall GetCoeffB1(void) const = 0;
	virtual double __stdcall SetCoeffB0(const double &NewB0Coeff) = 0;
	virtual double __stdcall GetCoeffB0(void) const = 0;

	virtual double __stdcall SetCoeffA1(const double &NewA1Coeff) = 0;
	virtual double __stdcall GetCoeffA1(void) const = 0;
	virtual double __stdcall SetCoeffA0(const double &NewA0Coeff) = 0;
	virtual double __stdcall GetCoeffA0(void) const = 0;



	virtual void __stdcall SethWnd(HWND hWnd) = 0;
	virtual HWND __stdcall GethWnd(void) const = 0;

	virtual void __stdcall AddCharToValidSet(_TCHAR NewChar) = 0;
	virtual void __stdcall RemoveCharFromValidSet(_TCHAR Char) = 0;
	virtual void __stdcall ClearValidCharSet(void) = 0;
	virtual void __stdcall SetValidCharSet(const _TCHAR *NewCharSet,unsigned int Size) = 0;
	virtual _TCHAR *__stdcall GetValidCharSet(void) = 0;

	virtual const _TCHAR *__stdcall GetValidCharSet(void) const  = 0;
	virtual unsigned int __stdcall GetNumCharsInSet(void) const  = 0;

	virtual unsigned int __stdcall GetNumStreams(void) const = 0;
	virtual IzsMatrixStream** __stdcall GetStreams(void) = 0;
	virtual const IzsMatrixStream** __stdcall GetStreams(void) const = 0;

	virtual IzsMatrixStream *__stdcall CreateStream(void) const = 0;
	virtual IzsMatrixStream *__stdcall CreateStream(const IzsMatrixStream &Orig) const = 0;

};

class IzsMatrixFactory : public IClassFactory
{
public:
	virtual HRESULT __stdcall CreatezsMatrix(IzsMatrix **ppNewMatrix,HWND TargetWindow,HBITMAP BGBitmap) = 0;
	virtual HRESULT __stdcall CreatezsMatrix(IzsMatrix **ppNewMatrix,const IzsMatrix &OtherMatrix) = 0;
};


STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void** ppObject);
HRESULT _stdcall DllCanUnloadNow();
STDAPI DllRegisterServer(void);
STDAPI DllUnregisterServer(void);

#endif