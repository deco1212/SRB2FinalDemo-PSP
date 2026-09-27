// r_psp.c
// PSP sceGu hardware renderer for SRB2 PSP port.
// Modeled function-by-function on hardware/r_opengl/r_opengl.c, translating
// each GL call to its sceGu equivalent.
//
// KNOWN LIMITATIONS (v1 - get something textured on screen, refine after):
//  - No triangle fan primitive on PSP GE: DrawPolygon manually expands
//    the incoming fan into a triangle list before submitting.
//  - SetTexture always re-uploads (no mipmap cache reuse yet, matching
//    gr_cachehead/gr_cachetail in r_opengl.c) - fine for correctness,
//    will hit VRAM/bandwidth limits before HW OpenGL would.
//  - SetBlend implements the common cases (Masked, Translucent, opaque);
//    Additive/Substractive/Environment fall back to normal alpha blend.
//  - Draw2DLine, DrawMD2, SetSpecialState, ReadRect are stubs for now.
//  - Coronas (PF_Corona) are skipped entirely (early-return), since they
//    need a depth-buffer readback that's expensive on PSP; revisit later.
//
// IMPORTANT: this driver calls sceGuInit()/sceGuStart() itself. If SDL's
// PSP video backend (SDL_pspvideo.c) already did so for the software
// renderer's screen blit, we must NOT double-init the GE. See the Init()
// comment below - this needs coordinating with i_video.c once integrated.

#include "r_psp.h"
#include <string.h>
#include <malloc.h>

// ==========================================================================
//                                                                    GLOBALS
// ==========================================================================

int psp_screen_width  = 480;
int psp_screen_height = 272;

static I_Error_t I_Error_PSP;

// Display lists live in uncached RAM, one per frame (double list, matching
// the classic sceGu double-buffer pattern).
static unsigned int __attribute__((aligned(16))) displayList[262144];

// Current polygon/blend flags, mirrors CurrentPolyFlags in r_opengl.c
static FBITFIELD CurrentPolyFlags = 0;

// ==========================================================================
//                                                                       INIT
// ==========================================================================

EXPORT boolean HWRAPI( Init ) (I_Error_t FatalErrorFunction)
{
	I_Error_PSP = FatalErrorFunction;

	// NOTE: if SDL's PSP backend already called sceGuInit() for the
	// software-render screen blit, calling it again here is undefined.
	// For now this assumes we own the GE outright (i.e. i_video.c's PSP
	// path is being changed to skip its own sceGuInit when HWRENDER is
	// defined). Flag this as a TODO to wire up when integrating.
	sceGuInit();

	sceGuStart(GU_DIRECT, displayList);

	sceGuDrawBuffer(GU_PSM_8888, (void *)0, 512);
	sceGuDispBuffer(psp_screen_width, psp_screen_height, (void *)0x88000, 512);
	sceGuDepthBuffer((void *)0x110000, 512);

	sceGuOffset(2048 - (psp_screen_width / 2), 2048 - (psp_screen_height / 2));
	sceGuViewport(2048, 2048, psp_screen_width, psp_screen_height);
	sceGuDepthRange(65535, 0);

	sceGuScissor(0, 0, psp_screen_width, psp_screen_height);
	sceGuEnable(GU_SCISSOR_TEST);

	sceGuDepthFunc(GU_GEQUAL);
	sceGuEnable(GU_DEPTH_TEST);

	sceGuFrontFace(GU_CW);
	sceGuShadeModel(GU_SMOOTH);
	sceGuEnable(GU_TEXTURE_2D);

	sceGuFinish();
	sceGuSync(0, 0);

	sceDisplayWaitVblankStart();
	sceGuDisplay(GU_TRUE);

	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadIdentity();
	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();
	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();

	DBG_Printf("%s\n", DRIVER_STRING);
	return 1;
}

// ==========================================================================
//                                                            FRAME LIFECYCLE
// ==========================================================================

EXPORT void HWRAPI( ClearBuffer ) ( FBOOLEAN ColorMask, FBOOLEAN DepthMask, FRGBAFloat *ClearColor )
{
	unsigned int clearMode = 0;
	unsigned int color = 0xFF000000;

	if (ColorMask)
	{
		if (ClearColor)
		{
			unsigned int r = (unsigned int)(ClearColor->red   * 255.0f);
			unsigned int g = (unsigned int)(ClearColor->green * 255.0f);
			unsigned int b = (unsigned int)(ClearColor->blue  * 255.0f);
			unsigned int a = (unsigned int)(ClearColor->alpha * 255.0f);
			color = (a << 24) | (b << 16) | (g << 8) | r;
		}
		clearMode |= GU_COLOR_BUFFER_BIT;
	}
	if (DepthMask)
		clearMode |= GU_DEPTH_BUFFER_BIT;

	sceGuStart(GU_DIRECT, displayList);
	sceGuClearColor(color);
	sceGuClearDepth(0);
	sceGuClear(clearMode);
	// NOTE: real frame start/finish bracketing (sceGuStart/sceGuFinish per
	// frame, not per ClearBuffer call) needs to be coordinated against
	// FinishUpdate - this is a placeholder structure until that's wired up.
}

