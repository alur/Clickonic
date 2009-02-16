#ifndef __UTILS_HEADER__
#define __UTILS_HEADER__

#pragma once

#define E_ERROR 1
#define E_WARNING 2
#define E_NOTICE 3
#define E_DEBUG 4

namespace utils
{
	int FixCoordinate (char *szCoordinate, bool isY);
	COLORREF String2Color (const char *szColor);
	bool String2Bool (const char *szBool);
	bool File_Exists (char* szFilePath);
	bool Is_Directory (const char* szPath);
	void GetFormatedTime (LPCSTR pszFormat, LPSTR pszReturn, size_t cchReturn); 
	void ErrorMessage (unsigned __int8 nLevel, LPCSTR pszFormat, ...);
	void SetEvar (LPCSTR pszGroup, LPCSTR pszEvar, LPCSTR pszFormat, ...);
	HRESULT CreateLink (LPCSTR lpszPathObj, LPCSTR lpszPathLink, LPCSTR lpszDesc);
	int MapXCoordinateToMonitor (int nNewMonitor, int nX, int nOldMonitor);
	int MapYCoordinateToMonitor (int nNewMonitor, int nY, int nOldMonitor);
};

#endif
