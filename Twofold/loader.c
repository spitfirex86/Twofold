#include "framework.h"
#include "loader.h"
#include "main.h"
#include "log.h"


#define LDR_C_InitProc "ModMain"

typedef int (*td_pfn_lInitProc)( BOOL bInit, void *reserved );

typedef struct tdstDllInfo
{
	HMODULE hDll;
	td_pfn_lInitProc pInitProc;

	int lResult;
	DWORD dwLastError;

	ACP_tdxBool bInLoadOrder;
	ACP_tdxBool bDisabled;
	ACP_tdxBool bLoaded;
	ACP_tdxBool bInitialized;

	unsigned short uwNameOffset;
	char szDllPathRel[MAX_PATH];
}
tdstDllInfo;


tdstDllInfo *LDR_g_dModsList = NULL;
long LDR_g_lNbMods = 0;
long LDR_g_lNbLoaded = 0;
long LDR_g_lNbInitialized = 0;


#define M_szGetDllName( pDll ) (char const *)((pDll)->szDllPathRel + (pDll)->uwNameOffset)

tdstDllInfo * fn_p_stAllocInfo( void )
{
	size_t newSize = (LDR_g_lNbMods + 1) * sizeof(tdstDllInfo);
	tdstDllInfo *pNewList = realloc(LDR_g_dModsList, newSize);

	if ( !pNewList )
		return NULL; /* TODO: this should probably exit the whole process */

	LDR_g_dModsList = pNewList;
	long idx = LDR_g_lNbMods++;

	tdstDllInfo *pInfo = pNewList + idx;
	*pInfo = (tdstDllInfo){ 0 };
	return pInfo;
}

tdstDllInfo * fn_p_stFindInfo( char const *szName )
{
	for ( int i = 0; i < LDR_g_lNbMods; i++ )
	{
		tdstDllInfo *pInfo = LDR_g_dModsList + i;
		if ( !_stricmp(szName, M_szGetDllName(pInfo)) )
			return pInfo;
	}
	return NULL;
}

void fn_vUpdateNameOffset( tdstDllInfo *pDll )
{
	char *szPath = pDll->szDllPathRel;
	char *pLastPart = strrchr(szPath, '\\');
	
	pDll->uwNameOffset = ( pLastPart )
		? (pLastPart - szPath) + 1
		: 0;
}

#define M_vSkipSpace( pCh )		\
{								\
	while ( isspace(*(pCh)) )	\
		(pCh)++;				\
}

void fn_vCreateLoadOrderFile( char const *szName )
{
	FILE *hFile = fopen(g_szLoadOrderFile, "w");
	if ( hFile )
	{
		fputs("; Twofold load order file\n\n", hFile);
		fputs("; One filename per line (no quotes), relative to the mods directory.\n", hFile);
		fputs("; Mods are loaded and initialized from top to bottom.\n", hFile);
		fputs("; To disable a mod, add a '-' before the filename.\n", hFile);

		fclose(hFile);
	}
}

void fn_vReadLoadOrderFile( FILE *hFile )
{
	char szBuffer[512];
	char *pCh;
	int lLine = 0;
	unsigned int ulMaxFileName = MAX_PATH - strlen(g_szLoadOrderFile) - 2;

	while ( (pCh = fgets(szBuffer, sizeof(szBuffer), hFile)) != NULL )
	{
		ACP_tdxBool bDisabled = FALSE;
		lLine++;

		M_vSkipSpace(pCh);
		if ( *pCh == ';' )
			continue;
		if ( !*pCh )
			continue;

		if ( *pCh == '-' )
		{
			bDisabled = TRUE;
			pCh++;
			M_vSkipSpace(pCh);
		}

		char *pBegin = pCh;
		char *pEnd = strchr(pBegin, 0);

		while ( pEnd > pBegin && isspace(pEnd[-1]) )
			pEnd--;

		if ( pBegin == pEnd )
		{
			LOG_Error("'%s': Expected a filename in line %d", g_szLoadOrderFile, lLine);
			continue;
		}

		unsigned int ulLength = pEnd - pBegin;
		if ( ulLength > ulMaxFileName )
		{
			LOG_Error("'%s': Filename too long", g_szLoadOrderFile);
			continue;
		}

		tdstDllInfo *pDll = fn_p_stAllocInfo();
		strncpy(pDll->szDllPathRel, pBegin, ulLength);
		pDll->szDllPathRel[ulLength] = 0;
		fn_vUpdateNameOffset(pDll);
		pDll->bInLoadOrder = TRUE;
		pDll->bDisabled = bDisabled;
	}
}

