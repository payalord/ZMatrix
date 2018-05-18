/*!=========================================================================
//
//  Program:   ZMatrix
//  Module:    $RCSfile: globals.h,v $
//  Language:  C/C++
//  Date:      $Date: 2003/05/10 17:06:18 $
//  Version:   $Revision: 1.26 $
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


#pragma warning( disable : 4786 )

#ifndef __globals_h
#define __globals_h








#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <commctrl.h>
#include <wingdi.h>
#include <string.h>
#include <string>
#include <vector>
#include <deque>
#include <algorithm>
#include "resource.h"
#include "zsMatrix/zsMatrix.h"
#include "MsgHook/MsgHook.h"

#ifdef USE_MEM_MANAGER
#include "mmgr.h"
#endif

using namespace std;

#define MB(txt) {(MessageBox(ghWnd,_TEXT(txt),_TEXT("debug"),MB_OK)); OutputDebugString(__TEXT(txt));OutputDebugString(__TEXT("\n"));}
#define WM_TRAYICON (WM_USER + 1)

#define WM_START_SCREENSAVE (WM_USER + 2)
#define WM_END_SCREENSAVE (WM_USER + 3)

#define WM_OTHERDESKTOPSETTINGCHANGED (WM_USER + 4)
#define WM_DESKTOPWALLPAPERCHANGED (WM_USER + 5)

#define WM_GET_MATRIX_OBJECT (WM_USER + 6)

#define WM_GET_COEFF_R0 (WM_USER + 7)
#define WM_SET_COEFF_R0 (WM_USER + 8)
#define WM_GET_COEFF_R1 (WM_USER + 9)
#define WM_SET_COEFF_R1 (WM_USER + 10)

#define WM_GET_COEFF_G0 (WM_USER + 11)
#define WM_SET_COEFF_G0 (WM_USER + 12)
#define WM_GET_COEFF_G1 (WM_USER + 13)
#define WM_SET_COEFF_G1 (WM_USER + 14)

#define WM_GET_COEFF_B0 (WM_USER + 15)
#define WM_SET_COEFF_B0 (WM_USER + 16)
#define WM_GET_COEFF_B1 (WM_USER + 17)
#define WM_SET_COEFF_B1 (WM_USER + 18)


#ifndef WM_UPDATEUISTATE
	#define WM_UPDATEUISTATE 0x0128
#endif

#define REFRESH_TIMER_ID 400

#define SINGLE_CLICK_TIMER_ID 401

#define DESKTOPREDRAW_EVENT_ID _T("DESKTOPREDRAW_EVENT-{1593B99A-5D70-41d5-A271-AE6113B50BB7}")

#define HOMEPAGE_URL _T("http://zmatrix.n3.net")
#define PROJECTSUMMARYPAGE_URL _T("http://sourceforge.net/projects/zmatrix/")
#define SUPPORTREQUESTS_URL _T("http://sourceforge.net/tracker/?atid=493576&group_id=60257&func=browse")
#define BUGREPORTS_URL _T("http://sourceforge.net/tracker/?atid=493575&group_id=60257&func=browse")
#define FEATUREREQUESTS_URL _T("http://sourceforge.net/tracker/?atid=493578&group_id=60257&func=browse")
#define DISCUSSIONFORUMS_URL _T("http://sourceforge.net/forum/?group_id=60257")
#define DONATE_URL _T("http://zmatrix.n3.net/donate.html")

extern unsigned int RefreshTime;

extern HANDLE DESKTOPREDRAW_Event;

typedef std::basic_string<_TCHAR> _tstring;
typedef std::basic_string<_TCHAR> tstring;
typedef std::basic_string<WCHAR> widestring;
typedef std::basic_string<CHAR> string;

extern _TCHAR szWinName[];


extern unsigned int gscreenWidth;
extern unsigned int gscreenHeight;
extern int gscreenTop;
extern int gscreenLeft;


extern HDC ghdc;
extern HMENU gSysTrayMenu;
extern HMENU gSysTrayPopup;

extern HINSTANCE ghInstance;
extern HWND ghWnd;
extern HWND ghProgman;
extern HWND WorkerW;
extern HWND ghShellDLL;
extern HWND ghSysListView;
extern bool LiteStepMode;

extern NOTIFYICONDATA IconData;
extern HRGN ValidRGN;

extern IzsMatrix *PreScreenSaveMatrixObject;
extern bool PreScreenSavePauseState;
extern unsigned int PreScreenSaveRefreshTime;
extern DWORD PreScreenSavePriorityClass;

extern IzsMatrix *MatrixObject;

extern bool Paused;

extern bool DesktopIsCleared;
extern bool WallpaperIsCleared;
extern bool DesktopColorIsCleared;

extern bool OrigDesktopColorIsValid;
extern COLORREF OrigDesktopColor;

extern bool OrigWallpaperIsValid;
extern _tstring OrigWallpaperString;
extern bool OrigActiveDesktopWallpaperIsValid;
extern widestring OrigActiveDesktopWallpaperString;

extern TCHAR AllUsersStartupDirectoryPath[MAX_PATH];
extern _tstring AllUsersStartupShortcutPath;
extern TCHAR CurrentUserStartupDirectoryPath[MAX_PATH];
extern _tstring CurrentUserStartupShortcutPath;

extern _tstring MatrixCommandLine;

extern TCHAR CurrentUserAppDataDirectoryPath[MAX_PATH];
extern _tstring AppConfigDirectoryPath;
extern _tstring AppConfigFilePath;
extern _tstring AppScreenSaverConfigFilePath;
extern _tstring AppMiscConfigFilePath;

extern bool ReadyToScreenSave;
extern bool OutstandingScreenSaveRequest;
extern bool InScreenSaveMode;
extern bool UnSetSSWhenDone;
typedef pair<WPARAM,LPARAM> OutstandingScreenSaveRequestPair;
extern vector<OutstandingScreenSaveRequestPair> OutstandingScreenSaveRequestParams;
extern POINT MouseScreenPointAtScreenSaveStart;
extern bool AlwaysSetAsScreenSaverWhileRunning;
extern bool BlendScreenSaverWithBGOnly;

void SetAlwaysSetAsScreenSaverWhileRunning(bool NewVal);
void SetBlendScreenSaverWithBGOnly(bool NewVal);

typedef int (__stdcall *AboutFormLauncher)(void *Parent);
typedef int (__stdcall *HireFormLauncher)(void *Parent);
typedef int (__stdcall *ConfigFormLauncher)(IzsMatrix *Matrix,unsigned int &RefreshTime,DWORD &Priority);
typedef int (__stdcall *ConfigSaver)(IzsMatrix *Matrix,unsigned int RefreshTime,DWORD Priority,_TCHAR *FileName);
typedef int (__stdcall *ConfigLoader)(IzsMatrix *Matrix,unsigned int &RefreshTime,DWORD &Priority,_TCHAR *FileName);


void MinimizeAll(void);
void UndoMinimizeAll(void);

void StoreOrigWallpaper(const _TCHAR *NewPaper = NULL);
void StoreOrigActiveDesktopWallpaper(const WCHAR *NewActiveDesktopPaper = NULL);
void StoreOrigDesktopColor(const COLORREF *NewColor = NULL);
void StoreOrigDesktop(const _TCHAR *NewPaper = NULL,const WCHAR *NewActiveDesktopPaper = NULL,const COLORREF *NewColor = NULL);

void RestoreOrigWallpaper(void);
void RestoreOrigDesktopColor(void);
void RestoreOrigDesktop(void);

void ClearWallpaper(void);
void ClearDesktopColor(void);
void ClearDesktop(void);

void EnforceDesktop(void);

typedef _tstring IgnoredWallpaperChangeElement;
extern deque<IgnoredWallpaperChangeElement> WallpaperChangesToIgnore;

typedef COLORREF IgnoredBGColorChangeElement;
extern deque<IgnoredBGColorChangeElement> BGColorChangesToIgnore;

extern int SelfPostedDesktopChildAttached;

BYTE GetBGAlpha(void);
HBITMAP UpdateBG(void);
void UpdateRegions(void);
HBITMAP Refresh(void);

bool DirectoryExists(const _TCHAR *DirName);
bool FileExists(const _TCHAR *DirName);
HRESULT CreateLink(const TCHAR *LinkTarget, const TCHAR *LinkLocation);
void RemoveLink(const TCHAR *LinkLocation);


bool GetMatrixCommandLine(_tstring &Target);


void BeginScreenSaveMode(HWND WindowToNotify = NULL,UINT MessageToSend = 0);
void EndScreenSaveMode(void);
void SetZMatrixSS(void);
void UnSetZMatrixSS(void);
void SetZMatrixSS_NoBackup(void);


void ProcessMiscConfiguration(void);

bool LaunchConfig(IzsMatrix *&ObjectToConfig);
bool LaunchScreenSaverConfig(IzsMatrix *&ObjectToConfig);
bool SaveConfig(IzsMatrix *Matrix,unsigned int RefreshTime,const _TCHAR *ConfigFile = NULL);
bool LoadConfig(IzsMatrix *Matrix,unsigned int &RefreshTime,const _TCHAR *ConfigFile = NULL);

void BeforeClose(void);

bool SaveBitmap( HBITMAP hbmp, LPCTSTR pszFile );
void LogToFile(const _TCHAR *format, ...);

#endif
