#include "stdafx.h"
#include "Group.h"
#include "utils.h"
#include "pidl.h"
#include "lsapiex.h"
#include "DropTarget.h"
using namespace utils;

/**************************************************************************************************
	This is the constructor, it's called when the group is created.
**************************************************************************************************/
CGroup::CGroup(const char *szName)
{
	IShellBrowserImpl();

	// Some defaults
	m_bInitialized = false;
	m_pDropTarget = NULL;
	m_nLastItem = -1;
	m_nBackFolderCount = 0;
	m_nForwardFolderCount = 0;
	m_wpOrigListViewProc = NULL;
	m_NextIconPosition.x = -1;
	m_bInFolderChange = false;

	StringCchCopy(m_szName, MAX_PATH, szName); // Store the group name
	
	if (!ReadSettings(false)) // Read settings from step.rc
	{
		return;
	}

	m_hWnd = FindWindow("DesktopBackgroundClass", NULL);

	if (!m_hWnd)
	{
		StringCchCopy(m_szInitError, sizeof(m_szInitError), "Unable to find desktop window. Most likely your desktop module isn't loaded.");
		return;
	}

	if (!InitFolderView()) // Set up the listview
	{
		return;
	}

	m_hwndNextClipViewer = SetClipboardViewer(m_hwndView); // Register for clipboard notifications

	m_bInitialized = true; // Let the main code know that the group loaded properly.
}

/**************************************************************************************************
	This is the destructor, it's called when the module is unloaded.
**************************************************************************************************/
CGroup::~CGroup()
{
	SaveState(); // Save icon positions
	
	m_bInitialized = false; // Prevent onremove events from fireing.

	ChangeClipboardChain(m_hwndListView, m_hwndNextClipViewer); // Unregister from the windows clipboard chain

	if ( m_wpOrigListViewProc != NULL )
        SetWindowLongPtr(m_hwndListView, GWLP_WNDPROC, (LONG) m_wpOrigListViewProc); // Remove the subclass from the listview. 

	if (m_pDropTarget)
	{
		m_pDropTarget->Revoke(m_hwndView);
		delete m_pDropTarget;
	}
	if (m_pView2)
		m_pView2->Release();

	if (m_pView)
	{
		m_pView->DestroyViewWindow();
		m_pView->Release();
	}
	if (m_pFolder)
		m_pFolder->Release();

	if (m_hFont)
		DeleteObject(m_hFont);
}

/**************************************************************************************************
	This function reads all the settings from step.rc and sets all the defaults.
**************************************************************************************************/
bool CGroup::ReadSettings(bool bIsRefresh)
{
	char szBuf[MAX_LINE_LENGTH];

	// We store these for simple access
	SHGetSpecialFolderPath(NULL, m_szMyDocumentsPath, CSIDL_PERSONAL, false);
	SHGetSpecialFolderPath(NULL, m_szDesktopPath, CSIDL_DESKTOP, 0);

	m_bPersistantFolderLocation = LiteStep::GetPrefixedRCBool(m_szName, "PersistantFolderLocation", false);

	m_bExplicitCalls = LiteStep::GetPrefixedRCBool(m_szName, "ExplicitCalls", false);

	// TODO:This needs to be os dependant...
	StringCchCopy(m_szRegDeskIconsPath, MAX_PATH, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel");

	// ViewMode
	LiteStep::GetPrefixedRCString(szBuf, m_szName, "ViewMode", "icons");
	if (strcmp(szBuf, "tiles") == 0)
		m_uViewMode = FVM_TILE;
	else if (strcmp(szBuf, "thumbnails") == 0)
		m_uViewMode = FVM_THUMBNAIL;
	else if (strcmp(szBuf, "details") == 0)
		m_uViewMode = FVM_DETAILS;
	else if (strcmp(szBuf, "smallicons") == 0)
		m_uViewMode = FVM_SMALLICON;
	else
		m_uViewMode = FVM_ICON;

	LiteStep::GetPrefixedRCString(m_szTextPasteFormat, m_szName, "TextPasteFormat", "%A, %B %d.txt");

	m_bShortcutMode = LiteStep::GetPrefixedRCBool(m_szName, "ShortcutMode", FALSE);

	// Desktop icon settings
	m_bDontUseRegistryIcons = LiteStep::GetPrefixedRCBool(m_szName, "DontUseRegistryIcons", FALSE);

	// Icon position saving
	m_bUseIconPositionFile = true;
	LiteStep::GetPrefixedRCLine(m_szIconPositionFile, m_szName, "IconPositionFile", ".registry");
	if (_stricmp(m_szIconPositionFile, ".registry") == 0)
		m_bUseIconPositionFile = false;
	LiteStep::GetPrefixedRCString(szBuf, m_szName, "CustomDrive", "-");
	if (_stricmp(szBuf, ".current") == 0)
	{
		char lpCurrentDirectory[MAX_PATH];
		GetCurrentDirectory( MAX_PATH, lpCurrentDirectory );
		m_CustomDrive = lpCurrentDirectory[0];
	}
	else 
		m_CustomDrive = szBuf[0];

	// Init events
	LoadEvents();

	// Positions
	m_nMonitor	= LiteStep::GetPrefixedRCInt(m_szName, "Monitor", 0);
	
	POINTL McPos = { LiteStep::GetPrefixedRCInt(m_szName, "X", 0),
	                 LiteStep::GetPrefixedRCInt(m_szName, "Y", 0) };
	POINTL DcPos = DcFromSc(ScFromMc(McPos, m_nMonitor));
	m_DcX = DcPos.x;
	m_DcY = DcPos.y;
	
	m_nWidth	= LiteStep::GetPrefixedRCInt(m_szName, "Width", g_Monitors[m_nMonitor].ResX);
	m_nHeight	= LiteStep::GetPrefixedRCInt(m_szName, "Height", g_Monitors[m_nMonitor].ResY);
	m_nSpacingX	= LiteStep::GetPrefixedRCInt(m_szName, "SpacingX",  -1);
	m_nSpacingY	= LiteStep::GetPrefixedRCInt(m_szName, "SpacingY", -1);
	
	m_bHidden	= LiteStep::GetPrefixedRCBool(m_szName, "StartHidden", FALSE);
	utils::SetEvar(m_szName, "IsHidden", m_bHidden ? "true" : "false");

	m_bNoScroll	= LiteStep::GetPrefixedRCBool(m_szName, "NoScroll", FALSE);

	m_bDontPasteText = LiteStep::GetPrefixedRCBool(m_szName, "DontPasteText", FALSE);
	m_bDontPasteImages = LiteStep::GetPrefixedRCBool(m_szName, "DontPasteImages", FALSE);
	m_bDontPasteAudio = LiteStep::GetPrefixedRCBool(m_szName, "DontPasteAudio", FALSE);

	m_bDontUseRecycleBin	= LiteStep::GetPrefixedRCBool(m_szName, "DontUseRecycleBin", FALSE);
	m_bConfirmFileDelete	= LiteStep::GetPrefixedRCBool(m_szName, "ConfirmFileDelete", FALSE);
	m_bSimpleProgress		= LiteStep::GetPrefixedRCBool(m_szName, "SimpleProgress", FALSE);
	m_bNoConnectedFiles		= LiteStep::GetPrefixedRCBool(m_szName, "NoConnectedFiles", FALSE);
	m_bVirtualFileDelete	= LiteStep::GetPrefixedRCBool(m_szName, "VirtualFileDelete", FALSE);

	m_bExplorerDesktop = LiteStep::GetPrefixedRCBool(m_szName, "ExplorerDesktop", FALSE);

	m_wSelectionRectangleMod = GetModkeyWord("RectangleModifierKey", ".none");
	m_wSystemMenuMod = GetModkeyWord("SystemMenuModifierKey", ".none");

	m_bStealFocus	= LiteStep::GetPrefixedRCBool(m_szName, "StealFocus", FALSE);

	m_bNoInfoTips		= LiteStep::GetPrefixedRCBool(m_szName, "NoInfoTips", FALSE);
	m_bSnapToGrid		= LiteStep::GetPrefixedRCBool(m_szName, "SnapToGrid", TRUE);
	m_bHotTracking		= LiteStep::GetPrefixedRCBool(m_szName, "HotTracking", FALSE);
	m_nHotTrackingTime	= LiteStep::GetPrefixedRCInt(m_szName, "HotTrackingTime", -1);

	m_bInlineBrowsing = LiteStep::GetPrefixedRCBool(m_szName, "InlineBrowsing", FALSE);
	m_bNoVirtualSwitch = LiteStep::GetPrefixedRCBool(m_szName, "NoVirtualSwitch", FALSE);

	// Background settings
	LiteStep::GetPrefixedRCString(szBuf, m_szName, "Background", "");
	StringCchCopy(m_szBackground, MAX_PATH, szBuf);
	if (m_szBackground && m_szBackground[0])
	{
		m_bTiled = LiteStep::GetPrefixedRCBool(m_szName, "Tiled", false);
		m_nOffsetPercentX = LiteStep::GetPrefixedRCInt(m_szName, "OffsetPercentX", 0);
		m_nOffsetPercentY = LiteStep::GetPrefixedRCInt(m_szName, "OffsetPercentY", 0);
	}
	m_crBackgroundColor= LiteStep::GetPrefixedRCColor(m_szName, "BackgroundColor", CLR_NONE);

	m_bAutoArrange = LiteStep::GetPrefixedRCBool(m_szName, "AutoArrange", FALSE);
	m_bAlignTop = LiteStep::GetPrefixedRCBool(m_szName, "AlignTop", FALSE);
	m_bAlwaysShowSelection =  LiteStep::GetPrefixedRCBool(m_szName, "AlwaysShowSelection", FALSE);

	// Grab the PIDL for the folder
	if (!bIsRefresh)
	{
		LiteStep::GetPrefixedRCLine(szBuf, m_szName, "Folder", ".desktop");

		if (m_bPersistantFolderLocation)
		{
			GetStoredFolderLocation(szBuf);
		}

		if (_stricmp(szBuf, ".desktop") == 0)
		{
			StringCchCopy(m_szFolderLocation, MAX_PATH, m_szDesktopPath);
			SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &m_pidlFolder);
			utils::SetEvar(m_szName, "CurrentFolder", ".desktop");
		}
		else if (_stricmp(szBuf, ".mycomputer") == 0)
		{
			SHGetSpecialFolderPath(NULL, m_szFolderLocation, CSIDL_DRIVES, 0);
			SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &m_pidlFolder);
			utils::SetEvar(m_szName, "CurrentFolder", ".mycomputer");
		}
		else
		{
			StringCchCopy(m_szFolderLocation, MAX_PATH, szBuf);
			m_pidlFolder = PIDL::ParseDisplayName(szBuf);
			utils::SetEvar(m_szName, "CurrentFolder", "%s", m_szFolderLocation);
		}

		if (!m_pidlFolder)
		{
			StringCchCopy(m_szInitError, sizeof(m_szInitError), "Invalid desktop folder.");
			return false;
		}
	}

	// Font Settings
	m_crTextColor = LiteStep::GetPrefixedRCColor(m_szName, "FontColor", RGB(255, 255, 255));
	m_crTextBackgroundColor = LiteStep::GetPrefixedRCColor(m_szName, "FontBackgroundColor", CLR_NONE);
	int nFontQuality;
	LiteStep::GetPrefixedRCString(szBuf, m_szName, "FontQuality", "default");
	if (_stricmp(szBuf, "antialiased") == 0)
		nFontQuality = ANTIALIASED_QUALITY;
	else if (_stricmp(szBuf, "cleartype") == 0)
		nFontQuality = CLEARTYPE_QUALITY;
	else if (_stricmp(szBuf, "draft") == 0)
		nFontQuality = DRAFT_QUALITY;
	else if (_stricmp(szBuf, "nonantialiased") == 0)
		nFontQuality = NONANTIALIASED_QUALITY;
	else if (_stricmp(szBuf, "proof") == 0)
		nFontQuality = PROOF_QUALITY;
	else 
		nFontQuality = DEFAULT_QUALITY;

	LiteStep::GetPrefixedRCLine(szBuf, m_szName, "Font", "Arial");

	m_hFont = CreateFont(
		LiteStep::GetPrefixedRCInt(m_szName, "FontSize", 14), 0, 0, 0,
		LiteStep::GetPrefixedRCInt(m_szName, "FontWeight", LiteStep::GetPrefixedRCBool(m_szName, "FontBold", false) ? FW_BOLD : FW_NORMAL),
		LiteStep::GetPrefixedRCBool(m_szName, "FontItalic", false),
		LiteStep::GetPrefixedRCBool(m_szName, "FontUnderline", false),
		LiteStep::GetPrefixedRCBool(m_szName, "FontStrikeout", false),
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		nFontQuality, DEFAULT_PITCH | FF_DONTCARE, szBuf);
	if (!m_hFont)
		utils::ErrorMessage(E_WARNING, "Could not load font %s", szBuf);

	return true;
}

