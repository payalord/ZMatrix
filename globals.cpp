/*!=========================================================================
//
//  Program:   ZMatrix
//  Module:    $RCSfile: globals.cpp,v $
//  Language:  C/C++
//  Date:      $Date: 2003/04/13 22:01:43 $
//  Version:   $Revision: 1.29 $
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
#include "RegistryListenerThread.h"
#include "TopLevelListenerWindow.h"

#include <wininet.h>
#include <shlguid.h>
#include <shlobj.h>

unsigned int RefreshTime = 50;
HANDLE DESKTOPREDRAW_Event = CreateEvent(NULL,FALSE,FALSE,DESKTOPREDRAW_EVENT_ID);

_TCHAR szWinName[] = _TEXT("ZMatrix");
_TCHAR DummyMatrixWindowClassName[] = _TEXT("DummyZMatrixWindowClass");

unsigned int gscreenWidth = 0;
unsigned int gscreenHeight = 0;
int gscreenTop = 0;
int gscreenLeft = 0;


HDC ghdc;
HMENU gSysTrayMenu;
HMENU gSysTrayPopup;

HINSTANCE ghInstance;
HWND ghWnd = NULL;
HWND ghProgman = NULL;
HWND ghShellDLL = NULL;
HWND ghSysListView = NULL;
bool LiteStepMode = false;

NOTIFYICONDATA IconData;
HRGN ValidRGN = NULL;

IzsMatrix *PreScreenSaveMatrixObject = NULL;
bool PreScreenSavePauseState = false;
unsigned int PreScreenSaveRefreshTime = 50;
DWORD PreScreenSavePriorityClass = IDLE_PRIORITY_CLASS;
WNDCLASSEX ScreenSaverDummyClass = {
		/* Define a window class */
	sizeof(WNDCLASSEX),
	0,
	DefWindowProc,
	0,
	0,
	ghInstance,
	LoadIcon(NULL, IDI_WINLOGO),
	LoadCursor(NULL,IDC_ARROW),
	(HBRUSH)GetStockObject(NULL_BRUSH),
	NULL,
	_TEXT("DummyZMatrixWindowClass"),
	LoadIcon(NULL, IDI_WINLOGO)
};


HWND ScreenSaverDummyWindow = NULL;

IzsMatrix *MatrixObject = NULL;

bool Paused = false;

bool DesktopIsCleared = false;
bool WallpaperIsCleared = false;
bool DesktopColorIsCleared = false;

bool OrigDesktopColorIsValid = false;
COLORREF OrigDesktopColor = GetSysColor(COLOR_DESKTOP);

bool OrigWallpaperIsValid = false;
_tstring OrigWallpaperString;
bool OrigActiveDesktopWallpaperIsValid = false;
widestring OrigActiveDesktopWallpaperString;


TCHAR AllUsersStartupDirectoryPath[MAX_PATH];
_tstring AllUsersStartupShortcutPath;
TCHAR CurrentUserStartupDirectoryPath[MAX_PATH];
_tstring CurrentUserStartupShortcutPath;

_tstring MatrixCommandLine;

TCHAR CurrentUserAppDataDirectoryPath[MAX_PATH];
_tstring AppConfigDirectoryPath;
_tstring AppConfigFilePath;
_tstring AppScreenSaverConfigFilePath;
_tstring AppMiscConfigFilePath;

bool ReadyToScreenSave = false;
bool OutstandingScreenSaveRequest = false;
bool InScreenSaveMode = false;
bool UnSetSSWhenDone = false;
vector<OutstandingScreenSaveRequestPair> OutstandingScreenSaveRequestParams;
POINT MouseScreenPointAtScreenSaveStart = {0,0};
bool AlwaysSetAsScreenSaverWhileRunning = false;
bool BlendScreenSaverWithBGOnly = false;

typedef pair<HWND,UINT> NotificationPair;
vector<NotificationPair> WindowsToNotifyWhenSSDone;

deque<IgnoredWallpaperChangeElement> WallpaperChangesToIgnore;
deque<IgnoredBGColorChangeElement> BGColorChangesToIgnore;

int SelfPostedDesktopChildAttached = 0;


vector<HWND> MinimizedWindows;

static bool AlreadyInConfig = false;

