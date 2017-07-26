/*!=========================================================================
//
//  Program:   ZMatrix
//  Module:    $RCSfile: ZMatrixSS.cpp,v $
//  Language:  C/C++
//  Date:      $Date: 2003/02/01 19:15:50 $
//  Version:   $Revision: 1.8 $
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



#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include <tchar.h>
#include <time.h>
#include <vector>
#include <string>
#include "resource.h"


#include "../globals.h"


#define UPDATE_SCREENSAVER_TIMER_ID 1



HWND ghWnd = NULL;
HINSTANCE ghInstance = NULL;
HWND hMatrixWnd = NULL;

_tstring MatrixCommandLine(_T("matrix.exe"));

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

//===========================================================================
//===========================================================================
HWND FindMatrixWindow(void)
{
	FindWindowEnumProcRetVal = NULL;
	EnumWindows(FindWindowEnumProc,(LPARAM)_T("ZMatrixListenerClass"));

	return FindWindowEnumProcRetVal;
/*
	HWND RetVal = NULL;
	RetVal = FindWindow(_T("ZMatrix"),NULL);
	if(RetVal != NULL)
	{
		return RetVal;
	}

	return FindWindowEx(FindWindowEx(FindWindow(_TEXT("Progman"), _TEXT("Program Manager")), 0, _TEXT("SHELLDLL_DefView"), NULL),NULL,_TEXT("ZMatrix"),NULL);
*/
}
//===========================================================================
//===========================================================================
bool GetMatrixCommandLine(_tstring &Target)
{
	unsigned int RetStrSize = 128;
	_TCHAR *RetStr = (_TCHAR *)malloc(sizeof(_TCHAR)*RetStrSize);


	while((RetStrSize - 1) == GetPrivateProfileString(_T("ZMatrixSS"),_T("MatrixCommandLine"),_T("matrix.exe"),RetStr,RetStrSize,_T("ZMatrixSS.ini")))
	{
		RetStrSize += 128;
		RetStr = (_TCHAR *)realloc(RetStr,sizeof(_TCHAR)*RetStrSize);
	}

	Target = RetStr;

	free(RetStr);

	return false;
}

//===========================================================================
//===========================================================================
LRESULT CALLBACK WindowProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
LRESULT CALLBACK AlternateWindowProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
//===========================================================================
//===========================================================================

