#include "framework.h"
#include "loader.h"

//long (CALLBACK *GAM_fn_WndProc)( HANDLE hWnd, unsigned int uMsg, unsigned int wParam, long lParam ) = (void*)0x4022D0;
//int (*R2_CreateGameWindow)( HINSTANCE hInstance, int nShowCmd ) = (void *)0x402020;
void (*R2_fn_vInitEngineWhenInitApplication)( void ) = (void *)0x401000;

void InitHook( void )
{
	R2_fn_vInitEngineWhenInitApplication();
	MessageBox(NULL, "Twofold says hello", "Twofold", MB_OK);
}


BOOL APIENTRY DllMain( HMODULE hModule, DWORD dwReason, LPVOID lpReserved )
{
	if ( DetourIsHelperProcess() )
		return TRUE;

	switch ( dwReason )
	{
		case DLL_PROCESS_ATTACH:
			DetourRestoreAfterWith();
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach((PVOID *)&R2_fn_vInitEngineWhenInitApplication, (PVOID)InitHook);
			DetourTransactionCommit();
			break;

		case DLL_PROCESS_DETACH:
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourDetach((PVOID *)&R2_fn_vInitEngineWhenInitApplication, (PVOID)InitHook);
			DetourTransactionCommit();
			break;

		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
			break;
	}

	return TRUE;
}
