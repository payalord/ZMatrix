/*!=========================================================================
//
//  Program:   ZMatrix
//  Module:    $RCSfile: matrix.cpp,v $
//  Language:  C/C++
//  Date:      $Date: 2003/05/14 04:34:13 $
//  Version:   $Revision: 1.36 $
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

#include "globals.h"
#include "zsMatrix/zsMatrix.h"
#include "TopLevelListenerWindow.h"
#include "RegistryListenerThread.h"
#include <stdio.h>
#include <shlobj.h>
//#include "hooks/hooks.h"

#define WM_COEFF_GETTER(CoeffSuffix) \
case(WM_GET_COEFF_##CoeffSuffix): \
	{ \
		if(MatrixObject != NULL) \
		{ \
			float Coeff = (float) MatrixObject->GetCoeff##CoeffSuffix(); \
 \
			return (LRESULT)(Coeff); \
		} \
		else \
		{ \
			return 0; \
		} \
	} \
	break;

#define WM_COEFF_SETTER(CoeffSuffix) \
case(WM_SET_COEFF_##CoeffSuffix): \
	{ \
		if(MatrixObject != NULL) \
		{ \
			MatrixObject->SetCoeff##CoeffSuffix((float)lParam); \
 \
			return 0; \
		} \
		else \
		{ \
			return 0; \
		} \
	} \
	break;


LRESULT CALLBACK WindowProc(HWND,UINT,WPARAM,LPARAM);
//===========================================================================
//===========================================================================

bool IsWin7OrLater() {
	DWORD version = GetVersion();
	DWORD major = (DWORD)(LOBYTE(LOWORD(version)));
	DWORD minor = (DWORD)(HIBYTE(LOWORD(version)));

	return (major > 6) || ((major == 6) && (minor >= 1));
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInst, LPTSTR lpszArgs, int nWinMode)
{
	CreateMutex(NULL, FALSE,_T("ZMatrix"));

	// I need this because windows continously calls the screen saver to start the program
	// in no time at all I would have 2000 instances of this code *shiver*
	if (!(FindWindow(ListenerClassName,NULL)==NULL))
		return 0;

	ghInstance = hInstance;

	MSG msg;
	WNDCLASSEX wc;

	//Measure screen parameters
	//RECT ScreenRect;
	//SystemParametersInfo(SPI_GETWORKAREA,0,(void *)&ScreenRect,0);
	gscreenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);//ScreenRect.right - ScreenRect.left;
	if(gscreenWidth == 0) gscreenWidth = GetSystemMetrics(SM_CXSCREEN);
	gscreenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);//ScreenRect.bottom - ScreenRect.top;
	if(gscreenHeight == 0) gscreenHeight = GetSystemMetrics(SM_CYSCREEN);
	gscreenTop = GetSystemMetrics(SM_YVIRTUALSCREEN);
	gscreenLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);

	ValidRGN = CreateRectRgn(0,0,gscreenWidth,gscreenHeight);

	//Find the desktop window classes
	ghProgman = FindWindow(_TEXT("Progman"), _TEXT("Program Manager"));

	//Create WorkerW
	if (IsWin7OrLater()) SendMessageTimeout(ghProgman, 0x052C, 0, 0, SMTO_NORMAL, 1000, NULL);

	if(ghProgman != NULL)
	{
		ghShellDLL = FindWindowEx(ghProgman, 0, _TEXT("SHELLDLL_DefView"), NULL);
		ghSysListView = FindWindowEx(ghShellDLL,0,_TEXT("SysListView32"),NULL);
	}
	else
	{
		ghProgman = FindWindow(_TEXT("DesktopBackgroundClass"), NULL);

		if(ghProgman != NULL)
		{
			ghShellDLL = FindWindowEx(ghProgman, 0, _TEXT("DeskFolder"), NULL);
			ghSysListView = FindWindowEx(ghShellDLL,0,_TEXT("SysListView32"),NULL);
			LiteStepMode = true;
		}
		else
		{
			ghShellDLL = NULL;
			ghSysListView = NULL;
		}
	}


	/* Define a window class */
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = hInstance;
	wc.lpszClassName = szWinName;
	wc.lpfnWndProc = WindowProc;
	wc.style = 0;

	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL,IDC_ARROW);

	wc.lpszMenuName = NULL;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

	if (!RegisterClassEx(&wc)) return 0;

	//CoInitialize(NULL);
	CoInitializeEx(NULL,COINIT_APARTMENTTHREADED );

	StoreOrigDesktop();

	gSysTrayMenu  = LoadMenu(hInstance,MAKEINTRESOURCE(ID_SYSTRAYMENU));
	gSysTrayPopup = GetSubMenu(gSysTrayMenu,0);
	SetMenuDefaultItem(gSysTrayPopup,0,true);

	if(LiteStepMode)
	{
		if(ghProgman != NULL)
		{
			ghWnd = CreateWindowEx(WS_EX_TRANSPARENT,szWinName,_TEXT("Matrix Code"), WS_CHILDWINDOW | WS_OVERLAPPED |WS_CLIPSIBLINGS|WS_CLIPCHILDREN,0,0,gscreenWidth,gscreenHeight,ghProgman,NULL,hInstance,NULL);
			EnableWindow(ghWnd,FALSE);
			SetWindowPos(ghWnd,HWND_BOTTOM,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
		}
		else
		{
			ghWnd = CreateWindowEx(WS_EX_TRANSPARENT,szWinName,_TEXT("Matrix Code"), WS_CHILDWINDOW /*| WS_OVERLAPPED |WS_CLIPSIBLINGS*/|WS_CLIPCHILDREN,gscreenLeft,gscreenTop,gscreenWidth,gscreenHeight,GetDesktopWindow(),NULL,hInstance,NULL);
		}
	}
	else
	{
		if (IsWin7OrLater() && ghProgman) {
			ghWnd = CreateWindowEx(WS_EX_TRANSPARENT, szWinName, _TEXT("Matrix Code"), WS_CHILDWINDOW | WS_CLIPCHILDREN, gscreenLeft, gscreenTop, gscreenWidth, gscreenHeight, ghProgman, NULL, hInstance, NULL);
		} 
		else if(ghShellDLL != NULL)
		{
			ghWnd = CreateWindowEx(WS_EX_TRANSPARENT,szWinName,_TEXT("Matrix Code"), WS_CHILDWINDOW | WS_OVERLAPPED /*|WS_CLIPSIBLINGS*/|WS_CLIPCHILDREN,0,0,gscreenWidth,gscreenHeight,ghShellDLL,NULL,hInstance,NULL);
			EnableWindow(ghWnd,FALSE);
			SetWindowPos(ghWnd,HWND_BOTTOM,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
		}
		else
		{
			ghWnd = CreateWindowEx(WS_EX_TRANSPARENT,szWinName,_TEXT("Matrix Code"), WS_CHILDWINDOW /*| WS_OVERLAPPED |WS_CLIPSIBLINGS*/|WS_CLIPCHILDREN,gscreenLeft,gscreenTop,gscreenWidth,gscreenHeight,GetDesktopWindow(),NULL,hInstance,NULL);
		}
	}




	if(!SHGetSpecialFolderPath(ghWnd,AllUsersStartupDirectoryPath,CSIDL_COMMON_STARTUP,FALSE))
	{
		EnableMenuItem(gSysTrayPopup,ID_ALL_USERS_AUTO_START,MF_GRAYED);
		//MB("Failed to get All Users Startup folder");
	}
	AllUsersStartupShortcutPath = AllUsersStartupDirectoryPath;
	AllUsersStartupShortcutPath += _T(".\\ZMatrix.lnk");

	if(!SHGetSpecialFolderPath(ghWnd,CurrentUserStartupDirectoryPath,CSIDL_STARTUP,FALSE))
	{
		MB("Failed to get Current User Startup folder");
	}
	CurrentUserStartupShortcutPath = CurrentUserStartupDirectoryPath;
	CurrentUserStartupShortcutPath += _T(".\\ZMatrix.lnk");

	if(GetMatrixCommandLine(MatrixCommandLine))
	{
		MB("Failed to get ZMatrix command line path");
	}

	if(!SHGetSpecialFolderPath(ghWnd,CurrentUserAppDataDirectoryPath,CSIDL_APPDATA,TRUE))
	{
		MB("Failed to get Application Data folder");
	}

	AppConfigDirectoryPath.append(CurrentUserAppDataDirectoryPath);
	AppConfigDirectoryPath.append(_TEXT("\\.ZMatrix"));
	if(!DirectoryExists(AppConfigDirectoryPath.c_str()))
	{
		CreateDirectory(AppConfigDirectoryPath.c_str(),NULL);
	}
	AppConfigFilePath = AppConfigDirectoryPath;
	AppConfigFilePath.append(_TEXT("\\ZMatrix.cfg"));

	AppScreenSaverConfigFilePath = AppConfigDirectoryPath;
	AppScreenSaverConfigFilePath.append(_TEXT("\\ZMatrixScreenSaver.cfg"));

	AppMiscConfigFilePath = AppConfigDirectoryPath;
	AppMiscConfigFilePath.append(_TEXT("\\ZMatrixMisc.cfg"));



	CreateTopLevelListener();
	CreateRegistryListenerThread();


	DWORD Err = GetLastError();

	IconData.cbSize = sizeof(NOTIFYICONDATA);
	IconData.hWnd = ghWnd;
	IconData.uID = 0;
	IconData.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	IconData.uCallbackMessage = WM_TRAYICON;
	IconData.hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_MATRIX_ICON));
	_tcscpy(IconData.szTip,_TEXT("ZMatrix"));

	Shell_NotifyIcon(NIM_ADD,&IconData);







	ShowWindow(ghWnd,nWinMode);
	UpdateWindow(ghWnd);

	HRESULT Result = NO_ERROR;
	MULTI_QI Qi;
	Qi.pIID = &IID_IZSMATRIX;
	Qi.pItf = NULL;
	Qi.hr = 0;

	if(FAILED(Result = CoCreateInstanceEx(CLSID_ZSMATRIX,NULL,CLSCTX_ALL,NULL,1,&Qi) ) )
	{
		MB("Failed to create MatrixObject");
		

		StopRegistryListenerThread();
		DestroyTopLevelListener();

		RestoreOrigDesktop();

		Shell_NotifyIcon(NIM_DELETE,&IconData);

		MatrixObject->Release();

		DeleteObject(ValidRGN);
		DestroyMenu(gSysTrayMenu);
		DestroyIcon(IconData.hIcon);

		if(!DestroyWindow(ghWnd))
			MB("Failed to delete ZMatrix rendering window");

		if(!UnregisterClass(szWinName,ghInstance))
			MB("Failed to unregister ZMatrix rendering class");


		CoUninitialize();


		return 0;
	};

	MatrixObject = (IzsMatrix *)Qi.pItf;

	SetDesktopMonitorHook(ghWnd);
	//MatrixObject->SethWnd(ghWnd);
	//MatrixObject->SetBGBitmap(Refresh());
	HBITMAP NewBGBitmap = Refresh();
	MatrixObject->UpdateTarget(ghWnd,NewBGBitmap);
	if(!DeleteObject(NewBGBitmap))
	{
		MB("Failed to delete new BG bitmap during startup");
	}

	LoadConfig(MatrixObject,RefreshTime);

	ProcessMiscConfiguration();



	ReadyToScreenSave = true;
	if(OutstandingScreenSaveRequest)
	{
		vector<OutstandingScreenSaveRequestPair>::iterator Iter = OutstandingScreenSaveRequestParams.begin();
		for(;Iter != OutstandingScreenSaveRequestParams.end(); ++Iter)
		{
			PostMessage(ghWnd,WM_START_SCREENSAVE,Iter->first,Iter->second);
		}
		OutstandingScreenSaveRequestParams.clear();
	}

	while (GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	BeforeClose();


	CoUninitialize();

	return msg.wParam;
}

