#include "framework.h"
#include "main.h"
#include "loader.h"
#include "log.h"


BOOL g_bAllInit = FALSE;


void InitHook( void )
{
	LOG_OpenFile(".\\TwofoldLog.log");
	LOG_Info("Welcome to Twofold(tm)!");

	LOG_Info("Attaching hooks...");
	HK_OnInit();

	LOG_Info("Calling fn_vInitEngineWhenInitApplication()...");
	GAM_fn_vInitEngineWhenInitApplication();

	LOG_Info("Loading and initializing mods...");
	LDR_ReadLoadOrder(".\\Mods");
	LDR_LoadAllDlls();
	LDR_InitAllDlls();

	LOG_Info("InitHook finished, handing over control to the game.");
	LOG_Info("- - -");
	g_bAllInit = TRUE;
}

void DesInitHook( void )
{
	if ( !g_bAllInit )
		return;

	LOG_Info("Deinitializing mods...");
	LDR_DesInitAllDlls();
	LDR_UnLoadAllDlls();

	LOG_Info("Detaching hooks...");
	HK_OnDesInit();

	LOG_Info("The game is shutting down. Goodbye!");
	LOG_CloseFile();
	g_bAllInit = FALSE;
}


BOOL APIENTRY DllMain( HMODULE hModule, DWORD dwReason, LPVOID lpReserved )
{
	if ( DetourIsHelperProcess() )
		return TRUE;

	switch ( dwReason )
	{
		case DLL_PROCESS_ATTACH:
			DetourRestoreAfterWith();

			char szModuleName[MAX_PATH];
			GetModuleFileName(GetModuleHandle(NULL), szModuleName, MAX_PATH);
			char *pBaseName = strrchr(szModuleName, '\\');

			if ( !_stricmp(pBaseName, "Rayman2.exe") )
				return FALSE; /* fail the load if not R2 */

			HK_OnDllAttach();
			break;

		case DLL_PROCESS_DETACH:
			HK_OnDllDetach();
			break;

		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
	}

	return TRUE;
}
