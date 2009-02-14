#include "stdafx.h"
#include "lsapiex.h"

////////////////////////////////////////////////////////////////////////////
//
// Read prefixed string option ([szPrefix][szOption]) from .RC
// 
void LiteStep::GetPrefixedRCString(char *szDest, const char *szPrefix, const char *szOption, const char *szDefault)
{
	char szOptionName[MAX_LINE_LENGTH];
	StringCchPrintf(szOptionName, MAX_LINE_LENGTH, "%s%s", szPrefix, szOption);
	GetRCString(szOptionName, szDest, szDefault, MAX_LINE_LENGTH);
}


////////////////////////////////////////////////////////////////////////////
//
// Read prefixed line option ([szPrefix][szOption]) from .RC
// 
void LiteStep::GetPrefixedRCLine(char *szDest, const char *szPrefix, const char *szOption, const char *szDefault)
{
	char szOptionName[MAX_LINE_LENGTH];
	StringCchPrintf(szOptionName, MAX_LINE_LENGTH, "%s%s", szPrefix, szOption);
	GetRCLine(szOptionName, szDest, MAX_LINE_LENGTH, szDefault);
	PathUnquoteSpaces(szDest);
}


////////////////////////////////////////////////////////////////////////////
//
// Read prefixed BOOL option ([szPrefix][szOption]) from .RC
// 
bool LiteStep::GetPrefixedRCBool(const char *szPrefix, const char *szOption, bool bDefault)
{
	char szOptionName[MAX_LINE_LENGTH];
	StringCchPrintf(szOptionName, MAX_LINE_LENGTH, "%s%s", szPrefix, szOption);
	return GetRCBoolDef(szOptionName, bDefault) != FALSE;
}


////////////////////////////////////////////////////////////////////////////
//
// Read prefixed INT option ([szPrefix][szOption]) from .RC
// 
int  LiteStep::GetPrefixedRCInt (const char *szPrefix, const char *szOption, int nDefault)
{
	char szOptionName[MAX_LINE_LENGTH];
	StringCchPrintf(szOptionName, MAX_LINE_LENGTH, "%s%s", szPrefix, szOption);
	return GetRCInt(szOptionName, nDefault);
}

////////////////////////////////////////////////////////////////////////////
//
// Read prefixed COORDINATE option ([szPrefix][szOption]) from .RC
// 
int  LiteStep::GetPrefixedRCCoordinate(const char *szPrefix, const char *szOption, int nDefault, int nMax)
{
	char szOptionName[MAX_LINE_LENGTH];
	StringCchPrintf(szOptionName, MAX_LINE_LENGTH, "%s%s", szPrefix, szOption);
	return GetRCCoordinate(szOptionName, nDefault, nMax);
}

////////////////////////////////////////////////////////////////////////////
//
// Read prefixed COORDINATE option ([szPrefix][szOption]) from .RC
// 
COLORREF LiteStep::GetPrefixedRCColor (const char *szPrefix, const char *szOption, COLORREF crDefault)
{
	char szOptionName[MAX_LINE_LENGTH];
	StringCchPrintf(szOptionName, MAX_LINE_LENGTH, "%s%s", szPrefix, szOption);
	return GetRCColor(szOptionName, crDefault);
}

////////////////////////////////////////////////////////////////////////////
//
// Execute bang command
// 
void LiteStep::ExecuteCommand(const char *szCommand)
{
	if (szCommand && szCommand[0] && _stricmp(szCommand, "!none") != 0)
	{
		LSExecute(NULL, szCommand, NULL);
	}
}
