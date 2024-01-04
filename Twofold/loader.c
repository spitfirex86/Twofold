#include "framework.h"
#include "loader.h"


char **LDR_gModList = NULL;
long LDR_gNbMods = 0;

BOOL LDR_ReadLoadOrder( void )
{
	char szBuffer[MAX_PATH];

	// TODO

	return FALSE;
}

void LDR_FreeLoadOrder( void )
{
	free(LDR_gModList);
	LDR_gModList = NULL;
	LDR_gNbMods = 0;
}
