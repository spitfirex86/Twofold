#include "framework.h"
#include "extra.h"


BOOL GetModifyTimeOfFile( char const *szFileName, FILETIME *pstTimeOut )
{
	FILETIME stFileTime;
	HANDLE hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	if ( hFile == INVALID_HANDLE_VALUE )
		return FALSE;

	if ( !GetFileTime(hFile, NULL, NULL, &stFileTime) )
	{
		CloseHandle(hFile);
		return FALSE;
	}
	CloseHandle(hFile);

	*pstTimeOut = stFileTime;
	return TRUE;
}

int FixAcpDllLocation( void )
{
	char const *szAcpDll = ".\\ACP_Ray2x.dll";
	char const *szAcpDllWrong = ".\\Mods\\ACP_Ray2x.dll";

	DWORD dwAttrib = GetFileAttributes(szAcpDll);
	BOOL bAcpExists = (dwAttrib != INVALID_FILE_ATTRIBUTES) && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);

	dwAttrib = GetFileAttributes(szAcpDllWrong);
	BOOL bAcpInWrongDir = (dwAttrib != INVALID_FILE_ATTRIBUTES) && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY);

	if ( !bAcpInWrongDir ) /* no need to fix */
		return 0;

	if ( bAcpExists ) /* two copies of the dll */
	{
		FILETIME stFileTime, stFileTimeWrong;

		if ( !GetModifyTimeOfFile(szAcpDll, &stFileTime) || !GetModifyTimeOfFile(szAcpDllWrong, &stFileTimeWrong) )
			return -1;

		if ( CompareFileTime(&stFileTime, &stFileTimeWrong) >= 0 ) /* correct copy is newer */
			return ( DeleteFile(szAcpDllWrong) ? 0 : 3 );

		/* copy in the wrong location is newer -> delete the old one, then move */
		if ( !DeleteFile(szAcpDll) )
			return 2;
	}
	/* dll in the wrong dir -> move */
	return ( MoveFileEx(szAcpDllWrong, szAcpDll, MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH) ? 0 : 1 );
}


BOOL PreLaunch( void )
{
	char szBuffer[64];

	/* disable Ray2Fix modloader, if enabled */
	GetPrivateProfileString("Ray2Fix", "Tweaks", "0", szBuffer, sizeof(szBuffer), ".\\Ubi.ini");
	if ( strtol(szBuffer, NULL, 0) > 0 )
		WritePrivateProfileString("Ray2Fix", "Tweaks", "0", ".\\Ubi.ini");

	/* check and fix the location of ACP_Ray2x.dll */
	int lResult = FixAcpDllLocation();
	if ( lResult != 0 )
	{
		char szMsg[512] =
			"The location of the ACP_Ray2x.dll library is incorrect. An attempt to fix this automatically was unsuccessful.\n"
			"To fix this issue:\n\n";

		char *szMsg2;
		switch ( lResult )
		{
			case 3:
				szMsg2 = "Multiple copies of the library were found, delete 'Mods\\ACP_Ray2x.dll' and relaunch Twofold.";
				break;
			case 2:
				szMsg2 = "Delete 'ACP_Ray2x.dll' located in the Rayman 2 directory and replace it with the copy found at 'Mods\\ACP_Ray2x.dll'.";
				break;
			case 1:
				szMsg2 = "Move 'Mods\\ACP_Ray2x.dll' into the main Rayman 2 directory and relaunch Twofold.";
				break;
			default:
				szMsg2 = "Unknown error.\nMake sure there is only one copy of ACP_Ray2x.dll and that it is located in the main Rayman 2 directory (not in the Mods folder).";
				break;
		}

		strcat(szMsg, szMsg2);
		MessageBox(NULL, szMsg, "Twofold Error", MB_OK | MB_ICONWARNING);
		return FALSE;
	}

	return TRUE;
}
