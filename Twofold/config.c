#include "framework.h"
#include "log.h"


char const *g_szCfgFile = ".\\Twofold.ini";
char const *g_szLogFile = ".\\TwofoldLog.txt";
char const *g_szLoadOrderFile = ".\\LoadOrder.cfg";

char CFG_g_szModsDir[MAX_PATH] = ".\\Mods";
char CFG_g_szDefaultCmdLine[256] = "";


BOOL CFG_fn_bDoesFileExist( char const *szFile )
{
	DWORD dwAttrib = GetFileAttributes(szFile);
	return ( (dwAttrib != INVALID_FILE_ATTRIBUTES) && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY) );
}

void CFG_fn_vWriteDefaultConfig( void )
{
	WritePrivateProfileString("TwofoldLoader", "ModsDirectory", ".\\Mods", g_szCfgFile);
	WritePrivateProfileString("TwofoldLoader", "DefaultCmdLine", "", g_szCfgFile);
}

void CFG_fn_vReadConfig( void )
{
	GetPrivateProfileString("TwofoldLoader", "ModsDirectory", ".\\Mods", CFG_g_szModsDir, MAX_PATH, g_szCfgFile);
	GetPrivateProfileString("TwofoldLoader", "DefaultCmdLine", NULL, CFG_g_szDefaultCmdLine, 256, g_szCfgFile);
}
