/*!=========================================================================
//
//  Program:   ZMatrix
//  Module:    $RCSfile: vis_zmx.cpp,v $
//  Language:  C/C++
//  Date:      $Date: 2003/05/12 07:43:57 $
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

#include <windows.h>

#include "vis.h"
#include "zmxVUModulateModule.h"
#include "zmxFreqModulateModule.h"
#include "../globals.h"

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

// getmodule routine from the main header. Returns NULL if an invalid module was requested,
// otherwise returns either mod1, mod2 or mod3 depending on 'which'.
winampVisModule *getModule(int which)
{
	switch (which)
	{
		case 0: return &zmxFreqModulateModule::mod;
		case 1: return &zmxVUModulateModule::mod;
		//case 2: return &mod3;
		default:return NULL;
	}
}



// Module header, includes version, description, and address of the module retriever function
winampVisHeader hdr =
{
	VIS_HDRVER,
	"ZMatrix Visualization Library v1.0",
	getModule
};


// this is the only exported symbol. returns our main header.
// if you are compiling C++, the extern "C" { is necessary, so we just #ifdef it
#ifdef __cplusplus
extern "C" {
#endif
__declspec( dllexport ) winampVisHeader * __stdcall winampVisGetHeader()
{
	return &hdr;
}
#ifdef __cplusplus
}
#endif


HWND FindWindowEnumProcRetVal = NULL;
BOOL CALLBACK FindWindowEnumProc(HWND hwnd,LPARAM lParam)
{
	_TCHAR *ClassName = (_TCHAR *)lParam;
	static _TCHAR TempClassName[256];

	GetClassName(hwnd,TempClassName,sizeof(TempClassName)/sizeof(_TCHAR));

	if(!_tcsicmp(TempClassName,ClassName))
	{
		FindWindowEnumProcRetVal = hwnd;
		return FALSE;
	}

	return TRUE;
}

HWND FindZMatrixWindow(void)
{
	FindWindowEnumProcRetVal = NULL;
	
	FindWindowEnumProcRetVal = FindWindowEx(FindWindowEx(FindWindow("Progman", "Program Manager"), 0, "SHELLDLL_DefView", NULL),NULL,"ZMatrix",NULL);
	
	return FindWindowEnumProcRetVal;
}

#ifdef __cplusplus
};
#endif //__cplusplus
