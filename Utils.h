#ifndef __UTILS_HEADER__
#define __UTILS_HEADER__

#pragma once

#define E_ERROR 1
#define E_WARNING 2
#define E_NOTICE 3
#define E_DEBUG 4

namespace utils
{
	int DcParseCoordinate (char *szCoordinate, bool isY);
	COLORREF String2Color (const char *szColor);
	bool String2Bool (const char *szBool);
	bool File_Exists (char* szFilePath);
	bool Is_Directory (const char* szPath);
	void GetFormatedTime (LPCSTR pszFormat, LPSTR pszReturn, size_t cchReturn); 
	void ErrorMessage (unsigned __int8 nLevel, LPCSTR pszFormat, ...);
	void SetEvar (LPCSTR pszGroup, LPCSTR pszEvar, LPCSTR pszFormat, ...);
	HRESULT CreateLink (LPCSTR lpszPathObj, LPCSTR lpszPathLink, LPCSTR lpszDesc);

	// Coordinate systems:
	//  Sc: Screen coordinates
	//    The one true coordinate system, in which the primary monitor has its
	//    origin at (0,0) and other monitors may have origins at negative
	//    coordinates.
	//  Dc: Desk coordinates
	//    The coordinate system of a desktop window that spans all monitors. The
	//    origin of this coordinate system is the top left of the virtual screen,
	//    which may be negative in screen coordinates, and may not actually be
	//    visible.
	//  Mc: Monitor coordinates
	//    The coordinate system of a particular monitor. For "monitor 0" (the
	//    virtual monitor), this is the same as desk coordinates. For monitor
	//    1, this is the same as screen coordinates, but with different limits.
	//    For all other monitors, this is different from either one.
	//  Gc: Group coordinates
	//    The coordinate system of an icon group (CGroup). The origin is at the
	//    group's top left corner.
	POINTL ScFromDc (POINTL point);
	POINTL DcFromSc (POINTL point);
	POINTL ScFromMc (POINTL point, int monitor);
	POINTL McFromSc (POINTL point, int monitor);
	POINTL ScFromGc (POINTL point, const CGroup *group);
	POINTL GcFromSc (POINTL point, const CGroup *group);
};

#endif
