// r_psp.h
// PSP sceGu hardware renderer for SRB2 PSP port.
// Modeled on hardware/r_opengl/r_opengl.h

#ifndef _R_PSP_H_
#define _R_PSP_H_

#include <pspgu.h>
#include <pspgum.h>
#include <pspdisplay.h>

#define _CREATE_DLL_ // necessary for the HWRAPI/EXPORT macros to expand to real functions
#include "../../doomdef.h"
#include "../hw_drv.h"

#define DRIVER_STRING "HWRAPI Init(): SRB2 PSP GU renderer"

// A single vertex as fed to sceGuDrawArray: texcoord + color + xyz.
// Matches GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF.
typedef struct
{
	float u, v;
	unsigned int color;
	float x, y, z;
} psp_vertex_t;

extern int psp_screen_width;
extern int psp_screen_height;
extern RGBA_t myPaletteData[];

#endif //_R_PSP_H_
