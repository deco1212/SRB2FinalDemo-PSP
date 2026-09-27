// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// $Id: i_net.c,v 1.2 2000/09/10 10:56:00 metzgermeister Exp $
//
// Copyright (C) 1993-1996 by id Software, Inc.
// Portions Copyright (C) 1998-2000 by DooM Legacy Team.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
// $Log: i_net.c,v $
// Revision 1.2  2000/09/10 10:56:00  metzgermeister
// clean up & made it work again
//
// Revision 1.1  2000/08/21 21:17:32  metzgermeister
// Initial import to CVS
//-----------------------------------------------------------------------------
/// \file
/// \brief SDL network interface

#ifndef _WIN32_WCE
#include <errno.h>
#endif

#include "../doomdef.h"

#include "../i_system.h"
#include "../d_event.h"
#include "../d_net.h"
#include "../m_argv.h"

#include "../doomstat.h"

#include "../i_net.h"

#include "../z_zone.h"

#ifdef PSP
#include "../command.h"
#include "../doomdef.h"

// --- GROUP 1: MULTIPLAYER & MASTER SERVER STUBS ---
consvar_t cv_internetserver = {"internetserver", "Off", 0, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_masterserver = {"masterserver", "Off", 0, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_servername = {"servername", "PSP_Server", 0, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
boolean bannednode = false;

void I_ClearBans(void) { }
void I_Ban(int node) { }
void UnregisterServer(void) { }
void RegisterServer(int a, int b) { }
void SendPingToMasterServer(void) { }
void* GetShortServersList(void) { return NULL; }
void AddMServCommands(void) { }
boolean I_InitNetwork(void) { return false; }
boolean I_InitTcpNetwork(void) { return false; }

// --- GROUP 2: DEHACKED SPRITE TABLE STUBS ---
// Forces the entity trackers to recognize the hardware sprite allocation channels
void* lspr[10000];   
void* t_lspr[10000]; 

// --- GROUP 3: CORE HARDWARE RENDERER (HWR) SYSTEM INTERFACES ---
int gr_basewindowcentery = 136;
int gr_viewheight = 272;

consvar_t cv_grtranswall = {"gr_transwall", "On", 0, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_grfogdensity = {"gr_fogdensity", "0", 0, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_grfov = {"gr_fov", "90", 0, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_grfiltermode = {"gr_filtermode", "0", 0, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};

void HWR_RenderPlayerView(void* player) { }
void HWR_GetTextureUsed(void) { }
void HWR_drawAMline(void* a, void* b, int c) { }
void HWR_clearAutomap(void) { }
void HWR_DrawViewBorder(void) { }
void HWR_SetPaletteColor(int a, int b, int c, int d) { }
void HWR_Screenshot(void) { }
void HWR_ResetLights(void) { }
void HWR_CorrectSWTricks(void) { }
void HWR_CreatePlanePolygons(int map) { }
#endif


#if 0
int I_InitTcpNetwork(void);
//
// NETWORKING
//

void Internal_Get(void)
{
	I_Error("Get without netgame\n");
}

void Internal_Send(void)
{
	I_Error("Send without netgame\n");
}

void Internal_FreeNodenum(int nodenum)
{}
#endif

//
// I_InitNetwork
// Only required for DOS, so this is more a dummy
//
boolean I_InitNetwork (void)
{
	if( M_CheckParm ("-net") )
	{
		I_Error("-net not supported, use -server and -connect\n"
			"see docs for more\n");
	}
	return false;
}
