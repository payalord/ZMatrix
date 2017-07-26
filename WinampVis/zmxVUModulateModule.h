/*!=========================================================================
//
//  Program:   vis_zmx
//  Module:    $RCSfile: zmxVUModulateModule.h,v $
//  Language:  C/C++
//  Date:      $Date: 2003/05/09 06:20:42 $
//  Version:   $Revision: 1.2 $
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


#ifndef __zmxVUModulateModule_h
#define __zmxVUModulateModule_h

#include "VIS.H"
#include "vis_zmx.h"


namespace zmxVUModulateModule
{

//this module
extern winampVisModule mod;




// "member" functions
void config(struct winampVisModule *this_mod); // configuration dialog

int init(struct winampVisModule *this_mod);	   // initialization for module
int render(struct winampVisModule *this_mod);  // rendering for module
void quit(struct winampVisModule *this_mod);   // deinitialization for module

};

#endif