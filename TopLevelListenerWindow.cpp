/*!=========================================================================
//
//  Program:   ZMatrix
//  Module:    $RCSfile: TopLevelListenerWindow.cpp,v $
//  Language:  C/C++
//  Date:      $Date: 2002/08/20 05:13:03 $
//  Version:   $Revision: 1.3 $
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

#include "TopLevelListenerWindow.h"



HWND TopLevelListenerWindow = NULL;
_TCHAR ListenerClassName[] = {_TEXT("ZMatrixListenerClass")};


//===========================================================================
//===========================================================================
LRESULT CALLBACK	TopLevelListenerWindowProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	SendMessage(ghWnd,msg,wParam,lParam);
	return DefWindowProc(hWnd,msg,wParam,lParam);
}
//===========================================================================
//===========================================================================
void CreateTopLevelListener(void)
{
	WNDCLASSEX wc;

	/* Define a window class */
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = ghInstance;
	wc.lpszClassName = ListenerClassName;
	wc.lpfnWndProc = TopLevelListenerWindowProc;
	wc.style = 0;

	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL,IDC_ARROW);

	wc.lpszMenuName = NULL;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

	if (!RegisterClassEx(&wc))
	{
		MB("Failed to create top level listener window");
		return;
	}

	TopLevelListenerWindow = CreateWindow(ListenerClassName,_TEXT("Matrix Code Listener"), 0,0,0,0,0,GetDesktopWindow(),NULL,ghInstance,NULL);
	
}
//===========================================================================
//===========================================================================
void DestroyTopLevelListener(void)
{
	if(!DestroyWindow(TopLevelListenerWindow))
		MB("Failed to delete top level listener window");

	if(!UnregisterClass(ListenerClassName,ghInstance))
		MB("Failed to unregister top level listener class");
}