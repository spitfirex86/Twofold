#include "framework.h"
#include "main.h"
#include "log.h"


char g_szVersionTxt[] = "/o200:Twofold v1.0";

JFFTXT_tdstString g_stVersionTxt = {
	&g_szVersionTxt,
	5.0f, 980.0f, 7.0f, 160
};

BOOL g_bInMainMenu = FALSE;


void fn_vChooseTheGoodDesInit( void )
{
	if ( GAM_g_stEngineStructure->eEngineMode == E_EM_ModeStoppingProgram )
		DesInitHook();

	GAM_fn_vChooseTheGoodDesInit();
}

void fn_vInitLevelLoop( void )
{
	g_bInMainMenu = !_stricmp(GAM_fn_p_szGetLevelName(), "menu");

	GAM_fn_vInitLevelLoop();
}

void fn_vAffiche( void *pContext )
{
	if ( *AI_g_bInGameMenu || g_bInMainMenu )
		JFFTXT_vDrawString(pContext, &g_stVersionTxt);

	JFFTXT_vAffiche(pContext);
}


void HK_OnInit( void )
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	DetourAttach((PVOID *)&GAM_fn_vChooseTheGoodDesInit, (PVOID)fn_vChooseTheGoodDesInit);
	DetourAttach((PVOID *)&GAM_fn_vInitLevelLoop, (PVOID)fn_vInitLevelLoop);
	DetourAttach((PVOID *)&JFFTXT_vAffiche, (PVOID)fn_vAffiche);

	DetourTransactionCommit();
}

void HK_OnDesInit( void )
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());

	DetourDetach((PVOID *)&GAM_fn_vChooseTheGoodDesInit, (PVOID)fn_vChooseTheGoodDesInit);
	DetourDetach((PVOID *)&GAM_fn_vInitLevelLoop, (PVOID)fn_vInitLevelLoop);
	DetourDetach((PVOID *)&JFFTXT_vAffiche, (PVOID)fn_vAffiche);

	DetourTransactionCommit();
}


void HK_OnDllAttach( void )
{
	/* early hook init */
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((PVOID *)&GAM_fn_vInitEngineWhenInitApplication, (PVOID)InitHook);
	DetourTransactionCommit();
}

void HK_OnDllDetach( void )
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach((PVOID *)&GAM_fn_vInitEngineWhenInitApplication, (PVOID)InitHook);
	DetourTransactionCommit();
}
