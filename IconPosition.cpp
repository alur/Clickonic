#include "stdafx.h"
#include "Group.h"
#include "pidl.h"

int CALLBACK SortByNameCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK SortBySizeCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK SortByTypeCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
int CALLBACK SortByDateCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

/**************************************************************************************************
	Sorting functions called by bangs. They need to be in CGroup
**************************************************************************************************/
void CGroup::SortByName()
{
	ListView_SortItems(m_hwndListView, SortByNameCompareFunc, m_pFolder);
}

void CGroup::SortBySize()
{
	ListView_SortItems(m_hwndListView, SortBySizeCompareFunc, m_pFolder);
}

void CGroup::SortByType()
{
	ListView_SortItems(m_hwndListView, SortByTypeCompareFunc, m_pFolder);
}

void CGroup::SortByDate()
{
	ListView_SortItems(m_hwndListView, SortByDateCompareFunc, m_pFolder);
}

/**************************************************************************************************
	Sorting routines...
**************************************************************************************************/
int SortPIDLs(LPSHELLFOLDER psf, LPARAM lColumn, LPITEMIDLIST pidl1, LPITEMIDLIST pidl2)
{
	HRESULT hres = psf->CompareIDs(lColumn, pidl1, pidl2);
	return static_cast<signed short>(HRESULT_CODE(hres));
}
int CALLBACK SortByNameCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	return SortPIDLs(
		reinterpret_cast<LPSHELLFOLDER>(lParamSort), 
		0,
		reinterpret_cast<LPITEMIDLIST>(lParam1),
		reinterpret_cast<LPITEMIDLIST>(lParam2)
	);
}
int CALLBACK SortBySizeCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	return SortPIDLs(
		reinterpret_cast<LPSHELLFOLDER>(lParamSort), 
		1,
		reinterpret_cast<LPITEMIDLIST>(lParam1),
		reinterpret_cast<LPITEMIDLIST>(lParam2)
	);
}
int CALLBACK SortByTypeCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	return SortPIDLs(
		reinterpret_cast<LPSHELLFOLDER>(lParamSort), 
		2,
		reinterpret_cast<LPITEMIDLIST>(lParam1),
		reinterpret_cast<LPITEMIDLIST>(lParam2)
	);
}
int CALLBACK SortByDateCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	return SortPIDLs(
		reinterpret_cast<LPSHELLFOLDER>(lParamSort), 
		3,
		reinterpret_cast<LPITEMIDLIST>(lParam1),
		reinterpret_cast<LPITEMIDLIST>(lParam2)
	);
}