/**************************************************************************************************
	This function returns the saved folder location or refrains from modify the string if no
	folder location has been saved.
**************************************************************************************************/
void CGroup::GetStoredFolderLocation(LPSTR pszLocation)
{
	if (m_bUseIconPositionFile)
	{
		GetPrivateProfileString("Settings", "CurrentFolder", pszLocation, pszLocation, MAX_PATH, m_szIconPositionFile);
	}
	else
	{
		char szKeyName[MAX_PATH], szRet[MAX_PATH];
		StringCchPrintf(szKeyName, MAX_PATH, "%s%s", REGKEY_MODULE, m_szName);
		DWORD dwRegType = REG_SZ, dwRegSize = MAX_PATH;

		if (ERROR_SUCCESS == SHGetValue(HKEY_CURRENT_USER, szKeyName, "CurrentFolder", &dwRegType, szRet, &dwRegSize))
		{
			StringCchCopy(pszLocation, MAX_PATH, szRet);
		}
	}
	if (!m_bNoVirtualSwitch && !_stricmp(pszLocation, m_szDesktopPath))
	{
		StringCchCopy(pszLocation, MAX_PATH, ".desktop");
	}
}

/**************************************************************************************************
	This function should hide the items we don't want to display on the desktop. It's called
	when an item is inserted on the desktop. iItem is the id of the inserted item. This function
	returns true if the item was added and false if it was removed.
**************************************************************************************************/
bool CGroup::HandleInsItem(int iItem)
{
	char szFileName[MAX_PATH];

	if (!GetNameFromId(iItem, szFileName, sizeof(szFileName)))
		return true;

	if (IconShouldBeHidden(szFileName))
	{
		ListView_DeleteItem(m_hwndListView, iItem);
		return false;
	}

	if (m_NextIconPosition.x != -1) {
		ListView_SetItemPosition(m_hwndListView, iItem, m_NextIconPosition.x, m_NextIconPosition.y);
		m_NextIconPosition.x = -1;
	}

	return true;
}

/**************************************************************************************************
	This function scans all desktop icons and hides the ones that should be hidden.
**************************************************************************************************/
void CGroup::ScanIcons()
{
	char szFileName[MAX_PATH];
	int nCount = ListView_GetItemCount(m_hwndListView);

	for (int i = 0; i < nCount; i++)
	{
		if (GetNameFromId(i, szFileName, sizeof(szFileName)))
		{
			if (IconShouldBeHidden(szFileName))
			{
				ListView_DeleteItem(m_hwndListView, i); // Delete the item
				nCount--; i--; // the list reorders itself after every deletion
			}
		}
	}
}

/**************************************************************************************************
	This function determines if an icon should be hidden or not.
**************************************************************************************************/
bool CGroup::IconShouldBeHidden(const char *szFileName)
{
	IconMap::iterator iter = m_HiddenIcons.find(szFileName);
	if (iter != m_HiddenIcons.end())
		return true;

	return false;
}