//===========================================================================
//===========================================================================
static int getNumColors( int nBits )
{
	
	switch ( nBits ) {
	case 1 : return 2;
	case 4 : return 16;
	case 8 : return 256;
	case 24: return 0;
//	default:
//		assert( false );
	} return 0;
}
//===========================================================================
//===========================================================================
static BITMAP *getInfo( HBITMAP hbmp )
{
	
	static BITMAP bitmap = { { 0 } };
	GetObject( hbmp, sizeof bitmap, &bitmap );
	return &bitmap;
}
//===========================================================================
//===========================================================================
bool SaveBitmap( HBITMAP hbmp, LPCTSTR pszFile )
{
	
	const BITMAP *pBitmap = getInfo( hbmp );
	const int nNumColors = getNumColors( pBitmap->bmBitsPixel );
	const UINT nPalSize = nNumColors * sizeof( RGBQUAD );
	const long lBytes = pBitmap->bmHeight *
		MulDiv( 4, pBitmap->bmWidthBytes + 3, 4 );
	BYTE *pImage = (BYTE *) malloc( lBytes );
	if ( 0 == pImage ) {
		return false;
	}
	
	BITMAPINFOHEADER bitmapInfoHeader = {
		sizeof( BITMAPINFOHEADER ),
			pBitmap->bmWidth,
			pBitmap->bmHeight,
			1,
			pBitmap->bmBitsPixel,
			BI_RGB,
			lBytes,
			0, 0,
			//1 << pBitmap->bmBitsPixel,
	};
	
	const int nHeaderSize = sizeof( BITMAPFILEHEADER ) +
		sizeof( BITMAPINFOHEADER ) + nPalSize;
	const BITMAPFILEHEADER bitmapFileHeader = {
		MAKEWORD( 'B', 'M' ), nHeaderSize + lBytes, 0, 0, nHeaderSize,
	};
	
	HDC hdc = GetDC( 0 );
	const BOOL bOK = GetDIBits(
		hdc, hbmp, 0, (WORD) pBitmap->bmHeight, pImage,
		(BITMAPINFO *) &bitmapInfoHeader, DIB_RGB_COLORS );
	ReleaseDC( 0, hdc );
	if ( !bOK ) {
		return false;
	}
	
	HANDLE h = CreateFile( pszFile,
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0 );
	if ( INVALID_HANDLE_VALUE == h ) {
		return false;
	}
	
	DWORD dwBytesWritten = 0;
	WriteFile( h, &bitmapFileHeader, sizeof bitmapFileHeader,
		&dwBytesWritten, 0 );
	WriteFile( h, &bitmapInfoHeader, sizeof bitmapInfoHeader,
		&dwBytesWritten, 0 );
	
	if ( 0 < nPalSize ) { // Note -- this has never been tested!
		return false;
		MB("Palette will be wrong!");
		
		const BOOL hasPalette = IsClipboardFormatAvailable( CF_PALETTE );
		
		RGBQUAD      rgb[ 256 ] = { 0 };
		PALETTEENTRY pe [ 256 ] = { 0 };
		
//		assert( nPalSize <= 256 );
		HPALETTE hpalette = 0;
		const UINT nColors = GetPaletteEntries( hpalette, 0, nPalSize, pe );
//		assert( nColors == nPalSize );
		
		for ( UINT iColor = 0; iColor < nColors; iColor++) {
			rgb[ iColor ].rgbRed      = pe[ iColor ].peRed  ;
			rgb[ iColor ].rgbGreen    = pe[ iColor ].peGreen;
			rgb[ iColor ].rgbBlue     = pe[ iColor ].peBlue ;
			rgb[ iColor ].rgbReserved = 0;
		}
		
		// Save color table:
		WriteFile( h, rgb, nPalSize, &dwBytesWritten, 0 );
//		assert( dwBytesWritten == nPalSize );
	}
	
	// Save image:
	WriteFile( h, pImage, lBytes, &dwBytesWritten, 0 );
//	assert( dwBytesWritten == lBytes );
	
	CloseHandle( h );
	free( pImage );
	return true;
}
//===========================================================================
//===========================================================================
void LogToFile(const _TCHAR *format, ...)
{
	static bool FirstRun = true;

	static _TCHAR buffer[2048];
	va_list	ap;
	va_start(ap, format);
	_vsntprintf(buffer,sizeof(buffer)/sizeof(_TCHAR), format, ap);
	va_end(ap);

	// Cleanup the log?

	if (FirstRun)
	{
		_tunlink(_T("ZMatrixLog.txt"));
		FirstRun = false;
	}

	// Open the log file

	FILE	*fp = _tfopen(_T("ZMatrixLog.txt"),_T("ab"));

	if (!fp) return;

	// Spit out the data to the log

	_ftprintf(fp,_T("%s\r\n"), buffer);
	fclose(fp);
}
//===========================================================================
//===========================================================================
BOOL CALLBACK MinimizeWindowsProc(HWND hWnd,LPARAM lParam)
{
	if ( !IsWindowVisible( hWnd ) )
		return TRUE;

	if ( GetWindow( hWnd, GW_OWNER ) )
		return TRUE;

	LONG styleEx = GetWindowLong( hWnd, GWL_EXSTYLE );

	if (!( styleEx & WS_EX_APPWINDOW ))
	{
		if ( styleEx & WS_EX_TOOLWINDOW )
			return TRUE;
	}


	if(!IsIconic(hWnd))
	{
		SendMessage(hWnd,WM_SETREDRAW,FALSE,0);
		ShowWindow(hWnd,SW_SHOWMINNOACTIVE);
		SendMessage(hWnd,WM_SETREDRAW,TRUE,0);

		MinimizedWindows.push_back(hWnd);
	}

	return TRUE;
}
//===========================================================================
//===========================================================================
void MinimizeAll(void)
{
	MinimizedWindows.clear();
	EnumWindows(MinimizeWindowsProc,NULL);
}
//===========================================================================
//===========================================================================
void UndoMinimizeAll(void)
{
	vector<HWND>::reverse_iterator Iter = MinimizedWindows.rbegin();
	for(;Iter != MinimizedWindows.rend();++Iter)
	{
		SendMessage(*Iter,WM_SETREDRAW,FALSE,0);
		ShowWindow(*Iter,SW_RESTORE);
		SendMessage(*Iter,WM_SETREDRAW,TRUE,0);
	}
	MinimizedWindows.clear();
}
//===========================================================================
//===========================================================================
void StoreOrigWallpaper(const _TCHAR *NewPaper)
{
	if(NewPaper != NULL)
	{
		OrigWallpaperString = NewPaper;
		OrigWallpaperIsValid = true;
	}
	else
	{
#ifdef WIN9X
		HKEY hKey;


		if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,_T("Control Panel\\Desktop"),0,KEY_QUERY_VALUE,&hKey))
		{
			_TCHAR WallpaperBuffer[MAX_PATH + 1];
			DWORD BufferPathInBytes = sizeof(_TCHAR)*(MAX_PATH + 1);
			DWORD BufferPathInTCHARs = (MAX_PATH + 1);
			memset(WallpaperBuffer,0,BufferPathInBytes);


			DWORD TypeQueried = 0;
			DWORD QueriedSize = BufferPathInBytes;
			if(ERROR_SUCCESS == RegQueryValueEx(hKey,_T("Wallpaper"),0,&TypeQueried,(LPBYTE)WallpaperBuffer,&QueriedSize))
			{
				if(TypeQueried == REG_SZ)
				{
					WallpaperBuffer[QueriedSize/sizeof(_TCHAR) - 1] = _T(0);
					OrigWallpaperString = WallpaperBuffer;
					OrigWallpaperIsValid = true;
				}
				else
				{
					MB("Wallpaper not stored because of bizzare queried type when reading current screensaver.");
				}
			}



			RegCloseKey(hKey);
		}
		else
		{
			MB("Failed to store the original wallpaper.(RegOpenKeyEx)");
		}
#else
		_TCHAR TempBuff[MAX_PATH + 1];
		SystemParametersInfo(SPI_GETDESKWALLPAPER,MAX_PATH+1,TempBuff,0);
		OrigWallpaperString = TempBuff;
		OrigWallpaperIsValid = true;
