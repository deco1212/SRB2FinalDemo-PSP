#include "command.h"
#include "doomdef.h"

// --- 1. MASTER MULTIPLAYER CONSOLE VARIABLES ---
consvar_t cv_internetserver = {"internetserver", "Off", 0, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_masterserver = {"masterserver", "Off", 0, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_servername = {"servername", "PSP_Server", 0, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cd_volume = {"cd_volume", "0", 0, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
boolean bannednode = false;

// --- 2. MULTIPLAYER LAYER FUNCTION STUBS ---
void I_ClearBans(void) { }
void I_Ban(int node) { }
void UnregisterServer(void) { }
void RegisterServer(int a, int b) { }
void SendPingToMasterServer(void) { }
void* GetShortServersList(void) { return NULL; }
void AddMServCommands(void) { }
void cdUpdate(void) { }
boolean I_InitNetwork(void) { return false; }
boolean I_InitTcpNetwork(void) { return false; }

// --- 3. RENDERING ENGINE CONSOLE OPTIONS ---
consvar_t cv_grtranswall = {"gr_transwall", "On", 0, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_grfogdensity = {"gr_fogdensity", "0", 0, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_grfov = {"gr_fov", "90", 0, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_grfiltermode = {"gr_filtermode", "0", 0, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};

int gr_basewindowcentery = 136;
int gr_viewheight = 272;
int grfovadjust = 0;

// --- 4. HARDWARE SPRITE TABLES ---
void* lspr = 0;   
void* t_lspr = 0; 

// --- 5. HIGH-LEVEL HARDWARE RENDERER (HWR) STUBS ---
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
void HWR_PrepLevelCache(void) { }
void HWR_CreateStaticLightmaps(void) { }
void HWR_SuperSonicLightToggle(void) { }
void HWR_InitTextureMapping(void) { }
void HWR_SetViewSize(void) { }
void HWR_AddCommands(void) { }
void HWR_SetPalette(void) { }
void HWR_DrawSmallPatch(void) { }
void HWR_DrawPatch(void) { }
void HWR_DrawFill(void) { }
void HWR_DrawMappedPatch(void) { }
void HWR_DrawTranslucentPatch(void) { }
void HWR_DrawFlatFill(void) { }
void HWR_DrawClippedPatch(void) { }
void HWR_FadeScreenMenuBack(void) { }
void HWR_MakePatch(void) { }

void OglSdlShutdown(void) { }
void OglSdlSetPalette(void* pal) { }

// --- 6. MASTER HWDRIVER STRUCT FOR EXT-DRIVER LAYER ---
// Instantly satisfies all 20+ missing hwsym_sdl function lookups!
typedef struct {
    void* funcs[30];
} hwdriver_t;
hwdriver_t hwdriver = {0};
