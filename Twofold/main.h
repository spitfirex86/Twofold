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
