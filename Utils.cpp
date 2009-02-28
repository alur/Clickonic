#include "stdafx.h"
#include "utils.h"
#include "time.h"
#include "group.h"

void utils::ErrorMessage(unsigned __int8 nLevel, LPCSTR pszFormat, ...)
{
	char szError[MAX_LINE_LENGTH] = { 0 };
	va_list argList;

	va_start(argList, pszFormat);
	StringCchVPrintfA(szError, MAX_LINE_LENGTH, pszFormat, argList);
	va_end(argList);

	LSLog(nLevel, "Clickonic", szError); // We always log errors

	if (g_nErrorTranslucency >= nLevel)
	{
		switch (nLevel)
		{
		case E_ERROR:
			MessageBoxA(NULL, szError, "Clickonic: Error", MB_OK);
			break;
		case E_WARNING:
			MessageBoxA(NULL, szError, "Clickonic: Warning", MB_OK);
			break;
		case E_NOTICE:
			MessageBoxA(NULL, szError, "Clickonic: Notice", MB_OK);
			break;
		case E_DEBUG:
			MessageBoxA(NULL, szError, "Clickonic: Debug", MB_OK);
			break;
		}
	}
}

void utils::GetFormatedTime (LPCSTR pszFormat, LPSTR pszReturn, size_t cchReturn)
{
	time_t CurrentTime = time(NULL);
	tm TimeInfo;
	localtime_s(&TimeInfo, &CurrentTime);
	strftime(pszReturn, cchReturn, pszFormat, &TimeInfo);
}

void utils::SetEvar(LPCSTR pszGroup, LPCSTR pszEvar, LPCSTR pszFormat, ...)
{
	char szValue[MAX_LINE_LENGTH] = { 0 };
	char szEvar[MAX_RCCOMMAND] = { 0 };
	va_list argList;

	va_start(argList, pszFormat);
	StringCchVPrintfA(szValue, MAX_LINE_LENGTH, pszFormat, argList);
	va_end(argList);

	StringCchPrintfA(szEvar, sizeof(szEvar), "%s%s", pszGroup, pszEvar);

	LSSetVariable(szEvar, szValue);
}

int utils::DcParseCoordinate(char *szCoordinate, bool isY)
{
	if (szCoordinate[1] != '@')
	{
		int ScPos = atoi(szCoordinate);
		int DcPos = ScPos - GetSystemMetrics(SM_XVIRTUALSCREEN);
		return DcPos;
	}
	else
	{
		int iMon = szCoordinate[0] - '0';
		int McPos = atoi(szCoordinate + 2);
		int DcPos = McPos + (isY?g_Monitors[iMon].Top:g_Monitors[iMon].Left);
		return DcPos;
	}
}

COLORREF utils::String2Color(const char *szColor)
{
	COLORREF crTextColor = strtol(szColor, NULL, 16); // convert from BGR to RGB
	return RGB(GetBValue(crTextColor), GetGValue(crTextColor), GetRValue(crTextColor));
}

bool utils::String2Bool(const char *szBool)
{
	return (lstrcmpi(szBool, "off") && lstrcmpi(szBool, "false") && lstrcmpi(szBool, "no"));
}

bool utils::File_Exists (char* szFilePath)
{
	return GetFileAttributes(szFilePath) != INVALID_FILE_ATTRIBUTES;
}

bool utils::Is_Directory (const char *szPath)
{
	DWORD dwResult = GetFileAttributes(szPath);
	if (dwResult == INVALID_FILE_ATTRIBUTES)
		return false;
	if (dwResult & FILE_ATTRIBUTE_DIRECTORY)
		return true;
	return false;
}

HRESULT utils::CreateLink(LPCSTR lpszPathObj, LPCSTR lpszPathLink, LPCSTR lpszDesc)
{
	HRESULT hres; 
	IShellLink* psl; 

	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *) &psl); 
	if (SUCCEEDED(hres))
	{
		IPersistFile* ppf; 
		psl->SetPath(lpszPathObj); 
		psl->SetDescription(lpszDesc);

		hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf); 
		if (SUCCEEDED(hres))
		{
			WCHAR wsz[MAX_PATH]; 
			MultiByteToWideChar(CP_ACP, 0, lpszPathLink, -1, wsz, MAX_PATH);  
			hres = ppf->Save(wsz, TRUE); 
			ppf->Release(); 
		}
		psl->Release(); 
	}
	return hres; 
} 


POINTL utils::ScFromDc(POINTL point)
{
	POINTL ret = {
		point.x + GetSystemMetrics(SM_XVIRTUALSCREEN),
		point.y + GetSystemMetrics(SM_YVIRTUALSCREEN)
		};
	return ret;
}
POINTL utils::DcFromSc(POINTL point)
{
	POINTL ret = {
		point.x - GetSystemMetrics(SM_XVIRTUALSCREEN),
		point.y - GetSystemMetrics(SM_YVIRTUALSCREEN)
		};
	return ret;
}

POINTL utils::ScFromMc(POINTL point, int monitor)
{
	POINTL ret = {
		point.x + g_Monitors[monitor].Left,
		point.y + g_Monitors[monitor].Top
		};
	return ret;
}
POINTL utils::McFromSc(POINTL point, int monitor)
{
	POINTL ret = {
		point.x - g_Monitors[monitor].Left,
		point.y - g_Monitors[monitor].Top
		};
	return ret;
}

POINTL utils::ScFromGc(POINTL point, const CGroup *group)
{
	POINTL ret = {
		point.x + (group->m_DcX + GetSystemMetrics(SM_XVIRTUALSCREEN)),
		point.y + (group->m_DcY + GetSystemMetrics(SM_YVIRTUALSCREEN))
		};
	return ret;
}
POINTL utils::GcFromSc(POINTL point, const CGroup *group)
{
	POINTL ret = {
		point.x - (group->m_DcX + GetSystemMetrics(SM_XVIRTUALSCREEN)),
		point.y - (group->m_DcY + GetSystemMetrics(SM_YVIRTUALSCREEN))
		};
	return ret;
}
