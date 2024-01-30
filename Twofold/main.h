#pragma once


extern BOOL g_bAllInit;


void fn_vInitHook( void );
void fn_vDesInitHook( void );

void HK_fn_vOnDllAttach( void );
void HK_fn_vOnDllDetach( void );
void HK_fn_vOnInit( void );
void HK_fn_vOnDesInit( void );
