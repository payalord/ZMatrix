/*!=========================================================================
//
//  Program:   ZMatrix
//  Module:    $RCSfile: MsgHook.cpp,v $
//  Language:  C/C++
//  Date:      $Date: 2003/01/02 05:12:41 $
//  Version:   $Revision: 1.14 $
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

#include "MsgHook.h"
#include "../globals.h"
#include <commctrl.h>


#ifdef USE_MEM_MANAGER
#include "../mmgr.h"
#endif


#pragma data_seg(".ZS")
bool FirstLoad = true;
HWND hMouseMessageHookProcTargetWindow = NULL;
HWND hDesktopMonitorHookProcTargetWindow = NULL;
HWND hSysListView = NULL;
HWND hShellDll = NULL;
UINT UWM_DESKTOPREDRAW = 0;
UINT UWM_DESKTOPITEMCHANGED = 0;
UINT UWM_DESKTOPCHILDATTACHED = 0;
#pragma data_seg()
#pragma comment(linker, "/section:.ZS,rws")



HINSTANCE hInst;

HHOOK MouseMessageHook;
static LRESULT CALLBACK MouseMessageHookProc(int nCode, WPARAM wParam, LPARAM lParam);

HHOOK DesktopMonitorHook;
static LRESULT CALLBACK DesktopMonitorHookProc(int nCode, WPARAM wParam, LPARAM lParam);

BOOL APIENTRY DllMain( HINSTANCE hInstance, 
					  DWORD  Reason, 
					  LPVOID Reserved
					  )
{
	switch(Reason)
	{ /* reason */
	case(DLL_PROCESS_ATTACH):
		{
			if(FirstLoad)
			{
				UWM_DESKTOPREDRAW = RegisterWindowMessage(UWM_DESKTOPREDRAW_ID);
				UWM_DESKTOPITEMCHANGED = RegisterWindowMessage(UWM_DESKTOPITEMCHANGED_ID);
				UWM_DESKTOPCHILDATTACHED = RegisterWindowMessage(UWM_DESKTOPCHILDATTACHED_ID);

				hShellDll = FindWindowEx(
										FindWindow(_TEXT("Progman"), _TEXT("Program Manager"))
									, 0, _TEXT("SHELLDLL_DefView"), NULL);
				hSysListView = FindWindowEx(hShellDll,0,_TEXT("SysListView32"),NULL);
				FirstLoad = false;
			}

			hInst = hInstance;

			return true;
		}
		break;
	case(DLL_PROCESS_DETACH):
		{
			if(hMouseMessageHookProcTargetWindow != NULL)
				ClearMouseMessageHook(hMouseMessageHookProcTargetWindow);

			if(hDesktopMonitorHookProcTargetWindow != NULL)
				ClearDesktopMonitorHook(hDesktopMonitorHookProcTargetWindow);

			return true;
		}
		break;
	}

    return true;
}


/****************************************************************************
*                                 SetMouseMessageHook
* Inputs:
*       HWND hWnd: Window to notify
* Result: bool
*       true if successful
*       false if error
* Effect: 
*       Sets the hook
****************************************************************************/

bool SetMouseMessageHook(HWND hWnd)
{
	if(hMouseMessageHookProcTargetWindow != NULL)
		return FALSE; // already hooked!

	MouseMessageHook = SetWindowsHookEx(WH_GETMESSAGE,(HOOKPROC)MouseMessageHookProc,hInst,0);

	if(MouseMessageHook != NULL)
	{ /* success */
		hMouseMessageHookProcTargetWindow = hWnd;
		return true;
	}

	return false; // failed to set hook

}

/****************************************************************************
*                                 ClearMouseMessageHook
* Inputs:
*       HWND hWnd: Window hook
* Result: bool
*       true if successful
*       false if error
* Effect: 
*       Removes the hook that has been set
****************************************************************************/
bool ClearMouseMessageHook(HWND hWnd)
{
	if(hWnd != hMouseMessageHookProcTargetWindow || hWnd == NULL)
		return FALSE;

	bool unhooked = (0 != UnhookWindowsHookEx(MouseMessageHook));

	if(unhooked)
		hMouseMessageHookProcTargetWindow = NULL;

	return unhooked;

}

/****************************************************************************
*                                   MouseMessageHookProc
* Inputs:
*       int nCode: Code value
*       WPARAM wParam:
*       LPARAM lParam:
* Result: LRESULT
*       Either 0 or the result of CallNextHookEx
* Effect: 
*       Hook processing function. If the message is a mouse-move message,
*       posts the coordinates to the parent window
****************************************************************************/