/**************************************************************************************************
	This function resets stored icon positions
**************************************************************************************************/
void CGroup::Reset(const char *szFolder, bool bDontSort)
{
	char szKeyName[MAX_PATH], szNum[10], szFile[MAX_PATH];
	HKEY hKey;
	
	if (_stricmp(szFolder, ".all") == 0)
	{ 
		if (m_bUseIconPositionFile)
		{
			remove(m_szIconPositionFile); // Just delete the file
		}
		else
		{
			StringCchPrintf(szKeyName, MAX_PATH, "%s%s", REGKEY_MODULE, m_szName);
			SHDeleteKey(HKEY_CURRENT_USER, szKeyName);
		}

		if (!bDontSort)
		{
			ListView_SortItems(m_hwndListView, SortByNameCompareFunc, m_pFolder);
		}
	}
	else if (_stricmp(szFolder, "") == 0)
	{
		if (m_bUseIconPositionFile)
		{
			StringCchCopy(m_szFolderLocation, sizeof(m_szFolderLocation), szFile);
			if (szFile[0] == m_CustomDrive)
				szFile[0] = '?';
			WritePrivateProfileSection(szFile, "", m_szIconPositionFile);
		}
		else
		{
			StringCchPrintf(szKeyName, MAX_PATH, "%s%s\\%s", REGKEY_MODULE, m_szName, m_szFolderLocation);
			if (ERROR_SUCCESS != RegOpenKeyEx( HKEY_CURRENT_USER, szKeyName, 0, KEY_ALL_ACCESS, &hKey))
			{
				return;
			}

			for (int k = 0; ; k++)
			{
				StringCchPrintf(szNum, 10, "%d", k);
				if (RegDeleteValue(hKey, szNum) != ERROR_SUCCESS)
				{
					break;
				}
			}

			RegCloseKey(hKey);
		}

		if (!bDontSort)
		{
			ListView_SortItems(m_hwndListView, SortByNameCompareFunc, m_pFolder);
		}
	}
	else
	{
		if (m_bUseIconPositionFile)
		{
			memcpy(szFile, szFolder, sizeof(szFolder));
			if (szFile[0] == m_CustomDrive)
				szFile[0] = '?';
			WritePrivateProfileSection(szFile, "", m_szIconPositionFile);
		}
		else
		{
			StringCchPrintf(szKeyName, MAX_PATH, "%s%s\\%s", REGKEY_MODULE, m_szName, szFolder);
			if (ERROR_SUCCESS != RegOpenKeyEx( HKEY_CURRENT_USER, szKeyName, 0, KEY_ALL_ACCESS, &hKey))
			{
				return;
			}

			for (int k = 0; ; k++)
			{
				StringCchPrintf(szNum, 10, "%d", k);
				if (RegDeleteValue(hKey, szNum) != ERROR_SUCCESS)
				{
					break;
				}
			}
			RegCloseKey(hKey);
		}

		if (_stricmp(szFolder, m_szFolderLocation) == 0 && !bDontSort)
		{
			ListView_SortItems(m_hwndListView, SortByNameCompareFunc, m_pFolder);
		}
	}
}

/**************************************************************************************************
	This function saves the icons current position.
**************************************************************************************************/
void CGroup::SaveState()
{
	HKEY hKey;
	DWORD dwDisposition;
	PBYTE pBuf;
	POINT pt;
	LPITEMIDLIST pidl;
	int nCount, nSize, npidlSize;
	char szKeyName[MAX_PATH], szNum[10], szFileName[MAX_PATH];

	if (strcmp(m_szIconPositionFile, ".dontsave") == 0)
		return;

	if (m_bUseIconPositionFile)
		return SaveToFile();

	StringCchPrintf(szKeyName, MAX_PATH, "%s%s\\%s", REGKEY_MODULE, m_szName, m_szFolderLocation);

	Reset("", true);

	if (ERROR_SUCCESS != RegCreateKeyEx( HKEY_CURRENT_USER, szKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition))
		return;

	// save a string (x,y,pidl) for each item
	nCount = ListView_GetItemCount(m_hwndListView);
	for (int i = 0; i < nCount; i++)
	{
		ListView_GetItemText(m_hwndListView, i, 0, szFileName, MAX_PATH);
		if (!IconShouldBeHidden(szFileName))
		{
			pidl = GetPIDLFromId(i);
			ListView_GetItemPosition(m_hwndListView, i, &pt);
			npidlSize = PIDL::GetSize(pidl);
			nSize = 2 * sizeof(ULONG) + npidlSize;
			pBuf = reinterpret_cast<BYTE*>(HeapAlloc(GetProcessHeap(),0, nSize));
			((ULONG*)pBuf)[0] = pt.x;
			((ULONG*)pBuf)[1] = pt.y;
			CopyMemory(pBuf + 2 * sizeof(ULONG), pidl, npidlSize);
			StringCchPrintf(szNum, 10, "%d", i);
			RegSetValueEx(hKey, szNum, 0, REG_BINARY, pBuf, nSize);
			HeapFree(GetProcessHeap(), 0, pBuf);
		}
	}

	RegCloseKey(hKey);

	if (m_bPersistantFolderLocation)
	{
		ZeroMemory(&szKeyName, sizeof(szKeyName));
		StringCchPrintf(szKeyName, MAX_PATH, "%s%s", REGKEY_MODULE, m_szName);
		if (ERROR_SUCCESS != RegCreateKeyEx( HKEY_CURRENT_USER, szKeyName, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition))
			return;
		pBuf = reinterpret_cast<BYTE*>(HeapAlloc(GetProcessHeap(), 0, MAX_PATH));
		CopyMemory(pBuf, m_szFolderLocation, MAX_PATH);
		RegSetValueEx(hKey, "CurrentFolder", 0, REG_SZ, pBuf, MAX_PATH);
		HeapFree(GetProcessHeap(), 0, pBuf);
		RegCloseKey(hKey);
	}
}

