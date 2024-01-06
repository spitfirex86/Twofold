#pragma once


void LOG_OpenFile( char *szFilePath );
void LOG_CloseFile( void );

void LOG_PrintFmt( char const *szFmt, ... );
void LOG_Info( char const *szFmt, ... );
void LOG_Warn( char const *szFmt, ... );
void LOG_Error( char const *szFmt, ... );
