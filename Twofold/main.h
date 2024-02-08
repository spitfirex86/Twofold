#pragma once


typedef struct TwofoldHeader
{
	unsigned int ulHeader;
	unsigned char ucVersionA;
	unsigned char ucVersionB;
}
TwofoldHeader;


/* main.c */

extern BOOL g_bAllInit;

extern TwofoldHeader const g_stHeaderConst;


void fn_vInitHook( void );
void fn_vDesInitHook( void );


/* hook.c */

void HK_fn_vOnDllAttach( void );
void HK_fn_vOnDllDetach( void );
void HK_fn_vOnInit( void );
void HK_fn_vOnDesInit( void );


/* config.c */

extern char const *g_szCfgFile;
extern char const *g_szLogFile;
extern char const *g_szLoadOrderFile;

extern char CFG_g_szModsDir[MAX_PATH];
extern char CFG_g_szDefaultCmdLine[256];


BOOL CFG_fn_bDoesFileExist( char const *szFile );
void CFG_fn_vWriteDefaultConfig( void );
void CFG_fn_vReadConfig( void );
