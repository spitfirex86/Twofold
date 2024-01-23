#include "framework.h"
#include "main.h"
#include "loader.h"
#include "log.h"


BOOL g_bAllInit = FALSE;


void ParseCommandLine( char const *szCmdLine )
{
	char const *szCurrent;
	if ( szCurrent = strstr(szCmdLine, "-d") )
	{
		MessageBox(NULL, "If you want to debug, attach the debugger now.", "Twofold", MB_OK | MB_ICONINFORMATION);
	}
	if ( szCurrent = strstr(szCmdLine, "-level:") )
	{
		char szLevel[30];
		if ( sscanf(szCurrent+7, "%29s", szLevel) > 0 )
		{
			GAM_fn_vSetFirstLevelName(szLevel);
			*GAM_g_cIsLevelOk = 1;
			LOG_Info("Set first level to '%s'", szLevel);
		}
	}
	if ( szCurrent = strstr(szCmdLine, "-v") )
	{
		LOG_SetVerbose(TRUE);
	}
}

void InitHook( void )
{
	LOG_OpenFile(".\\TwofoldLog.log");
	LOG_Info("Welcome to Twofold(tm)!");

	LOG_Info("Attaching hooks...");
	HK_OnInit();

	LOG_Info("Calling fn_vInitEngineWhenInitApplication()...");
	GAM_fn_vInitEngineWhenInitApplication();

	ParseCommandLine(GAM_g_szCmdLine);

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
