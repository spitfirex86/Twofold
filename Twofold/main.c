#include "framework.h"
#include "main.h"
#include "loader.h"
#include "log.h"


BOOL g_bAllInit = FALSE;


void fn_vParseCommandLine( char const *szCmdLine )
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
		LOG_fn_vSetVerbose(TRUE);
	}
}

void fn_vInitHook( void )
{
	LOG_fn_vOpenFile(".\\TwofoldLog.log");
	LOG_Info("Welcome to Twofold(tm)!");

	LOG_Info("Attaching hooks...");
	HK_fn_vOnInit();

	LOG_Info("Calling fn_vInitEngineWhenInitApplication()...");
	GAM_fn_vInitEngineWhenInitApplication();

	fn_vParseCommandLine(GAM_g_szCmdLine);

	LOG_Info("Loading and initializing mods...");
	LDR_fn_bReadLoadOrder(".\\Mods");
	LDR_fn_vLoadAllDlls();
	LDR_fn_vInitAllDlls();

	LOG_Info("InitHook finished, handing over control to the game.");
	LOG_Info("- - -");
	g_bAllInit = TRUE;
}

void fn_vDesInitHook( void )
{
	if ( !g_bAllInit )
		return;

	LOG_Info("Deinitializing mods...");
	LDR_fn_vDesInitAllDlls();
	LDR_fn_vUnLoadAllDlls();

	LOG_Info("Detaching hooks...");
	HK_fn_vOnDesInit();

	LOG_Info("Cleaning up...");
	LDR_fn_vFreeLoadOrder();

	LOG_Info("The game is shutting down. Goodbye!");
	LOG_fn_vCloseFile();
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

			HK_fn_vOnDllAttach();
			break;

		case DLL_PROCESS_DETACH:
			HK_fn_vOnDllDetach();
			break;

		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
	}

	return TRUE;
}
