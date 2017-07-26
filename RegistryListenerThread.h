/*!=========================================================================
//
//  Program:   ZMatrix
//  Module:    $RCSfile: RegistryListenerThread.h,v $
//  Language:  C/C++
//  Date:      $Date: 2002/12/18 07:18:20 $
//  Version:   $Revision: 1.6 $
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


#ifndef __RegistryListenerThread_h
#define __RegistryListenerThread_h


#include "globals.h"

void RegQueryStringAndRealloc(HKEY hKey,const _TCHAR *ValueName,_TCHAR * &Buffer,DWORD &BufferSizeInTCHARs);

void CreateRegistryListenerThread(void);
void StopRegistryListenerThread(void);

#endif