/**************************************************************************************************
	This function saves the icons positions into a file. It's called by SaveState()
**************************************************************************************************/
void CGroup::SaveToFile()
{
	POINT pt;
	char szFileData[MAX_LINE_LENGTH*2] = {0}, szFileName[MAX_PATH], szPoint[20], szFile[MAX_PATH];
	LPSTR pszPos = szFileData;
	int nCount = ListView_GetItemCount(m_hwndListView);

	if (!(nCount > 0))
		return;

	for (int i = 0; i < nCount; i++)
	{
		ListView_GetItemText(m_hwndListView, i, 0, szFileName, MAX_PATH);
		GetNameFromId(i, szFileName, MAX_PATH);
		if (szFileName[0] == '[')
			szFileName[0] = '>';
		if (!IconShouldBeHidden(szFileName))
		{
			StringCchCopy(pszPos, szFileData+sizeof(szFileData)-pszPos, szFileName);
			pszPos += strlen(szFileName);
			ListView_GetItemPosition(m_hwndListView, i, &pt);
			StringCchPrintf(szPoint, 20, "=?%d?%d", pt.x, pt.y);
			StringCchCopy(pszPos, szFileData+sizeof(szFileData)-pszPos, szPoint);
			pszPos += strlen(szPoint)+1;
		}
	}
	StringCchCopy(szFile, sizeof(szFile), m_szFolderLocation);
	if (szFile[0] == m_CustomDrive)
		szFile[0] = '?';

	WritePrivateProfileSection(szFile, szFileData, m_szIconPositionFile);

	if (m_bPersistantFolderLocation)
	{
		ZeroMemory(&szFileData, sizeof(szFileData));
		StringCchPrintf(szFileData, sizeof(szFileData), "CurrentFolder=%s", m_szFolderLocation);
		WritePrivateProfileSection("Settings", szFileData, m_szIconPositionFile);
	}
}

/**************************************************************************************************
	This function restores saved icon positions
**************************************************************************************************/
void CGroup::RestoreState()
{
	HKEY hKey;
	BYTE *pBuf;
	POINT pt;
	LPITEMIDLIST pidl, pidlSearch;
	int nCount;
	char szKeyName[MAX_PATH], szNum[10];
	DWORD dwSize;

	if (m_bUseIconPositionFile)
		return RestoreFromFile();

	// open the registry key
	StringCchPrintf(szKeyName, MAX_PATH, "%s%s\\%s", REGKEY_MODULE, m_szName, m_szFolderLocation);

	if (ERROR_SUCCESS != RegOpenKeyEx( HKEY_CURRENT_USER, szKeyName, 0, KEY_ALL_ACCESS, &hKey))
		return;

	// don't snap icons to grid
	ListView_SetExtendedListViewStyle(m_hwndListView, 0);

	// for each record in the registry key...
	for (int k = 0; ; k++)
	{
		StringCchPrintf(szNum, 10, "%d", k);
		if (RegQueryValueEx(hKey, szNum, NULL, NULL, NULL, &dwSize) != ERROR_SUCCESS)
			break;

		pBuf = reinterpret_cast<BYTE*>(HeapAlloc(GetProcessHeap(),0, dwSize));

		if (RegQueryValueEx(hKey, szNum, NULL, NULL, pBuf, &dwSize) != ERROR_SUCCESS)
			break;

		pt.x = ((ULONG*)pBuf)[0];
		pt.y = ((ULONG*)pBuf)[1];

		pidl = (LPITEMIDLIST)(pBuf + 2 * sizeof(ULONG));

		// try to find this pidl in the current FolderView

		nCount = ListView_GetItemCount(m_hwndListView);
		for (int i = 0; i < nCount; i++)
		{
			pidlSearch = GetPIDLFromId(i);
			if (pidlSearch != NULL && m_pFolder->CompareIDs(0, pidl, pidlSearch) == 0)
			{
				ListView_SetItemPosition(m_hwndListView, i, pt.x, pt.y);
				break;
			}
		}
		HeapFree(GetProcessHeap(), 0, pBuf);
	}

	RegCloseKey(hKey);
	DWORD dListStyleFlags = LVS_EX_TRANSPARENTBKGND | LVS_EX_LABELTIP;
	if (!m_bNoInfoTips)
		dListStyleFlags |= LVS_EX_INFOTIP;
	if (m_bHotTracking)
		dListStyleFlags |= LVS_EX_TRACKSELECT;
	if (m_bSnapToGrid)
		dListStyleFlags |= LVS_EX_SNAPTOGRID;
	ListView_SetExtendedListViewStyle(m_hwndListView, dListStyleFlags);

	if (m_bSnapToGrid)
		ListView_Arrange(m_hwndListView, LVA_SNAPTOGRID);
}

