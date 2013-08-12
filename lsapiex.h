#pragma once

namespace LiteStep {
	void GetPrefixedRCString(char *szDest, size_t cchDest, const char *szPrefix, const char *szOption, const char *szDefault);
	void GetPrefixedRCLine	(char *szDest, size_t cchDest, const char *szPrefix, const char *szOption, const char *szDefault);
	bool GetPrefixedRCBool	(const char *szPrefix, const char *szOption, bool bDefault);
	int  GetPrefixedRCInt	(const char *szPrefix, const char *szOption, int  nDefault);
	int  GetPrefixedRCCoordinate(const char *szPrefix, const char *szOption, int nDefault, int nMax);
	COLORREF GetPrefixedRCColor (const char *szPrefix, const char *szOption, COLORREF crDefault);
	void ExecuteCommand(const char *szCommand);
}