/**************************************************************************************************
	This function keeps the registry and the icon hiding variables updated.
**************************************************************************************************/
void CGroup::CheckVirtualIcon(LPCTSTR pszFileName, bool bDeleted)
{
	if (m_bDontUseRegistryIcons)
		return; // No reason to bother with this.

	if (!m_bInitialized)
		return; // These settings are never changed by clickonic during initialization.

	if (strcmp(m_szFolderLocation, m_szDesktopPath) != 0)
		return; // No reason to bother for non desktop icons.
	
	DWORD dwBuffer, dwSize = sizeof(DWORD), dwType = REG_DWORD, dwRegSetting = bDeleted ? 1 : 0;

	if (!strcmp(pszFileName, m_szMyDocumentsPath))
	{
		// My Documents
		SHGetValue(HKEY_CURRENT_USER, m_szRegDeskIconsPath, "{450D8FBA-AD25-11D0-98A8-0800361B1103}", &dwType, &dwBuffer, &dwSize);
		if (dwBuffer != dwRegSetting)
		{
			SHSetValue(HKEY_CURRENT_USER, m_szRegDeskIconsPath, "{450D8FBA-AD25-11D0-98A8-0800361B1103}", REG_DWORD, &dwRegSetting, sizeof(dwRegSetting));
			SendNotifyMessage(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Windows");
		}
	}
	else if (!strcmp(pszFileName, "::{871C5380-42A0-1069-A2EA-08002B30309D}"))
	{
		// Internet Explorer
		SHGetValue(HKEY_CURRENT_USER, m_szRegDeskIconsPath, "{871C5380-42A0-1069-A2EA-08002B30309D}", &dwType, &dwBuffer, &dwSize);
		if (dwBuffer != dwRegSetting)
		{
			SHSetValue(HKEY_CURRENT_USER, m_szRegDeskIconsPath, "{871C5380-42A0-1069-A2EA-08002B30309D}", REG_DWORD, &dwRegSetting, sizeof(dwRegSetting));
			SendNotifyMessage(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Windows");
		}
	}
	else if (!strcmp(pszFileName, "::{208D2C60-3AEA-1069-A2D7-08002B30309D}"))
	{
		// My Network Places
		SHGetValue(HKEY_CURRENT_USER, m_szRegDeskIconsPath, "{208D2C60-3AEA-1069-A2D7-08002B30309D}", &dwType, &dwBuffer, &dwSize);
		if (dwBuffer != dwRegSetting)
		{
			SHSetValue(HKEY_CURRENT_USER, m_szRegDeskIconsPath, "{208D2C60-3AEA-1069-A2D7-08002B30309D}", REG_DWORD, &dwRegSetting, sizeof(dwRegSetting));
			SendNotifyMessage(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Windows");
		}
	}
	else if (!strcmp(pszFileName, "::{20D04FE0-3AEA-1069-A2D8-08002B30309D}"))
	{
		// My Computer
		SHGetValue(HKEY_CURRENT_USER, m_szRegDeskIconsPath, "{20D04FE0-3AEA-1069-A2D8-08002B30309D}", &dwType, &dwBuffer, &dwSize);
		if (dwBuffer != dwRegSetting)
		{
			SHSetValue(HKEY_CURRENT_USER, m_szRegDeskIconsPath, "{20D04FE0-3AEA-1069-A2D8-08002B30309D}", REG_DWORD, &dwRegSetting, sizeof(dwRegSetting));
			SendNotifyMessage(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Windows");
		}
	}
	else if (!strcmp(pszFileName, "::{645FF040-5081-101B-9F08-00AA002F954E}"))
	{
		// Recycle Bin
		SHGetValue(HKEY_CURRENT_USER, m_szRegDeskIconsPath, "{645FF040-5081-101B-9F08-00AA002F954E}", &dwType, &dwBuffer, &dwSize);
		if (dwBuffer != dwRegSetting)
		{
			SHSetValue(HKEY_CURRENT_USER, m_szRegDeskIconsPath, "{645FF040-5081-101B-9F08-00AA002F954E}", REG_DWORD, &dwRegSetting, sizeof(dwRegSetting));
			SendNotifyMessage(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Windows");
		}
	}
	else if (strcmp(pszFileName, "::{26EE0668-A00A-44D7-9371-BEB064C98683}"))
	{
		// Control Panel
		SHGetValue(HKEY_CURRENT_USER, m_szRegDeskIconsPath, "{59031a47-3f72-44a7-89c5-5595fe6b30ee}", &dwType, &dwBuffer, &dwSize);
		if (dwBuffer != dwRegSetting)
		{
			SHSetValue(HKEY_CURRENT_USER, m_szRegDeskIconsPath, "{59031a47-3f72-44a7-89c5-5595fe6b30ee}", REG_DWORD, &dwRegSetting, sizeof(dwRegSetting));
			SendNotifyMessage(HWND_BROADCAST, WM_SETTINGCHANGE, 0, (LPARAM)"Windows");
		}
	}
}

/**************************************************************************************************
	This function toggles whether an icon should be hidden or not
**************************************************************************************************/
void CGroup::ToggleIcon(LPCTSTR pszFileName, bool bVisible)
{
	IconMap::iterator iter = m_HiddenIcons.find(pszFileName);
	bool bIsHidden = (iter != m_HiddenIcons.end());

	if (bIsHidden && bVisible) // Show the icon
	{
		// TODO::Fix this

		// Temporary method for showing the icon.
		m_HiddenIcons.erase(iter);
		SaveState();
		m_pView->Refresh();
		RestoreState();

		CheckVirtualIcon(pszFileName, false);
	}
	else if (!bIsHidden && !bVisible) // Hide the icon
	{
		int iD = GetIdFromName(pszFileName);
		if (iD != -1) // We have the item id
		{
			m_HiddenIcons.insert(IconMap::value_type(pszFileName, true));

			ListView_DeleteItem(m_hwndListView, iD);

			CheckVirtualIcon(pszFileName, true);
		}
		else
		{
			utils::ErrorMessage(E_NOTICE, "Failed to toggle icon %s. The icon does not seem to exist.", pszFileName);
		}
	}
}
void CGroup::ToggleIcon(int iD, bool bVisible)
{
	char szFileName[MAX_PATH];
	
	if (!GetNameFromId(iD, szFileName, sizeof(szFileName)))
		return;

	IconMap::iterator iter = m_HiddenIcons.find(szFileName);
	bool bIsHidden = (iter != m_HiddenIcons.end());

	if (bIsHidden && bVisible) // Show the icon
	{
		// TODO::Fix this

		// Temporary method for showing the icon.
		m_HiddenIcons.erase(iter);
		SaveState();
		m_pView->Refresh();
		RestoreState();

		CheckVirtualIcon(szFileName, false);
	}
	else if (!bIsHidden && !bVisible) // Hide the icon
	{
		m_HiddenIcons.insert(IconMap::value_type(szFileName, true));

		ListView_DeleteItem(m_hwndListView, iD);

		CheckVirtualIcon(szFileName, true);
	}
}

/**************************************************************************************************
	This function reads windows desktop icon preferences from the registry.
**************************************************************************************************/
void CGroup::ReadIconSettings()
{
	DWORD dwBuffer, dwSize = sizeof(DWORD), dwType = REG_DWORD;

	// Internet Explorer (XP only?)
	SHGetValue(HKEY_CURRENT_USER, m_szRegDeskIconsPath, "{871C5380-42A0-1069-A2EA-08002B30309D}", &dwType, &dwBuffer, &dwSize);
	if ((!m_bDontUseRegistryIcons && dwBuffer == 1) || LiteStep::GetPrefixedRCBool(m_szName, "HideInternetExplorer", false))
		ToggleIcon("::{871C5380-42A0-1069-A2EA-08002B30309D}", false);
	else if (!m_bDontUseRegistryIcons)
		ToggleIcon("::{871C5380-42A0-1069-A2EA-08002B30309D}", true);

	// My Computer (XP only?)
	SHGetValue(HKEY_CURRENT_USER, m_szRegDeskIconsPath, "{20D04FE0-3AEA-1069-A2D8-08002B30309D}", &dwType, &dwBuffer, &dwSize);
	if ((!m_bDontUseRegistryIcons && dwBuffer == 1) || LiteStep::GetPrefixedRCBool(m_szName, "HideMyComputer", false))
		ToggleIcon("::{20D04FE0-3AEA-1069-A2D8-08002B30309D}", false);
	else if (!m_bDontUseRegistryIcons)
		ToggleIcon("::{20D04FE0-3AEA-1069-A2D8-08002B30309D}", true);

	// My Documents (XP only?)
	SHGetValue(HKEY_CURRENT_USER, m_szRegDeskIconsPath, "{450D8FBA-AD25-11D0-98A8-0800361B1103}", &dwType, &dwBuffer, &dwSize);
	if ((!m_bDontUseRegistryIcons && dwBuffer == 1) || LiteStep::GetPrefixedRCBool(m_szName, "HideMyDocuments", false))
		ToggleIcon(m_szMyDocumentsPath, false);
	else if (!m_bDontUseRegistryIcons)
		ToggleIcon(m_szMyDocumentsPath, true);

	// My Network Places (XP and Vista)
	SHGetValue(HKEY_CURRENT_USER, m_szRegDeskIconsPath, "{208D2C60-3AEA-1069-A2D7-08002B30309D}", &dwType, &dwBuffer, &dwSize);
	if ((!m_bDontUseRegistryIcons && dwBuffer == 1) || LiteStep::GetPrefixedRCBool(m_szName, "HideMyNetworkPlaces", false))
		ToggleIcon("::{208D2C60-3AEA-1069-A2D7-08002B30309D}", false);
	else if (!m_bDontUseRegistryIcons)
		ToggleIcon("::{208D2C60-3AEA-1069-A2D7-08002B30309D}", true);

	// Recycle Bin (XP only?)
	SHGetValue(HKEY_CURRENT_USER, m_szRegDeskIconsPath, "{645FF040-5081-101B-9F08-00AA002F954E}", &dwType, &dwBuffer, &dwSize);
	if ((!m_bDontUseRegistryIcons && dwBuffer == 1) || LiteStep::GetPrefixedRCBool(m_szName, "HideRecycleBin", false))
		ToggleIcon("::{645FF040-5081-101B-9F08-00AA002F954E}", false);
	else if (!m_bDontUseRegistryIcons)
		ToggleIcon("::{645FF040-5081-101B-9F08-00AA002F954E}", true);

	// Control Panel (Vista Only)
	SHGetValue(HKEY_CURRENT_USER, m_szRegDeskIconsPath, "{59031a47-3f72-44a7-89c5-5595fe6b30ee}", &dwType, &dwBuffer, &dwSize);
	if ((!m_bDontUseRegistryIcons && dwBuffer == 1) || LiteStep::GetPrefixedRCBool(m_szName, "HideControlPanel", false))
		ToggleIcon("::{26EE0668-A00A-44D7-9371-BEB064C98683}", false);
	else if (!m_bDontUseRegistryIcons)
		ToggleIcon("::{26EE0668-A00A-44D7-9371-BEB064C98683}", true);
}

/**************************************************************************************************
	Sorting routines...
**************************************************************************************************/
int SortPIDLs(LPSHELLFOLDER psf, LPARAM lColumn, LPITEMIDLIST pidl1, LPITEMIDLIST pidl2)
{
	HRESULT hres = psf->CompareIDs(lColumn, pidl1, pidl2);
	return static_cast<signed short>(HRESULT_CODE(hres));
}
int CALLBACK SortByNameCompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
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
	This function moves and resizes the group
**************************************************************************************************/
void CGroup::Reposition (int DcX, int DcY, int width, int height, bool relative)
{
	if (relative)
	{
		m_DcX += DcX;
		m_DcY += DcY;
		m_nWidth += width;
		m_nHeight += height;
	}
	else
	{
		m_DcX = DcX;
		m_DcY = DcY;
		m_nWidth = width;
		m_nHeight = height;
	}

	// There should be a better way of doing this...
	/*
	SaveState();
	if (m_pView2)
	{
		m_pView2->Release();
	}
	if (m_pView)
	{
		m_pView->DestroyViewWindow();
		m_pView->Release();
	}
	InitFolderView();*/

	// This method has potential to be even slower than the previous one...
	SaveState();
	MoveWindow(m_hwndView, m_DcX, m_DcY, m_nWidth, m_nHeight, true);
	RestoreState();
}

/**************************************************************************************************
	This function checks if a set of modifier keys are pressed.
**************************************************************************************************/
bool CGroup::CheckModkeys(WORD wKeys)
{
	if (((wKeys & MODKEY_SHIFT) == MODKEY_SHIFT) && !(GetKeyState( VK_SHIFT ) < 0))
	{
		return false;
	}
	if (((wKeys & MODKEY_ALT) == MODKEY_ALT) && !(GetKeyState( VK_MENU ) < 0))
	{
		return false;
	}
	if (((wKeys & MODKEY_CTRL) == MODKEY_CTRL) && !(GetKeyState( VK_CONTROL ) < 0))
	{
		return false;
	}
	return true;
}

/**************************************************************************************************
	This function changes the directory
**************************************************************************************************/
void CGroup::ChangeDir(const char *szNewDir)
{
	char szCDPath[MAX_PATH];
	bool bSetFocus = (GetFocus() == m_hwndListView);

	// Determine what folder to browse to and if the folder is real.
	StringCchCopy(szCDPath, sizeof(szCDPath), szNewDir);

	if (_stricmp(szCDPath, ".back") == 0)
	{
		if (m_nBackFolderCount == 0)
		{
			return;
		}

		m_nBackFolderCount--;
		m_nForwardFolderCount++;
	}
	if (_stricmp(szCDPath, ".forward") == 0)
	{
		if (m_nForwardFolderCount == 0)
			return;

		m_nForwardFolderCount--;
		m_nBackFolderCount++;
	}

	if (_stricmp(szCDPath, ".selected") == 0)
	{
		int iItem = ListView_GetNextItem(m_hwndListView, -1, LVNI_FOCUSED);
		if (iItem == -1)
			return;
		if (!GetNameFromId(iItem, szCDPath, sizeof(szCDPath)))
			return;
		if (!utils::Is_Directory(szCDPath))
			return;
	}
	if (_stricmp(szCDPath, ".up") == 0)
	{
		int iLast = -1, iCnt = 0, s = sizeof(m_szFolderLocation), i;
		for (i = 0; i <= s; i++)
		{
			if (m_szFolderLocation[i] == 92)
			{
				iLast = i;
				iCnt++;
			}
		}
		if (iCnt == 1) // This takes some special managment...
		{
			if (m_szFolderLocation[3] == 0)
			{
				return; // We are already at the root drive.
			}
			else
			{
				iLast++; // We want to keep the \ at the end of the path.
			}
		}
		memcpy(szCDPath, m_szFolderLocation, sizeof(m_szFolderLocation));
		szCDPath[iLast] = 0; // End the string there.
	}
	if (!m_bNoVirtualSwitch && !_stricmp(szCDPath, m_szDesktopPath))
	{
		StringCchCopy(szCDPath, MAX_LINE_LENGTH, ".desktop");
	}
	else if (!utils::Is_Directory(szCDPath))
	{
		if ((_stricmp(szCDPath, ".desktop") != 0) && (_stricmp(szCDPath, ".mycomputer") != 0))
		{
			utils::ErrorMessage(E_WARNING, "Invalid Directory: %s", szNewDir);
			return;
		}
	}

	// Destroy the current folder view.
	m_bInFolderChange = true; // No input while changing folders.

	SaveState();

	if (m_pDropTarget)
	{
		m_pDropTarget->Revoke(m_hwndView);
		delete m_pDropTarget;
	}

	if (m_pView2)
	{
		m_pView2->Release();
	}

	if (m_pView)
	{
		m_pView->DestroyViewWindow();
		m_pView->Release();
	}

	if (m_pFolder)
	{
		m_pFolder->Release();
	}

	// Load the contents of the new folder.
	if (_stricmp(szCDPath, ".desktop") == 0)
	{
		StringCchCopy(m_szFolderLocation, MAX_PATH, m_szDesktopPath);
		SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &m_pidlFolder);
		//FixMyDocs();
	}
	else if (_stricmp(szCDPath, ".mycomputer") == 0)
	{
		SHGetSpecialFolderPath(NULL, m_szFolderLocation, CSIDL_DRIVES, 0);
		SHGetSpecialFolderLocation(NULL, CSIDL_DRIVES, &m_pidlFolder);
	}
	else
	{
		StringCchCopy(m_szFolderLocation, sizeof(m_szFolderLocation), szCDPath);
		m_pidlFolder = PIDL::ParseDisplayName(szCDPath);
	}

	utils::SetEvar(m_szName, "CurrentFolder", "%s", szCDPath);

	InitFolderView(); // Recreate the folder view.

	if (bSetFocus)
	{
		SetFocus(m_hwndListView);
	}

	HandleEvent(EVENT_ONFOLDERCHANGE); // Execute OnFolderChange
	m_bInFolderChange = false; // Done changing folders, unlock the listview procedure
}

/**************************************************************************************************
	This function changes the video mode
**************************************************************************************************/
void CGroup::SetViewMode(const char *szNewMode)
{
	UINT uNewMode;

	if (_stricmp(szNewMode, "tiles") == 0)
		uNewMode = FVM_TILE;
	else if (_stricmp(szNewMode, "thumbnails") == 0)
		uNewMode = FVM_THUMBNAIL;
	else if (_stricmp(szNewMode, "details") == 0)
		uNewMode = FVM_DETAILS;
	else if (_stricmp(szNewMode, "icons") == 0)
		uNewMode = FVM_ICON;
	else if (_stricmp(szNewMode, "smallicons") == 0)
		uNewMode = FVM_SMALLICON;
	else
		return;

	if (uNewMode == m_uViewMode)
		return;

	m_uViewMode = uNewMode;

	// ListView_SetView doesn't do that good of a job
	SaveState();
	if (m_pView2)
		m_pView2->Release();

	if (m_pView)
	{
		m_pView->DestroyViewWindow();
		m_pView->Release();
	}
	InitFolderView();
}

/**************************************************************************************************
	This function changes the hottracking setting
**************************************************************************************************/
void CGroup::HotTracking( bool bEnabled )
{
	if (bEnabled)
	{
		ListView_SetExtendedListViewStyleEx(m_hwndListView, LVS_EX_TRACKSELECT, LVS_EX_TRACKSELECT);
		m_bHotTracking = true;
	}
	else
	{
		ListView_SetExtendedListViewStyleEx(m_hwndListView, LVS_EX_TRACKSELECT, 0);
		m_bHotTracking = false;
	}
	
}

/**************************************************************************************************
	This function changes the infotip setting
**************************************************************************************************/
void CGroup::InfoTip( bool bEnabled )
{
	if (bEnabled)
	{
		ListView_SetExtendedListViewStyleEx(m_hwndListView, LVS_EX_INFOTIP, LVS_EX_INFOTIP);
		m_bNoInfoTips = false;
	}
	else
	{
		ListView_SetExtendedListViewStyleEx(m_hwndListView, LVS_EX_INFOTIP, 0);
		m_bNoInfoTips = true;
	}	
}

/**************************************************************************************************
	This function changes the aligntop setting
**************************************************************************************************/
void CGroup::AlignTop( bool bEnabled )
{
	if (bEnabled)
	{
		if (!m_bAlignTop)
		{
			m_longListViewStyle ^= LVS_ALIGNLEFT;
			m_bAlignTop = true;
		}
	}
	else if (m_bAlignTop)
	{
		m_longListViewStyle |= LVS_ALIGNLEFT;
		m_bAlignTop = false;
	}
	SetWindowLongPtr(m_hwndListView, GWL_STYLE, m_longListViewStyle);
}


/**************************************************************************************************
	This function changes the snap to grid setting
**************************************************************************************************/
void CGroup::SnapToGrid( bool bEnabled )
{
	if (bEnabled)
	{
		m_bSnapToGrid = true;
		ListView_SetExtendedListViewStyleEx(m_hwndListView, LVS_EX_SNAPTOGRID, LVS_EX_SNAPTOGRID);
		ListView_Arrange(m_hwndListView, LVA_SNAPTOGRID);
	}
	else
	{
		m_bSnapToGrid = false;
		ListView_SetExtendedListViewStyleEx(m_hwndListView, LVS_EX_SNAPTOGRID, 0);
	}
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
	This function deletes the selected files
**************************************************************************************************/
void CGroup::DeleteFiles()
{
	char szFileName[MAX_PATH], szFullCommand[MAX_LINE_LENGTH*2] = {0};
	LPSTR pszCommand = szFullCommand;
	int iItem = -1;
	UINT i, j;
	UINT iNumSelected = ListView_GetSelectedCount(m_hwndListView);
	UINT iFakeFiles[256], iRealFiles[256];
	UINT iFakeFileCounter = 0, iRealFileCounter = 0;
	SHFILEOPSTRUCT SHFileDelete = {0};

	if (iNumSelected == 0)
		return; // No files selected

	if (m_bVirtualFileDelete)
		return DeleteFilesVirtually();

	for (i = 0; i < iNumSelected; i++)
	{
		iItem = ListView_GetNextItem(m_hwndListView, iItem, LVNI_SELECTED);
		if (GetNameFromId(iItem, szFileName, sizeof(szFileName)))
		{
			if (utils::File_Exists(szFileName))
			{
				iRealFiles[iRealFileCounter++] = iItem;
				StringCchCopy(pszCommand, szFullCommand+sizeof(szFullCommand)-pszCommand, szFileName);
				pszCommand += strlen(szFileName)+1;
			}
			else
			{
				iFakeFiles[iFakeFileCounter++] = iItem;
			}
		}
	}
	// The deletion of the real files effects the fake files ids.
	for (i = 0; i < iFakeFileCounter; i++)
	{
		for (j = 0; j < iRealFileCounter; j++)
		{
			if (iRealFiles[j] < iFakeFiles[i])
				iFakeFiles[i]--;
		}
	}
	if (iRealFileCounter == 0) // There are no real files selected
		return DeleteFilesVirtually();

	SHFileDelete.wFunc = FO_DELETE;
	SHFileDelete.hwnd = NULL;
	SHFileDelete.pFrom = szFullCommand;
	SHFileDelete.pTo = NULL;
	SHFileDelete.fFlags = 0;
	if (m_bSimpleProgress)
		SHFileDelete.fFlags |= FOF_SIMPLEPROGRESS;
	if (!m_bDontUseRecycleBin)
		SHFileDelete.fFlags |= FOF_ALLOWUNDO;
	if (!m_bConfirmFileDelete)
		SHFileDelete.fFlags |= FOF_NOCONFIRMATION | FOF_WANTNUKEWARNING;
	if (m_bNoConnectedFiles)
		SHFileDelete.fFlags |= FOF_NO_CONNECTED_ELEMENTS;
	if ((SHFileOperation(&SHFileDelete) == 0) && !SHFileDelete.fAnyOperationsAborted)
	{
		for (i = 0; i < iFakeFileCounter; i++)
		{
			ToggleIcon(iFakeFiles[i], false);
		}
	}
}

/**************************************************************************************************
	This function removes files from the view
**************************************************************************************************/
void CGroup::DeleteFilesVirtually()
{
	char szFileName[256], szTemp[512];
	int nSelected = ListView_GetSelectedCount(m_hwndListView), iItem = -1, i;
	if (nSelected == 1)
	{
		ListView_GetItemText(m_hwndListView, ListView_GetNextItem(m_hwndListView, iItem, LVNI_SELECTED), 0, szFileName, 256);
		StringCchPrintf(szTemp, sizeof(szTemp), "Are you sure that you want to delete %s?", szFileName);
	}
	else
	{
		StringCchPrintf(szTemp, sizeof(szTemp), "Are you sure that you want to delete these %i files?", nSelected);
	}
	if (MessageBox(NULL, szTemp, "Confirm File Delete", MB_YESNO | MB_ICONQUESTION) == IDYES)
	{
		for (i = 0; i < nSelected; i++)
		{
			iItem = ListView_GetNextItem(m_hwndListView, iItem, LVNI_SELECTED);
			ToggleIcon(iItem, false);
		}
	}
}

/**************************************************************************************************
	Clears the cutmark when the clipboard changes
**************************************************************************************************/
void CGroup::ClearCutMark()
{
	if (!m_bCutMarkedFiles)
		return;
	ListView_SetItemState(m_hwndListView, -1, 0, LVIS_CUT);
	m_bCutMarkedFiles = false;
}

/**************************************************************************************************
	This function puts selected icons in the clipboard
**************************************************************************************************/
void CGroup::CopyFiles(bool cut)
{
	char szFileName[MAX_PATH];
	char szFullCommand[MAX_LINE_LENGTH*4] = {0};
	int iItem = -1;
	UINT i, iFileCounter = 0;
	UINT iNumSelected = ListView_GetSelectedCount(m_hwndListView);
	LPSTR pszPos = szFullCommand;

	if (!(iNumSelected > 0))
		return; // No files selected

	ClearCutMark();

	// Build the list of files
	for (i = 0; i < iNumSelected; i++)
	{
		iItem = ListView_GetNextItem(m_hwndListView, iItem, LVNI_SELECTED);
		if (GetNameFromId(iItem, szFileName, sizeof(szFileName)))
		{
			if (utils::File_Exists(szFileName)) // We ignore virtual files.
			{
				if (cut)
					ListView_SetItemState(m_hwndListView, iItem, LVIS_CUT, LVIS_CUT);

				iFileCounter++;
				StringCchCopyA(pszPos, sizeof(szFullCommand)+szFullCommand-pszPos, szFileName);
				pszPos += strlen(szFileName)+1;
			}
		}
	}

	if (iFileCounter < 1)
		return; // In case they tried to copy my computer or something like that

	// Put the list of files in global memory
	DROPFILES dobj = { sizeof(DROPFILES), { 0, 0 }, 0, 0 };
	UINT iDataSize = (UINT)(sizeof(dobj) + pszPos - szFullCommand);
	HGLOBAL hClipData = GlobalAlloc( GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_DDESHARE, iDataSize );
	LPSTR szData = (LPSTR)GlobalLock(hClipData);
	memcpy(szData, &dobj, 20);
	memcpy(szData + sizeof(dobj), &szFullCommand, iDataSize-sizeof(dobj));
	GlobalUnlock(hClipData);

	// Put the dropeffect in global memory
	HGLOBAL hEffect = GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE, sizeof(DWORD) );
	PDWORD pdw1 = (PDWORD)GlobalLock(hEffect);
	*pdw1 = cut ? DROPEFFECT_MOVE : DROPEFFECT_COPY;
	GlobalUnlock(hEffect);
	
	// Put everything in the clipboard
	if (OpenClipboard(NULL))
	{
		EmptyClipboard();
		SetClipboardData(CF_HDROP, hClipData);
		SetClipboardData(RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT), hEffect);
		CloseClipboard();
	}
	if (cut)
		m_bCutMarkedFiles = true;
}

/**************************************************************************************************
	This function determines the type of data in the clipboard and calls the appropriate function
**************************************************************************************************/
void CGroup::DoPaste()
{
	if (IsClipboardFormatAvailable(CF_HDROP))
		return PasteFiles();
	if (IsClipboardFormatAvailable(CF_TEXT))
		return PasteText();
	if (IsClipboardFormatAvailable(CF_BITMAP))
		return PasteBitmap();
	if (IsClipboardFormatAvailable(CF_WAVE))
		return PasteWave();
}
/**************************************************************************************************
	This function pastes files in the clipboard into the group's folder
**************************************************************************************************/
void CGroup::PasteFiles()
{
	char *files, szFiles[MAX_LINE_LENGTH*2] = {0};
	bool bMove = false, bBreak = false;
	DWORD dropeffect;
	UINT cfDropEffect, i = 0;
	SHFILEOPSTRUCT SHFilePaste = {0};
	
	if (!OpenClipboard(NULL))
		return;

	// Build the pFrom string.
	files = (char*)GetClipboardData(CF_HDROP)+20;
	while (!bBreak)
	{
		szFiles[i/2] = files[i];
		if (files[i] == 0 && files[i+2]== 0)
			bBreak = true;
		i += 2;
	}

	// Check if the file should be moved rather than copied.
	cfDropEffect = RegisterClipboardFormat(CFSTR_PREFERREDDROPEFFECT);
	if (IsClipboardFormatAvailable(cfDropEffect))
	{
		// This won't work it litestep is hosting the folder window
		memcpy(&dropeffect, GetClipboardData(cfDropEffect), sizeof(DWORD));
		if ((dropeffect & DROPEFFECT_MOVE) == DROPEFFECT_MOVE)
			bMove = true;
	}

	SHFilePaste.wFunc = bMove ? FO_MOVE : FO_COPY;
	SHFilePaste.hwnd = NULL;
	SHFilePaste.pFrom = szFiles;
	SHFilePaste.pTo = m_szFolderLocation;
	SHFilePaste.fFlags = FOF_NOCONFIRMMKDIR;
	if (m_bSimpleProgress)
		SHFilePaste.fFlags |= FOF_SIMPLEPROGRESS;
	if (m_bNoConnectedFiles)
		SHFilePaste.fFlags |= FOF_NO_CONNECTED_ELEMENTS;
	SHFileOperation(&SHFilePaste);
	if (bMove && !SHFilePaste.fAnyOperationsAborted)
		EmptyClipboard(); // Perhaps we should msg explorer to do this for us, but why?
	CloseClipboard();
}

/**************************************************************************************************
	This function pastes text in the clipboard into the group's folder
**************************************************************************************************/
void CGroup::PasteText()
{
	if (m_bDontPasteText)
		return;
	if (!OpenClipboard(NULL))
		return;

	FILE* pFile;
	char *szText = (char*)GetClipboardData(CF_TEXT);
	char szFileLoc[MAX_PATH], szFileName[MAX_PATH];

	utils::GetFormatedTime(m_szTextPasteFormat, szFileName, MAX_PATH);
	StringCchPrintf(szFileLoc, MAX_PATH, "%s\\%s", m_szFolderLocation, szFileName);

	fopen_s (&pFile, szFileLoc, "a");
	if (pFile)
	{
		fputs (szText, pFile);
		fclose (pFile);
	}
	else
	{
		utils::ErrorMessage(E_WARNING, "Unable to create file \"%s\". Probably due to an invalid file name", szFileName);
	}

	CloseClipboard();
}

/**************************************************************************************************
	This function pastes bitmaps in the clipboard into the group's folder
**************************************************************************************************/
void CGroup::PasteBitmap()
{
	if (m_bDontPasteImages)
		return;

	/*if (!OpenClipboard(NULL))
		return;

	FILE* pFile;
	HBITMAP hBMP = (HBITMAP)GetClipboardData(CF_BITMAP);
	char szFileLoc[MAX_PATH];

	SYSTEMTIME st;
	GetSystemTime(&st);
	StringCchPrintf(szFileLoc, MAX_PATH, "%s\\clipboard-%d-%d-%d-%d-%d-%d-%d.bmp", m_szFolderLocation,
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

	fopen_s (&pFile, szFileLoc, "a");
	fputs ((char*)hBMP, pFile);
	fclose (pFile);

	CloseClipboard();*/
}

/**************************************************************************************************
	This function pastes waves in the clipboard into the group's folder
**************************************************************************************************/
void CGroup::PasteWave()
{
	if (m_bDontPasteAudio)
		return;
}

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
	This function changes the text color
**************************************************************************************************/
void CGroup::SetTextColor(COLORREF crColor)
{
	ListView_SetTextColor(m_hwndListView, crColor);
	// force update
	InvalidateRect(m_hwndListView, NULL, TRUE);
	UpdateWindow(m_hwndListView);
}

/**************************************************************************************************
	This function changes the text background color
**************************************************************************************************/
void CGroup::SetTextBkColor(COLORREF crColor)
{
	ListView_SetTextBkColor(m_hwndListView, crColor);
	// force update
	InvalidateRect(m_hwndListView, NULL, TRUE);
	UpdateWindow(m_hwndListView);
}

/**************************************************************************************************
	This function changes the background color
**************************************************************************************************/
void CGroup::SetBackgroundColor(COLORREF crColor)
{
	ListView_SetBkColor(m_hwndListView, crColor);
	// force update
	InvalidateRect(m_hwndListView, NULL, TRUE);
	UpdateWindow(m_hwndListView);
}

/**************************************************************************************************
	This function changes the background.
**************************************************************************************************/
void CGroup::SetBackground(const char *szBackground, int nOffsetPercentX, int nOffsetPercentY, bool bTiled)
{
	LVBKIMAGE lvbki;
	lvbki.ulFlags = LVBKIF_SOURCE_URL | (bTiled ? LVBKIF_STYLE_TILE : LVBKIF_STYLE_NORMAL);
	lvbki.hbm = NULL;
	lvbki.pszImage = const_cast<LPSTR>(szBackground);
	lvbki.xOffsetPercent = nOffsetPercentX;
	lvbki.yOffsetPercent = nOffsetPercentY;
	ListView_SetBkImage(m_hwndListView, &lvbki);
	// force update
	InvalidateRect(m_hwndListView, NULL, TRUE);
	UpdateWindow(m_hwndListView);
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
			ListView_GetItemText(m_hwndListView, i, 0, szTempName, MAX_PATH);
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


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Drag&Drop support


IDropTarget* CGroup::GetLastItemTarget(bool *pbExecute)
{
	IDropTarget* rv = NULL;
	*pbExecute = false;
	if (m_nLastItem != -1)
	{
		LPITEMIDLIST pidl = GetPIDLFromId(m_nLastItem);
		SFGAOF flags = SFGAO_DROPTARGET;

		m_pFolder->GetAttributesOf(1, (LPCITEMIDLIST*)&pidl, &flags);
		bool bIsDropTarget = (flags & SFGAO_DROPTARGET) != 0;

		flags = SFGAO_FILESYSTEM;
		m_pFolder->GetAttributesOf(1, (LPCITEMIDLIST*)&pidl, &flags);
		bool bIsFileSystemObject = 0 != (flags & SFGAO_FILESYSTEM);

		flags = SFGAO_FOLDER;
		m_pFolder->GetAttributesOf(1, (LPCITEMIDLIST*)&pidl, &flags);
		bool bIsFolder = 0 != (flags & SFGAO_FOLDER);

		// file system objects except folders should be managed another way...
		if (bIsFileSystemObject && !bIsFolder)
			*pbExecute = true;
		else if (bIsDropTarget)
			m_pFolder->GetUIObjectOf(m_hWnd, 1, (LPCITEMIDLIST*)&pidl, IID_IDropTarget, NULL, (void**)&rv);
	}
	return rv;
}


void CGroup::Drop(DWORD  /*grfKeyState*/, POINTL *ScPt, DWORD *pdwEffect, CIDA *pida, POINT *pOffsets)
{
	LPITEMIDLIST pidl;	// single file object
	UINT i = 0;

	// file dropped on the icon
	if (m_nLastItem != -1)
	{
		pidl = GetPIDLFromId(m_nLastItem);
		bool bMoveOnly = false;

		if (pidl != NULL)
		{
			for (UINT j = 0; j < pida->cidl; j++)
			{
				if (m_pFolder->CompareIDs(0, pidl, HIDA_GetPIDLItem(pida, j)) == 0) {
					bMoveOnly = true;
					break;
				}
			}

			if (!bMoveOnly)
			{
				char szFileName[MAX_PATH] = {0};
				LPITEMIDLIST pidlFull = PIDL::Append(m_pidlFolder, pidl);
				if (SHGetPathFromIDList(pidlFull, szFileName))
				{
					CoTaskMemFree(pidlFull);
					while (i < pida->cidl)
					{
						char szParameter[MAX_PATH] = {0};
						pidlFull = PIDL::Append(
							HIDA_GetPIDLParent(pida),
							HIDA_GetPIDLItem(pida, i));
						if (SHGetPathFromIDList(pidlFull, szParameter))
						{
							PathQuoteSpaces(szParameter);
							ShellExecute(m_hWnd, NULL, szFileName, szParameter, NULL, SW_SHOWNORMAL);
						}
						CoTaskMemFree(pidlFull);
						++i;
					}
				}
			}
		}

		ListView_SetItemState(m_hwndView, -1, 0, LVIS_DROPHILITED);
		if (!bMoveOnly) {
			*pdwEffect = DROPEFFECT_COPY;
			return;
		}
	}

	while (i < pida->cidl)
	{
		// we need just to reorder icons...
		int nCount = ListView_GetItemCount(m_hwndListView);
		for (int j = 0; j < nCount; j++)
		{
			LPITEMIDLIST pidlItem = GetPIDLFromId(j);
			if (pidlItem != NULL && m_pFolder->CompareIDs(0, HIDA_GetPIDLItem(pida, i), pidlItem) == 0)
			{
				RECT ScRect;
				GetWindowRect(m_hwndListView, &ScRect);
				
				POINTL GcPos = {ScPt->x - ScRect.left, ScPt->y - ScRect.top};
				ListView_SetItemPosition( m_hwndListView, j,
					GcPos.x + pOffsets[1 + i].x,
					GcPos.y + pOffsets[1 + i].y);
				//POINT pt; pt.x = ScPt->x;pt.y = ScPt->y;
				//m_pView2->SelectAndPositionItem(HIDA_GetPIDLItem(pida, i), SVSI_SELECT | SVSI_TRANSLATEPT, &pt);
				break;
			}
		}
		++i;
	}
	ClearDropHover();
}

void CGroup::SetDropHover(POINTL *pt, DWORD *pdwEffect)
{
	// Damn, FindItem doesn't work =(

	RECT rect;

	GetWindowRect(m_hwndListView, &rect);
	pt->x -= rect.left;
	pt->y -= rect.top;

	if (m_nLastItem != -1)
	{
		ListView_GetItemRect(m_hwndListView, m_nLastItem, &rect, LVIR_BOUNDS);
		if (!(rect.left <= pt->x && pt->x < rect.right && rect.top <= pt->y && pt->y <= rect.bottom))
		{
			ListView_SetItemState(m_hwndListView, m_nLastItem, 0, LVIS_DROPHILITED);
			m_nLastItem = -1;
		}
	}

	if (m_nLastItem == -1)
	{
		int nItems = ListView_GetItemCount(m_hwndListView);
		for (int i = 0; i < nItems; i++)
		{
			ListView_GetItemRect(m_hwndListView, i, &rect, LVIR_BOUNDS);
			if (rect.left <= pt->x && pt->x < rect.right && rect.top <= pt->y && pt->y <= rect.bottom) {
				m_nLastItem = i;
				ListView_SetItemState(m_hwndListView, m_nLastItem, LVIS_DROPHILITED, LVIS_DROPHILITED);
				break;
			}
		}
	}
	
	if (m_nLastItem != -1)
	{
		LPITEMIDLIST pidl = GetPIDLFromId(m_nLastItem);

		if (pidl != NULL)
		{
			SFGAOF flags = SFGAO_DROPTARGET;
			m_pFolder->GetAttributesOf(1, (LPCITEMIDLIST*)(&(pidl)), &flags);
			if ((flags & SFGAO_DROPTARGET) != SFGAO_DROPTARGET)
				*pdwEffect = DROPEFFECT_NONE;
		}
	}
}

/**************************************************************************************************
	This function clears the drophilited flag
**************************************************************************************************/
void CGroup::ClearDropHover()
{
	if (m_nLastItem != -1)
		ListView_SetItemState(m_hwndListView, m_nLastItem, 0, LVIS_DROPHILITED);
}

/**************************************************************************************************
	This function sets up the list view
**************************************************************************************************/
bool CGroup::InitFolderView() 
{
	// Get the PIDL of the current folder
	m_pFolder = PIDL::GetFolder(m_pidlFolder);
	if (m_pFolder == NULL)
	{
		StringCchCopy(m_szInitError, sizeof(m_szInitError), "Folder access failure.");
		return false;
	}

	// Create an IShellView Interface
	m_pFolder->CreateViewObject(m_hWnd, IID_IShellView, (void**)&m_pView);
	if (!m_pView)
	{
		StringCchCopy(m_szInitError, sizeof(m_szInitError), "Failed to create view object.");
		return false;
	}

	// Set up settings for IShellViews window
	FOLDERSETTINGS fs;
	fs.ViewMode = m_uViewMode; // FVM_ICON, FVM_SMALLICON, FVM_LIST, FVM_DETAILS, FVM_THUMBNAIL, FVM_TILE, FVM_THUMBSTRIP
	fs.fFlags = FWF_NOCLIENTEDGE | FWF_SNAPTOGRID | FWF_SHOWSELALWAYS /*| FWF_TRANSPARENT | FWF_DESKTOP | FWF_BESTFITWINDOW*/;
	if (!m_szBackground || !(m_szBackground[0]))
		fs.fFlags |= FWF_TRANSPARENT /*| FWF_NOWEBVIEW*/ | FWF_DESKTOP;
	else
		fs.fFlags |= FWF_NOWEBVIEW;

	if (m_bNoScroll)
		fs.fFlags |= FWF_NOSCROLL;

	if (m_bAutoArrange)
		fs.fFlags |= FWF_AUTOARRANGE;
	
	RECT DcRect = { m_DcX, m_DcY, m_DcX + m_nWidth, m_DcY + m_nHeight };

	// Create the viewing window for IShellView
	m_pView->CreateViewWindow(NULL, &fs, (IShellBrowser*)this, &DcRect, &m_hwndView);
	m_pView->UIActivate(SVUIA_ACTIVATE_NOFOCUS);
	if (!m_hwndView)
	{
		StringCchCopy(m_szInitError, sizeof(m_szInitError), "Failed to create view window.");
		return false;
	}
	SetWindowPos(m_hwndView, HWND_BOTTOM, 0,0,0,0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

	m_pView->QueryInterface(IID_IShellView2, (void**)&m_pView2);
	if (m_pView2 == NULL)
	{
		StringCchCopy(m_szInitError, sizeof(m_szInitError), "Failed to create IShellView2 object for m_pView.");
		return false;
	}
	m_hwndListView = ::GetWindow(m_hwndView, GW_CHILD);

	m_longListViewStyle = GetWindowLong(m_hwndListView, GWL_STYLE);

	if (m_bAlignTop)
		m_longListViewStyle ^= LVS_ALIGNLEFT;

	if (m_bAlwaysShowSelection)
		m_longListViewStyle |= LVS_SHOWSELALWAYS;

	SetWindowLongPtr(m_hwndListView, GWL_STYLE, m_longListViewStyle);

	SetWindowLongPtr(m_hwndView, GWLP_USERDATA, (LONG)this);
	m_wpOrigGroupProc = (WNDPROC) SetWindowLongPtr(m_hwndView, GWLP_WNDPROC, (LONG) GroupProc);

	m_pDropTarget = new CDropTarget();
	m_pDropTarget->Register(this);

	if (m_hFont)
		SendMessage(m_hwndListView, WM_SETFONT, (WPARAM)m_hFont, MAKELPARAM(TRUE, 0));

	ListView_SetTextBkColor(m_hwndListView,m_crTextBackgroundColor);
	ListView_SetTextColor(m_hwndListView, m_crTextColor);
	ListView_SetIconSpacing(m_hwndListView, m_nSpacingX, m_nSpacingY);
	ListView_SetBkColor(m_hwndListView, m_crBackgroundColor);

	if (m_uViewMode == FVM_SMALLICON)
		ListView_SetView(m_hwndListView, LV_VIEW_SMALLICON);

	if (m_szBackground && m_szBackground[0])
		SetBackground(m_szBackground, m_nOffsetPercentX, m_nOffsetPercentY, m_bTiled);

	// Hijack the window procedure for the listview.
	SetWindowLongPtr(m_hwndListView, GWLP_USERDATA, (LONG)this);
	m_wpOrigListViewProc = (WNDPROC) SetWindowLongPtr(m_hwndListView, GWLP_WNDPROC, (LONG) ListViewProc);

	ShowWindow(m_hwndView, m_bHidden ? SW_HIDE : SW_SHOW);

	ReadIconSettings();
	ScanIcons();
	RestoreState();

	// Export Evars
	POINTL DcPos = {m_DcX, m_DcY};
	POINTL ScPos = ScFromDc(DcPos);
	utils::SetEvar(m_szName, "CurrentX", "%d", ScPos.x);
	utils::SetEvar(m_szName, "CurrentY", "%d", ScPos.y);
	utils::SetEvar(m_szName, "CurrentWidth", "%d", m_nWidth);
	utils::SetEvar(m_szName, "CurrentHeight", "%d", m_nHeight);
	utils::SetEvar(m_szName, "ItemCount", "%d", ListView_GetItemCount(m_hwndListView));

	ShowWindow(m_hwndView, m_bHidden ? SW_HIDE : SW_SHOW);

	return true;
}

/**************************************************************************************************
	This function forwards mouse messages to m_hWnd when they aren't handled by clickonic.
**************************************************************************************************/
void CGroup::relayMouseMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	MapWindowPoints( ((message == WM_MOUSEWHEEL) || (message == WM_MOUSEHWHEEL)) ? HWND_DESKTOP : m_hwndListView, m_hWnd, &pt, 1);
	PostMessage(m_hWnd, message, wParam, MAKELPARAM((short) pt.x, (short) pt.y));
}

/**************************************************************************************************
	This function fires when a "Windows" User Policy changes (such as desktop icon settings)
**************************************************************************************************/
void CGroup::HandleSettingChange()
{
	if (m_bDontUseRegistryIcons)
		return;
	ReadIconSettings();
}

/**************************************************************************************************
	This function takes a listview item id and returns it's PIDL
**************************************************************************************************/
LPITEMIDLIST CGroup::GetPIDLFromId(int iItem)
{
	LVITEM lvitem = {0};
	lvitem.mask = LVIF_PARAM;
	lvitem.iItem = iItem;
	ListView_GetItem(m_hwndListView, &lvitem);

	if (lvitem.lParam == NULL)
	{
		// Vista or later, there is no lParam anymore :/
		IFolderView *pFolderView = NULL;
		if (SUCCEEDED(m_pView->QueryInterface(IID_IFolderView, (void**)&pFolderView)))
		{
			if (SUCCEEDED(pFolderView->Item(iItem, (LPITEMIDLIST*)&lvitem.lParam)))
			{
				CoTaskMemFree(&lvitem.lParam);
			}
			pFolderView->Release();
		}
	}

	return (LPITEMIDLIST)lvitem.lParam;
}

/**************************************************************************************************
	This function takes a listview item id and returns it's full parsename
**************************************************************************************************/
bool CGroup::GetNameFromId(int iItem, char* szFileName, UINT cchLength)
{
	STRRET strDispName;
	LPITEMIDLIST pidl = GetPIDLFromId(iItem);
	
	if (pidl != NULL)
	{
		if (SUCCEEDED(m_pFolder->GetDisplayNameOf((LPCITEMIDLIST)pidl, SHGDN_FORPARSING, &strDispName)))
		{
			if (SUCCEEDED(StrRetToBuf(&strDispName, (LPCITEMIDLIST)pidl, szFileName, cchLength)))
			{
				return true;
			}
		}
	}

	return false;
}

/**************************************************************************************************
	This function takes the name of an item and returns it's listview id
**************************************************************************************************/
int CGroup::GetIdFromName(LPCSTR pszFileName)
{
	char szFileName[MAX_PATH];
	int nCount = ListView_GetItemCount(m_hwndListView);

	for (int i = 0; i < nCount; i++)
	{
		if (GetNameFromId(i, szFileName, sizeof(szFileName)))
		{
			if (strcmp(szFileName, pszFileName) == 0)
				return i;
		}
	}
	return -1;
}

/**************************************************************************************************
	This function takes user input and turns it into a modkey word.
**************************************************************************************************/
WORD CGroup::GetModkeyWord(char* szOption, char* szDefault)
{
	WORD wReturn = 0;
	char szBuf[MAX_LINE_LENGTH];
	LiteStep::GetPrefixedRCLine(szBuf, m_szName, szOption, szDefault);
	if (strstr(szBuf, ".shift") != NULL)
		wReturn |= MODKEY_SHIFT;
	if (strstr(szBuf, ".alt") != NULL)
		wReturn |= MODKEY_ALT;
	if (strstr(szBuf, ".ctrl") != NULL)
		wReturn |= MODKEY_CTRL;
	return wReturn;
}

/**************************************************************************************************
	This function handles events.
**************************************************************************************************/
void CGroup::HandleEvent(UINT uEvent)
{
	if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_NONE) == 0)
	{
		return;
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_RENAME) == 0)
	{
		ListView_EditLabel(m_hwndListView, ListView_GetNextItem(m_hwndListView, -1, LVNI_FOCUSED));
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_EXECUTE) == 0)
	{
		if (ListView_GetSelectedCount(m_hwndListView) == 1)
		{
			int iItem = ListView_GetNextItem(m_hwndListView, -1, LVNI_SELECTED);
			char szFileName[MAX_PATH];
			if (GetNameFromId(iItem, szFileName, sizeof(szFileName)))
			{
				if (utils::Is_Directory(szFileName))
				{
					if (m_bInlineBrowsing)
					{
						return ChangeDir(".selected");
					}
					else if (m_bExplicitCalls)
					{
						LSExecuteEx(NULL, "open", "explorer.exe", szFileName, NULL, SW_SHOWNORMAL);
						return;
					}
				}
			}
		}
		CallWindowProc(m_wpOrigListViewProc, m_hwndListView, WM_KEYDOWN, VK_RETURN, NULL);
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_DELETE) == 0)
	{
		DeleteFiles();
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_PASTE) == 0)
	{
		DoPaste();
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_UNDO) == 0)
	{
		// TODO::Add support for this
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_COPY) == 0)
	{
		CopyFiles(false);
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_CUT) == 0)
	{
		CopyFiles(true);
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_SELECTALL) == 0)
	{
		ListView_SetItemState(m_hwndListView, -1, LVIS_SELECTED, LVIS_SELECTED);
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_REFRESH) == 0)
	{
		SaveState();
		m_pView2->Refresh();
		RestoreState();
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_UP) == 0)
	{
		ChangeDir(".up");
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_CHANGEDIR) == 0)
	{
		ChangeDir(".selected");
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_DOWNKEY) == 0)
	{
		CallWindowProc(m_wpOrigListViewProc, m_hwndListView, WM_KEYDOWN, VK_DOWN, NULL);
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_UPKEY) == 0)
	{
		CallWindowProc(m_wpOrigListViewProc, m_hwndListView, WM_KEYDOWN, VK_UP, NULL);
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_LEFTKEY) == 0)
	{
		CallWindowProc(m_wpOrigListViewProc, m_hwndListView, WM_KEYDOWN, VK_LEFT, NULL);
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_RIGHTKEY) == 0)
	{
		CallWindowProc(m_wpOrigListViewProc, m_hwndListView, WM_KEYDOWN, VK_RIGHT, NULL);
	}
	else
	{
		LSExecute(m_hwndListView, m_szEventCommand[uEvent], SW_SHOWNORMAL);
	}
}

