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


char LDR_g_szModDir[MAX_PATH] = { 0 };

DllInfo *LDR_g_dModList = NULL;
long LDR_g_lNbMods = 0;
long LDR_g_lNbLoaded = 0;
long LDR_g_lNbInitialized = 0;


DllInfo * LDR_fn_p_stAllocInfo( void )
{
	size_t newSize = (LDR_g_lNbMods + 1) * sizeof(DllInfo);
	DllInfo *pNewList = realloc(LDR_g_dModList, newSize);

	if ( !pNewList )
		return NULL; /* TODO: this should probably exit the whole process */

	LDR_g_dModList = pNewList;
	long idx = LDR_g_lNbMods++;

	DllInfo *pInfo = pNewList + idx;
	*pInfo = (DllInfo){ 0 };
	return pInfo;
}

BOOL LDR_fn_bReadLoadOrder( char const *szModDir )
{
	strcpy(LDR_g_szModDir, szModDir);
	CreateDirectory(LDR_g_szModDir, NULL);

	/* TODO: actual load order */

	char szSearchPath[MAX_PATH];
	sprintf(szSearchPath, "%s\\%s", LDR_g_szModDir, "*.dll");

	WIN32_FIND_DATA ffd = { 0 };
	HANDLE hFind = FindFirstFile(szSearchPath, &ffd);
	if ( hFind != INVALID_HANDLE_VALUE )
	{
		do
		{
			DllInfo *pDll = LDR_fn_p_stAllocInfo();
			sprintf(pDll->szDllPath, "%s\\%s", LDR_g_szModDir, ffd.cFileName);
		}
		while ( FindNextFile(hFind, &ffd) );
		FindClose(hFind);
	}

	LOG_Info("Found %d DLLs in '%s'", LDR_g_lNbMods, LDR_g_szModDir);
	return TRUE;
}

void LDR_fn_vFreeLoadOrder( void )
{
	free(LDR_g_dModList);
	LDR_g_dModList = NULL;
	LDR_g_lNbMods = 0;
}

BOOL LDR_fn_bLoadOneDll( DllInfo *pDll )
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

BOOL LDR_fn_bUnLoadOneDll( DllInfo *pDll )
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

void LDR_fn_vLoadAllDlls( void )
{
	for ( int i = 0; i < LDR_g_lNbMods; ++i )
	{
		if ( LDR_fn_bLoadOneDll(LDR_g_dModList + i) )
			++LDR_g_lNbLoaded;
	}
	LOG_Info("Loaded %d (out of %d) mods", LDR_g_lNbLoaded, LDR_g_lNbMods);
}

void LDR_fn_vUnLoadAllDlls( void )
{
	long lNbUnLoaded = 0;
	for ( int i = 0; i < LDR_g_lNbMods; ++i )
	{
		if ( LDR_fn_bUnLoadOneDll(LDR_g_dModList + i) )
			++lNbUnLoaded;
	}
	LOG_Info("Unloaded %d (out of %d) mods", lNbUnLoaded, LDR_g_lNbMods);
}

BOOL LDR_fn_bInitOneDll( DllInfo *pDll )
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

BOOL LDR_fn_bDesInitOneDll( DllInfo *pDll )
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

void LDR_fn_vInitAllDlls( void )
{
	for ( int i = 0; i < LDR_g_lNbMods; ++i )
	{
		if ( LDR_fn_bInitOneDll(LDR_g_dModList + i) )
			++LDR_g_lNbInitialized;
	}
	LOG_Info("Initialized %d (out of %d) mods", LDR_g_lNbInitialized, LDR_g_lNbMods);
}

void LDR_fn_vDesInitAllDlls( void )
{
	long lNbDesInit = 0;

	for ( int i = 0; i < LDR_g_lNbMods; ++i )
	{
		if ( LDR_fn_bDesInitOneDll(LDR_g_dModList + i) )
			++lNbDesInit;
	}
	LOG_Info("Deinitialized %d (out of %d) mods", lNbDesInit, LDR_g_lNbMods);
}
