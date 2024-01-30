#include "framework.h"
#include "log.h"


FILE * LOG_hFile = NULL;
BOOL LOG_bVerbose = FALSE;


void LOG_fn_vOpenFile( char *szFilePath )
{
	FILE *hFile = fopen(szFilePath, "w");
	LOG_hFile = hFile;
}

void LOG_fn_vCloseFile( void )
{
	fclose(LOG_hFile);
}

void LOG_PrintHF( char const *szHeader, char const *szFooter, char const *szFmt, va_list args )
{
	if ( !LOG_hFile )
		return;

	fputs(szHeader, LOG_hFile);
	vfprintf(LOG_hFile, szFmt, args);
	fputs(szFooter, LOG_hFile);
	fflush(LOG_hFile);
}

void LOG_PrintFmt( char const *szFmt, ... )
{
	if ( !LOG_hFile )
		return;

	va_list args;
	va_start(args, szFmt);

	vfprintf(LOG_hFile, szFmt, args);
	fflush(LOG_hFile);

	va_end(args);
}

void LOG_Void()
{
}

void (*LOG_InfoVerbose)( char const *szFmt, ... ) = LOG_Void;

void LOG_Info( char const *szFmt, ... )
{
	va_list args;
	va_start(args, szFmt);
	LOG_PrintHF("[Info] ", "\n", szFmt, args);
	va_end(args);
}

void LOG_Warn( char const *szFmt, ... )
{
	va_list args;
	va_start(args, szFmt);
	LOG_PrintHF("[Warn] ", "\n", szFmt, args);
	va_end(args);
}

void LOG_Error( char const *szFmt, ... )
{
	va_list args;
	va_start(args, szFmt);
	LOG_PrintHF("\n[ERROR] --> ", "\n\n", szFmt, args);
	va_end(args);
}

void LOG_fn_vSetVerbose( BOOL bVerbose )
{
	LOG_bVerbose = bVerbose;
	LOG_InfoVerbose = ((bVerbose) ? LOG_Info : LOG_Void);
}