static LRESULT CALLBACK MouseMessageHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode != HC_ACTION)
	{ /* pass it on */
		return CallNextHookEx(MouseMessageHook, nCode, wParam, lParam);
	}


	LPMSG msg = (LPMSG)lParam;

	if((msg != NULL) && (msg->hwnd != hMouseMessageHookProcTargetWindow))
	{
		if((msg->message == WM_MOUSEMOVE) || (msg->message == WM_NCMOUSEMOVE))
		{
			POINT MousePoint;
			MousePoint.x = GET_X_LPARAM(lParam);
			MousePoint.y = GET_Y_LPARAM(lParam);

			ClientToScreen(msg->hwnd,&MousePoint);
			ClientToScreen(hMouseMessageHookProcTargetWindow,&MousePoint);

			LPARAM NewLParam = MAKELPARAM(MousePoint.x,MousePoint.y);

			PostMessage(hMouseMessageHookProcTargetWindow, WM_MOUSEMOVE, 0, NewLParam);
		}
		else if((msg->message == WM_LBUTTONDOWN) || (msg->message == WM_NCLBUTTONDOWN))
		{
			PostMessage(hMouseMessageHookProcTargetWindow, WM_LBUTTONDOWN, 0, 0);
		}
		else if((msg->message == WM_RBUTTONDOWN) || (msg->message == WM_NCRBUTTONDOWN))
		{
			PostMessage(hMouseMessageHookProcTargetWindow, WM_RBUTTONDOWN, 0, 0);
		}
		else if((msg->message == WM_MBUTTONDOWN) || (msg->message == WM_NCMBUTTONDOWN))
		{
			PostMessage(hMouseMessageHookProcTargetWindow, WM_MBUTTONDOWN, 0, 0);
		}
		else if(msg->message == WM_KEYDOWN)
		{
			PostMessage(hMouseMessageHookProcTargetWindow, WM_KEYDOWN, 0, 0);
		}
		else if(msg->message == WM_SYSKEYDOWN)
		{
			PostMessage(hMouseMessageHookProcTargetWindow, WM_SYSKEYDOWN, 0, 0);
		}
		else if(msg->message == WM_SYSCOMMAND)
		{
			PostMessage(hMouseMessageHookProcTargetWindow, WM_SYSCOMMAND, 0, 0);
		}
	}

	return CallNextHookEx(MouseMessageHook, nCode, wParam, lParam);
}






/****************************************************************************
*                                 SetDesktopMonitorHook
* Inputs:
*       HWND hWnd: Window to notify
* Result: bool
*       true if successful
*       false if error
* Effect: 
*       Sets the hook
****************************************************************************/

bool SetDesktopMonitorHook(HWND hWnd)
{
	if(hDesktopMonitorHookProcTargetWindow != NULL)
		return FALSE; // already hooked!

	DesktopMonitorHook = SetWindowsHookEx(WH_CALLWNDPROCRET,(HOOKPROC)DesktopMonitorHookProc,hInst,0);

	if(DesktopMonitorHook != NULL)
	{ /* success */
		hDesktopMonitorHookProcTargetWindow = hWnd;
		return true;
	}

	return false; // failed to set hook

}

/****************************************************************************
*                                 ClearDesktopMonitorHook
* Inputs:
*       HWND hWnd: Window hook
* Result: bool
*       true if successful
*       false if error
* Effect: 
*       Removes the hook that has been set
****************************************************************************/
bool ClearDesktopMonitorHook(HWND hWnd)
{
	if(hWnd != hDesktopMonitorHookProcTargetWindow || hWnd == NULL)
		return FALSE;

	bool unhooked = (0 != UnhookWindowsHookEx(DesktopMonitorHook));

	if(unhooked)
		hDesktopMonitorHookProcTargetWindow = NULL;

	return unhooked;

}

/****************************************************************************
*                                   DesktopMonitorHookProc
* Inputs:
*       int nCode: Code value
*       WPARAM wParam:
*       LPARAM lParam:
* Result: LRESULT
*       Either 0 or the result of CallNextHookEx
* Effect: 
*       Hook processing function.
****************************************************************************/

static LRESULT CALLBACK DesktopMonitorHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	static bool ChangesOccurredToDesktopIcons = false;

	if(nCode != HC_ACTION)
	{ /* pass it on */
		return CallNextHookEx(DesktopMonitorHook, nCode, wParam, lParam);
	}


	CWPRETSTRUCT *msg = (CWPRETSTRUCT *)lParam;

	if(msg != NULL)
	{
		if(msg->hwnd == hSysListView)
		{
			switch(msg->message)
			{
			case(WM_PAINT):
				{
					if(ChangesOccurredToDesktopIcons)
					{
						PostMessage(hDesktopMonitorHookProcTargetWindow,UWM_DESKTOPITEMCHANGED,0,0);
						ChangesOccurredToDesktopIcons = false;
					}
				}
				break;
			case(WM_NCPAINT):
				{
					if(ChangesOccurredToDesktopIcons)
					{
						PostMessage(hDesktopMonitorHookProcTargetWindow,UWM_DESKTOPITEMCHANGED,0,0);
						ChangesOccurredToDesktopIcons = false;
					}
					if(msg->wParam == 1)
					{
						PostMessage(hDesktopMonitorHookProcTargetWindow, UWM_DESKTOPREDRAW, 0, 0);
					}
				}
				break;
			case(LVM_SETITEM):
			case(LVM_SETITEMTEXT):
			case(LVM_DELETEITEM):
			case(LVM_ARRANGE):
			case(LVM_SETITEMPOSITION32):
			case(LVM_SETITEMSTATE):
				{
					ChangesOccurredToDesktopIcons = true;
				}
				break;
			default:
				{
				}
				break;
			}
		}
		else if(msg->hwnd == hShellDll)
		{
			switch(msg->message)
			{
			case(WM_PARENTNOTIFY):
				{
					if(WM_CREATE == LOWORD(msg->wParam))
					{
						PostMessage(hDesktopMonitorHookProcTargetWindow,UWM_DESKTOPCHILDATTACHED,msg->wParam,msg->lParam);
					}
				}
				break;
			default:
				{
				}
				break;
			}
		}

	}

	return CallNextHookEx(DesktopMonitorHook, nCode, wParam, lParam);
}




