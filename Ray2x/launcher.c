#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <detours.h>
#include <stdio.h>


#define C_Ray2Exe "Rayman2.exe"
#define C_Twofold "Twofold.dll"


int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow )
{
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	si.cb = sizeof(si);

	BOOL bResult = DetourCreateProcessWithDllEx(
		NULL, C_Ray2Exe,
		NULL, NULL,
		FALSE, 0,
		NULL, NULL,
		&si, &pi,
		C_Twofold,
		NULL
	);

	if ( !bResult )
	{
		DWORD dwError = GetLastError();
		char szErr[256];
		sprintf(szErr, "Could not launch %s.\nError code: 0x%08x", C_Ray2Exe, dwError);
		MessageBox(NULL, szErr, "Twofold Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return 0;
}