EXPORT void HWRAPI( FinishUpdate ) ( int waitvbl )
{
	sceGuFinish();
	sceGuSync(0, 0);

	if (waitvbl)
		sceDisplayWaitVblankStart();

	sceGuSwapBuffers();
}

// ==========================================================================
//                                                                    TEXTURE
// ==========================================================================

EXPORT void HWRAPI( SetTexture ) ( FTextureInfo *pTexInfo )
{
	int w, h;
	static RGBA_t tex[256 * 256] __attribute__((aligned(16)));

	if (pTexInfo->downloaded)
	{
		sceGuTexImage(0, pTexInfo->width, pTexInfo->height, pTexInfo->width, (const void *)(unsigned long)pTexInfo->downloaded);
		return;
	}

	w = pTexInfo->width;
	h = pTexInfo->height;

	// Convert whatever source format into flat RGBA8888, same approach
	// r_opengl.c takes (see its SetTexture for the full format list -
	// this covers the common paletted case; extend as other formats
	// show up in testing).
	if (pTexInfo->grInfo.format == GR_TEXFMT_P_8 || pTexInfo->grInfo.format == GR_TEXFMT_AP_88)
	{
		unsigned char *pImgData = (unsigned char *)pTexInfo->grInfo.data;
		int i, j;
		for (j = 0; j < h; j++)
		{
			for (i = 0; i < w; i++)
			{
				if ((*pImgData == HWR_PATCHES_CHROMAKEY_COLORINDEX) && (pTexInfo->flags & TF_CHROMAKEYED))
				{
					tex[w*j+i].s.red = tex[w*j+i].s.green = tex[w*j+i].s.blue = tex[w*j+i].s.alpha = 0;
				}
				else
				{
					tex[w*j+i] = myPaletteData[*pImgData];
				}
				pImgData++;
				if (pTexInfo->grInfo.format == GR_TEXFMT_AP_88)
				{
					if (!(pTexInfo->flags & TF_CHROMAKEYED))
						tex[w*j+i].s.alpha = *pImgData;
					pImgData++;
				}
			}
		}
	}
	else if (pTexInfo->grInfo.format == GR_RGBA)
	{
		memcpy(tex, pTexInfo->grInfo.data, w * h * sizeof(RGBA_t));
	}
	else
	{
		DBG_Printf("SetTexture(bad format) %d\n", pTexInfo->grInfo.format);
		return;
	}

	// Allocate texture memory via the GU allocator and copy the converted
	// data in. A real implementation should swizzle this for performance
	// (sceGuTexMode with swizzle flag + a swizzling pass) - left as a
	// follow-up since correctness comes first.
	{
		void *texMem = sceGuGetMemory(w * h * sizeof(RGBA_t));
		memcpy(texMem, tex, w * h * sizeof(RGBA_t));
		pTexInfo->downloaded = (FUINT)(unsigned long)texMem;
	}

	sceGuTexMode(GU_PSM_8888, 0, 0, 0);
	sceGuTexImage(0, w, h, w, (const void *)(unsigned long)pTexInfo->downloaded);
	sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);

	sceGuTexWrap(
		(pTexInfo->flags & TF_WRAPX) ? GU_REPEAT : GU_CLAMP,
		(pTexInfo->flags & TF_WRAPY) ? GU_REPEAT : GU_CLAMP
	);
	sceGuTexFilter(GU_LINEAR, GU_LINEAR);
}

EXPORT void HWRAPI( ClearMipMapCache ) ( void )
{
	// TODO: mirror r_opengl.c's gr_cachehead walk once texture reuse is
	// implemented. For now, textures are re-uploaded each SetTexture call
	// with downloaded==0, so there's nothing persistent to free yet.
}

// ==========================================================================
//                                                                     BLEND
// ==========================================================================

EXPORT void HWRAPI( SetBlend ) ( FBITFIELD PolyFlags )
{
	FBITFIELD Xor = CurrentPolyFlags ^ PolyFlags;

	if (Xor & PF_Blending)
	{
		switch (PolyFlags & PF_Blending)
		{
			case PF_Masked:
				sceGuEnable(GU_ALPHA_TEST);
				sceGuAlphaFunc(GU_GREATER, 0x00, 0xff);
				sceGuDisable(GU_BLEND);
				break;
			case PF_Translucent:
			case PF_Additive: // simplified: real additive needs GU_ADD blend eq
				sceGuDisable(GU_ALPHA_TEST);
				sceGuEnable(GU_BLEND);
				sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
				break;
			default: // opaque
				sceGuDisable(GU_ALPHA_TEST);
				sceGuDisable(GU_BLEND);
				break;
		}
	}

	if (Xor & PF_NoDepthTest)
		(PolyFlags & PF_NoDepthTest) ? sceGuDisable(GU_DEPTH_TEST) : sceGuEnable(GU_DEPTH_TEST);

	CurrentPolyFlags = PolyFlags;
}

