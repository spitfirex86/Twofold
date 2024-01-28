#include "framework.h"
#include "extra.h"


#define C_Ray2Exe "Rayman2.exe"
#define C_Twofold "Twofold.dll"


int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow )
{
	STARTUPINFO si = { 0 };
	PROCESS_INFORMATION pi = { 0 };
	si.cb = sizeof(si);

	unsigned int ulCmdLineLen = strlen(lpCmdLine);
	char *szCmdLine = NULL;

	if ( ulCmdLineLen > 0 )
	{
		szCmdLine = malloc(ulCmdLineLen + sizeof(C_Ray2Exe) + 2);
		if ( !szCmdLine )
			return 1;

		sprintf(szCmdLine, "%s %s", C_Ray2Exe, lpCmdLine);
	}

	if ( !PreLaunch() && !strstr(lpCmdLine, "-ignoreproblems") )
	{
		free(szCmdLine);
		return 1;
	}

	BOOL bResult = DetourCreateProcessWithDllEx(
		C_Ray2Exe, szCmdLine,
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
	free(szCmdLine);
	return 0;
}
