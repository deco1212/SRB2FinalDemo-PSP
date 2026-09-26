// psp/i_stub.c
// Stub implementations of networking and CD-audio functions that were
// removed for the PSP port (no TCP/IP master-server networking, no CD drive).
#include <pspmoduleinfo.h>
#include <pspthreadman.h>

extern char __lib_ent_top[], __lib_ent_bottom[];
extern char __lib_stub_top[], __lib_stub_bottom[];

SceModuleInfo module_info
    __attribute__((section(".rodata.sceModuleInfo"), aligned(16), used)) = {
    0, {1, 0}, "SRB2PSP", 0, _gp,
    __lib_ent_top, __lib_ent_bottom,
    __lib_stub_top, __lib_stub_bottom
};

PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

#include "../doomdef.h"
#include "../doomtype.h"
#include "../d_net.h"
#include "../command.h"
#include "../i_net.h"
#include "../i_tcp.h"
#include "../mserv.h"
#include "../i_sound.h"
#include "../s_sound.h"


// ---- i_tcp.h ----
boolean I_InitTcpNetwork(void)
{
	return false;
}

boolean I_Ban(int node)
{
	(void)node;
	return false;
}

void I_ClearBans(void)
{
}

boolean bannednode[MAXNETNODES + 1];

// ---- i_net.h ----
boolean I_InitNetwork(void)
{
	return false;
}

// ---- mserv.h ----
consvar_t cv_masterserver = {"masterserver", "", 0, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_servername   = {"servername", "", 0, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cv_internetserver = {"internetserver", "0", 0, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};

void RegisterServer(int s, int port)
{
	(void)s;
	(void)port;
}

void UnregisterServer(void)
{
}

void SendPingToMasterServer(void)
{
}

const msg_server_t *GetShortServersList(void)
{
	return NULL;
}

void AddMServCommands(void)
{
}

// ---- i_sound.h (CD audio) ----
void I_InitCD(void)
{
}

void I_PauseCD(void)
{
}

void I_ResumeCD(void)
{
}

void I_ShutdownCD(void)
{
}

void I_UpdateCD(void)
{
}

void I_PlayCD(int track, boolean looping)
{
	(void)track;
	(void)looping;
}

// ---- s_sound.h ----
consvar_t cd_volume = {"cdvolume", "0", 0, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};
consvar_t cdUpdate  = {"cdupdate", "0", 0, NULL, NULL, 0, NULL, NULL, 0, 0, NULL};