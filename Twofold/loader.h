#pragma once


BOOL LDR_ReadLoadOrder( char const *szModDir );
void LDR_FreeLoadOrder( void );

void LDR_LoadAllDlls( void );
void LDR_UnLoadAllDlls( void );
void LDR_InitAllDlls( void );
void LDR_DesInitAllDlls( void );

extern long LDR_gNbMods;
extern long LDR_gNbLoaded;
extern long LDR_gNbInitialized;
