#include "stdafx.h"
#include "main.h"
#include "group.h"
#include "utils.h"
#include "bangs.h"
#include "DropTarget.h"

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// initModuleEx
//
int initModuleEx(HWND hwndParent, HINSTANCE hDllInstance, LPCSTR szPath)
{
	// Store some constants
	g_hParent = hwndParent;
	g_hInstance = hDllInstance;
	g_nErrorTranslucency = GetRCInt("ClickonicErrorTranslucency", 2);
	StringCchCopy(g_szPath, sizeof(g_szPath), szPath);

	// Get information about the operating system.
	g_osVersion.dwOSVersionInfoSize = sizeof(g_osVersion);
	GetVersionEx(&g_osVersion);

	if (!CreateMessageHandler())
		return 1;

	if (FAILED(OleInitialize(NULL)))
	{
		utils::ErrorMessage(E_ERROR, "Failed to initalize COM");
		return 1;
	}

	InitCommonControls(); // What is this used for?
	CDropTarget::RegisterFormats();

	UpdateMonitorInfo();

	RegisterBangs();

	LoadGroups();
	
	if (g_osVersion.dwMajorVersion >= 6)
	{
		SetTimer(g_hwndMessageHandler, 1337, 500, NULL); // UGLY VISTA HACK!
	}

	return 0;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// quitModule
//
void quitModule(HINSTANCE hDllInstance)
{
	UnregisterBangs();

	// Destroy all groups
	for (GroupMap::iterator iter = g_Groups.begin(); iter != g_Groups.end(); ++iter)
	{
		delete iter->second;
	}
	g_Groups.clear();

	if (g_hwndMessageHandler)
	{
		SendMessage(GetLitestepWnd(),LM_UNREGISTERMESSAGE,(WPARAM)g_hwndMessageHandler, (LPARAM)g_lsMessages);
		DestroyWindow(g_hwndMessageHandler);
	}

	UnregisterClass(g_szMsgHandler, hDllInstance);

	OleUninitialize();
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// LoadGroups
//
void LoadGroups()
{
	LPVOID f = LCOpen(NULL);
	if (!f)
	{
		throw std::runtime_error("Error: Cannot open step.rc for reading");
	}

	char szLine[MAX_LINE_LENGTH], szToken[MAX_LINE_LENGTH];

	while (LCReadNextConfig(f, "*Clickonic", szLine, sizeof(szLine)))
	{
		LPCSTR pszNext = szLine;
		GetToken(pszNext, szToken, &pszNext, false);
		while (GetToken(pszNext, szToken, &pszNext, false))
		{
			if (strcmp(szToken, "") != 0)
			{
				CreateGroup(szToken);
			}
		}
	}

	LCClose(f);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// CreateGroup
//
void CreateGroup(const char *szName)
{
	GroupMap::iterator iter = g_Groups.find(szName);
	if (iter == g_Groups.end())
	{
		CGroup *pGroup = new CGroup(szName);
		if (pGroup->m_bInitialized)
		{
			g_Groups.insert(GroupMap::value_type(szName, pGroup));
		}
		else
		{
			utils::ErrorMessage(E_ERROR, "Initialization of %s failed. %s", szName, pGroup->m_szInitError);
			delete pGroup;
		}
	}
	else
	{
		utils::ErrorMessage(E_WARNING, "Failed to create group: %s; the group already exists.", szName);
	}
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// DestroyGroup
//
void DestroyGroup(const char *szName)
{
	GroupMap::iterator iter = g_Groups.find(szName);
	if (iter == g_Groups.end())
	{
		utils::ErrorMessage(E_WARNING, "Failed to destroy group %s, the group doesn't exist.", szName);
		return;
	}
	// Destroy the group
	delete iter->second;

	// Remove it from the list
	g_Groups.erase(iter);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// GetGroupByName
//
CGroup* GetGroupByName(const char *szName)
{
	GroupMap::iterator iter = g_Groups.find(szName);
	if (iter == g_Groups.end())
	{
		return NULL;
	}
	return iter->second;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// CreateMessageHandler
//
bool CreateMessageHandler()
{
	WNDCLASSEX wc = {0};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = MessageHandlerProc;
	wc.hInstance = g_hInstance;
	wc.lpszClassName = g_szMsgHandler;
	wc.hIconSm = 0;
	wc.style = CS_NOCLOSE;

	if (!RegisterClassEx(&wc))
	{
		utils::ErrorMessage(E_ERROR, "Error: Cannot register handler window class...");
		return false;
	}

	g_hwndMessageHandler = CreateWindowEx(
		WS_EX_TOOLWINDOW, // Extended style
		g_szMsgHandler, // Class name
		0, // Window name
		WS_POPUP, // Style
		0, 0, 0, 0, // x,y,w,h
		g_hParent, // hParent
		0, // hMenu
		g_hInstance, // hInstance
		0
		);

	if (!g_hwndMessageHandler)
	{
		utils::ErrorMessage(E_ERROR, "Error: Cannot create main window...");
		return false;
	}	
	SendMessage(GetLitestepWnd(), LM_REGISTERMESSAGE, (WPARAM)g_hwndMessageHandler, (LPARAM) g_lsMessages);

	return true;
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// UpdateMonitorInfo
//
void UpdateMonitorInfo()
{
	g_Monitors[0].Left = GetSystemMetrics(SM_XVIRTUALSCREEN);
	g_Monitors[0].Top = GetSystemMetrics(SM_YVIRTUALSCREEN);
	g_Monitors[0].ResX = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	g_Monitors[0].ResY = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	g_iMon = 2;

	EnumDisplayMonitors(NULL, NULL, SetMonitorVars, 0);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// SetMonitorVars
//
// Callback function for UpdateMonitorInfo
//
BOOL CALLBACK SetMonitorVars(HMONITOR hMonitor, HDC  /* hdcMonitor */, LPRECT /* lprcMonitor */, LPARAM /* dwData */)
{
	int iMonNum;
	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(hMonitor, &mi);

	if ((mi.dwFlags & MONITORINFOF_PRIMARY) == MONITORINFOF_PRIMARY)
	{
		iMonNum = 1;
	}
	else
	{
		iMonNum = g_iMon++;
	}

	g_Monitors[iMonNum].Top = mi.rcMonitor.top;
	g_Monitors[iMonNum].Left = mi.rcMonitor.left;
	g_Monitors[iMonNum].ResY = mi.rcMonitor.bottom - mi.rcMonitor.top;
	g_Monitors[iMonNum].ResX = mi.rcMonitor.right - mi.rcMonitor.left;

	return TRUE;
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// MessageHandlerProc
//
// Main message handler. Deals with LiteStep messages and system messages.
//
LRESULT WINAPI MessageHandlerProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case LM_REFRESH:
		{
			/* // This needs a lot of work...
			for (GroupMap::iterator iter = g_Groups.begin(); iter != g_Groups.end(); ++iter)
			{
				iter->second->ReadSettings(true);
			}*/
			return 0;
		}
	case LM_GETREVID:
		{
			StringCchPrintf((char*)lParam, 64, "%s: %s", g_szAppName, g_rcsRevision);
			size_t uLength;
			if (SUCCEEDED(StringCchLength((char*)lParam, 64, &uLength)))
				return uLength;
			lParam = NULL;
			return 0;
		}
	/*case WM_SETTINGCHANGE:
		{ // May cause crashes
			if (wParam == 0 && !_stricmp("Windows", (char*)lParam)) {
				for (GroupMap::iterator iter = g_Groups.begin(); iter != g_Groups.end(); ++iter)
				{
					iter->second->HandleSettingChange();
				}
			}
			break;
		}*/
	case WM_DISPLAYCHANGE:
		{
			UpdateMonitorInfo();
			break;
		}
	case WM_SETCURSOR:
		{
			// For some reason leting this message pass through can cause an exception, so we stop it here.
			return true;
		}
	case WM_TIMER:
		{
			switch (wParam)
			{
			case 1337: // Ugly vista hack :/ Restores icon positions after clickonic finishes loading.
				{
					for (GroupMap::iterator iter = g_Groups.begin(); iter != g_Groups.end(); ++iter)
					{
						iter->second->RestoreState();
					}

					KillTimer(g_hwndMessageHandler, 1337);

					break;
				}
			}
			break;
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// GroupProc
//
// Hijacked messagehandler for the parent window of the listview
//
LRESULT WINAPI GroupProc(HWND hListView, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CGroup* group = (CGroup *)GetWindowLongPtr(hListView, GWLP_USERDATA);
	if (!group)
	{
		return DefWindowProc(hListView, msg, wParam, lParam);
	}

	switch(msg)
	{
	case WM_NOTIFY:
		{
			switch (((LPNMHDR)lParam)->code)
			{
			case LVN_INSERTITEM:
				{
					NMLISTVIEW *nmListView = (LPNMLISTVIEW)lParam;
					if (group->HandleInsItem( nmListView->iItem ) && group->m_bInitialized && !group->m_bInFolderChange)
					{
						char szEvar[MAX_RCCOMMAND], szNum[10];
						StringCchPrintf(szEvar, MAX_RCCOMMAND, "%s%s", group->m_szName, "ItemCount");
						_itoa_s(ListView_GetItemCount(group->m_hwndListView), szNum, 10, 10);
						LSSetVariable(szEvar, szNum);
						utils::SetEvar(group->m_szName, "ItemCount", "%d", ListView_GetItemCount(group->m_hwndListView));
						group->HandleEvent(EVENT_ONINSERTITEM);
					}
					return 0;
				} // LVN_INSERTITEM
			case LVN_DELETEITEM:
				{
					NMLISTVIEW *nmListView = (LPNMLISTVIEW)lParam;
					char szFileName[MAX_PATH];

					if (!group->GetNameFromId(nmListView->iItem, szFileName, sizeof(szFileName)))
						break;

					if (!group->IconShouldBeHidden( szFileName ) && group->m_bInitialized && !group->m_bInFolderChange)
					{
						group->HandleEvent(EVENT_ONREMOVEITEM);
					}
					utils::SetEvar(group->m_szName, "ItemCount", "%d", ListView_GetItemCount(group->m_hwndListView)-1);
					break;
				} // LVN_DELETEITEM
			} // ((LPNMHDR)lParam)->code
			break;
		} // WM_NOTIFY
	case WM_DRAWCLIPBOARD:
		{
			group->ClearCutMark();
			SendMessage(group->m_hwndNextClipViewer, WM_DRAWCLIPBOARD, 0, 0);
			return 0;
		} // WM_DRAWCLIPBOARD:
	case WM_CHANGECBCHAIN:
		{
			if ((HWND)wParam == group->m_hwndNextClipViewer)
				group->m_hwndNextClipViewer = (HWND)lParam;
			else if (group->m_hwndNextClipViewer != NULL)
				SendMessage(group->m_hwndNextClipViewer, WM_CHANGECBCHAIN, wParam, lParam);
			return 0;
		} // WM_CHANGECBCHAIN
	} // msg
	// Forward the message to the origional messagehandler if we don't handle it.
	return CallWindowProc(group->m_wpOrigGroupProc, hListView, msg, wParam, lParam);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// ListViewProc
//
// Hijacked ListView procedure
//
LRESULT WINAPI ListViewProc(HWND hListView, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CGroup* group = (CGroup *)GetWindowLongPtr(hListView, GWLP_USERDATA);
	if (!group)
		return DefWindowProc(hListView, msg, wParam, lParam);

	switch(msg)
	{
	case LVM_SETICONSPACING:
		{
			// Prevent explorers "Align to Grid" setting from messing up the spacing.
			return CallWindowProc(group->m_wpOrigListViewProc, hListView, msg, wParam, (LPARAM)MAKELONG(group->m_nSpacingX, group->m_nSpacingY));
		}
	/*case LVM_SETEXTENDEDLISTVIEWSTYLE:
		{
			// Take some time to fix group variables, just in case this is called by an external app.
			DWORD dwExMask = (DWORD)wParam, dwExStyle = (DWORD)lParam;
			if (dwExMask == 0)
				dwExMask = dwExStyle;

			DWORD dwCurrentStyle = ListView_GetExtendedListViewStyle(group->m_hwndListView);

			// wtf, dwExMask is 0 even when it disables snap to grid...
			if ((dwExMask & LVS_EX_SNAPTOGRID) == LVS_EX_SNAPTOGRID) {
				if ((dwExStyle & LVS_EX_SNAPTOGRID) == LVS_EX_SNAPTOGRID)
					group->m_bSnapToGrid = true;
				else
					group->m_bSnapToGrid = false;
			}

			break;
		}*/
	/*case LVM_INSERTITEMA: // LVM_INSERITEM is defined as LVM_INSERTITEMA, but the system sends LVM_INSERTITEMW
	case LVM_INSERTITEMW:
		{	// Unfortunately we don't have the name of the item yet.
			LVITEM *pItem = (LVITEM*)lParam;
			break;
		}*/
	case WM_KEYDOWN:
		{
			if (group->m_bInFolderChange)
				return 0;
			switch (wParam)
			{
			case VK_DELETE:
				{
					group->HandleEvent(EVENT_ONDELDOWN);
					return 0;
				}
			case VK_F1:
				{
					group->HandleEvent(EVENT_ONF1DOWN);
					return 0;
				}
			case VK_F2:
				{
					group->HandleEvent(EVENT_ONF2DOWN);
					return 0;
				}
			case VK_F3:
				{
					group->HandleEvent(EVENT_ONF3DOWN);
					return 0;
				}
			case VK_F4:
				{
					group->HandleEvent(EVENT_ONF4DOWN);
					return 0;
				}
			case VK_F5:
				{
					group->HandleEvent(EVENT_ONF5DOWN);
					return 0;
				}
			case VK_F6:
				{
					group->HandleEvent(EVENT_ONF6DOWN);
					return 0;
				}
			case VK_F7:
				{
					group->HandleEvent(EVENT_ONF7DOWN);
					return 0;
				}
			case VK_F8:
				{
					group->HandleEvent(EVENT_ONF8DOWN);
					return 0;
				}
			case VK_F9:
				{
					group->HandleEvent(EVENT_ONF9DOWN);
					return 0;
				}
			case VK_F10:
				{
					group->HandleEvent(EVENT_ONF10DOWN);
					return 0;
				}
			case VK_F11:
				{
					group->HandleEvent(EVENT_ONF11DOWN);
					return 0;
				}
			case VK_F12:
				{
					group->HandleEvent(EVENT_ONF12DOWN);
					return 0;
				}
			case VK_RETURN:
				{
					group->HandleEvent(EVENT_ONENTERDOWN);
					return 0;
				}
			case 0x41:
				{ // A
					if (GetKeyState( VK_CONTROL ) < 0)
						group->HandleEvent(EVENT_ONCTRLA);
					break;
				}
			case 0x43:
				{ // C
					if (GetKeyState( VK_CONTROL ) < 0)
						group->HandleEvent(EVENT_ONCTRLC);
					break;
				}
			case 0x58:
				{ // X
					if (GetKeyState( VK_CONTROL ) < 0)
						group->HandleEvent(EVENT_ONCTRLX);
					break;
				}
			case 0x56:
				{ // V
					if (GetKeyState( VK_CONTROL ) < 0)
						group->HandleEvent(EVENT_ONCTRLV);
					break;
				}
			case 0x5A:
				{ // Z
					if (GetKeyState( VK_CONTROL ) < 0)
						group->HandleEvent(EVENT_ONCTRLZ);
					break;
				}
			case VK_LEFT:
				{
					group->HandleEvent(EVENT_ONLEFTKEYDOWN);
					return 0;
				}
			case VK_RIGHT:
				{
					group->HandleEvent(EVENT_ONRIGHTKEYDOWN);
					return 0;
				}
			case VK_UP:
				{
					group->HandleEvent(EVENT_ONUPKEYDOWN);
					return 0;
				}
			case VK_DOWN:
				{
					group->HandleEvent(EVENT_ONDOWNKEYDOWN);
					return 0;
				}
			} // wParam
			break;
		} // WM_KEYDOWN
	case WM_KEYUP:
		{
			switch (wParam)
			{
			case VK_RETURN:
				{
					group->HandleEvent(EVENT_ONENTERUP);
					return 0;
				}
			case VK_DELETE:
				{
					group->HandleEvent(EVENT_ONDELUP);
					return 0;
				}
			case VK_F1:
				{
					group->HandleEvent(EVENT_ONF1UP);
					return 0;
				}
			case VK_F2:
				{
					group->HandleEvent(EVENT_ONF2UP);
					return 0;
				}
			case VK_F3:
				{
					group->HandleEvent(EVENT_ONF3UP);
					return 0;
				}
			case VK_F4:
				{
					group->HandleEvent(EVENT_ONF4UP);
					return 0;
				}
			case VK_F5:
				{
					group->HandleEvent(EVENT_ONF5UP);
					return 0;
				}
			case VK_F6:
				{
					group->HandleEvent(EVENT_ONF6UP);
					return 0;
				}
			case VK_F7:
				{
					group->HandleEvent(EVENT_ONF7UP);
					return 0;
				}
			case VK_F8:
				{
					group->HandleEvent(EVENT_ONF8UP);
					return 0;
				}
			case VK_F9:
				{
					group->HandleEvent(EVENT_ONF9UP);
					return 0;
				}
			case VK_F10:
				{
					group->HandleEvent(EVENT_ONF10UP);
					return 0;
				}
			case VK_F11:
				{
					group->HandleEvent(EVENT_ONF11UP);
					return 0;
				}
			case VK_F12:
				{
					group->HandleEvent(EVENT_ONF12UP);
					return 0;
				}
			case VK_LEFT:
				{
					group->HandleEvent(EVENT_ONLEFTKEYUP);
					return 0;
				}
			case VK_RIGHT:
				{
					group->HandleEvent(EVENT_ONRIGHTKEYUP);
					return 0;
				}
			case VK_UP:
				{
					group->HandleEvent(EVENT_ONUPKEYUP);
					return 0;
				}
			case VK_DOWN:
				{
					group->HandleEvent(EVENT_ONDOWNKEYUP);
					return 0;
				}
			} // wParam
			break;
		} // WM_KEYUP
	case WM_MOUSEWHEEL:
		{
			if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
				group->HandleMouseEvent(EVENT_ONWHEELUP, msg, wParam, lParam);
			else
				group->HandleMouseEvent(EVENT_ONWHEELDOWN, msg, wParam, lParam);
			return 0;
		}
	case WM_MOUSEHWHEEL:
		{
			if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
				group->HandleMouseEvent(EVENT_ONWHEELRIGHT, msg, wParam, lParam);
			else
				group->HandleMouseEvent(EVENT_ONWHEELLEFT, msg, wParam, lParam);
			return 0;
		}
	case WM_LBUTTONDBLCLK:
		{
			LVHITTESTINFO lvhi = {0};
			lvhi.flags = LVHT_ONITEM;
			lvhi.pt.x = GET_X_LPARAM(lParam);
			lvhi.pt.y = GET_Y_LPARAM(lParam);
			int nFileid = ListView_HitTest(group->m_hwndListView, &lvhi);
			if ( nFileid != -1 )
			{
				char szFileName[MAX_PATH];
				if (!group->GetNameFromId(nFileid, szFileName, sizeof(szFileName)))
					break;
				/*if (_stricmp(szFileName, "::{20D04FE0-3AEA-1069-A2D8-08002B30309D}") == 0)
				{
					group->ChangeDir(".mycomputer");
					return 0;
				}*/
				if (utils::Is_Directory(szFileName))
				{
					if (group->m_bInlineBrowsing)
					{
						if (!group->m_bNoVirtualSwitch && (_stricmp(szFileName, group->m_szDesktopPath) == 0))
							group->ChangeDir(".desktop");
						else
							group->ChangeDir(szFileName);
						return 0;
					} // group->m_bInlineBrowsing
					else if (group->m_bExplicitCalls)
					{
						LSExecuteEx(NULL, "open", "explorer.exe", szFileName, NULL, SW_SHOWNORMAL);
						return 0;
					}
				} // utils::Is_Directory(szFileName)
			} // nFileid != -1
			group->HandleMouseEvent(EVENT_ONLEFTDBLCLICK, msg, wParam, lParam);
			return 0;
		} // WM_LBUTTONDBLCLK
	case WM_LBUTTONDOWN:
		{
			group->HandleMouseEvent(EVENT_ONLEFTCLICKDOWN, msg, wParam, lParam);
			return 0;
		}
	case WM_LBUTTONUP:
		{
			group->HandleMouseEvent(EVENT_ONLEFTCLICKUP, msg, wParam, lParam);
			return 0;
		}
	case WM_MBUTTONDBLCLK:
		{
			group->HandleMouseEvent(EVENT_ONMIDDLEDBLCLICK, msg, wParam, lParam);
			return 0;
		}
	case WM_MBUTTONDOWN:
		{
			group->HandleMouseEvent(EVENT_ONMIDDLECLICKDOWN, msg, wParam, lParam);
			return 0;
		}
	case WM_MBUTTONUP:
		{
			group->HandleMouseEvent(EVENT_ONMIDDLECLICKUP, msg, wParam, lParam);
			return 0;
		}
	case WM_XBUTTONDBLCLK:
		{
			if (HIWORD(wParam) == XBUTTON1)
				group->HandleMouseEvent(EVENT_ONX1DBLCLICK, msg, wParam, lParam);
			else if (HIWORD(wParam) == XBUTTON2) // There could technically be more xbuttons
				group->HandleMouseEvent(EVENT_ONX2DBLCLICK, msg, wParam, lParam);
			else // Since we don't handle more than 2 xbuttons we might as well forward the rest
				group->relayMouseMessage(msg, wParam, lParam);
			return 0;
		}
	case WM_XBUTTONDOWN:
		{
			if (HIWORD(wParam) == XBUTTON1)
				group->HandleMouseEvent(EVENT_ONX1CLICKDOWN, msg, wParam, lParam);
			else if (HIWORD(wParam) == XBUTTON2)
				group->HandleMouseEvent(EVENT_ONX2CLICKDOWN, msg, wParam, lParam);
			else // Since we don't handle more than 2 xbuttons we might as well forward the rest
				group->relayMouseMessage(msg, wParam, lParam);
			return 0;
		}
	case WM_XBUTTONUP:
		{
			if (HIWORD(wParam) == XBUTTON1)
				group->HandleMouseEvent(EVENT_ONX1CLICKUP, msg, wParam, lParam);
			else if (HIWORD(wParam) == XBUTTON2)
				group->HandleMouseEvent(EVENT_ONX2CLICKUP, msg, wParam, lParam);
			else // Since we don't handle more than 2 xbuttons we might as well forward the rest
				group->relayMouseMessage(msg, wParam, lParam);
			return 0;
		}
	case WM_RBUTTONDBLCLK:
		{
			group->HandleMouseEvent(EVENT_ONRIGHTDBLCLICK, msg, wParam, lParam);
			return 0;
		}
	case WM_RBUTTONDOWN:
		{
			group->HandleMouseEvent(EVENT_ONRIGHTCLICKDOWN, msg, wParam, lParam);
			return 0;
		}
	case WM_RBUTTONUP:
		{
			group->HandleMouseEvent(EVENT_ONRIGHTCLICKUP, msg, wParam, lParam);
			return 0;
		}
	} // switch(msg)
	return CallWindowProc(group->m_wpOrigListViewProc, hListView, msg, wParam, lParam);
}

//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//
// DllMain
//
// DLL entry point
//
BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID /* pvReserved */)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hInst);
	}
	return TRUE;
}