/**************************************************************************************************
	This function handles mouse events.
**************************************************************************************************/
void CGroup::HandleMouseEvent(UINT uEvent, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (m_bExplorerDesktop)
	{
		CallWindowProc(m_wpOrigListViewProc, m_hwndListView, msg, wParam, lParam);
		return;
	}

	if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_LBUTTONDBLCLK)
	{
		LVHITTESTINFO lvhi = {0};
		lvhi.flags = LVHT_ONITEM;
		lvhi.pt.x = GET_X_LPARAM(lParam);
		lvhi.pt.y = GET_Y_LPARAM(lParam);

		if ( ListView_HitTest(m_hwndListView, &lvhi) != -1 )
		{
			// We could add a whole set of events for this scenario as well, but I don't really see a need.
			CallWindowProc(m_wpOrigListViewProc, m_hwndListView, msg, wParam, lParam);
			return; // An icon was clicked.
		}
	}

	if (m_bStealFocus)
		SetFocus(m_hwndListView);

	if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_CHANGEDIR) == 0)
	{
		// We can override HandleEvent actions like this ;)
		LVHITTESTINFO lvhi = {0};
		lvhi.flags = LVHT_ONITEM;
		lvhi.pt.x = GET_X_LPARAM(lParam);
		lvhi.pt.y = GET_Y_LPARAM(lParam);

		if (msg == WM_MOUSEWHEEL || msg == WM_MOUSEHWHEEL)
		{
			MapWindowPoints(HWND_DESKTOP, m_hwndListView, &lvhi.pt, 1);
		}

		int nFileid = ListView_HitTest(m_hwndListView, &lvhi);
		if ( nFileid != -1 )
		{
			char szFileName[MAX_PATH];
			if (GetNameFromId(nFileid, szFileName, sizeof(szFileName)))
			{
				if (utils::Is_Directory(szFileName))
				{
					if (!m_bNoVirtualSwitch && (_stricmp(szFileName, m_szDesktopPath) == 0))
					{
						ChangeDir(".desktop");
					}
					else
					{
						ChangeDir(szFileName);
					}
					return;
				}
			}
		}
	}

	// We just need to handle the mouse specific events and then forward the event to HandleEvent
	if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_RECTANGLE) == 0)
	{
		if (CheckModkeys(m_wSelectionRectangleMod))
		{
			CallWindowProc(m_wpOrigListViewProc, m_hwndListView, WM_LBUTTONDOWN, wParam, lParam);
		}
		else
		{
			relayMouseMessage(msg, wParam, lParam);
		}
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_SYSTEMMENU) == 0)
	{
		if (CheckModkeys(m_wSystemMenuMod))
		{
			CallWindowProc(m_wpOrigListViewProc, m_hwndListView, WM_RBUTTONDOWN, wParam, lParam);
		}
		else
		{
			relayMouseMessage(msg, wParam, lParam);
		}
	}
	else if (_stricmp(m_szEventCommand[uEvent], EVENT_ACTION_FORWARD) == 0)
	{
		relayMouseMessage(msg, wParam, lParam);
	}
	else
	{
		HandleEvent(uEvent);
	}
}

