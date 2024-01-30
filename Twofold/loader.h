#pragma once


extern long LDR_g_lNbMods;
extern long LDR_g_lNbLoaded;
extern long LDR_g_lNbInitialized;


BOOL LDR_fn_bReadLoadOrder( char const *szModDir );
void LDR_fn_vFreeLoadOrder( void );

void LDR_fn_vLoadAllDlls( void );
void LDR_fn_vUnLoadAllDlls( void );
void LDR_fn_vInitAllDlls( void );
void LDR_fn_vDesInitAllDlls( void );
