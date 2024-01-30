#pragma once


void LOG_fn_vOpenFile( char *szFilePath );
void LOG_fn_vCloseFile( void );

void LOG_PrintFmt( char const *szFmt, ... );
void LOG_Info( char const *szFmt, ... );
void LOG_Warn( char const *szFmt, ... );
void LOG_Error( char const *szFmt, ... );

extern void (*LOG_InfoVerbose)( char const *szFmt, ... );
void LOG_fn_vSetVerbose( BOOL bVerbose );