// We have 3 types of inputs command line arguments
// - /c:dddd  configuration dialog
// - /p dddd  preview, when we first switch to the window properties screen savers dialog
// - /a		  password dialog??
// - /s		  starts the screensaver
// -		  no command line starts the config dialog
//===========================================================================
//===========================================================================
int WINAPI _tWinMain(HINSTANCE hThisInst, HINSTANCE hPrevInst, LPTSTR lpszArgs, int nWinMode)
{
	// I need this because windows continously calls the screen saver to start the program
	// in no time at all I would have 2000 instances of this code *shiver*
	if (!(FindWindow(_T("ZMatrixSS"),NULL)==NULL))
		return 0;

	ghInstance = hThisInst;
/*
	HDESK hDesk = GetThreadDesktop(GetCurrentThreadId());

	DWORD BytesNeeded = 0;
	GetUserObjectInformation(hDesk,UOI_NAME,NULL,0,&BytesNeeded);
	BYTE *NameBuffer = new BYTE[BytesNeeded];
	memset(NameBuffer,0,BytesNeeded);
	if(GetUserObjectInformation(hDesk,UOI_NAME,NameBuffer,BytesNeeded,&BytesNeeded))
	{
		//MessageBox(NULL,(TCHAR *)NameBuffer,_T("Desktop Name"),0);
		if( (!_tcsicmp((TCHAR *)NameBuffer,_T("Screen-Saver"))) ||
			(!_tcsicmp((TCHAR *)NameBuffer,_T("Winlogon")))
		  )
		{

			//This scenario happens if the screensaver is being run in
			//password protected mode.  Should behave gracefully since
			//the program can't deal with the high security desktop
			//used by the password dialog...






			//First thing to try in this scenario is running the default screensaver...

			//Make this buffer long enought to hold the longest path, plus 3 more characters for the ' /s' command
			//line switch, and one more character for the terminating NULL.
			_TCHAR ScreenSaverPathBuffer[MAX_PATH + 3 +1];
			DWORD BufferPathInBytes = sizeof(_TCHAR)*(MAX_PATH + 1);
			DWORD BufferPathInTCHARs = (MAX_PATH + 1);
			memset(ScreenSaverPathBuffer,0,BufferPathInBytes);
			_tcscpy(ScreenSaverPathBuffer,_T("logon.scr"));

			HKEY hKey;

			if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_USERS,_T(".Default\\Control Panel\\Desktop"),0,KEY_READ,&hKey))
			{
				DWORD TypeQueried = 0;
				DWORD QueriedSize = BufferPathInBytes;
				RegQueryValueEx(hKey,_T("SCRNSAVE.EXE"),0,&TypeQueried,(LPBYTE)ScreenSaverPathBuffer,&QueriedSize);

				RegCloseKey(hKey);
			}

			STARTUPINFO si;
			PROCESS_INFORMATION pi;

			ZeroMemory( &si, sizeof(si) );
			si.cb = sizeof(si);
			ZeroMemory( &pi, sizeof(pi) );

			_tcscat(ScreenSaverPathBuffer,_T(" /s"));

			// Start the child process. 
			if( CreateProcess( NULL, // No module name (use command line). 
				ScreenSaverPathBuffer, // Command line. 
				NULL,             // Process handle not inheritable. 
				NULL,             // Thread handle not inheritable. 
				FALSE,            // Set handle inheritance to FALSE. 
				0,                // No creation flags. 
				NULL,             // Use parent's environment block. 
				NULL,             // Use parent's starting directory. 
				&si,              // Pointer to STARTUPINFO structure.
				&pi )             // Pointer to PROCESS_INFORMATION structure.
			) 
			{
				// Wait until child process exits.
				WaitForSingleObject( pi.hProcess, INFINITE );

				// Close process and thread handles. 
				CloseHandle( pi.hProcess );
				CloseHandle( pi.hThread );

				delete[] NameBuffer;
				return 0;
			}








			//If running the default screensaver failed, try working with the logon.scr
			//screensaver.

			_tcscpy(ScreenSaverPathBuffer,_T("logon.scr /s"));

			// Start the child process. 
			if( CreateProcess( NULL, // No module name (use command line). 
				ScreenSaverPathBuffer, // Command line. 
				NULL,             // Process handle not inheritable. 
				NULL,             // Thread handle not inheritable. 
				FALSE,            // Set handle inheritance to FALSE. 
				0,                // No creation flags. 
				NULL,             // Use parent's environment block. 
				NULL,             // Use parent's starting directory. 
				&si,              // Pointer to STARTUPINFO structure.
				&pi )             // Pointer to PROCESS_INFORMATION structure.
			) 
			{
				// Wait until child process exits.
				WaitForSingleObject( pi.hProcess, INFINITE );

				// Close process and thread handles. 
				CloseHandle( pi.hProcess );
				CloseHandle( pi.hThread );

				delete[] NameBuffer;
				return 0;
			}








			//Final attempt is to just use a plain, blank screensaver...



			if(NULL != FindWindow(_T("ZMatrixSS"),_T("ZMatrix ScreenSaver")))
			{
				delete[] NameBuffer;
				return 0;
			}

			MSG msg;
			WNDCLASSEX wc;


			// Define a window class 
			wc.cbSize = sizeof(WNDCLASSEX);
			wc.hInstance = ghInstance;
			wc.lpszClassName = _T("ZMatrixSS");
			wc.lpfnWndProc = AlternateWindowProc;
			wc.style = 0;

			wc.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_MATRIX_ICON));
			wc.hIconSm = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_MATRIX_ICON));
			wc.hCursor = LoadCursor(NULL,IDC_ARROW);

			wc.lpszMenuName = NULL;
			wc.cbClsExtra = 0;
			wc.cbWndExtra = 0;
			
			wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

			if (!RegisterClassEx(&wc))
			{
				delete[] NameBuffer;
				return 0;
			}

			int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
			int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
			int screenTop = GetSystemMetrics(SM_YVIRTUALSCREEN);
			int screenLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);

			ghWnd = CreateWindow(_T("ZMatrixSS"),_T("ZMatrix ScreenSaver"),WS_POPUP,screenLeft,screenTop,screenWidth,screenHeight,GetDesktopWindow(),NULL,ghInstance,NULL);

			ShowCursor(false);
			ShowWindow(ghWnd,SW_SHOW);


			while (GetMessage(&msg,NULL,0,0))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			DestroyWindow(ghWnd);
			ShowCursor(true);

			UnregisterClass(_T("ZMatrixSS"),ghInstance);

			
			delete[] NameBuffer;
			return msg.wParam;

		}
	}
	else
	{
		return 0;
	}
	delete[] NameBuffer;
*/

	if(!_tcsncicmp(lpszArgs,_T("/a"),2))
	{
		return 0;
	}
	else if(!_tcsncicmp(lpszArgs,_T("/p"),2))
	{
		return 0;
	}


	hMatrixWnd = NULL;

	hMatrixWnd = FindMatrixWindow();

	bool LaunchedMatrixAppSelf = false;

	if(hMatrixWnd == NULL)
	{
		LaunchedMatrixAppSelf = true;

		GetMatrixCommandLine(MatrixCommandLine);

		_TCHAR *NewBuffer = new _TCHAR[MatrixCommandLine.length() + 1];
		memset(NewBuffer,0,sizeof(_TCHAR)*(MatrixCommandLine.length() + 1));
		memcpy(NewBuffer,MatrixCommandLine.c_str(),sizeof(_TCHAR)*MatrixCommandLine.length());


		STARTUPINFO si;
		PROCESS_INFORMATION pi;

		ZeroMemory( &si, sizeof(si) );
		si.cb = sizeof(si);
		ZeroMemory( &pi, sizeof(pi) );

		// Start the child process. 
		if( !CreateProcess( NULL, // No module name (use command line). 
			NewBuffer, // Command line. 
			NULL,             // Process handle not inheritable. 
			NULL,             // Thread handle not inheritable. 
			FALSE,            // Set handle inheritance to FALSE. 
			0,                // No creation flags. 
			NULL,             // Use parent's environment block. 
			NULL,             // Use parent's starting directory. 
			&si,              // Pointer to STARTUPINFO structure.
			&pi )             // Pointer to PROCESS_INFORMATION structure.
		) 
		{
			_tstring ErrString(_T("Failed to create matrix process ("));
			ErrString.append(MatrixCommandLine);
			ErrString.append(_T(")."));
			MessageBox(NULL,ErrString.c_str(),_T("ZMatrixSS"),0);
		}
		else
		{

			// Close process and thread handles. 
			CloseHandle( pi.hProcess );
			CloseHandle( pi.hThread );

			unsigned int WaitTime = 0;
			while(NULL == (hMatrixWnd = FindMatrixWindow()))
			{
				Sleep(200);
				WaitTime+=200;

				if(WaitTime > 15000)
				{
					_tstring ErrString(_T("Launching of the matrix app took too long...("));
					ErrString.append(MatrixCommandLine);
					ErrString.append(_T(")."));
					MessageBox(NULL,ErrString.c_str(),_T("ZMatrixSS"),0);
					break;
				}
			}
		}
		delete[] NewBuffer;


	}
	

	if(!_tcsncicmp(lpszArgs,_T("/c"),2))
	{
		if(hMatrixWnd == NULL) return 0;

		PostMessage(hMatrixWnd,WM_COMMAND,ID_EDIT_SCREEN_SAVER_CONFIG,0);
		return 0;
	}


	if(hMatrixWnd == NULL) return 0;


	if(NULL != FindWindow(_T("ZMatrixSS"),_T("ZMatrix ScreenSaver"))) return 0;

	MSG msg;
	WNDCLASSEX wc;


	/* Define a window class */
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = ghInstance;
	wc.lpszClassName = _T("ZMatrixSS");
	wc.lpfnWndProc = WindowProc;
	wc.style = 0;

	wc.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_MATRIX_ICON));
	wc.hIconSm = LoadIcon(ghInstance, MAKEINTRESOURCE(IDI_MATRIX_ICON));
	wc.hCursor = LoadCursor(NULL,IDC_ARROW);

	wc.lpszMenuName = NULL;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

	if (!RegisterClassEx(&wc)) return 0;


	ghWnd = CreateWindowEx(WS_EX_TOOLWINDOW,_T("ZMatrixSS"),_T("ZMatrix ScreenSaver"), WS_POPUP,0,0,0,0,GetDesktopWindow(),NULL,ghInstance,NULL);

	ShowCursor(false);
	ShowWindow(ghWnd,SW_SHOW);
	PostMessage(hMatrixWnd,WM_START_SCREENSAVE,(LPARAM)ghWnd,(WPARAM)WM_DESTROY);

	while (GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	ShowCursor(true);

	if(LaunchedMatrixAppSelf)
	{
		SendMessage(hMatrixWnd,WM_CLOSE,0,0);
	}

	DestroyWindow(ghWnd);

	UnregisterClass(_T("ZMatrixSS"),ghInstance);

	return msg.wParam;
}
//===========================================================================
//===========================================================================
LRESULT CALLBACK WindowProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{

	switch(message)
	{
	case(WM_DESTROY):
	case(WM_CLOSE):
		{
			PostQuitMessage(0);
			return 0;
		}
		break;
	default:
		{
			return DefWindowProc(hWnd,message,wParam,lParam);
		}
		break;
	}
	return 0;
}
//===========================================================================
//===========================================================================
LRESULT CALLBACK AlternateWindowProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	static bool FirstMouseMove = true;
	static int TextPosX = 0;
	static int TextPosY = 0;

	switch(message)
	{
	case(WM_CREATE):
		{
			srand(time(NULL));
			SetTimer(hWnd,UPDATE_SCREENSAVER_TIMER_ID,2000,(TIMERPROC)NULL);
		}
		break;
	case(WM_TIMER):
		{
			switch(wParam)
			{
			case(UPDATE_SCREENSAVER_TIMER_ID):
				{
					RECT Rect;
					GetWindowRect(hWnd,&Rect);
					TextPosX = (((double)rand())/RAND_MAX)*(Rect.right - Rect.left);
					TextPosY = (((double)rand())/RAND_MAX)*(Rect.bottom - Rect.top);
					InvalidateRect(hWnd,NULL,true);
					UpdateWindow(hWnd);
				}
				break;
			default:
				{
					return DefWindowProc(hWnd,message,wParam,lParam);
				}
				break;
			}
		}
		break;
	case(WM_PAINT):
		{
			PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

			SetTextColor(hdc,RGB(255,0,0));
			SetBkMode(hdc,TRANSPARENT);

			TextOut(hdc,TextPosX,TextPosY,_T("ZMatrix cannot be run as a 'Password protected' screensaver..."),_tcslen(_T("ZMatrix cannot be run as a 'Password protected' screensaver...")));

			EndPaint(hWnd, &ps);
		}
		break;
	case(WM_MOUSEMOVE):
		{
			if(FirstMouseMove)
			{
				FirstMouseMove = false;
				return DefWindowProc(hWnd,message,wParam,lParam);
			}
			else
			{
				PostQuitMessage(0);
				return 0;
			}
		}
		break;
	case(WM_LBUTTONDOWN):
	case(WM_RBUTTONDOWN):
	case(WM_MBUTTONDOWN):
	case(WM_KEYDOWN):
	case(WM_SYSKEYDOWN):
	case(WM_DESTROY):
	//case(WM_CLOSE):
		{
			PostQuitMessage(0);
			return 0;
		}
		break;
	default:
		{
			return DefWindowProc(hWnd,message,wParam,lParam);
		}
		break;
	}
	return 0;
}


