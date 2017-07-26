/*!=========================================================================
//
//  Program:   ZMatrix
//  Module:    $RCSfile: MsgHook.h,v $
//  Language:  C/C++
//  Date:      $Date: 2003/01/01 05:00:49 $
//  Version:   $Revision: 1.10 $
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

#ifndef __MsgHook_h
#define __MsgHook_h



//=======================================================
//Code is a modification of published code found at:
//
//http://www.pgh.net/~newcomer/hooks.htm
//
//which was originally authored by:
//
//Joseph M. Newcomer
//
//=======================================================




#include <windows.h>
#include <windowsx.h>
#include <tchar.h>


#ifdef MSGHOOK_EXPORTS
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




#define UWM_DESKTOPREDRAW_ID _T("UWM_DESKTOPREDRAW-{FDE2E62D-58EA-4d5c-8EDB-EBA3E9555C9C}")
#define UWM_DESKTOPITEMCHANGED_ID _T("UWM_DESKTOPITEMCHANGED-{D0B60191-7AC6-424b-B94D-9D6822A17D17}")
#define UWM_DESKTOPCHILDATTACHED_ID _T("UWM_DESKTOPCHILDATTACHED-{4CE52887-1AEC-4664-8484-300065D79057}")




#ifdef __cplusplus
extern "C" {
#endif // __cplusplus





IMPEXP bool SetMouseMessageHook(HWND hWnd);
IMPEXP bool ClearMouseMessageHook(HWND hWnd);



IMPEXP bool SetDesktopMonitorHook(HWND hWnd);
IMPEXP bool ClearDesktopMonitorHook(HWND hWnd);



#ifdef __cplusplus
}
#endif // __cplusplus




#endif

