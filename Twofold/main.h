#pragma once


extern BOOL g_bAllInit;


void InitHook( void );
void DesInitHook( void );

void HK_OnDllAttach( void );
void HK_OnDllDetach( void );
void HK_OnInit( void );
void HK_OnDesInit( void );
