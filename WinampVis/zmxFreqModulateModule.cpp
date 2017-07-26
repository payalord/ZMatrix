/*!=========================================================================
//
//  Program:   vis_zmx
//  Module:    $RCSfile: zmxFreqModulateModule.cpp,v $
//  Language:  C/C++
//  Date:      $Date: 2003/05/12 07:43:57 $
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

#include "zmxFreqModulateModule.h"
#include "../globals.h"
#include "zmxCommonConfigUnit.h"

#define MODULE_NAME "Frequency Modulate"


namespace zmxFreqModulateModule
{

//IzsMatrix *MatrixObject = NULL;
HWND MatrixWindow = NULL;

float BaseScales[3] = {0.0f,0.0f,0.0f};
float BaseOffsets[3] = {0.0f,0.0f,0.0f};
float PeakScales[3] = {2.0f,2.0f,2.0f};
float PeakOffsets[3] = {0.0f,0.0f,0.0f};
float GlobalScale = 5.0f;
float GlobalOffset = 0.0f;

// first module (oscilliscope)
winampVisModule mod =
{
	MODULE_NAME,
	NULL,	// hwndParent
	NULL,	// hDllInstance
	0,		// sRate
	0,		// nCh
	50,		// latencyMS
	50,		// delayMS
	2,		// spectrumNch
	0,		// waveformNch
	{ 0, },	// spectrumData
	{ 0, },	// waveformData
	config,
	init,
	render,
	quit
};

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
void config(struct winampVisModule *this_mod)
{
	//MessageBox(this_mod->hwndParent,"ZMatrix Winamp Visualization -- by Happy Dude","ZMatrix Viz",MB_OK);

    TzmxCommonConfigForm *ConfigForm = new TzmxCommonConfigForm(this_mod->hwndParent,
                                                                BaseScales,
                                                                BaseOffsets,
                                                                PeakScales,
                                                                PeakOffsets,
                                                                &GlobalScale,
                                                                &GlobalOffset);

    if(ConfigForm != NULL)
    {
        if(mrOk == ConfigForm->ShowModal())
        {
            SaveCommonConfig(GetConfigFileName(),
                             MODULE_NAME,
                             BaseScales,
                             BaseOffsets,
                             PeakScales,
                             PeakOffsets,
                             &GlobalScale,
                             &GlobalOffset);
        }
        else
        {
            ConfigForm->RestoreOriginals();
        }
    }

    delete ConfigForm;
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
int init(struct winampVisModule *this_mod)
{

    LoadCommonConfig(GetConfigFileName(),
                     MODULE_NAME,
                     BaseScales,
                     BaseOffsets,
                     PeakScales,
                     PeakOffsets,
                     &GlobalScale,
                     &GlobalOffset);

	MatrixWindow = FindZMatrixWindow();

	if(MatrixWindow)
	{
	}
	else
	{
		//MessageBox(this_mod->hwndParent,"Failed to find the ZMatrix window -- is ZMatrix running?.","ZMatrix Viz",MB_OK);
		return 0;
	}

	return 0;
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
int render(struct winampVisModule *this_mod)
{
	if(!IsWindow(MatrixWindow))
	{
		MatrixWindow = NULL;
	}

	if(MatrixWindow != NULL)
	{

        int x, y;

        double total=0;
        double totalCount=0;
        for (y = 0; y < this_mod->nCh; y ++)
        {
            for (x = 0; x < 288; x ++)
            {
                total += x*this_mod->spectrumData[y][x];
                totalCount += this_mod->spectrumData[y][x];
            }
        }

        float NormFactor = 0;

        if(totalCount > 0)
        {
            NormFactor = total/(288.0*totalCount);
        }

        NormFactor = NormFactor*GlobalScale + GlobalOffset;

        if(NormFactor > 1.0) NormFactor = 1.0;
        if(NormFactor < 0.0) NormFactor = 0.0;

        float OneMinusNormFactor = 1.0 - NormFactor;

        float RScale = (NormFactor*PeakScales[0]) + (OneMinusNormFactor*BaseScales[0]);
        float GScale = (NormFactor*PeakScales[1]) + (OneMinusNormFactor*BaseScales[1]);
        float BScale = (NormFactor*PeakScales[2]) + (OneMinusNormFactor*BaseScales[2]);

        float ROffset = (NormFactor*PeakOffsets[0]) + (OneMinusNormFactor*BaseOffsets[0]);
        float GOffset = (NormFactor*PeakOffsets[1]) + (OneMinusNormFactor*BaseOffsets[1]);
        float BOffset = (NormFactor*PeakOffsets[2]) + (OneMinusNormFactor*BaseOffsets[2]);

		SendMessage(MatrixWindow,WM_SET_COEFF_R1,NULL,RScale);
		SendMessage(MatrixWindow,WM_SET_COEFF_G1,NULL,GScale);
		SendMessage(MatrixWindow,WM_SET_COEFF_B1,NULL,BScale);

		SendMessage(MatrixWindow,WM_SET_COEFF_R0,NULL,ROffset);
		SendMessage(MatrixWindow,WM_SET_COEFF_G0,NULL,GOffset);
		SendMessage(MatrixWindow,WM_SET_COEFF_B0,NULL,BOffset);


	}
	else
	{
		MatrixWindow = FindZMatrixWindow();
	}

	return 0;
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
void quit(struct winampVisModule *this_mod)
{
		SendMessage(MatrixWindow,WM_SET_COEFF_R1,NULL,1.0f);
		SendMessage(MatrixWindow,WM_SET_COEFF_G1,NULL,1.0f);
		SendMessage(MatrixWindow,WM_SET_COEFF_B1,NULL,1.0f);

		SendMessage(MatrixWindow,WM_SET_COEFF_R0,NULL,0.0f);
		SendMessage(MatrixWindow,WM_SET_COEFF_G0,NULL,0.0f);
		SendMessage(MatrixWindow,WM_SET_COEFF_B0,NULL,0.0f);
}
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

};