static COLORREF ExpectedBGColor = GetSysColor(COLOR_DESKTOP);
//===========================================================================
//===========================================================================
LRESULT CALLBACK WindowProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	static UINT UWM_DESKTOPREDRAW = RegisterWindowMessage(UWM_DESKTOPREDRAW_ID);
	static UINT UWM_DESKTOPITEMCHANGED = RegisterWindowMessage(UWM_DESKTOPITEMCHANGED_ID);
	static UINT UWM_DESKTOPCHILDATTACHED = RegisterWindowMessage(UWM_DESKTOPCHILDATTACHED_ID);
	static POINT pt;
	static int mouseCounter = 0;
	static int mouseDelay = 2;			// how many pixels moved by the mouse to wait before killing screensaver
	static bool ScheduledRegionUpdate = false;

	switch(message)
	{
	//case(WM_SYSCOMMAND):
	case(WM_LBUTTONDOWN):
	case(WM_RBUTTONDOWN):
	case(WM_MBUTTONDOWN):
	case(WM_KEYDOWN):
	case(WM_SYSKEYDOWN):
		{
			if(InScreenSaveMode)
			{
				EndScreenSaveMode();
				//LogToFile("Ending screensave mode because of a button/key/syscommand (0x%X)",message);

			}
		}
		break;
	case(WM_MOUSEMOVE):
		{
			if(InScreenSaveMode)
			{
				POINT MousePoint;
				GetCursorPos(&MousePoint);
				mouseCounter += abs(MousePoint.x - MouseScreenPointAtScreenSaveStart.x);
				mouseCounter += abs(MousePoint.y - MouseScreenPointAtScreenSaveStart.y);

				if (mouseCounter > mouseDelay)
				{
					EndScreenSaveMode();
					mouseCounter = 0;
					//LogToFile("Ending screensave mode because of a mouse move");
				}
			}

		}
		break;
	case(WM_START_SCREENSAVE):
		{
			if(ReadyToScreenSave)
			{
				BeginScreenSaveMode((HWND)wParam,(UINT)lParam);
				OutstandingScreenSaveRequest = false;
			}
			else
			{
				OutstandingScreenSaveRequestParams.push_back(OutstandingScreenSaveRequestPair(wParam,lParam));
				OutstandingScreenSaveRequest = true;
			}
		}
		break;
	case(WM_END_SCREENSAVE):
		{
			EndScreenSaveMode();
		}
		break;
	case(WM_TIMER):
		{
			switch(wParam)
			{
			case(REFRESH_TIMER_ID):
				{
					if(ScheduledRegionUpdate)
					{
						UpdateRegions();
						ScheduledRegionUpdate = false;
					}

					EnforceDesktop();

					if(InScreenSaveMode)
					{
						HDC hdc = GetDC(0);
						SetViewportOrgEx(hdc,gscreenLeft,gscreenTop,NULL);
						MatrixObject->Render(hdc);
						SetViewportOrgEx(hdc,-gscreenLeft,-gscreenTop,NULL);
						ReleaseDC(0,hdc);
					}
					else
					{
						HWND hp = 0, p = 0, desktop = GetDesktopWindow();
						WorkerW = 0;
						while (!WorkerW) {
							hp = FindWindowEx(desktop, hp, _T("WorkerW"), 0);
							if (hp) {
								p = FindWindowEx(hp, 0, _T("SHELLDLL_DefView"), 0);
								if (p) {
									WorkerW = FindWindowEx(desktop, hp, _T("WorkerW"), 0);
									if (WorkerW && IsWindowVisible(WorkerW)) ShowWindow(WorkerW, SW_HIDE);
									break;
								}
							}
							else break;
						}

						POINT TestPoint = {0,0};
						ClientToScreen(ghWnd,&TestPoint);

						HDC hdc = GetDC(hWnd);
						SelectClipRgn(hdc,ValidRGN);

						XFORM xForm;
						xForm.eM11 = 1.0f;
						xForm.eM12 = 0.0f;
						xForm.eM21 = 0.0f;
						xForm.eM22 = 1.0f;
						xForm.eDx  = 0.0f;
						xForm.eDy  = 0.0f;
						SetWorldTransform(hdc, &xForm);
						OffsetViewportOrgEx(hdc,gscreenLeft-TestPoint.x,gscreenTop-TestPoint.y,NULL);

						MatrixObject->Render(hdc);

						ReleaseDC(hWnd,hdc);
					}

				}
				break;
			case(SINGLE_CLICK_TIMER_ID):
				{
					KillTimer(ghWnd,SINGLE_CLICK_TIMER_ID);
					if(Paused)
					{
						CheckMenuItem(gSysTrayPopup,ID_PAUSE,MF_UNCHECKED);
						SetTimer(ghWnd,REFRESH_TIMER_ID,RefreshTime,0);
						Paused = false;
					}
					else
					{
						CheckMenuItem(gSysTrayPopup,ID_PAUSE,MF_CHECKED);
						KillTimer(ghWnd,REFRESH_TIMER_ID);
						Paused = true;
					}
				}
				break;
			default:
				{
				}
				break;
			}
		}
		break;
	case(WM_COMMAND):
		{
			WORD wID = LOWORD(wParam);

			switch(wID)
			{
			case(IDCLOSE):
				{
					//Do this here and now because otherwise, the program may be
					//killed before it gets done.
					BeforeClose();
					PostQuitMessage(0);
				}
				break;
			case(ID_REFRESH):
				{
					HBITMAP NewBGBitmap = Refresh();
					MatrixObject->SetBGBitmap(NewBGBitmap);
					if(!DeleteObject(NewBGBitmap))
					{
						MB("Failed to delete new BG bitmap during a refresh");
					}
				}
				break;
			case(ID_SCREENSAVE):
				{
					if(InScreenSaveMode)
					{
						EndScreenSaveMode();
					}
					else
					{
						BeginScreenSaveMode();
					}
				}
				break;
			case(ID_SET_AS_SCREENSAVER):
				{
					SetZMatrixSS_NoBackup();
				}
				break;
			case(ID_ALWAYS_SET_AS_SCREENSAVER_WHILE_RUNNING):
				{
					SetAlwaysSetAsScreenSaverWhileRunning(!AlwaysSetAsScreenSaverWhileRunning);
				}
				break;
			case(ID_BLEND_SCREENSAVER_WITH_BG_ONLY):
				{
					SetBlendScreenSaverWithBGOnly(!BlendScreenSaverWithBGOnly);
				}
				break;
			case(ID_DELETE_SCREEN_SAVER_CONFIG):
				{
					DeleteFile(AppScreenSaverConfigFilePath.c_str());
				}
				break;
			case(ID_EDIT_SCREEN_SAVER_CONFIG):
				{
					LaunchScreenSaverConfig(MatrixObject);
				}
				break;
			case(ID_ALL_USERS_AUTO_START):
				{
					UINT State = GetMenuState(gSysTrayPopup,ID_ALL_USERS_AUTO_START,MF_BYCOMMAND);

					if(State & MF_CHECKED)
					{
						RemoveLink(AllUsersStartupShortcutPath.c_str());
					}
					else
					{
						CreateLink(MatrixCommandLine.c_str(),AllUsersStartupShortcutPath.c_str());
					}
				}
				break;
			case(ID_CURRENT_USER_AUTO_START):
				{
					UINT State = GetMenuState(gSysTrayPopup,ID_CURRENT_USER_AUTO_START,MF_BYCOMMAND);

					if(State & MF_CHECKED)
					{
						RemoveLink(CurrentUserStartupShortcutPath.c_str());
					}
					else
					{
						CreateLink(MatrixCommandLine.c_str(),CurrentUserStartupShortcutPath.c_str());
					}
				}
				break;
			case(ID_CONFIGURE):
				{
					LaunchConfig(MatrixObject);
				}
				break;
			case(ID_ABOUT):
				{
					//Launch the about dialog using the method provided in config.dll
					HINSTANCE hDLL = LoadLibrary(_TEXT("Config.dll"));

					if(hDLL != NULL)
					{
						AboutFormLauncher AboutLauncher = (AboutFormLauncher)GetProcAddress(hDLL,"LaunchAboutForm");

						if(AboutLauncher != NULL)
						{
							try
							{
								AboutLauncher(NULL);
							}
							catch(...)
							{
								MB("Exception while launching About form");
							}
						}

						FreeLibrary(hDLL);
					}
				}
				break;
			case(ID_DONATE):
				{
					ShellExecute(NULL,_T("open"),DONATE_URL, NULL, NULL, SW_SHOWNORMAL);
				}
				break;
			case(ID_HELP_REQ):
				{
					ShellExecute(ghWnd,_TEXT("open"),_TEXT("ZMatrixHelp.chm"), NULL, NULL, SW_SHOWNORMAL);
				}
				break;
			case(ID_HIRE):
				{
					//Launch the about dialog using the method provided in config.dll
					HINSTANCE hDLL = LoadLibrary(_TEXT("Config.dll"));

					if(hDLL != NULL)
					{
						HireFormLauncher HireLauncher = (HireFormLauncher)GetProcAddress(hDLL,"LaunchHireForm");

						if(HireLauncher != NULL)
						{
							try
							{
								HireLauncher(NULL);
							}
							catch(...)
							{
								MB("Exception while launching Hire form");
							}
						}

						FreeLibrary(hDLL);
					}
				}
				break;
			case(ID_PAUSE):
				{
					if(Paused)
					{
						CheckMenuItem(gSysTrayPopup,ID_PAUSE,MF_UNCHECKED);
						SetTimer(ghWnd,REFRESH_TIMER_ID,RefreshTime,0);
						Paused = false;
					}
					else
					{
						CheckMenuItem(gSysTrayPopup,ID_PAUSE,MF_CHECKED);
						KillTimer(ghWnd,REFRESH_TIMER_ID);
						Paused = true;
					}
				}
			default:
				{
					return DefWindowProc(hWnd,message,wParam,lParam);
				}
				break;
			}
		}
		break;
	case(WM_TRAYICON):
		{
			switch(lParam)
			{
			case(WM_RBUTTONUP):
				{
					if(Paused)
					{
						CheckMenuItem(gSysTrayPopup,ID_PAUSE,MF_CHECKED);
					}
					else
					{
						CheckMenuItem(gSysTrayPopup,ID_PAUSE,MF_UNCHECKED);
					}

					if(FileExists(AllUsersStartupShortcutPath.c_str()))
					{
						CheckMenuItem(gSysTrayPopup,ID_ALL_USERS_AUTO_START,MF_CHECKED);
					}
					else
					{
						CheckMenuItem(gSysTrayPopup,ID_ALL_USERS_AUTO_START,MF_UNCHECKED);
					}

					if(FileExists(CurrentUserStartupShortcutPath.c_str()))
					{
						CheckMenuItem(gSysTrayPopup,ID_CURRENT_USER_AUTO_START,MF_CHECKED);
					}
					else
					{
						CheckMenuItem(gSysTrayPopup,ID_CURRENT_USER_AUTO_START,MF_UNCHECKED);
					}

					if(FileExists(AppScreenSaverConfigFilePath.c_str()))
					{
						EnableMenuItem(gSysTrayPopup,ID_DELETE_SCREEN_SAVER_CONFIG,MF_ENABLED);
					}
					else
					{
						EnableMenuItem(gSysTrayPopup,ID_DELETE_SCREEN_SAVER_CONFIG,MF_GRAYED);
					}


					GetCursorPos(&pt);
					SetForegroundWindow(hWnd);
					TrackPopupMenu(gSysTrayPopup,0,pt.x,pt.y,0,hWnd,NULL);
					PostMessage(hWnd, WM_NULL, 0, 0);

				}
				break;
			case(WM_LBUTTONDOWN):
				{
					SetTimer(ghWnd,SINGLE_CLICK_TIMER_ID,GetDoubleClickTime()+1,0);
				}
				break;
			case(WM_LBUTTONDBLCLK):
				{
					KillTimer(ghWnd,SINGLE_CLICK_TIMER_ID);
					PostMessage(hWnd, WM_NULL, 0, 0);
					LaunchConfig(MatrixObject);
				}
				break;
			default:
				{
				}
				break;
			}
		}
		break;
	case(WM_DESTROY):
	case(WM_CLOSE):
		{
			BeforeClose();
			PostQuitMessage(0);
			return DefWindowProc(hWnd,message,wParam,lParam);
		}
		break;
	case(WM_ENDSESSION):
	case(WM_QUERYENDSESSION):
		{
			//Do this here and now because otherwise, the program may be
			//killed before it gets done.
			BeforeClose();
			PostQuitMessage(0);
			return true;
		}
		break;
	case(WM_DESKTOPWALLPAPERCHANGED):
		{

			HKEY hKey;

			if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,_TEXT("Control Panel\\Desktop"),0,KEY_READ,&hKey))
			{
				DWORD WallpaperBufferSize = MAX_PATH + 1;
				_TCHAR *WallpaperBuffer = (_TCHAR *) malloc(WallpaperBufferSize*sizeof(_TCHAR));

				RegQueryStringAndRealloc(hKey,_T("Wallpaper"),WallpaperBuffer,WallpaperBufferSize);


				if(WallpaperBuffer != NULL)
				{
					if((!WallpaperChangesToIgnore.empty()) && (!_tcsicmp(WallpaperChangesToIgnore.front().c_str(),WallpaperBuffer)) )
					{
						WallpaperChangesToIgnore.pop_front();
					}
					else
					{
						StoreOrigWallpaper(WallpaperBuffer);

						if(DesktopColorIsCleared)
						{
							RestoreOrigDesktopColor();
							HBITMAP NewBGBitmap = UpdateBG();
							MatrixObject->SetBGBitmap(NewBGBitmap);
							if(!DeleteObject(NewBGBitmap))
							{
								MB("Failed to delete new BG bitmap during a desktop wallpaper change");
							}
							if(WallpaperIsCleared)
							{
								ClearWallpaper();
							}
							ClearDesktopColor();
						}
						else
						{
							HBITMAP NewBGBitmap = UpdateBG();
							MatrixObject->SetBGBitmap(NewBGBitmap);
							if(!DeleteObject(NewBGBitmap))
							{
								MB("Failed to delete new BG bitmap during a desktop wallpaper change");
							}
							if(WallpaperIsCleared)
							{
								ClearWallpaper();
							}
						}

					}

					free(WallpaperBuffer);
				}

				RegCloseKey(hKey);

			}


		}
		break;
	case(WM_SETTINGCHANGE):
		{
			switch(wParam)
			{
			case(SPI_SETWORKAREA):
				{
					ScheduledRegionUpdate = true;
				}
				break;
			default:
				{
				}
				break;
			}
		}
		break;
	case(WM_SYSCOLORCHANGE):
		{

			COLORREF CurrentBGColor = GetSysColor(COLOR_DESKTOP);

			if((!BGColorChangesToIgnore.empty()) && (BGColorChangesToIgnore.front() == CurrentBGColor) )
			{
				BGColorChangesToIgnore.pop_front();
			}
			else
			{
				if(CurrentBGColor != ExpectedBGColor)
				{
					StoreOrigDesktopColor(&CurrentBGColor);

					if(WallpaperIsCleared)
					{
						RestoreOrigWallpaper();
						HBITMAP NewBGBitmap = UpdateBG();
						MatrixObject->SetBGBitmap(NewBGBitmap);
						if(!DeleteObject(NewBGBitmap))
						{
							MB("Failed to delete new BG bitmap during a desktop color change");
						}
						if(DesktopColorIsCleared)
						{
							ClearDesktopColor();
						}
						ClearWallpaper();
					}
					else
					{
						HBITMAP NewBGBitmap = UpdateBG();
						MatrixObject->SetBGBitmap(NewBGBitmap);
						if(!DeleteObject(NewBGBitmap))
						{
							MB("Failed to delete new BG bitmap during a desktop color change");
						}
						if(DesktopColorIsCleared)
						{
							ClearDesktopColor();
						}
					}

				}

			}

			ExpectedBGColor = CurrentBGColor;


		}
		break;
	case(WM_OTHERDESKTOPSETTINGCHANGED):
		{
			HBITMAP NewBGBitmap = Refresh();
			MatrixObject->SetBGBitmap(NewBGBitmap);
			if(!DeleteObject(NewBGBitmap))
			{
				MB("Failed to delete new BG bitmap during a desktop wallpaper change");
			}
		}
		break;
	case(WM_ERASEBKGND):
		{
			return 0;
		}
		break;
	case(WM_GET_MATRIX_OBJECT):
		{
			return (LRESULT)MatrixObject;
		}
		break;
	WM_COEFF_GETTER(R0);
	WM_COEFF_SETTER(R0);
	WM_COEFF_GETTER(R1);
	WM_COEFF_SETTER(R1);

	WM_COEFF_GETTER(G0);
	WM_COEFF_SETTER(G0);
	WM_COEFF_GETTER(G1);
	WM_COEFF_SETTER(G1);

	WM_COEFF_GETTER(B0);
	WM_COEFF_SETTER(B0);
	WM_COEFF_GETTER(B1);
	WM_COEFF_SETTER(B1);
	default:
		{
			if(message == UWM_DESKTOPREDRAW)
			{
				return DefWindowProc(hWnd,message,wParam,lParam);
			}
			else if(message == UWM_DESKTOPITEMCHANGED)
			{
				UpdateRegions();
			}
			else if(message == UWM_DESKTOPCHILDATTACHED)
			{
				const int ClassNameBufferSize = 256;
				TCHAR ClassNameBuffer[ClassNameBufferSize];

				GetClassName((HWND)lParam,ClassNameBuffer,ClassNameBufferSize);

				if(SelfPostedDesktopChildAttached)
				{
					SelfPostedDesktopChildAttached--;

					if(!_tcscmp(ClassNameBuffer,_T("Internet Explorer_Server")))
					{
						SetWindowPos(ghWnd,HWND_BOTTOM,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
					}
				}
				else
				{
					if(!_tcscmp(ClassNameBuffer,_T("Internet Explorer_Server")))
					{
						if(DesktopIsCleared)
						{
							ClearWallpaper();
						}

						SetWindowPos(ghWnd,HWND_BOTTOM,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
					}
				}
			}
			else
			{
				return DefWindowProc(hWnd,message,wParam,lParam);
			}
		}
		break;
	}
	return DefWindowProc(hWnd,message,wParam,lParam);
}
