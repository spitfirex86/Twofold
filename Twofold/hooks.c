#include "framework.h"
#include "main.h"
#include "log.h"


char g_szVersionTxt[] = "/o200:Twofold v1.0";

JFFTXT_tdstString g_stVersionTxt = {
	(char *)&g_szVersionTxt,
	5.0f, 980.0f, 7.0f, 160
};

BOOL g_bInMainMenu = FALSE;


void fn_vChooseTheGoodDesInit( void )
{
	if ( GAM_g_stEngineStructure->eEngineMode == E_EM_ModeStoppingProgram )
		fn_vDesInitHook();

	GAM_fn_vChooseTheGoodDesInit();
}

void fn_vInitGameLoop( void )
{
	HWND hWnd = GAM_fn_hGetWindowHandle();
	HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(1));
	SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
	LOG_InfoVerbose("fn_vInitGameLoop : applied icon patch");

	GAM_fn_vInitGameLoop();
}

void fn_vNeutralizePTC_SuperObj( HIE_tdstSuperObject *hSuperObj )
{
	HIE_tdstStandardGame *pStdGame = HIE_M_hSuperObjectGetStdGame(hSuperObj);
	pStdGame->ulCustomBits |= Std_C_CustBit_NoAI;
	pStdGame->ulSaveCustomBits |= Std_C_CustBit_NoAI;
	pStdGame->ucMiscFlags |= Std_C_MiscFlag_DesactivateAtAll;
	pStdGame->ucMiscFlags &= ~Std_C_MiscFlag_Activable;
	pStdGame->ucSaveMiscFlags |= Std_C_MiscFlag_DesactivateAtAll;
	pStdGame->ucSaveMiscFlags &= ~Std_C_MiscFlag_Activable;
	pStdGame->eInitFlagWhenDeadOrTaken = Std_C_OI_NeverBackWhenTaken;
	pStdGame->eInitFlagWhenOutOfZone = Std_C_OI_NeverBackWhenTaken;
}

void fn_vNeutralizePTC( void )
{
	HIE_tdxObjectType lType1 = HIE_fn_lFindModelTypeByName("DS1_GEN_PTC_GenCKS");
	HIE_tdxObjectType lType2 = HIE_fn_lFindModelTypeByName("DS1_GEN_PTC_GenBigFile");
	LOG_InfoVerbose("PTC : model types: DS1_GEN_PTC_GenCKS = %d, DS1_GEN_PTC_GenBigFile = %d", lType1, lType2);

	if ( lType1 == Std_C_InvalidObjectType || lType2 == Std_C_InvalidObjectType )
		return;

	HIE_tdstSuperObject *hSuperObj;
	LST_M_DynamicForEach(*GAM_g_p_stDynamicWorld, hSuperObj)
	{
		if ( HIE_M_bSuperObjectIsActor(hSuperObj)
			&& ( HIE_M_hSuperObjectGetStdGame(hSuperObj)->lObjectModelType == lType1
				|| HIE_M_hSuperObjectGetStdGame(hSuperObj)->lObjectModelType == lType2 ) )
		{
			fn_vNeutralizePTC_SuperObj(hSuperObj);
			LOG_InfoVerbose("PTC : Found and neutralized '%s' (%p)", HIE_fn_szGetObjectPersonalName(hSuperObj), hSuperObj);
		}
	}
	LST_M_DynamicForEach(*GAM_g_p_stInactiveDynamicWorld, hSuperObj)
	{
		if ( HIE_M_bSuperObjectIsActor(hSuperObj)
			&& ( HIE_M_hSuperObjectGetStdGame(hSuperObj)->lObjectModelType == lType1
				|| HIE_M_hSuperObjectGetStdGame(hSuperObj)->lObjectModelType == lType2 ) )
		{
			fn_vNeutralizePTC_SuperObj(hSuperObj);
			LOG_InfoVerbose("PTC : Found and neutralized '%s' (%p) (inactive)", HIE_fn_szGetObjectPersonalName(hSuperObj), hSuperObj);
		}
	}
}

void fn_vInitLevelLoop( void )
{
	char const *szLevelName = GAM_fn_p_szGetLevelName();
	g_bInMainMenu = !_stricmp(szLevelName, "menu");
	LOG_InfoVerbose("fn_vInitLevelLoop : current level is '%s'", szLevelName);

	GAM_fn_vInitLevelLoop();

	fn_vNeutralizePTC();
}

void fn_vAffiche( void *pContext )
{
	if ( *AI_g_bInGameMenu || g_bInMainMenu )
		JFFTXT_vDrawString(pContext, &g_stVersionTxt);

	JFFTXT_vAffiche(pContext);
}


void HK_fn_vOnInit( void )
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	DetourAttach((PVOID *)&GAM_fn_vChooseTheGoodDesInit, (PVOID)fn_vChooseTheGoodDesInit);
	DetourAttach((PVOID *)&GAM_fn_vInitGameLoop, (PVOID)fn_vInitGameLoop);
	DetourAttach((PVOID *)&GAM_fn_vInitLevelLoop, (PVOID)fn_vInitLevelLoop);
	DetourAttach((PVOID *)&JFFTXT_vAffiche, (PVOID)fn_vAffiche);

	DetourTransactionCommit();
}

void HK_fn_vOnDesInit( void )
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	DetourDetach((PVOID *)&GAM_fn_vChooseTheGoodDesInit, (PVOID)fn_vChooseTheGoodDesInit);
	DetourDetach((PVOID *)&GAM_fn_vInitGameLoop, (PVOID)fn_vInitGameLoop);
	DetourDetach((PVOID *)&GAM_fn_vInitLevelLoop, (PVOID)fn_vInitLevelLoop);
	DetourDetach((PVOID *)&JFFTXT_vAffiche, (PVOID)fn_vAffiche);

	DetourTransactionCommit();
}


void HK_fn_vOnDllAttach( void )
{
	/* early hook init */
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((PVOID *)&GAM_fn_vInitEngineWhenInitApplication, (PVOID)fn_vInitHook);
	DetourTransactionCommit();
}

void HK_fn_vOnDllDetach( void )
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach((PVOID *)&GAM_fn_vInitEngineWhenInitApplication, (PVOID)fn_vInitHook);
	DetourTransactionCommit();
}