#endif
	}
}
//===========================================================================
//===========================================================================
void StoreOrigActiveDesktopWallpaper(const WCHAR *NewActiveDesktopPaper)
{
	if(NewActiveDesktopPaper != NULL)
	{
		OrigActiveDesktopWallpaperString = NewActiveDesktopPaper;
		OrigActiveDesktopWallpaperIsValid = true;
	}
	else
	{
		WCHAR TempBuff[MAX_PATH + 1];

		if(HWND IESrv = FindWindowEx(ghShellDLL,0,_TEXT("Internet Explorer_Server"),NULL))
		{
			IActiveDesktop *pActiveDesktop = NULL;

			CoCreateInstance( CLSID_ActiveDesktop, NULL, CLSCTX_SERVER,
								IID_IActiveDesktop, (LPVOID *) &pActiveDesktop );
			if(pActiveDesktop)
			{
				if(S_OK == pActiveDesktop->GetWallpaper(TempBuff,MAX_PATH+1,0))
				{
					OrigActiveDesktopWallpaperString = TempBuff;
					OrigActiveDesktopWallpaperIsValid = true;
				}

				pActiveDesktop->Release();
			}
		}
	}

}
//===========================================================================
//===========================================================================
void StoreOrigDesktopColor(const COLORREF *NewColor)
{
	if(NewColor != NULL)
	{
		OrigDesktopColor = *NewColor;
		OrigDesktopColorIsValid = true;
	}
	else
	{
		OrigDesktopColor = GetSysColor(COLOR_DESKTOP);
		OrigDesktopColorIsValid = true;
	}
}
//===========================================================================
//===========================================================================
void StoreOrigDesktop(const _TCHAR *NewPaper, const WCHAR *NewActiveDesktopPaper, const COLORREF *NewColor)
{
	StoreOrigWallpaper(NewPaper);
	StoreOrigActiveDesktopWallpaper(NewActiveDesktopPaper);
	StoreOrigDesktopColor(NewColor);
}
//===========================================================================
//===========================================================================
void RestoreOrigWallpaper(void)
{
	if(OrigWallpaperIsValid)
	{
		//LogToFile(_T("Restoring wallpaper to %s\n"),OrigWallpaperString.c_str());
		//SystemParametersInfo(SPI_SETDESKWALLPAPER,0,NULL,0);
		SystemParametersInfo(SPI_SETDESKWALLPAPER,0,const_cast<_TCHAR *>(OrigWallpaperString.c_str()),0);
		WallpaperIsCleared = false;
		DesktopIsCleared = (WallpaperIsCleared && DesktopColorIsCleared);
	}

	if(OrigActiveDesktopWallpaperIsValid)
	{
		if(HWND IESrv = FindWindowEx(ghShellDLL,0,_TEXT("Internet Explorer_Server"),NULL))
		{

			//SelfPostedDesktopStyleChanges++;
			//SendMessage(ghSysListView,LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_REGIONAL,0xFFFF);

			IActiveDesktop *pActiveDesktop = NULL;

			CoCreateInstance( CLSID_ActiveDesktop, NULL, CLSCTX_SERVER,
								IID_IActiveDesktop, (LPVOID *) &pActiveDesktop );
			if(pActiveDesktop)
			{
				if(S_OK != pActiveDesktop->SetWallpaper(OrigActiveDesktopWallpaperString.c_str(),0))
				{
					MB("Failed to restore active desktop wallpaper");
				}

				SelfPostedDesktopChildAttached++;

				pActiveDesktop->ApplyChanges(AD_APPLY_HTMLGEN|AD_APPLY_REFRESH|AD_APPLY_FORCE);

				pActiveDesktop->Release();
			}
#ifdef WIN9X
	/*This magic message is supposed to force Win9x to refresh the background... Don't ask me why*/

	//RedrawWindow(ghProgman,NULL,NULL,RDW_ERASE|RDW_INVALIDATE|RDW_UPDATENOW|RDW_ALLCHILDREN);
	SelfPostedDesktopChildAttached++;
	PostMessage(ghProgman,WM_COMMAND,0x1A220,NULL);
	//RedrawWindow(GetDesktopWindow(),NULL,NULL,RDW_ERASE|RDW_INVALIDATE|RDW_UPDATENOW|RDW_ALLCHILDREN);
	//SHChangeNotify(SHCNE_ALLEVENTS,SHCNF_IDLIST,NULL,NULL);
	//PostMessage(ghProgman,WM_KEYDOWN,VK_F5,0);
	//PostMessage(ghProgman,WM_KEYUP,VK_F5,0);
#endif

		}
	}

}
//===========================================================================
//===========================================================================
void RestoreOrigDesktopColor(void)
{
	if(OrigDesktopColorIsValid)
	{
		int DesktopColorID = COLOR_DESKTOP;

		BGColorChangesToIgnore.push_back(OrigDesktopColor);
		SetSysColors(1,&DesktopColorID,&OrigDesktopColor);
		DesktopColorIsCleared = false;

		DesktopIsCleared = (WallpaperIsCleared && DesktopColorIsCleared);
	}

}
//===========================================================================
//===========================================================================
void RestoreOrigDesktop(void)
{
	RestoreOrigDesktopColor();
	RestoreOrigWallpaper();
}
//===========================================================================
//===========================================================================
void ClearWallpaper(void)
{

	if(HWND IESrv = FindWindowEx(ghShellDLL,0,_TEXT("Internet Explorer_Server"),NULL))
	{
		IActiveDesktop *pActiveDesktop = NULL;

		CoCreateInstance( CLSID_ActiveDesktop, NULL, CLSCTX_SERVER,
							IID_IActiveDesktop, (LPVOID *) &pActiveDesktop );
		if(pActiveDesktop)
		{
			if(S_OK != pActiveDesktop->SetWallpaper(L"",0))
			{
				MB("Failed to set active desktop wallpaper");
			}

			SelfPostedDesktopChildAttached++;
			pActiveDesktop->ApplyChanges(AD_APPLY_HTMLGEN|AD_APPLY_REFRESH|AD_APPLY_FORCE);

			pActiveDesktop->Release();

			SetWindowPos(ghWnd,IESrv,0,0,0,0,SWP_NOSIZE|SWP_NOMOVE);
		}

#ifdef WIN9X
	/*This magic message is supposed to force Win9x to refresh the background... Don't ask me why*/

	//RedrawWindow(ghProgman,NULL,NULL,RDW_ERASE|RDW_INVALIDATE|RDW_UPDATENOW|RDW_ALLCHILDREN);
	SelfPostedDesktopChildAttached++;
	PostMessage(ghProgman,WM_COMMAND,0x1A220,NULL);
	//SHChangeNotify(SHCNE_ALLEVENTS,SHCNF_IDLIST,NULL,NULL);
	//PostMessage(ghProgman,WM_KEYDOWN,VK_F5,0);
	//PostMessage(ghProgman,WM_KEYUP,VK_F5,0);
#endif
		//SelfPostedDesktopStyleChanges++;
		//SendMessage(ghSysListView,LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_REGIONAL,0x0000);
	}


	SystemParametersInfo(SPI_SETDESKWALLPAPER,0,(void *)_T(""),0);
	WallpaperIsCleared = true;


	DesktopIsCleared = (WallpaperIsCleared && DesktopColorIsCleared);
}
//===========================================================================
//===========================================================================
void ClearDesktopColor(void)
{
	INT DesktopColorID = COLOR_DESKTOP;
	COLORREF BlackColor = 0;

	BGColorChangesToIgnore.push_back(BlackColor);
	SetSysColors(1,&DesktopColorID,&BlackColor);
	DesktopColorIsCleared = true;

	DesktopIsCleared = (WallpaperIsCleared && DesktopColorIsCleared);
}
//===========================================================================
//===========================================================================
void ClearDesktop(void)
{
	ClearDesktopColor();
	ClearWallpaper();
}
//===========================================================================
//===========================================================================
void EnforceDesktop(void)
{
	BYTE CurrentBGAlpha = GetBGAlpha();

	if( !DesktopIsCleared && ((CurrentBGAlpha < 128) || (MatrixObject->GetBGMode() == bgmodeColor)))
	{
		ClearDesktop();
	}
	else if(DesktopIsCleared && ((CurrentBGAlpha >= 128) && (MatrixObject->GetBGMode() != bgmodeColor)))
	{
		RestoreOrigDesktop();
	}
}
//===========================================================================
//===========================================================================
BYTE GetBGAlpha(void)
{
	BYTE R,G,B,A;
	if(MatrixObject != NULL)
	{
		MatrixObject->GetBGColor(R,G,B,A);
		return A;
	}
	return 0;
}
//===========================================================================
//===========================================================================
HBITMAP UpdateBG(void)
{

	if(HWND IESrv = FindWindowEx(ghShellDLL,0,_TEXT("Internet Explorer_Server"),NULL))
	{

		MinimizeAll();

		BringWindowToTop(IESrv);
		RedrawWindow(IESrv,NULL,NULL,RDW_ERASE|RDW_INVALIDATE|RDW_UPDATENOW|RDW_ALLCHILDREN);


		HDC source = GetDC(IESrv);

		HBITMAP hBGBitmap = CreateCompatibleBitmap(source,gscreenWidth,gscreenHeight);

		HDC TempDC = CreateCompatibleDC(source);
		SelectObject(TempDC,hBGBitmap);

		BitBlt(TempDC,0,0,gscreenWidth,gscreenHeight,source,0,0,SRCCOPY);
//SaveBitmap(hBGBitmap,_T("Test.bmp"));
		DeleteDC(TempDC);

		ReleaseDC(IESrv,source);


		SetWindowPos(IESrv,ghSysListView,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
		//RedrawWindow(ghSysListView,NULL,NULL,RDW_ERASE|RDW_INVALIDATE|RDW_UPDATENOW|RDW_ALLCHILDREN);

		UndoMinimizeAll();

		return hBGBitmap;
	}
	else
	{

		HDC target = GetDC(0);
		SetViewportOrgEx(target,gscreenLeft,gscreenTop,NULL);

		RECT ClientRect;
		GetClientRect(ghWnd,&ClientRect);
		int Width = ClientRect.right - ClientRect.left;
		int Height = ClientRect.bottom - ClientRect.top;

		HBITMAP hBGBitmap = CreateCompatibleBitmap(target,Width,Height);


		if(!PaintDesktop(target))
		{
			MB("Failed to PaintDesktop");
		}

		HDC TempDC = CreateCompatibleDC(target);
		SelectObject(TempDC,hBGBitmap);
		BitBlt(TempDC,ClientRect.left,ClientRect.top,Width,Height,target,ClientRect.left,ClientRect.top,SRCCOPY);
	//SaveBitmap(hBGBitmap,_T("UpdatedBG.bmp"));
		DeleteDC(TempDC);

		SetViewportOrgEx(target,-gscreenLeft,-gscreenTop,NULL);
		ReleaseDC(0,target);

		InvalidateRect(NULL,NULL,TRUE);

		//if(MatrixObject != NULL)
		//	MatrixObject->SetBGBitmap(hBGBitmap);

		return hBGBitmap;
	}
}
//===========================================================================
//===========================================================================
void UpdateRegions(void)
{
	if(ghSysListView != NULL)
	{
		DWORD ListViewStyles;
		HRGN hListViewRGN = CreateRectRgn(0,0,0,0);
		HRGN hWndRGN = CreateRectRgn(0,0,gscreenWidth,gscreenHeight);
		const unsigned int MaxRgnAttempts = 2;


		ListViewStyles = ListView_GetExtendedListViewStyle(ghSysListView);
		//SelfPostedDesktopStyleChanges++;
		SendMessage(ghSysListView,LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_REGIONAL,0xFFFF);

		RedrawWindow(ghSysListView,NULL,NULL,RDW_ERASE|RDW_INVALIDATE|RDW_UPDATENOW|RDW_ALLCHILDREN);


		//hListViewRGN = CreateRectRgn(0,0,0,0);
		unsigned int RgnAttempts = 0;
		while((RgnAttempts < MaxRgnAttempts) &&
			  (GetWindowRgn(ghSysListView,hListViewRGN) == ERROR)
			 )
		{
			RgnAttempts++;
		}

		if(RgnAttempts >= MaxRgnAttempts)
		{
			//This is not neccessarily an error condition, because
			//it may arise if there are no icons on the desktop.
			//MB("Failed to get RGN for SysListView");
			DeleteObject(hListViewRGN);
			DeleteObject(hWndRGN);
			return;
		}

		//SelfPostedDesktopStyleChanges++;
		SendMessage(ghSysListView,LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_REGIONAL,ListViewStyles);

		//if(GetWindowRgn(hWnd,hWndRGN) == ERROR) MB("Error, couldn't get RGN for hWnd");

		if(CombineRgn(ValidRGN,hWndRGN,hListViewRGN,RGN_DIFF) == ERROR) MB("Error, couldn't calculate the diff region");

		DeleteObject(hListViewRGN);
		DeleteObject(hWndRGN);
	}
}
//===========================================================================
//===========================================================================
HBITMAP Refresh(void)
{
	if(DesktopIsCleared)
	{
		RestoreOrigDesktop();
		UpdateRegions();
		HBITMAP RetVal = UpdateBG();
		ClearDesktop();

		return RetVal;
	}
	else
	{
		UpdateRegions();
		HBITMAP RetVal = UpdateBG();

		return RetVal;
	}
}
//===========================================================================
//===========================================================================
bool DirectoryExists(const _TCHAR *DirName)
{
	if(DirName == NULL) return false;

	DWORD Attrib = GetFileAttributes((_TCHAR *)DirName);

	return ((Attrib != INVALID_FILE_ATTRIBUTES) && (FILE_ATTRIBUTE_DIRECTORY & Attrib));
}
//===========================================================================
//===========================================================================
bool FileExists(const _TCHAR *DirName)
{
	if(DirName == NULL) return false;

	DWORD Attrib = GetFileAttributes((_TCHAR *)DirName);

	return ((Attrib != INVALID_FILE_ATTRIBUTES) && (!(FILE_ATTRIBUTE_DIRECTORY & Attrib)));
}
//===========================================================================
//===========================================================================
HRESULT CreateLink(const TCHAR *LinkTarget, const TCHAR *LinkLocation) 
{ 
	if((LinkTarget == NULL) || (LinkLocation == NULL))
	{
		return -1;
	}

    HRESULT hres; 
    IShellLink* psl; 
 
    // Get a pointer to the IShellLink interface. 
    hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *) &psl); 

    if (SUCCEEDED(hres))
	{ 
        IPersistFile* ppf; 
 
        // Set the path to the shortcut target and add the 
        // description. 
        psl->SetPath(LinkTarget);
 
       // Query IShellLink for the IPersistFile interface for saving the 
       // shortcut in persistent storage. 
        hres = psl->QueryInterface(IID_IPersistFile,(LPVOID*)&ppf); 
 
        if (SUCCEEDED(hres))
		{
			unsigned int Length = _tcslen(LinkLocation);
			WCHAR *TempBuffer = new WCHAR[_tcslen(LinkLocation) + 1];
			for(unsigned int i = 0; i < Length; i++)
			{
				TempBuffer[i] = LinkLocation[i];
			}
			TempBuffer[Length] = (WCHAR)0;

            // Save the link by calling IPersistFile::Save. 
            hres = ppf->Save(TempBuffer, TRUE);
			delete[] TempBuffer;
            ppf->Release(); 
        } 
        psl->Release(); 
    } 
    return hres; 
}
//===========================================================================
//===========================================================================
void RemoveLink(const TCHAR *LinkLocation) 
{
    _tunlink(LinkLocation);
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
void BeginScreenSaveMode(HWND WindowToNotify,UINT MessageToSend)
{
	if(WindowToNotify != NULL)
	{
		NotificationPair NewPair(WindowToNotify,MessageToSend);
		if(WindowsToNotifyWhenSSDone.end() == find(WindowsToNotifyWhenSSDone.begin(),WindowsToNotifyWhenSSDone.end(),NewPair))
		{
			WindowsToNotifyWhenSSDone.push_back(NewPair);
		}
	}

	if(!InScreenSaveMode)
	{
		if(MatrixObject == NULL) return;

		HRESULT Result = NO_ERROR;
		MULTI_QI Qi;
		Qi.pIID = &IID_IZSMATRIX;
		Qi.pItf = NULL;
		Qi.hr = 0;

		if(FAILED(Result = CoCreateInstanceEx(CLSID_ZSMATRIX,NULL,CLSCTX_ALL,NULL,1,&Qi) ) )
		{
			MB("Failed to create BackupObjectToConfig");
			return;
		}

		PreScreenSaveMatrixObject = (IzsMatrix *)Qi.pItf;
		PreScreenSaveMatrixObject->CopyFrom(*MatrixObject);

		PreScreenSaveRefreshTime = RefreshTime;
		PreScreenSavePriorityClass = GetPriorityClass(GetCurrentProcess());
		PreScreenSavePauseState = Paused;
		Paused = false;

		if(BlendScreenSaverWithBGOnly)
		{
			if(FileExists(AppScreenSaverConfigFilePath.c_str()))
			{
				LoadConfig(MatrixObject,RefreshTime,AppScreenSaverConfigFilePath.c_str());
				SetTimer(ghWnd,REFRESH_TIMER_ID,RefreshTime,0);
			}
		}
		else
		{
			if(DesktopIsCleared)
			{
				RestoreOrigDesktop();
			}


			HDC target = GetDC(0);
			SetViewportOrgEx(target,gscreenLeft,gscreenTop,NULL);
			HDC TempDC = CreateCompatibleDC(target);



			RECT ClientRect;
			GetClientRect(ghWnd,&ClientRect);
			int Width = ClientRect.right - ClientRect.left;
			int Height = ClientRect.bottom - ClientRect.top;

			HBITMAP hBGBitmap = CreateCompatibleBitmap(target,Width,Height);


			SelectObject(TempDC,hBGBitmap);
			BitBlt(TempDC,0,0,Width,Height,target,ClientRect.left,ClientRect.top,SRCCOPY);
			DeleteDC(TempDC);

			SetViewportOrgEx(target,-gscreenLeft,-gscreenTop,NULL);
			ReleaseDC(0,target);


			if(FileExists(AppScreenSaverConfigFilePath.c_str()))
			{
				LoadConfig(MatrixObject,RefreshTime,AppScreenSaverConfigFilePath.c_str());
				SetTimer(ghWnd,REFRESH_TIMER_ID,RefreshTime,0);
			}



			MatrixObject->SetBGBitmap(hBGBitmap);

			if(!DeleteObject(hBGBitmap))
			{
				DWORD Temp = GetLastError();
				MB("Failed to delete old BG bitmap after starting a screensave");
			}


			BYTE CurrentBGAlpha = GetBGAlpha();
			if( !DesktopIsCleared && ((CurrentBGAlpha < 128) || (MatrixObject->GetBGMode() == bgmodeColor)))
			{
				ClearDesktop();
			}
		}



		if (!RegisterClassEx(&ScreenSaverDummyClass))
		{
			MB("Failed to register dummy matrix window class");
		}
		else
		{

			ScreenSaverDummyWindow = CreateWindowEx(WS_EX_TOPMOST|WS_EX_TOOLWINDOW,DummyMatrixWindowClassName,_T("DummyMatrixWindow"),WS_POPUP,gscreenLeft,gscreenTop,gscreenWidth,gscreenHeight,
													NULL,NULL,ghInstance,NULL);

			ShowWindow(ScreenSaverDummyWindow,SW_SHOW);
		}

		GetCursorPos(&MouseScreenPointAtScreenSaveStart);
		ShowCursor(FALSE);

		InScreenSaveMode = true;
		SetMouseMessageHook(ghWnd);
		CheckMenuItem(gSysTrayPopup,ID_SCREENSAVE,MF_CHECKED);
	}
}
//===========================================================================
//===========================================================================
void EndScreenSaveMode(void)
{

	if(InScreenSaveMode)
	{



		if((MatrixObject != NULL) && (PreScreenSaveMatrixObject != NULL))
		{
			MatrixObject->CopyFrom(*PreScreenSaveMatrixObject);
			PreScreenSaveMatrixObject->Release();
			PreScreenSaveMatrixObject = NULL;

			RefreshTime = PreScreenSaveRefreshTime;

			if(PreScreenSavePauseState)
			{
				KillTimer(ghWnd,REFRESH_TIMER_ID);
				Paused = true;
			}
			else
			{
				SetTimer(ghWnd,REFRESH_TIMER_ID,RefreshTime,0);
			}

			SetPriorityClass(GetCurrentProcess(),PreScreenSavePriorityClass);

/*
//SaveBitmap(PreScreenSaveBGBitmap,_T("BitmapSetAfterScreenSave.bmp"));
			HBITMAP OldBitmap = MatrixObject->SetBGBitmap_NonCopy(PreScreenSaveBGBitmap);
//SaveBitmap(OldBitmap,_T("BitmapReplacedAfterScreenSave.bmp"));

			PreScreenSaveBGBitmap = NULL;

			if(!DeleteObject(OldBitmap))
			{
				DWORD Temp = GetLastError();
				MB("Failed to delete old BG bitmap after a screensave");
			}
*/

		}


		InScreenSaveMode = false;
		ClearMouseMessageHook(ghWnd);
		CheckMenuItem(gSysTrayPopup,ID_SCREENSAVE,MF_UNCHECKED);


		ShowWindow(ScreenSaverDummyWindow,SW_HIDE);

		if(!DestroyWindow(ScreenSaverDummyWindow))
		{
			MB("Failed to destroy dummy matrix window");
		}


		if (!UnregisterClass(DummyMatrixWindowClassName,ghInstance))
		{
			MB("Failed to unregister dummy matrix window class");
		}

		ShowCursor(TRUE);

		EnforceDesktop();

		RedrawWindow(NULL,NULL,NULL,RDW_ALLCHILDREN | RDW_ERASE | RDW_INVALIDATE | RDW_ERASENOW);
		//SendMessage(HWND_BROADCAST,WM_PAINT,0,0);

	}


	vector<NotificationPair>::iterator Iter = WindowsToNotifyWhenSSDone.begin();

	for(;Iter != WindowsToNotifyWhenSSDone.end();++Iter)
	{
		PostMessage(Iter->first,Iter->second,0,0);
	}
	WindowsToNotifyWhenSSDone.clear();
}
//===========================================================================
//===========================================================================
void SetZMatrixSS(void)
{

	HKEY hKey;


	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,_T("Control Panel\\Desktop"),0,KEY_ALL_ACCESS,&hKey))
	{
		_TCHAR ScreenSaverPathBuffer[MAX_PATH+1];
		DWORD BufferPathInBytes = sizeof(_TCHAR)*(MAX_PATH + 1);
		DWORD BufferPathInTCHARs = (MAX_PATH + 1);
		memset(ScreenSaverPathBuffer,0,BufferPathInBytes);


		DWORD TypeQueried = 0;
		DWORD QueriedSize = BufferPathInBytes;
		if(ERROR_SUCCESS == RegQueryValueEx(hKey,_T("SCRNSAVE.EXE"),0,&TypeQueried,(LPBYTE)ScreenSaverPathBuffer,&QueriedSize))
		{
			if(TypeQueried == REG_SZ)
			{
				if(ERROR_SUCCESS != RegSetValueEx(hKey,_T("SCRNSAVE.EXE_PreZMatrix"),0,REG_SZ,(LPBYTE)ScreenSaverPathBuffer,sizeof(_TCHAR)*(_tcslen(ScreenSaverPathBuffer) + 1)))
				{
					MB("Failed to backup previous screensaver.");
				}
			}
			else
			{
				MB("Previous screensaver not backed up because of bizzare queried type.");
			}
		}

		GetWindowsDirectory(ScreenSaverPathBuffer,BufferPathInTCHARs);
		_tcscat(ScreenSaverPathBuffer,_T("\\ZMATRI~1.SCR"));

		if(ERROR_SUCCESS != RegSetValueEx(hKey,_T("SCRNSAVE.EXE"),0,REG_SZ,(LPBYTE)ScreenSaverPathBuffer,sizeof(_TCHAR)*(_tcslen(ScreenSaverPathBuffer) + 1)))
		{
			MB("Failed to set ZMatrixSS as the current screen saver.(RegSetValueEx)");
		}

		RegCloseKey(hKey);
	}
	else
	{
		MB("Failed to set ZMatrixSS as the current screen saver.(RegOpenKeyEx)");
	}
}
//===========================================================================
//===========================================================================
void UnSetZMatrixSS(void)
{

	HKEY hKey;


	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,_T("Control Panel\\Desktop"),0,KEY_ALL_ACCESS,&hKey))
	{
		_TCHAR ScreenSaverPathBuffer[MAX_PATH+1];
		DWORD BufferPathInBytes = sizeof(_TCHAR)*(MAX_PATH + 1);
		DWORD BufferPathInTCHARs = (MAX_PATH + 1);
		memset(ScreenSaverPathBuffer,0,BufferPathInBytes);


		DWORD TypeQueried = 0;
		DWORD QueriedSize = BufferPathInBytes;
		if(ERROR_SUCCESS == RegQueryValueEx(hKey,_T("SCRNSAVE.EXE"),0,&TypeQueried,(LPBYTE)ScreenSaverPathBuffer,&QueriedSize))
		{
			if(TypeQueried == REG_SZ)
			{
				_TCHAR AlternateBuffer[MAX_PATH+1];
				DWORD AlternateBufferPathInBytes = sizeof(_TCHAR)*(MAX_PATH + 1);
				DWORD AlternateBufferPathInTCHARs = (MAX_PATH + 1);
				memset(AlternateBuffer,0,BufferPathInBytes);

				GetWindowsDirectory(AlternateBuffer,AlternateBufferPathInTCHARs);
				_tcscat(AlternateBuffer,_T("\\ZMATRI~1.SCR"));

				if(!_tcsicmp(AlternateBuffer,ScreenSaverPathBuffer))
				{
					TypeQueried = 0;
					QueriedSize = BufferPathInBytes;
					memset(ScreenSaverPathBuffer,0,BufferPathInBytes);

					if(ERROR_SUCCESS == RegQueryValueEx(hKey,_T("SCRNSAVE.EXE_PreZMatrix"),0,&TypeQueried,(LPBYTE)ScreenSaverPathBuffer,&QueriedSize))
					{
						if(TypeQueried == REG_SZ)
						{
							if(ERROR_SUCCESS != RegSetValueEx(hKey,_T("SCRNSAVE.EXE"),0,REG_SZ,(LPBYTE)ScreenSaverPathBuffer,sizeof(_TCHAR)*(_tcslen(ScreenSaverPathBuffer) + 1)))
							{
								MB("Failed to restore previous screensaver.");
							}

							RegDeleteValue(hKey,_T("SCRNSAVE.EXE_PreZMatrix"));

						}
						else
						{
							MB("Previous screensaver not restored because of bizzare queried type when reading backed up screensaver.");
						}
					}
					else
					{
						RegDeleteValue(hKey,_T("SCRNSAVE.EXE"));
					}
				}
				else
				{
					RegDeleteValue(hKey,_T("SCRNSAVE.EXE_PreZMatrix"));
				}
			}
			else
			{
				MB("Previous screensaver not restored because of bizzare queried type when reading current screensaver.");
			}
		}



		RegCloseKey(hKey);
	}
	else
	{
		MB("Failed to restore the original screen saver.(RegOpenKeyEx)");
	}
}
//===========================================================================
//===========================================================================
void SetZMatrixSS_NoBackup(void)
{
	HKEY hKey;


	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,_T("Control Panel\\Desktop"),0,KEY_ALL_ACCESS,&hKey))
	{
		_TCHAR ScreenSaverPathBuffer[MAX_PATH+1];
		DWORD BufferPathInBytes = sizeof(_TCHAR)*(MAX_PATH + 1);
		DWORD BufferPathInTCHARs = (MAX_PATH + 1);
		memset(ScreenSaverPathBuffer,0,BufferPathInBytes);


		GetWindowsDirectory(ScreenSaverPathBuffer,BufferPathInTCHARs);
		_tcscat(ScreenSaverPathBuffer,_T("\\ZMATRI~1.SCR"));

		if(ERROR_SUCCESS != RegSetValueEx(hKey,_T("SCRNSAVE.EXE"),0,REG_SZ,(LPBYTE)ScreenSaverPathBuffer,sizeof(_TCHAR)*(_tcslen(ScreenSaverPathBuffer) + 1)))
		{
			MB("Failed to set ZMatrixSS as the current screen saver.(RegSetValueEx)");
		}

		RegCloseKey(hKey);
	}
	else
	{
		MB("Failed to set ZMatrixSS as the current screen saver.(RegOpenKeyEx)");
	}
}
//===========================================================================
//===========================================================================
void ProcessMiscConfiguration(void)
{
	unsigned int RetStrSize = 128;
	_TCHAR *RetStr = (_TCHAR *)malloc(sizeof(_TCHAR)*RetStrSize);

	while((RetStrSize - 1) == GetPrivateProfileString(_T("ZMatrix"),_T("SetAsScreenSaverWhileRunning"),_T("false"),RetStr,RetStrSize,_T("./ZMatrix.ini")))
	{
		RetStrSize += 128;
		RetStr = (_TCHAR *)realloc(RetStr,sizeof(_TCHAR)*RetStrSize);
	}

	if(!_tcsicmp(_T("true"),RetStr))
	{
		SetZMatrixSS();
		UnSetSSWhenDone = true;

		AlwaysSetAsScreenSaverWhileRunning = true;
		CheckMenuItem(gSysTrayPopup,ID_ALWAYS_SET_AS_SCREENSAVER_WHILE_RUNNING,MF_CHECKED);
	}
	else
	{
		AlwaysSetAsScreenSaverWhileRunning = false;
		CheckMenuItem(gSysTrayPopup,ID_ALWAYS_SET_AS_SCREENSAVER_WHILE_RUNNING,MF_UNCHECKED);
	}




	while((RetStrSize - 1) == GetPrivateProfileString(_T("General"),_T("BlendScreenSaverWithBGOnly"),_T("false"),RetStr,RetStrSize,AppMiscConfigFilePath.c_str()))
	{
		RetStrSize += 128;
		RetStr = (_TCHAR *)realloc(RetStr,sizeof(_TCHAR)*RetStrSize);
	}

	if(!_tcsicmp(_T("true"),RetStr))
	{
		BlendScreenSaverWithBGOnly = true;
		CheckMenuItem(gSysTrayPopup,ID_BLEND_SCREENSAVER_WITH_BG_ONLY,MF_CHECKED);
	}
	else
	{
		BlendScreenSaverWithBGOnly = false;
		CheckMenuItem(gSysTrayPopup,ID_BLEND_SCREENSAVER_WITH_BG_ONLY,MF_UNCHECKED);
	}



	free(RetStr);

}
//===========================================================================
//===========================================================================
void SetAlwaysSetAsScreenSaverWhileRunning(bool NewVal)
{
	if(NewVal != AlwaysSetAsScreenSaverWhileRunning)
	{
		if(NewVal)
		{
			AlwaysSetAsScreenSaverWhileRunning = true;
			CheckMenuItem(gSysTrayPopup,ID_ALWAYS_SET_AS_SCREENSAVER_WHILE_RUNNING,MF_CHECKED);

			SetZMatrixSS();
			UnSetSSWhenDone = true;

			WritePrivateProfileString(_T("ZMatrix"),_T("SetAsScreenSaverWhileRunning"),_T("true"),_T("./ZMatrix.ini"));
		}
		else
		{
			AlwaysSetAsScreenSaverWhileRunning = false;
			CheckMenuItem(gSysTrayPopup,ID_ALWAYS_SET_AS_SCREENSAVER_WHILE_RUNNING,MF_UNCHECKED);

			UnSetZMatrixSS();
			UnSetSSWhenDone = false;

			WritePrivateProfileString(_T("ZMatrix"),_T("SetAsScreenSaverWhileRunning"),_T("false"),_T("./ZMatrix.ini"));
		}
	}
}
//===========================================================================
//===========================================================================
void SetBlendScreenSaverWithBGOnly(bool NewVal)
{
	if(NewVal != BlendScreenSaverWithBGOnly)
	{
		if(NewVal)
		{
			BlendScreenSaverWithBGOnly = true;
			CheckMenuItem(gSysTrayPopup,ID_BLEND_SCREENSAVER_WITH_BG_ONLY,MF_CHECKED);

			WritePrivateProfileString(_T("General"),_T("BlendScreenSaverWithBGOnly"),_T("true"),AppMiscConfigFilePath.c_str());
		}
		else
		{
			BlendScreenSaverWithBGOnly = false;
			CheckMenuItem(gSysTrayPopup,ID_BLEND_SCREENSAVER_WITH_BG_ONLY,MF_UNCHECKED);

			WritePrivateProfileString(_T("General"),_T("BlendScreenSaverWithBGOnly"),_T("false"),AppMiscConfigFilePath.c_str());
		}
	}
}
//===========================================================================
//===========================================================================
bool LaunchConfig(IzsMatrix *&ObjectToConfig)
{
	bool RetVal = false;

	if(!AlreadyInConfig)
	{
		AlreadyInConfig = true;
		HINSTANCE hDLL = LoadLibrary(_TEXT("Config.dll"));

		if(hDLL == NULL)
		{
			MB("Failed to load Config.dll");
			RetVal = false;
		}
		else
		{
			ConfigFormLauncher ConfigLauncher = (ConfigFormLauncher)GetProcAddress(hDLL,"LaunchConfigForm");

			if(ConfigLauncher == NULL)
			{
				MB("Failed to find LaunchConfigForm in Config.dll");
				RetVal = false;
			}
			else
			{
				HRESULT Result = NO_ERROR;
				MULTI_QI Qi;
				Qi.pIID = &IID_IZSMATRIX;
				Qi.pItf = NULL;
				Qi.hr = 0;

				if(FAILED(Result = CoCreateInstanceEx(CLSID_ZSMATRIX,NULL,CLSCTX_ALL,NULL,1,&Qi) ) )
				{
					MB("Failed to create BackupObjectToConfig");
					RetVal = false;
				}
				else
				{

					IzsMatrix *BackupObjectToConfig = (IzsMatrix *)Qi.pItf;
					BackupObjectToConfig->CopyFrom(*ObjectToConfig);

					unsigned int BackupRefreshTime = RefreshTime;
					DWORD Priority = GetPriorityClass(GetCurrentProcess());
					DWORD BackupPriority = Priority;

					UINT PreviousPauseMenuItemState = EnableMenuItem(gSysTrayMenu,ID_PAUSE,MF_GRAYED);
					bool PreviousPauseState = Paused;
					if(Paused)
					{
						SetTimer(ghWnd,REFRESH_TIMER_ID,RefreshTime,0);
						Paused = false;
					}

					int Temp = 0;
					try
					{
						Temp = ConfigLauncher(ObjectToConfig,RefreshTime,Priority);
					}
					catch(...)
					{
						MB("Exception while in config dialog");
						Temp = 0;
					}

					if(0 == Temp)
					{
						//This scenario arises if the program is closed while the config form is open
						if(ObjectToConfig == NULL)
						{
							PostQuitMessage(0);
							RetVal = false;
						}
						else
						{
							ObjectToConfig->CopyFrom(*BackupObjectToConfig);
							RefreshTime = BackupRefreshTime;
							Priority = BackupPriority;
							RetVal = false;
						}
					}
					else
					{
						RetVal = true;
					}

					if(PreviousPauseState)
					{
						KillTimer(ghWnd,REFRESH_TIMER_ID);
						Paused = true;
					}
					else
					{
						SetTimer(ghWnd,REFRESH_TIMER_ID,RefreshTime,0);
					}

					EnableMenuItem(gSysTrayMenu,ID_PAUSE,PreviousPauseMenuItemState);

					SetPriorityClass(GetCurrentProcess(),Priority);

					BackupObjectToConfig->Release();
				}
				//ConfigLauncher(Temp);
			}

			FreeLibrary(hDLL);
		}
		AlreadyInConfig = false;
	}

	if(RetVal)
	{
		SaveConfig(ObjectToConfig,RefreshTime);
	}

	return RetVal;
}
//===========================================================================
//===========================================================================
bool LaunchScreenSaverConfig(IzsMatrix *&ObjectToConfig)
{
	bool RetVal = false;

	if(!AlreadyInConfig)
	{
		AlreadyInConfig = true;
		HINSTANCE hDLL = LoadLibrary(_TEXT("Config.dll"));

		if(hDLL == NULL)
		{
			MB("Failed to load Config.dll");
			RetVal = false;
		}
		else
		{
			ConfigFormLauncher ConfigLauncher = (ConfigFormLauncher)GetProcAddress(hDLL,"LaunchConfigForm");

			if(ConfigLauncher == NULL)
			{
				MB("Failed to find LaunchConfigForm in Config.dll");
				RetVal = false;
			}
			else
			{
				HRESULT Result = NO_ERROR;
				MULTI_QI Qi;
				Qi.pIID = &IID_IZSMATRIX;
				Qi.pItf = NULL;
				Qi.hr = 0;

				if(FAILED(Result = CoCreateInstanceEx(CLSID_ZSMATRIX,NULL,CLSCTX_ALL,NULL,1,&Qi) ) )
				{
					MB("Failed to create BackupObjectToConfig");
					RetVal = false;
				}
				else
				{

					IzsMatrix *BackupObjectToConfig = (IzsMatrix *)Qi.pItf;
					BackupObjectToConfig->CopyFrom(*ObjectToConfig);

					unsigned int BackupRefreshTime = RefreshTime;
					DWORD BackupPriority = GetPriorityClass(GetCurrentProcess());

					bool CreatedFile = false;
					if(!FileExists(AppScreenSaverConfigFilePath.c_str()))
					{
						CopyFile(AppConfigFilePath.c_str(),AppScreenSaverConfigFilePath.c_str(),TRUE);
						CreatedFile = true;
					}

					UINT PreviousPauseMenuItemState = EnableMenuItem(gSysTrayMenu,ID_PAUSE,MF_GRAYED);
					bool PreviousPauseState = Paused;
					Paused = false;

					LoadConfig(ObjectToConfig,RefreshTime,AppScreenSaverConfigFilePath.c_str());
					SetTimer(ghWnd,REFRESH_TIMER_ID,RefreshTime,0);
					DWORD Priority = GetPriorityClass(GetCurrentProcess());

					int Temp = 0;
					try
					{
						Temp = ConfigLauncher(ObjectToConfig,RefreshTime,Priority);
					}
					catch(...)
					{
						MB("Exception while in config dialog");
						Temp = 0;
					}

					if(0 == Temp)
					{
						//This scenario arises if the program is closed while the config form is open
						if(ObjectToConfig == NULL)
						{
							PostQuitMessage(0);
							RetVal = false;
						}
						else
						{
							ObjectToConfig->CopyFrom(*BackupObjectToConfig);
							RefreshTime = BackupRefreshTime;

							if(CreatedFile)
							{
								DeleteFile(AppScreenSaverConfigFilePath.c_str());
							}

							RetVal = false;
						}
					}
					else
					{
						SaveConfig(ObjectToConfig,RefreshTime,AppScreenSaverConfigFilePath.c_str());

						ObjectToConfig->CopyFrom(*BackupObjectToConfig);
						RefreshTime = BackupRefreshTime;
						RetVal = true;
					}

					if(PreviousPauseState)
					{
						KillTimer(ghWnd,REFRESH_TIMER_ID);
						Paused = true;
					}
					else
					{
						SetTimer(ghWnd,REFRESH_TIMER_ID,RefreshTime,0);
					}

					EnableMenuItem(gSysTrayMenu,ID_PAUSE,PreviousPauseMenuItemState);

					SetPriorityClass(GetCurrentProcess(),BackupPriority);

					BackupObjectToConfig->Release();
				}
				//ConfigLauncher(Temp);
			}

			FreeLibrary(hDLL);
		}
		AlreadyInConfig = false;
	}


	return RetVal;
}
//===========================================================================
//===========================================================================
bool SaveConfig(IzsMatrix *Matrix,unsigned int RefreshTime,const _TCHAR *ConfigFile)
{
	bool RetVal = false;

	if(Matrix == NULL)
		return false;
	
	if(ConfigFile == NULL)
	{
		ConfigFile = AppConfigFilePath.c_str();
	}

	//Save the configuration to file using the method provided in config.dll
	HINSTANCE hDLL = LoadLibrary(_TEXT("Config.dll"));

	if(hDLL != NULL)
	{
		ConfigSaver Saver = (ConfigSaver)GetProcAddress(hDLL,"SaveConfigToFile");

		if(Saver != NULL)
		{
			try
			{
				RetVal = (0 != Saver(Matrix,RefreshTime,GetPriorityClass(GetCurrentProcess()),(_TCHAR *)ConfigFile));
			}
			catch(...)
			{
				RetVal = false;
				MB("Exception while saving config file");
			}

			//Saver(MatrixObject,_TEXT("ZMatrix.cfg"));
		}

		FreeLibrary(hDLL);
	}

	return RetVal;
}
//===========================================================================
//===========================================================================
bool LoadConfig(IzsMatrix *Matrix,unsigned int &RefreshTime,const _TCHAR *ConfigFile)
{
	bool RetVal = false;

	if(Matrix == NULL)
		return false;
	
	if(ConfigFile == NULL)
	{
		ConfigFile = AppConfigFilePath.c_str();
	}

	//Load the configuration from file using the method provided in config.dll
	HINSTANCE hDLL = LoadLibrary(_TEXT("Config.dll"));
	DWORD PriorityClass = IDLE_PRIORITY_CLASS;

	if(hDLL != NULL)
	{
		ConfigLoader Loader = (ConfigLoader)GetProcAddress(hDLL,"LoadConfigFromFile");

		if(Loader != NULL)
		{

				try
				{
					if(!FileExists(ConfigFile))
					{
						CopyFile(_TEXT("default.cfg"),(_TCHAR *)ConfigFile,FALSE);
					}


					RetVal = (0 != Loader(Matrix,RefreshTime,PriorityClass,(_TCHAR *)ConfigFile));
				}
				catch(...)
				{
					MB("Exception while loading config file");
					RetVal = 0;
				}




		}

		FreeLibrary(hDLL);
	}

	SetPriorityClass(GetCurrentProcess(),PriorityClass);
	SetTimer(ghWnd,REFRESH_TIMER_ID,RefreshTime,NULL);

	Paused = false;

	return RetVal;
}
//===========================================================================
//===========================================================================
void BeforeClose(void)
{
	//Do these things here and now because otherwise, the program may be
	//killed before it gets done.

	static bool AlreadyClosing = false;

	if(!AlreadyClosing)
	{
		AlreadyClosing = true;

		ClearDesktopMonitorHook(ghWnd);

		//SaveConfig(MatrixObject,RefreshTime);

		if(UnSetSSWhenDone)
		{
			UnSetZMatrixSS();
		}


		StopRegistryListenerThread();
		DestroyTopLevelListener();

		RestoreOrigDesktop();
		ShowCursor(TRUE);


		DeleteObject(ValidRGN);
		DestroyMenu(gSysTrayMenu);
		DestroyIcon(IconData.hIcon);



		Shell_NotifyIcon(NIM_DELETE,&IconData);

		MatrixObject->Release();

		MatrixObject = NULL;

		if(PreScreenSaveMatrixObject != NULL)
		{
			PreScreenSaveMatrixObject->Release();
			PreScreenSaveMatrixObject = NULL;
		}

		if(!DestroyWindow(ghWnd))
			MB("Failed to delete ZMatrix rendering window");

		if(!UnregisterClass(szWinName,ghInstance))
			MB("Failed to unregister ZMatrix rendering class");
	}
}
//===========================================================================
//===========================================================================