// ==========================================================================
//                                                              DRAW POLYGON
// ==========================================================================

EXPORT void HWRAPI( DrawPolygon ) ( FSurfaceInfo *pSurf, FOutVector *pOutVerts, FUINT iNumPts, FBITFIELD PolyFlags )
{
	unsigned int color = 0xFFFFFFFF;
	psp_vertex_t *verts;
	FUINT i, triCount;

	if (PolyFlags & PF_MD2)
		return;
	if (PolyFlags & PF_Corona) // skipped for now, see file header
		return;
	if (iNumPts < 3)
		return;

	SetBlend(PolyFlags);

	if ((CurrentPolyFlags & PF_Modulated) && pSurf)
	{
		color = (pSurf->FlatColor.s.alpha << 24) | (pSurf->FlatColor.s.blue << 16) |
		        (pSurf->FlatColor.s.green << 8) | pSurf->FlatColor.s.red;
	}

	// PSP GE has no triangle-fan primitive: expand fan (0, i, i+1) into
	// a flat triangle list, same vertex data GL_TRIANGLE_FAN would walk.
	triCount = iNumPts - 2;
	verts = (psp_vertex_t *)sceGuGetMemory(triCount * 3 * sizeof(psp_vertex_t));

	for (i = 0; i < triCount; i++)
	{
		psp_vertex_t *v = &verts[i * 3];

		v[0].u = pOutVerts[0].sow;     v[0].v = pOutVerts[0].tow;
		v[0].x = pOutVerts[0].x;       v[0].y = pOutVerts[0].y;      v[0].z = pOutVerts[0].z;
		v[0].color = color;

		v[1].u = pOutVerts[i+1].sow;   v[1].v = pOutVerts[i+1].tow;
		v[1].x = pOutVerts[i+1].x;     v[1].y = pOutVerts[i+1].y;    v[1].z = pOutVerts[i+1].z;
		v[1].color = color;

		v[2].u = pOutVerts[i+2].sow;   v[2].v = pOutVerts[i+2].tow;
		v[2].x = pOutVerts[i+2].x;     v[2].y = pOutVerts[i+2].y;    v[2].z = pOutVerts[i+2].z;
		v[2].color = color;
	}

	sceGuDrawArray(GU_TRIANGLES,
		GU_TEXTURE_32BITF | GU_COLOR_8888 | GU_VERTEX_32BITF | GU_TRANSFORM_3D,
		triCount * 3, 0, verts);
}

// ==========================================================================
//                                                                TRANSFORM
// ==========================================================================

EXPORT void HWRAPI( SetTransform ) (FTransform *transform)
{
	ScePspFMatrix4 proj, view;

	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadIdentity();
	sceGumPerspective(transform->fovxangle, (float)psp_screen_width / (float)psp_screen_height, 1.0f, 32768.0f);

	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();
	// TODO: apply transform->anglex/angley (pitch/yaw) via sceGumRotateX/Y
	// and transform->x/y/z via sceGumTranslate, matching how r_opengl.c's
	// SetTransform builds its modelview stack. Left as identity for the
	// first pass to isolate other bugs before tackling camera orientation.
	(void)proj; (void)view;

	sceGumMatrixMode(GU_MODEL);
	sceGumLoadIdentity();
}

// ==========================================================================
//                                                          MISC / CLIPPING
// ==========================================================================

EXPORT void HWRAPI( GClipRect ) (int minx, int miny, int maxx, int maxy, float nearclip)
{
	sceGuScissor(minx, miny, maxx, maxy);
	(void)nearclip; // folded into SetTransform's perspective near-plane instead
}

EXPORT int HWRAPI( GetTextureUsed ) (void)
{
	return 0; // TODO: track VRAM used by uploaded textures
}

EXPORT int HWRAPI( GetRenderVersion ) (void)
{
	return 1;
}

// ==========================================================================
//                                                          NOT YET WIRED UP
// ==========================================================================

EXPORT void HWRAPI( Draw2DLine ) (F2DCoord *v1, F2DCoord *v2, RGBA_t Color)
{
	(void)v1; (void)v2; (void)Color;
	// TODO: HUD/console lines. Low priority vs getting 3D geometry correct.
}

EXPORT void HWRAPI( DrawMD2 ) (int *gl_cmd_buffer, md2_frame_t *frame, FTransform *pos, float scale)
{
	(void)gl_cmd_buffer; (void)frame; (void)pos; (void)scale;
	// TODO: player/enemy models. Software renderer already handles sprites;
	// this only matters once/if MD2 model support is desired on PSP.
}

EXPORT void HWRAPI( SetSpecialState ) (hwdspecialstate_t IdState, int Value)
{
	(void)IdState; (void)Value;
	// TODO: fog table/mode/color/density, FOV, palette color. Stubbed.
}

EXPORT void HWRAPI( ReadRect ) (int x, int y, int width, int height, int dst_stride, unsigned short *dst_data)
{
	(void)x; (void)y; (void)width; (void)height; (void)dst_stride; (void)dst_data;
	// TODO: used for coronas' depth-buffer readback, which we skip for now.
}
