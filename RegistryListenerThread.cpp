/*!=========================================================================
//
//  Program:   ZMatrix
//  Module:    $RCSfile: RegistryListenerThread.cpp,v $
//  Language:  C/C++
//  Date:      $Date: 2003/05/12 07:43:56 $
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

#include "RegistryListenerThread.h"


DWORD RegistryListenerThreadId = 0;
HANDLE RegistryThreadHandle = NULL;

DWORD ActiveDesktop_RegistryListenerThreadId = 0;
HANDLE ActiveDesktop_RegistryThreadHandle = NULL;

HANDLE StopRegistryListenerEvent = NULL;



//===========================================================================
//===========================================================================
void RegQueryStringAndRealloc(HKEY hKey,const _TCHAR *ValueName,_TCHAR * &Buffer,DWORD &BufferSizeInTCHARs)
{
	DWORD BufferSizeInBytes = BufferSizeInTCHARs*sizeof(_TCHAR);

	while(ERROR_MORE_DATA == RegQueryValueEx(hKey,ValueName,NULL,NULL,(LPBYTE)Buffer,&BufferSizeInBytes))
	{
		BufferSizeInTCHARs = BufferSizeInBytes/sizeof(_TCHAR);
		if(BufferSizeInBytes%sizeof(_TCHAR))
		{
			BufferSizeInTCHARs++;
		}
		BufferSizeInBytes = BufferSizeInTCHARs*sizeof(_TCHAR);

		Buffer = (_TCHAR *) realloc(Buffer,BufferSizeInBytes);
	}
	Buffer[(BufferSizeInBytes/sizeof(_TCHAR)) - 1] = '\0';
}
//===========================================================================
//===========================================================================
DWORD WINAPI RegistryListenerThread(LPVOID lpParameter)
{

	HANDLE HandlesToWaitFor[2];
	HANDLE RegistryChangedEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	HKEY hKey;
	DWORD SignaledHandle = 0;
	bool Done = false;

	_tstring WallpaperBeforeWait;
	_tstring WallpaperAfterWait;
	_tstring WallpaperStyleBeforeWait;
	_tstring WallpaperStyleAfterWait;
	_tstring TileWallpaperBeforeWait;
	_tstring TileWallpaperAfterWait;
	_tstring PatternBeforeWait;
	_tstring PatternAfterWait;
	_TCHAR *Buffer = (_TCHAR *)malloc(sizeof(_TCHAR)*(MAX_PATH + 1));
	DWORD BufferSizeInTCHARs = MAX_PATH + 1;

	HandlesToWaitFor[0] = StopRegistryListenerEvent;
	HandlesToWaitFor[1] = RegistryChangedEvent;



	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,_TEXT("Control Panel\\Desktop"),0,KEY_NOTIFY | KEY_READ,&hKey))
	{

		while(!Done)
		{

			RegQueryStringAndRealloc(hKey,_T("Wallpaper"),Buffer,BufferSizeInTCHARs);
			WallpaperBeforeWait = Buffer;
			RegQueryStringAndRealloc(hKey,_T("WallpaperStyle"),Buffer,BufferSizeInTCHARs);
			WallpaperStyleBeforeWait = Buffer;
			RegQueryStringAndRealloc(hKey,_T("TileWallpaper"),Buffer,BufferSizeInTCHARs);
			TileWallpaperBeforeWait = Buffer;
			RegQueryStringAndRealloc(hKey,_T("Pattern"),Buffer,BufferSizeInTCHARs);
			PatternBeforeWait = Buffer;



			if(ERROR_SUCCESS == RegNotifyChangeKeyValue(hKey,FALSE,REG_NOTIFY_CHANGE_LAST_SET,RegistryChangedEvent,TRUE))
			{
				SignaledHandle = WaitForMultipleObjects(2,HandlesToWaitFor,FALSE,INFINITE);

				switch(SignaledHandle)
				{
				//Triggered by a stop request
				case(WAIT_OBJECT_0):
					{
						Done = true;
					}
					break;
				//Triggered by a registry change
				case(WAIT_OBJECT_0 + 1):
					{

						RegQueryStringAndRealloc(hKey,_T("WallpaperStyle"),Buffer,BufferSizeInTCHARs);
						WallpaperStyleAfterWait = Buffer;
						RegQueryStringAndRealloc(hKey,_T("TileWallpaper"),Buffer,BufferSizeInTCHARs);
						TileWallpaperAfterWait = Buffer;
						RegQueryStringAndRealloc(hKey,_T("Pattern"),Buffer,BufferSizeInTCHARs);
						PatternAfterWait = Buffer;
						RegQueryStringAndRealloc(hKey,_T("Wallpaper"),Buffer,BufferSizeInTCHARs);
						WallpaperAfterWait = Buffer;


						if(WallpaperAfterWait.compare(WallpaperBeforeWait))
						{
							PostMessage(ghWnd,WM_DESKTOPWALLPAPERCHANGED,0,0);
						}
						else if((WallpaperStyleAfterWait.compare(WallpaperStyleBeforeWait)) ||
								(TileWallpaperAfterWait.compare(TileWallpaperBeforeWait)) ||
								(PatternAfterWait.compare(PatternBeforeWait))
							   )
						{
							PostMessage(ghWnd,WM_OTHERDESKTOPSETTINGCHANGED,0,0);
						}

					}
					break;
				default:
					{
					}
					break;
				}

			}
			else
			{
				Done = true;
			}
		}


		RegCloseKey(hKey);
	}

	CloseHandle(RegistryChangedEvent);

	if(Buffer != NULL)
	{
		free(Buffer);
	}

	ExitThread(0);
	return 0;

}
//===========================================================================
//===========================================================================
DWORD WINAPI ActiveDesktop_RegistryListenerThread(LPVOID lpParameter)
{

	HANDLE HandlesToWaitFor[2];
	HANDLE RegistryChangedEvent = CreateEvent(NULL,FALSE,FALSE,NULL);
	HKEY hKey;
	DWORD SignaledHandle = 0;
	bool Done = false;

	_tstring WallpaperBeforeWait;
	_tstring WallpaperAfterWait;
	_tstring WallpaperStyleBeforeWait;
	_tstring WallpaperStyleAfterWait;
	_tstring TileWallpaperBeforeWait;
	_tstring TileWallpaperAfterWait;
	_tstring PatternBeforeWait;
	_tstring PatternAfterWait;
	_TCHAR *Buffer = (_TCHAR *)malloc(sizeof(_TCHAR)*(MAX_PATH + 1));
	DWORD BufferSizeInTCHARs = MAX_PATH + 1;

	HandlesToWaitFor[0] = StopRegistryListenerEvent;
	HandlesToWaitFor[1] = RegistryChangedEvent;



	if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER,_TEXT("Software\\Microsoft\\Internet Explorer\\Desktop\\General"),0,KEY_NOTIFY | KEY_READ,&hKey))
	{

		while(!Done)
		{

			RegQueryStringAndRealloc(hKey,_T("Wallpaper"),Buffer,BufferSizeInTCHARs);
			WallpaperBeforeWait = Buffer;
			RegQueryStringAndRealloc(hKey,_T("WallpaperStyle"),Buffer,BufferSizeInTCHARs);
			WallpaperStyleBeforeWait = Buffer;
			RegQueryStringAndRealloc(hKey,_T("TileWallpaper"),Buffer,BufferSizeInTCHARs);
			TileWallpaperBeforeWait = Buffer;



			if(ERROR_SUCCESS == RegNotifyChangeKeyValue(hKey,FALSE,REG_NOTIFY_CHANGE_LAST_SET,RegistryChangedEvent,TRUE))
			{
				SignaledHandle = WaitForMultipleObjects(2,HandlesToWaitFor,FALSE,INFINITE);

				switch(SignaledHandle)
				{
				//Triggered by a stop request
				case(WAIT_OBJECT_0):
					{
						Done = true;
					}
					break;
				//Triggered by a registry change
				case(WAIT_OBJECT_0 + 1):
					{

						RegQueryStringAndRealloc(hKey,_T("WallpaperStyle"),Buffer,BufferSizeInTCHARs);
						WallpaperStyleAfterWait = Buffer;
						RegQueryStringAndRealloc(hKey,_T("TileWallpaper"),Buffer,BufferSizeInTCHARs);
						TileWallpaperAfterWait = Buffer;
						RegQueryStringAndRealloc(hKey,_T("Wallpaper"),Buffer,BufferSizeInTCHARs);
						WallpaperAfterWait = Buffer;


						if(WallpaperAfterWait.compare(WallpaperBeforeWait))
						{
							PostMessage(ghWnd,WM_DESKTOPWALLPAPERCHANGED,0,0);
						}
						else if((WallpaperStyleAfterWait.compare(WallpaperStyleBeforeWait)) ||
								(TileWallpaperAfterWait.compare(TileWallpaperBeforeWait))
							   )
						{
							PostMessage(ghWnd,WM_OTHERDESKTOPSETTINGCHANGED,0,0);
						}

					}
					break;
				default:
					{
					}
					break;
				}

			}
			else
			{
				Done = true;
			}
		}


		RegCloseKey(hKey);
	}

	CloseHandle(RegistryChangedEvent);

	if(Buffer != NULL)
	{
		free(Buffer);
	}

	ExitThread(0);
	return 0;

}
//===========================================================================
//===========================================================================
void CreateRegistryListenerThread(void)
{
	StopRegistryListenerEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

	RegistryThreadHandle = CreateThread(NULL,0,RegistryListenerThread,NULL,0,&RegistryListenerThreadId);

	//Doesn't work 100% yet, best to leave it disabled.
//	ActiveDesktop_RegistryThreadHandle = CreateThread(NULL,0,ActiveDesktop_RegistryListenerThread,NULL,0,&ActiveDesktop_RegistryListenerThreadId);
}
//===========================================================================
//===========================================================================
void StopRegistryListenerThread(void)
{
	SetEvent(StopRegistryListenerEvent);

	DWORD RetVal = 0;
	//Give it four seconds to exit, before terminating
	if(WAIT_OBJECT_0 != (RetVal = WaitForSingleObject(RegistryThreadHandle,4000)))
	{
		_tstring ErrMessage(_T("Forced to terminate registry listener thread : "));
		switch(RetVal)
		{
		case(WAIT_ABANDONED):
			{
				ErrMessage += _T("WaitForSingleObject returned WAIT_ABANDONED");
			}
			break;
		case(WAIT_TIMEOUT):
			{
				ErrMessage += _T("WaitForSingleObject returned WAIT_TIMEOUT");
			}
			break;
		case(WAIT_FAILED):
			{
				DWORD Err = GetLastError();
				_TCHAR ErrorAsString[32];
				_sntprintf(ErrorAsString,32,_T("%d"),Err);

				ErrMessage += _T("WaitForSingleObject returned WAIT_FAILED, and GetLastError returned error code ");
				ErrMessage += ErrorAsString;
			}
			break;
		}
		MessageBox(ghWnd,ErrMessage.c_str(),_T("Error"),0);
		TerminateThread(RegistryThreadHandle,0);
	}
/*
	//Give it four seconds to exit, before terminating
	if(WAIT_OBJECT_0 != (RetVal = WaitForSingleObject(ActiveDesktop_RegistryThreadHandle,4000)))
	{
		_tstring ErrMessage(_T("Forced to terminate active desktop registry listener thread : "));
		switch(RetVal)
		{
		case(WAIT_ABANDONED):
			{
				ErrMessage += _T("WaitForSingleObject returned WAIT_ABANDONED");
			}
			break;
		case(WAIT_TIMEOUT):
			{
				ErrMessage += _T("WaitForSingleObject returned WAIT_TIMEOUT");
			}
			break;
		case(WAIT_FAILED):
			{
				DWORD Err = GetLastError();
				_TCHAR ErrorAsString[32];
				_sntprintf(ErrorAsString,32,_T("%d"),Err);

				ErrMessage += _T("WaitForSingleObject returned WAIT_FAILED, and GetLastError returned error code ");
				ErrMessage += ErrorAsString;
			}
			break;
		}
		MessageBox(ghWnd,ErrMessage.c_str(),_T("Error"),0);
		TerminateThread(ActiveDesktop_RegistryThreadHandle,0);
	}
*/
	CloseHandle(StopRegistryListenerEvent);
}