/**************************************************************************************************
	This function loads commands for all events
**************************************************************************************************/
void CGroup::LoadEvents()
{
	// Backwards compability
	bool bUseDelete	= LiteStep::GetPrefixedRCBool(m_szName, "UseDelete", TRUE);
	bool bUseRename	= LiteStep::GetPrefixedRCBool(m_szName, "UseRename", TRUE);
	bool bDontUseCtrlA = LiteStep::GetPrefixedRCBool(m_szName, "DontUseCtrlA", FALSE);
	bool bDontUseCtrlC = LiteStep::GetPrefixedRCBool(m_szName, "DontUseCtrlC", FALSE);
	bool bDontUseCtrlV = LiteStep::GetPrefixedRCBool(m_szName, "DontUseCtrlV", FALSE);
	bool bDontUseCtrlX = LiteStep::GetPrefixedRCBool(m_szName, "DontUseCtrlX", FALSE);
	bool bUseSelectionRectangle = LiteStep::GetPrefixedRCBool(m_szName, "UseSelectionRectangle", FALSE);
	bool bUseSystemMenu = LiteStep::GetPrefixedRCBool(m_szName, "UseSystemMenu", FALSE);
	
	// Event Settings
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF1DOWN], m_szName, "OnF1Down", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF2DOWN], m_szName, "OnF2Down", bUseRename ? EVENT_ACTION_RENAME : EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF3DOWN], m_szName, "OnF3Down", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF4DOWN], m_szName, "OnF4Down", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF5DOWN], m_szName, "OnF5Down", EVENT_ACTION_REFRESH);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF6DOWN], m_szName, "OnF6Down", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF7DOWN], m_szName, "OnF7Down", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF8DOWN], m_szName, "OnF8Down", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF9DOWN], m_szName, "OnF9Down", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF10DOWN], m_szName, "OnF10Down", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF11DOWN], m_szName, "OnF11Down", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF12DOWN], m_szName, "OnF12Down", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF1UP], m_szName, "OnF1Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF2UP], m_szName, "OnF2Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF3UP], m_szName, "OnF3Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF4UP], m_szName, "OnF4Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF5UP], m_szName, "OnF5Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF6UP], m_szName, "OnF6Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF7UP], m_szName, "OnF7Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF8UP], m_szName, "OnF8Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF9UP], m_szName, "OnF9Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF10UP], m_szName, "OnF10Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF11UP], m_szName, "OnF11Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF12UP], m_szName, "OnF12Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF11UP], m_szName, "OnF11Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONF12UP], m_szName, "OnF12Up", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONDELDOWN], m_szName, "OnDELDown", bUseDelete ? EVENT_ACTION_DELETE : EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONDELUP], m_szName, "OnDELUp", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONFOLDERCHANGE], m_szName, "OnFolderChange", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONINSERTITEM], m_szName, "OnRemoveItem", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONREMOVEITEM], m_szName, "OnInsertItem", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONCTRLA], m_szName, "OnCtrlA", bDontUseCtrlA ? EVENT_ACTION_NONE : EVENT_ACTION_SELECTALL);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONCTRLV], m_szName, "OnCtrlV", bDontUseCtrlV ? EVENT_ACTION_NONE : EVENT_ACTION_PASTE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONCTRLX], m_szName, "OnCtrlX", bDontUseCtrlX ? EVENT_ACTION_NONE : EVENT_ACTION_CUT);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONCTRLC], m_szName, "OnCtrlC", bDontUseCtrlC ? EVENT_ACTION_NONE : EVENT_ACTION_COPY);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONCTRLZ], m_szName, "OnCtrlZ", EVENT_ACTION_UNDO);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONLEFTCLICKDOWN], m_szName, "OnLeftClickDown", bUseSelectionRectangle ? EVENT_ACTION_RECTANGLE : EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONRIGHTCLICKDOWN], m_szName, "OnRightClickDown", bUseSystemMenu ? EVENT_ACTION_SYSTEMMENU : EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONMIDDLECLICKDOWN], m_szName, "OnMiddleClickDown", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONX1CLICKDOWN], m_szName, "OnX1ClickDown", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONX2CLICKDOWN], m_szName, "OnX2ClickDown", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONLEFTCLICKUP], m_szName, "OnLeftClickUp", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONRIGHTCLICKUP], m_szName, "OnRightClickUp", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONMIDDLECLICKUP], m_szName, "OnMiddleClickUp", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONX1CLICKUP], m_szName, "OnX1ClickUp", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONX2CLICKUP], m_szName, "OnX2ClickUp", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONLEFTDBLCLICK], m_szName, "OnLeftDoubleClick", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONRIGHTDBLCLICK], m_szName, "OnRightDoubleClick", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONMIDDLEDBLCLICK], m_szName, "OnMiddleDoubleClick", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONX1DBLCLICK], m_szName, "OnX1DoubleClick", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONX2DBLCLICK], m_szName, "OnX2DoubleClick", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONWHEELUP], m_szName, "OnWheelUp", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONWHEELDOWN], m_szName, "OnWheelDown", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONWHEELLEFT], m_szName, "OnWheelLeft", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONWHEELRIGHT], m_szName, "OnWheelRight", EVENT_ACTION_FORWARD);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONRIGHTKEYDOWN], m_szName, "OnRightKeyDown", EVENT_ACTION_RIGHTKEY);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONLEFTKEYDOWN], m_szName, "OnLeftKeyDown", EVENT_ACTION_LEFTKEY);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONUPKEYDOWN], m_szName, "OnUpKeyDown", EVENT_ACTION_UPKEY);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONDOWNKEYDOWN], m_szName, "OnDownKeyDown", EVENT_ACTION_DOWNKEY);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONENTERDOWN], m_szName, "OnEnterDown", EVENT_ACTION_EXECUTE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONRIGHTKEYUP], m_szName, "OnRightKeyUp", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONLEFTKEYUP], m_szName, "OnLeftKeyUp", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONUPKEYUP], m_szName, "OnUpKeyUp", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONDOWNKEYUP], m_szName, "OnDownKeyUp", EVENT_ACTION_NONE);
	LiteStep::GetPrefixedRCLine(m_szEventCommand[EVENT_ONENTERUP], m_szName, "OnEnterUp", EVENT_ACTION_NONE);
}