/**************************************************************************************************
	This function restores saved icons from a file. It's called by RestoreState()
**************************************************************************************************/
void CGroup::RestoreFromFile()
{
	char szStoredPos[MAX_LINE_LENGTH], * pString, * pSub, *next;
	char szFileName[MAX_PATH], szTempName[MAX_PATH], szFile[MAX_PATH];
	POINT pt;
	size_t pos = 0;
	int nCount = ListView_GetItemCount(m_hwndListView);

	StringCchCopy(szFile, sizeof(szFile), m_szFolderLocation);
	if (szFile[0] == m_CustomDrive)
		szFile[0] = '?';

	GetPrivateProfileSection(szFile, szStoredPos, MAX_LINE_LENGTH, m_szIconPositionFile);

	ListView_SetExtendedListViewStyle(m_hwndListView, 0);

	pString = szStoredPos + pos;
	while(pString[0] != 0)
	{
		// This will get the file name
		pSub = strtok_s(pString, "?", &next);
		StringCchCopy(szFileName, sizeof(szFileName), pSub);
		szFileName[strlen(szFileName)-1] = 0;
		if (szFileName[0] == '>')
			szFileName[0] = '[';
		
		// This will get the x coordinate
		pos += (strlen(pString)+1);
		pString = szStoredPos + pos;
		pSub = strtok_s(pString, "?", &next);
		pt.x = atol(pSub);

		// This will get the y coordinate
		pos += (strlen(pString)+1);
		pString = szStoredPos + pos;
		pSub = strtok_s(pString, "?", &next);
		pt.y = atol(pSub);

		for (int i = 0; i < nCount; i++)
		{
			GetNameFromId(i, szTempName, MAX_PATH);
			if (_stricmp(szTempName, szFileName) == 0)
			{
				ListView_SetItemPosition(m_hwndListView, i, pt.x, pt.y);
				break;
			}
		}
		pos += (strlen(pString)+1);
		pString = szStoredPos + pos;
	}
	DWORD dListStyleFlags = LVS_EX_TRANSPARENTBKGND | LVS_EX_LABELTIP;
	if (!m_bNoInfoTips)
		dListStyleFlags |= LVS_EX_INFOTIP;
	if (m_bHotTracking)
		dListStyleFlags |= LVS_EX_TRACKSELECT;
	if (m_bSnapToGrid)
		dListStyleFlags |= LVS_EX_SNAPTOGRID;
	ListView_SetExtendedListViewStyle(m_hwndListView, dListStyleFlags);
	if (m_bSnapToGrid)
		ListView_Arrange(m_hwndListView, LVA_SNAPTOGRID);
}

