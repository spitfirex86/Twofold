#include "framework.h"
#include "loader.h"
#include "log.h"


#define LDR_C_InitProc "ModMain"

typedef int (*InitProc)( BOOL bInit, void *reserved );

typedef struct DllInfo
{
	HMODULE hDll;
	InitProc pInitProc;
	int lResult;
	DWORD dwLastError;
	char szDllPath[MAX_PATH];
}
DllInfo;


char LDR_gModDir[MAX_PATH] = { 0 };

DllInfo *LDR_gModList = NULL;
long LDR_gNbMods = 0;
long LDR_gNbLoaded = 0;
long LDR_gNbInitialized = 0;


DllInfo * LDR_AllocInfo( void )
{
	size_t newSize = (LDR_gNbMods + 1) * sizeof(DllInfo);
	DllInfo *pNewList = realloc(LDR_gModList, newSize);

	if ( !pNewList )
		return NULL; /* TODO: this should probably exit the whole process */

	LDR_gModList = pNewList;
	long idx = LDR_gNbMods++;

	DllInfo *pInfo = pNewList + idx;
	*pInfo = (DllInfo){ 0 };
	return pInfo;
}

BOOL LDR_ReadLoadOrder( char const *szModDir )
{
	strcpy(LDR_gModDir, szModDir);
	CreateDirectory(LDR_gModDir, NULL);

	/* TODO: actual load order */

	char szSearchPath[MAX_PATH];
	sprintf(szSearchPath, "%s\\%s", LDR_gModDir, "*.dll");

	WIN32_FIND_DATA ffd = { 0 };
	HANDLE hFind = FindFirstFile(szSearchPath, &ffd);
	if ( hFind != INVALID_HANDLE_VALUE )
	{
		do
		{
			DllInfo *pDll = LDR_AllocInfo();
			sprintf(pDll->szDllPath, "%s\\%s", LDR_gModDir, ffd.cFileName);
		}
		while ( FindNextFile(hFind, &ffd) );
		FindClose(hFind);
	}

	LOG_Info("Found %d DLLs in '%s'", LDR_gNbMods, LDR_gModDir);
	return TRUE;
}

void LDR_FreeLoadOrder( void )
{
	free(LDR_gModList);
	LDR_gModList = NULL;
	LDR_gNbMods = 0;
}

BOOL LDR_LoadOneDll( DllInfo *pDll )
{
	HMODULE hDll = LoadLibrary(pDll->szDllPath);
	pDll->hDll = hDll;
	if ( !hDll )
	{
		pDll->dwLastError = GetLastError();
		LOG_Error("LoadLibrary failed for '%s' (0x%08x)", pDll->szDllPath, pDll->dwLastError);
		return FALSE;
	}

	InitProc pInitProc = (InitProc)GetProcAddress(hDll, LDR_C_InitProc);
	pDll->pInitProc = pInitProc;
	if ( !pInitProc )
	{
		pDll->dwLastError = GetLastError();
		LOG_Warn(
			"%s export missing in '%s', probably a legacy mod or helper library? (0x%08x)",
			LDR_C_InitProc, pDll->szDllPath, pDll->dwLastError
		);
	}

	LOG_Info("Successfully loaded '%s' (HMODULE: %p)", pDll->szDllPath, pDll->hDll);
	return TRUE;
}

BOOL LDR_UnLoadOneDll( DllInfo *pDll )
{
	if ( !pDll->hDll )
		return FALSE;

	if ( !FreeLibrary(pDll->hDll) )
	{
		pDll->dwLastError = GetLastError();
		LOG_Error("FreeLibrary failed for '%s' (0x%08x)", pDll->szDllPath, pDll->dwLastError);
		return FALSE;
	}
	pDll->hDll = NULL;
	pDll->pInitProc = NULL;

	LOG_Info("Requested for '%s' to be unloaded", pDll->szDllPath);
	return TRUE;
}

void LDR_LoadAllDlls( void )
{
	for ( int i = 0; i < LDR_gNbMods; ++i )
	{
		if ( LDR_LoadOneDll(LDR_gModList + i) )
			++LDR_gNbLoaded;
	}
	LOG_Info("Loaded %d (out of %d) mods", LDR_gNbLoaded, LDR_gNbMods);
}

void LDR_UnLoadAllDlls( void )
{
	long lNbUnLoaded = 0;
	for ( int i = 0; i < LDR_gNbMods; ++i )
	{
		if ( LDR_UnLoadOneDll(LDR_gModList + i) )
			++lNbUnLoaded;
	}
	LOG_Info("Unloaded %d (out of %d) mods", lNbUnLoaded, LDR_gNbMods);
}

BOOL LDR_InitOneDll( DllInfo *pDll )
{
	if ( !pDll->pInitProc )
		return FALSE;

	int lResult = pDll->pInitProc(TRUE, NULL);
	pDll->lResult = lResult;

	if ( lResult != 0 )
	{
		LOG_Error("Initialization failed for '%s' (function returned %d)", pDll->szDllPath, lResult);
		return FALSE;
	}
		
	LOG_Info("Successfully initialized '%s'", pDll->szDllPath);
	return TRUE;
}

BOOL LDR_DesInitOneDll( DllInfo *pDll )
{
	if ( !pDll->pInitProc || pDll->lResult != 0 )
		return FALSE;

	int lResult = pDll->pInitProc(FALSE, NULL);
	pDll->lResult = lResult;

	if ( lResult != 0 )
	{
		LOG_Error("Deinitialization failed for '%s' (function returned %d)", pDll->szDllPath, lResult);
		return FALSE;
	}

	LOG_Info("Successfully deinitialized '%s'", pDll->szDllPath);
	return TRUE;
}

void LDR_InitAllDlls( void )
{
	for ( int i = 0; i < LDR_gNbMods; ++i )
	{
		if ( LDR_InitOneDll(LDR_gModList + i) )
			++LDR_gNbInitialized;
	}
	LOG_Info("Initialized %d (out of %d) mods", LDR_gNbInitialized, LDR_gNbMods);
}

void LDR_DesInitAllDlls( void )
{
	long lNbDesInit = 0;

	for ( int i = 0; i < LDR_gNbMods; ++i )
	{
		if ( LDR_DesInitOneDll(LDR_gModList + i) )
			++lNbDesInit;
	}
	LOG_Info("Deinitialized %d (out of %d) mods", lNbDesInit, LDR_gNbMods);
}