void fn_vUpdateLoadOrderFile( FILE *hFile )
{
	BOOL bGotHeader = FALSE;

	for ( int i = 0; i < LDR_g_lNbMods; ++i )
	{
		tdstDllInfo *pDll = LDR_g_dModsList + i;
		if ( pDll->bInLoadOrder )
			continue;

		if ( !bGotHeader )
		{
			time_t t = time(NULL);
			char szDate[32];
			strftime(szDate, sizeof(szDate), "%F", localtime(&t));
			fprintf(hFile, "\n; Added on %s\n", szDate);
			bGotHeader = TRUE;
		}

		fprintf(hFile, "%s%s\n",
			(pDll->bDisabled) ? "- " : "",
			pDll->szDllPathRel
		);
		pDll->bInLoadOrder = TRUE;
	}
}

BOOL LDR_fn_bReadLoadOrder( void )
{
	char const *szModsDir = CFG_g_szModsDir;
	CreateDirectory(szModsDir, NULL);

	FILE *hFile;
	BOOL bUpdateOrder = TRUE;

	if ( CFG_fn_bDoesFileExist(g_szLoadOrderFile) )
	{
		hFile = fopen(g_szLoadOrderFile, "r");
		if ( hFile )
		{
			fn_vReadLoadOrderFile(hFile);
			fclose(hFile);
		}
		else
		{
			LOG_Error("Could not open '%s' for reading", g_szLoadOrderFile);
			bUpdateOrder = FALSE;
		}
		LOG_Info("Got %d DLLs from '%s'", LDR_g_lNbMods, g_szLoadOrderFile);
	}
	else
	{
		LOG_Warn("Load order file missing - creating '%s", g_szLoadOrderFile);
		fn_vCreateLoadOrderFile(g_szLoadOrderFile);
	}

	int lNewMods = 0;
	char szSearchPath[MAX_PATH];
	sprintf(szSearchPath, "%s\\%s", szModsDir, "*.dll");

	WIN32_FIND_DATA ffd = { 0 };
	HANDLE hFind = FindFirstFile(szSearchPath, &ffd);
	if ( hFind != INVALID_HANDLE_VALUE )
	{
		do
		{
			tdstDllInfo *pDll = fn_p_stFindInfo(ffd.cFileName);
			if ( !pDll )
			{
				pDll = fn_p_stAllocInfo();
				sprintf(pDll->szDllPathRel, "%s", ffd.cFileName);
				fn_vUpdateNameOffset(pDll);
				lNewMods++;
			}
		}
		while ( FindNextFile(hFind, &ffd) );
		FindClose(hFind);
	}

	if ( lNewMods )
		LOG_Info("Found %d DLLs in '%s'", lNewMods, szModsDir);

	if ( bUpdateOrder )
	{
		hFile = fopen(g_szLoadOrderFile, "a");
		if ( hFile )
		{
			fn_vUpdateLoadOrderFile(hFile);
			fclose(hFile);
		}
		else
			LOG_Error("Could not open '%s' for writing", g_szLoadOrderFile);
	}

	return TRUE;
}

void LDR_fn_vFreeLoadOrder( void )
{
	free(LDR_g_dModsList);
	LDR_g_dModsList = NULL;
	LDR_g_lNbMods = 0;
}

BOOL LDR_fn_bLoadOneDll( tdstDllInfo *pDll )
{
	if ( pDll->bDisabled )
	{
		LOG_InfoVerbose(__FUNCTION__" : Skipping '%s' - disabled", M_szGetDllName(pDll));
		return FALSE;
	}
	if ( pDll->bLoaded )
		return FALSE;

	char szPath[MAX_PATH];
	sprintf(szPath, "%s\\%s", CFG_g_szModsDir, pDll->szDllPathRel);

	HMODULE hDll = LoadLibrary(szPath);
	pDll->hDll = hDll;
	if ( !hDll )
	{
		pDll->dwLastError = GetLastError();
		LOG_Error("LoadLibrary failed for '%s' (0x%08x)", pDll->szDllPathRel, pDll->dwLastError);
		return FALSE;
	}

	td_pfn_lInitProc pInitProc = (td_pfn_lInitProc)GetProcAddress(hDll, LDR_C_InitProc);
	pDll->pInitProc = pInitProc;
	if ( !pInitProc )
	{
		pDll->dwLastError = GetLastError();
		LOG_Warn(
			"%s export missing in '%s', possibly a legacy mod or helper library? (0x%08x)",
			LDR_C_InitProc, M_szGetDllName(pDll), pDll->dwLastError
		);
	}

	pDll->bLoaded = TRUE;
	LOG_Info("Successfully loaded '%s' (HMODULE: %p)", M_szGetDllName(pDll), pDll->hDll);
	return TRUE;
}

