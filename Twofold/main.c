#include "framework.h"
#include "main.h"
#include "loader.h"
#include "log.h"


TwofoldHeader const g_stHeaderConst = { 'R2X', 1, 0 };
TwofoldHeader *g_p_stHeaderInMem = OFFSET(0x4C0000);

BOOL g_bAllInit = FALSE;

BOOL g_bSetLevel = FALSE;
char g_szLevel[30];
char g_szCmdLine[512];


void fn_vParseCommandLine( char const *szCmdLine )
{
	char const *szCurrent;
	if ( szCurrent = strstr(szCmdLine, "-d") )
	{
		MessageBox(NULL, "If you want to debug, attach the debugger now.", "Twofold", MB_OK | MB_ICONINFORMATION);
	}
	if ( szCurrent = strstr(szCmdLine, "-level:") )
	{
		if ( sscanf(szCurrent+7, "%29s", g_szLevel) > 0 )
			g_bSetLevel = TRUE;
	}
	if ( szCurrent = strstr(szCmdLine, "-v") )
	{
		LOG_fn_vSetVerbose(TRUE);
	}
}

void fn_vInitAfterInitEngine( void )
{
	if ( g_bSetLevel )
	{
		GAM_fn_vSetFirstLevelName(g_szLevel);
		*GAM_g_cIsLevelOk = 1;
		LOG_Info("Set first level to '%s'", g_szLevel);
	}
}

void fn_vPrepareReadConfig( void )
{
	if ( !CFG_fn_bDoesFileExist(g_szCfgFile) )
	{
		LOG_Warn("Configuration file missing - writing default config to '%s'", g_szCfgFile);
		CFG_fn_vWriteDefaultConfig();
	}

	CFG_fn_vReadConfig();

	sprintf(g_szCmdLine, "%.255s %.255s", CFG_g_szDefaultCmdLine, GAM_g_szCmdLine);
	fn_vParseCommandLine(g_szCmdLine);

	LDR_fn_bReadLoadOrder();
}

void fn_vInitHook( void )
{
	LOG_fn_vOpenFile(g_szLogFile);
	LOG_Info("Welcome to Twofold(tm)!");

	LOG_Info("Reading configuration...");
	fn_vPrepareReadConfig();

	LOG_Info("Attaching hooks...");
	HK_fn_vOnInit();

	LOG_Info("Calling fn_vInitEngineWhenInitApplication()...");
	GAM_fn_vInitEngineWhenInitApplication();
	fn_vInitAfterInitEngine();

	LOG_Info("Loading and initializing mods...");
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

			if ( g_p_stHeaderInMem->ulHeader == g_stHeaderConst.ulHeader )
				return FALSE; /* fail if already loaded */

			*g_p_stHeaderInMem = g_stHeaderConst; /* write header */

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