BOOL LDR_fn_bUnLoadOneDll( tdstDllInfo *pDll )
{
	if ( !pDll->bLoaded || !pDll->hDll )
		return FALSE;

	if ( !FreeLibrary(pDll->hDll) )
	{
		pDll->dwLastError = GetLastError();
		LOG_Error("FreeLibrary failed for '%s' (0x%08x)", pDll->szDllPathRel, pDll->dwLastError);
		return FALSE;
	}
	pDll->hDll = NULL;
	pDll->pInitProc = NULL;
	pDll->bLoaded = FALSE;

	LOG_Info("Requested for '%s' to be unloaded", M_szGetDllName(pDll));
	return TRUE;
}

void LDR_fn_vLoadAllDlls( void )
{
	for ( int i = 0; i < LDR_g_lNbMods; ++i )
	{
		if ( LDR_fn_bLoadOneDll(LDR_g_dModsList + i) )
			++LDR_g_lNbLoaded;
	}
	LOG_Info("Loaded %d (out of %d) mods", LDR_g_lNbLoaded, LDR_g_lNbMods);
}

void LDR_fn_vUnLoadAllDlls( void )
{
	long lNbUnLoaded = 0;
	for ( int i = 0; i < LDR_g_lNbMods; ++i )
	{
		if ( LDR_fn_bUnLoadOneDll(LDR_g_dModsList + i) )
			++lNbUnLoaded;
	}
	LOG_Info("Unloaded %d (out of %d) mods", lNbUnLoaded, LDR_g_lNbMods);
}

BOOL LDR_fn_bInitOneDll( tdstDllInfo *pDll )
{
	if ( pDll->bDisabled )
	{
		LOG_InfoVerbose(__FUNCTION__" : Skipping '%s' - disabled", M_szGetDllName(pDll));
		return FALSE;
	}
	if ( pDll->bInitialized || !pDll->pInitProc )
		return FALSE;

	int lResult = pDll->pInitProc(TRUE, NULL);
	pDll->lResult = lResult;

	if ( lResult != 0 )
	{
		LOG_Error("Initialization failed for '%s' (function returned %d)", M_szGetDllName(pDll), lResult);
		return FALSE;
	}
	
	pDll->bInitialized = TRUE;
	LOG_Info("Successfully initialized '%s'", M_szGetDllName(pDll));
	return TRUE;
}

BOOL LDR_fn_bDesInitOneDll( tdstDllInfo *pDll )
{
	if ( !pDll->bInitialized || !pDll->pInitProc )
		return FALSE;

	int lResult = pDll->pInitProc(FALSE, NULL);
	pDll->lResult = lResult;

	if ( lResult != 0 )
	{
		LOG_Error("Deinitialization failed for '%s' (function returned %d)", M_szGetDllName(pDll), lResult);
		return FALSE;
	}

	pDll->bInitialized = FALSE;
	LOG_Info("Successfully deinitialized '%s'", M_szGetDllName(pDll));
	return TRUE;
}

void LDR_fn_vInitAllDlls( void )
{
	for ( int i = 0; i < LDR_g_lNbMods; ++i )
	{
		if ( LDR_fn_bInitOneDll(LDR_g_dModsList + i) )
			++LDR_g_lNbInitialized;
	}
	LOG_Info("Initialized %d (out of %d) mods", LDR_g_lNbInitialized, LDR_g_lNbMods);
}

void LDR_fn_vDesInitAllDlls( void )
{
	long lNbDesInit = 0;

	for ( int i = 0; i < LDR_g_lNbMods; ++i )
	{
		if ( LDR_fn_bDesInitOneDll(LDR_g_dModsList + i) )
			++lNbDesInit;
	}
	LOG_Info("Deinitialized %d (out of %d) mods", lNbDesInit, LDR_g_lNbMods);
}
